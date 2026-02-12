#include "livision/internal/sdf_loader.hpp"

#include <Eigen/Geometry>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>
#include <string_view>
#include <unordered_map>

#ifdef LIVISION_ENABLE_SDF
#include <assimp/config.h>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <curl/curl.h>

#include <assimp/Importer.hpp>
#include <gz/math/Pose3.hh>
#include <gz/math/Quaternion.hh>
#include <gz/math/Vector3.hh>
#include <sdf/Box.hh>
#include <sdf/Cone.hh>
#include <sdf/Cylinder.hh>
#include <sdf/Geometry.hh>
#include <sdf/Link.hh>
#include <sdf/Material.hh>
#include <sdf/Mesh.hh>
#include <sdf/Model.hh>
#include <sdf/ParserConfig.hh>
#include <sdf/Plane.hh>
#include <sdf/Root.hh>
#include <sdf/Sphere.hh>
#include <sdf/Visual.hh>
#include <sdf/World.hh>
#endif

namespace livision::internal::sdf_loader {

namespace {
namespace fs = std::filesystem;

bool StartsWith(std::string_view value, std::string_view prefix) {
  return value.size() >= prefix.size() &&
         value.substr(0, prefix.size()) == prefix;
}

std::vector<fs::path> SplitPathList(const char* raw) {
  std::vector<fs::path> result;
  if (!raw || *raw == '\0') {
    return result;
  }
#if defined(_WIN32)
  const char delimiter = ';';
#else
  const char delimiter = ':';
#endif
  std::string current;
  for (const char ch : std::string_view(raw)) {
    if (ch == delimiter) {
      if (!current.empty()) {
        result.emplace_back(current);
        current.clear();
      }
    } else {
      current.push_back(ch);
    }
  }
  if (!current.empty()) {
    result.emplace_back(current);
  }
  return result;
}

std::vector<fs::path> CollectSearchRoots(const fs::path& sdf_dir) {
  std::vector<fs::path> roots;
  if (!sdf_dir.empty()) {
    roots.push_back(sdf_dir);
  }

  const char* envs[] = {
      "GAZEBO_MODEL_PATH",
      "GZ_SIM_RESOURCE_PATH",
      "IGN_GAZEBO_RESOURCE_PATH",
      "SDF_PATH",
  };

  for (const char* env_name : envs) {
    const char* value = std::getenv(env_name);
    auto paths = SplitPathList(value);
    roots.insert(roots.end(), paths.begin(), paths.end());
  }

  return roots;
}

std::vector<fs::path> CollectResourceRoots(const fs::path& sdf_dir) {
  std::vector<fs::path> roots = CollectSearchRoots(sdf_dir);
  if (const char* env = std::getenv("GAZEBO_RESOURCE_PATH")) {
    const auto paths = SplitPathList(env);
    roots.insert(roots.end(), paths.begin(), paths.end());
  }

  roots.emplace_back("/usr/share/gazebo");
  roots.emplace_back("/usr/share/gazebo-11");
  roots.emplace_back("/usr/share");
  return roots;
}

#ifdef LIVISION_ENABLE_SDF
size_t CurlWriteToFile(void* contents, size_t size, size_t nmemb, void* userp) {
  std::ofstream* stream = static_cast<std::ofstream*>(userp);
  if (!stream || !stream->good()) {
    return 0;
  }
  const size_t total = size * nmemb;
  stream->write(static_cast<const char*>(contents),
                static_cast<std::streamsize>(total));
  return stream->good() ? total : 0;
}

std::string DownloadMeshToCache(const std::string& uri,
                                std::string* error_message) {
  fs::path cache_dir = fs::temp_directory_path() / "livision_mesh_cache";
  std::error_code ec;
  fs::create_directories(cache_dir, ec);
  if (ec) {
    if (error_message) {
      *error_message =
          "Failed to create mesh cache directory: " + cache_dir.string();
    }
    return {};
  }

  std::string_view view(uri);
  const size_t query_pos = view.find('?');
  const std::string_view path_part =
      (query_pos == std::string_view::npos) ? view : view.substr(0, query_pos);
  const size_t dot_pos = path_part.find_last_of('.');
  const std::string ext = (dot_pos == std::string_view::npos)
                              ? std::string()
                              : std::string(path_part.substr(dot_pos));

  const size_t uri_hash = std::hash<std::string>{}(uri);
  fs::path cached_path = cache_dir / (std::to_string(uri_hash) + ext);
  if (fs::exists(cached_path)) {
    return cached_path.string();
  }

  std::ofstream stream(cached_path, std::ios::binary);
  if (!stream) {
    if (error_message) {
      *error_message = "Failed to open cache file for mesh download: " +
                       cached_path.string();
    }
    return {};
  }

  CURL* curl = curl_easy_init();
  if (!curl) {
    if (error_message) {
      *error_message = "Failed to initialize curl for mesh download: " + uri;
    }
    fs::remove(cached_path, ec);
    return {};
  }

  char curl_error[CURL_ERROR_SIZE] = {0};
  curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteToFile);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &stream);
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_error);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "livision-sdf-loader/1.0");

  const CURLcode res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);
  stream.close();

  if (res != CURLE_OK) {
    fs::remove(cached_path, ec);
    if (error_message) {
      std::ostringstream oss;
      oss << "Failed to download mesh URI: " << uri;
      if (curl_error[0] != '\0') {
        oss << " (" << curl_error << ")";
      }
      *error_message = oss.str();
    }
    return {};
  }

  return cached_path.string();
}

fs::path ResolveResourceUri(const std::string& uri, const fs::path& sdf_dir,
                            const fs::path& base_dir) {
  if (uri.empty()) {
    return {};
  }

  std::string_view view(uri);
  if (StartsWith(view, "file://")) {
    view = view.substr(7);
  }

  fs::path candidate(view);
  if (candidate.is_absolute()) {
    if (fs::exists(candidate)) {
      return candidate;
    }
    return {};
  }

  fs::path local = base_dir / view;
  if (fs::exists(local)) {
    return local;
  }
  local = sdf_dir / view;
  if (fs::exists(local)) {
    return local;
  }

  const auto roots = CollectResourceRoots(sdf_dir);
  for (const auto& root : roots) {
    fs::path p = root / view;
    if (fs::exists(p)) {
      return p;
    }
  }

  return {};
}

std::string Trim(std::string s) {
  const auto is_space = [](unsigned char c) { return std::isspace(c) != 0; };
  while (!s.empty() && is_space(static_cast<unsigned char>(s.front()))) {
    s.erase(s.begin());
  }
  while (!s.empty() && is_space(static_cast<unsigned char>(s.back()))) {
    s.pop_back();
  }
  return s;
}

bool TryParseFloat(const std::string& text, float& out) {
  char* end = nullptr;
  const float v = std::strtof(text.c_str(), &end);
  if (end == text.c_str() || *end != '\0') {
    return false;
  }
  out = v;
  return true;
}

bool ParseOgreMaterialDiffuse(const fs::path& material_script_path,
                              const std::string& material_name,
                              Color& out_color) {
  std::ifstream file(material_script_path);
  if (!file.is_open()) {
    return false;
  }

  std::vector<std::string> tokens;
  std::string line;
  while (std::getline(file, line)) {
    const std::size_t comment = line.find("//");
    if (comment != std::string::npos) {
      line = line.substr(0, comment);
    }
    line = Trim(line);
    if (line.empty()) {
      continue;
    }
    std::string token;
    for (const char c : line) {
      if (std::isspace(static_cast<unsigned char>(c)) != 0) {
        if (!token.empty()) {
          tokens.push_back(token);
          token.clear();
        }
      } else if (c == '{' || c == '}') {
        if (!token.empty()) {
          tokens.push_back(token);
          token.clear();
        }
        tokens.emplace_back(1, c);
      } else {
        token.push_back(c);
      }
    }
    if (!token.empty()) {
      tokens.push_back(token);
    }
  }

  bool waiting_target_open = false;
  bool in_target_material = false;
  int brace_depth = 0;

  for (std::size_t i = 0; i < tokens.size(); ++i) {
    const std::string& tk = tokens[i];

    if (!in_target_material) {
      if (tk == "material" && i + 1 < tokens.size()) {
        waiting_target_open = (tokens[i + 1] == material_name);
        ++i;
      } else if (waiting_target_open && tk == "{") {
        in_target_material = true;
        brace_depth = 1;
      }
      continue;
    }

    if (tk == "{") {
      ++brace_depth;
      continue;
    }
    if (tk == "}") {
      --brace_depth;
      if (brace_depth == 0) {
        return false;
      }
      continue;
    }
    if (tk == "diffuse" && i + 3 < tokens.size()) {
      float r = 0.0F;
      float g = 0.0F;
      float b = 0.0F;
      if (!TryParseFloat(tokens[i + 1], r) ||
          !TryParseFloat(tokens[i + 2], g) ||
          !TryParseFloat(tokens[i + 3], b)) {
        continue;
      }
      float a = 1.0F;
      if (i + 4 < tokens.size()) {
        float maybe_a = 1.0F;
        if (TryParseFloat(tokens[i + 4], maybe_a)) {
          a = maybe_a;
        }
      }
      out_color = Color(r, g, b, a);
      return true;
    }
  }

  return false;
}

std::string ResolveMeshUri(const std::string& uri, const fs::path& sdf_dir,
                           const fs::path& base_dir,
                           std::string* error_message) {
  if (uri.empty()) {
    return {};
  }

  std::string normalized_uri = uri;
  normalized_uri = Trim(normalized_uri);

  std::string_view view(normalized_uri);
  if (StartsWith(view, "http://") || StartsWith(view, "https://")) {
    // SDF may wrap long URLs across lines. For HTTP(S), map whitespace runs to
    // "%20" so URLs with model names like "sonoma raceway" stay valid.
    std::string http_uri;
    http_uri.reserve(normalized_uri.size());
    bool pending_space = false;
    for (const char c : normalized_uri) {
      if (std::isspace(static_cast<unsigned char>(c)) != 0) {
        pending_space = true;
        continue;
      }
      if (pending_space) {
        http_uri += "%20";
        pending_space = false;
      }
      http_uri.push_back(c);
    }
    return DownloadMeshToCache(http_uri, error_message);
  }

  if (StartsWith(view, "file://")) {
    view = view.substr(7);
  }

  fs::path candidate(view);
  if (candidate.is_absolute()) {
    if (fs::exists(candidate)) {
      return candidate.string();
    }
    return {};
  }

  if (StartsWith(view, "model://")) {
    std::string_view rest = view.substr(8);
    const size_t slash = rest.find('/');
    const std::string model_name(rest.substr(0, slash));
    const std::string rel_path = (slash == std::string_view::npos)
                                     ? std::string()
                                     : std::string(rest.substr(slash + 1));

    if (!model_name.empty() && sdf_dir.filename() == model_name) {
      fs::path direct = sdf_dir / rel_path;
      if (fs::exists(direct)) {
        return direct.string();
      }
    }

    const auto roots = CollectSearchRoots(sdf_dir);
    for (const auto& root : roots) {
      if (model_name.empty()) {
        continue;
      }
      fs::path path = root / model_name / rel_path;
      if (fs::exists(path)) {
        return path.string();
      }
    }

    return {};
  }

  fs::path relative = base_dir / view;
  if (fs::exists(relative)) {
    return relative.string();
  }

  fs::path fallback = sdf_dir / view;
  if (fs::exists(fallback)) {
    return fallback.string();
  }

  return {};
}

#endif

#ifdef LIVISION_ENABLE_SDF
Eigen::Affine3d PoseToEigen(const gz::math::Pose3d& pose) {
  Eigen::Translation3d t(pose.Pos().X(), pose.Pos().Y(), pose.Pos().Z());
  const auto& r = pose.Rot();
  Eigen::Quaterniond q(r.W(), r.X(), r.Y(), r.Z());
  return t * q;
}

void SetNodePose(SdfNode& node, const gz::math::Pose3d& pose) {
  node.pos = Eigen::Vector3d(pose.Pos().X(), pose.Pos().Y(), pose.Pos().Z());
  const auto& r = pose.Rot();
  node.rot = Eigen::Quaterniond(r.W(), r.X(), r.Y(), r.Z());
}

void SetNodeIdentity(SdfNode& node, const std::string& tag,
                     const std::string& explicit_name,
                     std::unordered_map<std::string, std::size_t>* counters) {
  node.tag = tag;
  node.name = explicit_name;
  if (!explicit_name.empty()) {
    node.effective_name = explicit_name;
    return;
  }
  if (!counters) {
    node.effective_name = tag;
    return;
  }
  const std::size_t index = (*counters)[tag]++;
  node.effective_name = tag + "#" + std::to_string(index);
}

Color ColorFromMaterial(const sdf::Material* material,
                        const fs::path& sdf_dir) {
  if (!material) {
    return color::white;
  }

  if (!material->ScriptName().empty() && !material->ScriptUri().empty()) {
    const fs::path script =
        ResolveResourceUri(material->ScriptUri(), sdf_dir, sdf_dir);
    if (!script.empty()) {
      Color script_color = color::white;
      if (ParseOgreMaterialDiffuse(script, material->ScriptName(),
                                   script_color)) {
        return script_color;
      }
    }
  }

  const auto diffuse = material->Diffuse();
  const auto ambient = material->Ambient();
  const bool has_diffuse = diffuse.R() > 0.0 || diffuse.G() > 0.0 ||
                           diffuse.B() > 0.0 || diffuse.A() > 0.0;
  const auto chosen = has_diffuse ? diffuse : ambient;
  return Color(static_cast<float>(chosen.R()), static_cast<float>(chosen.G()),
               static_cast<float>(chosen.B()), static_cast<float>(chosen.A()));
}

bool HasExplicitSdfMaterialColor(const sdf::Material* material) {
  if (!material) {
    return false;
  }

  if (!material->ScriptName().empty() || !material->ScriptUri().empty()) {
    return true;
  }

  sdf::ElementPtr elem = material->Element();
  if (!elem) {
    return false;
  }

  return elem->HasElement("diffuse") || elem->HasElement("ambient") ||
         elem->HasElement("script");
}

struct AssimpMeshData {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  bool has_uv = false;
  std::string texture_uri;
  bool has_color = false;
  Color color = color::white;
};

bool HasNonZeroColor(const Color& c) {
  return c.base[0] > 0.0F || c.base[1] > 0.0F || c.base[2] > 0.0F ||
         c.base[3] > 0.0F;
}

bool GetAssimpDiffuse(const aiMaterial* material, Color& out) {
  if (!material) {
    return false;
  }
  aiColor4D diffuse;
  if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse) !=
      aiReturn_SUCCESS) {
    return false;
  }
  out = Color(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
  return true;
}

bool GetAssimpVertexColor(const aiMesh* mesh, Color& out) {
  if (!mesh || !mesh->HasVertexColors(0)) {
    return false;
  }
  const aiColor4D* colors = mesh->mColors[0];
  if (!colors) {
    return false;
  }
  aiColor4D avg(0, 0, 0, 0);
  for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
    avg.r += colors[i].r;
    avg.g += colors[i].g;
    avg.b += colors[i].b;
    avg.a += colors[i].a;
  }
  const float denom = static_cast<float>(mesh->mNumVertices);
  out = Color(avg.r / denom, avg.g / denom, avg.b / denom, avg.a / denom);
  return true;
}

bool LoadAssimpMeshes(const std::string& mesh_source,
                      std::vector<AssimpMeshData>& meshes,
                      std::string* error_message) {
  Assimp::Importer importer;
  // Keep source asset up-axis (e.g. COLLADA Z_UP) to match Gazebo/SDF
  // results.
  importer.SetPropertyBool(AI_CONFIG_IMPORT_COLLADA_IGNORE_UP_DIRECTION, true);
  const aiScene* scene = importer.ReadFile(
      mesh_source, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
                       aiProcess_PreTransformVertices);
  if (!scene || !scene->HasMeshes()) {
    if (error_message) {
      std::ostringstream oss;
      oss << "Assimp failed to load mesh: " << mesh_source;
      if (importer.GetErrorString() && importer.GetErrorString()[0] != '\0') {
        oss << " (" << importer.GetErrorString() << ")";
      }
      *error_message = oss.str();
    }
    return false;
  }

  meshes.clear();
  meshes.reserve(scene->mNumMeshes);

  for (unsigned int mi = 0; mi < scene->mNumMeshes; ++mi) {
    const aiMesh* mesh = scene->mMeshes[mi];
    if (!mesh || mesh->mNumVertices == 0) {
      continue;
    }

    AssimpMeshData out;
    out.vertices.reserve(mesh->mNumVertices);

    for (unsigned int vi = 0; vi < mesh->mNumVertices; ++vi) {
      const aiVector3D& v = mesh->mVertices[vi];
      Vertex vv{.x = v.x, .y = v.y, .z = v.z};
      if (mesh->HasTextureCoords(0) && mesh->mTextureCoords[0]) {
        vv.u = mesh->mTextureCoords[0][vi].x;
        vv.v = mesh->mTextureCoords[0][vi].y;
        out.has_uv = true;
      }
      out.vertices.push_back(vv);
    }

    for (unsigned int fi = 0; fi < mesh->mNumFaces; ++fi) {
      const aiFace& face = mesh->mFaces[fi];
      if (face.mNumIndices != 3) {
        continue;
      }
      out.indices.push_back(face.mIndices[0]);
      out.indices.push_back(face.mIndices[1]);
      out.indices.push_back(face.mIndices[2]);
    }

    Color mesh_color = color::white;
    bool has_color = GetAssimpVertexColor(mesh, mesh_color);
    if (scene->mMaterials && mesh->mMaterialIndex < scene->mNumMaterials) {
      const aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
      if (!has_color) {
        has_color = GetAssimpDiffuse(mat, mesh_color);
      }
      aiString tex_path;
      if (mat && aiGetMaterialTexture(mat, aiTextureType_DIFFUSE, 0,
                                      &tex_path) == aiReturn_SUCCESS) {
        const std::string tex_uri = tex_path.C_Str();
        if (!tex_uri.empty()) {
          const fs::path mesh_base = fs::path(mesh_source).parent_path();
          const std::string resolved_tex =
              ResolveMeshUri(tex_uri, mesh_base, mesh_base, nullptr);
          out.texture_uri = resolved_tex.empty() ? tex_uri : resolved_tex;
        }
      }
    }
    out.has_color = has_color;
    out.color = mesh_color;
    meshes.push_back(std::move(out));
  }

  return !meshes.empty();
}

bool BuildVisualNode(const sdf::Visual& visual, const fs::path& sdf_dir,
                     SdfNode& node, std::string* error_message,
                     bool& any_mesh_loaded,
                     std::unordered_map<std::string, std::size_t>& counters) {
  SetNodeIdentity(node, "visual", visual.Name(), &counters);

  const sdf::Geometry* geom = visual.Geom();
  if (!geom) {
    return false;
  }
  SetNodePose(node, visual.RawPose());
  const sdf::Material* visual_material = visual.Material();
  const Color sdf_color = ColorFromMaterial(visual_material, sdf_dir);
  const bool prefer_sdf_material_color =
      HasExplicitSdfMaterialColor(visual_material);

  switch (geom->Type()) {
    case sdf::GeometryType::BOX: {
      const sdf::Box* box = geom->BoxShape();
      if (!box) {
        return false;
      }
      const auto size = box->Size();
      node.scale = Eigen::Vector3d(size.X(), size.Y(), size.Z());
      node.color = sdf_color;
      node.primitive = SdfNode::PrimitiveType::Box;
      any_mesh_loaded = true;
      return true;
    }
    case sdf::GeometryType::SPHERE: {
      const sdf::Sphere* sphere = geom->SphereShape();
      if (!sphere) {
        return false;
      }
      const double r = sphere->Radius();
      node.scale = Eigen::Vector3d(2.0 * r, 2.0 * r, 2.0 * r);
      node.color = sdf_color;
      node.primitive = SdfNode::PrimitiveType::Sphere;
      any_mesh_loaded = true;
      return true;
    }
    case sdf::GeometryType::CYLINDER: {
      const sdf::Cylinder* cyl = geom->CylinderShape();
      if (!cyl) {
        return false;
      }
      const double r = cyl->Radius();
      const double len = cyl->Length();
      node.scale = Eigen::Vector3d(2.0 * r, 2.0 * r, len);
      node.color = sdf_color;
      node.primitive = SdfNode::PrimitiveType::Cylinder;
      any_mesh_loaded = true;
      return true;
    }
    case sdf::GeometryType::CONE: {
      const sdf::Cone* cone = geom->ConeShape();
      if (!cone) {
        return false;
      }
      const double r = cone->Radius();
      const double len = cone->Length();
      node.scale = Eigen::Vector3d(2.0 * r, 2.0 * r, len);
      node.color = sdf_color;
      node.primitive = SdfNode::PrimitiveType::Cone;
      any_mesh_loaded = true;
      return true;
    }
    case sdf::GeometryType::PLANE: {
      const sdf::Plane* plane = geom->PlaneShape();
      if (!plane) {
        return false;
      }
      const auto size = plane->Size();
      node.scale = Eigen::Vector3d(size.X(), size.Y(), 1.0);
      const auto normal = plane->Normal();
      Eigen::Vector3d n(normal.X(), normal.Y(), normal.Z());
      if (n.norm() > 1e-8) {
        n.normalize();
        Eigen::Quaterniond align =
            Eigen::Quaterniond::FromTwoVectors(Eigen::Vector3d::UnitZ(), n);
        node.rot = node.rot * align;
      }
      node.color = sdf_color;
      node.primitive = SdfNode::PrimitiveType::Plane;
      any_mesh_loaded = true;
      return true;
    }
    case sdf::GeometryType::MESH:
      break;
    default:
      return false;
  }

  const sdf::Mesh* mesh = geom->MeshShape();
  if (!mesh) {
    return false;
  }

  node.scale =
      Eigen::Vector3d(mesh->Scale().X(), mesh->Scale().Y(), mesh->Scale().Z());

  fs::path base_dir = sdf_dir;
  if (!mesh->FilePath().empty()) {
    base_dir = fs::path(mesh->FilePath()).parent_path();
  }

  const std::string resolved =
      ResolveMeshUri(mesh->Uri(), sdf_dir, base_dir, error_message);
  if (resolved.empty()) {
    if (error_message) {
      std::ostringstream oss;
      oss << "Failed to resolve mesh URI: " << mesh->Uri();
      *error_message = oss.str();
    }
    return false;
  }

  std::string assimp_error;
  std::vector<AssimpMeshData> assimp_meshes;
  if (!LoadAssimpMeshes(resolved, assimp_meshes, &assimp_error)) {
    if (error_message) {
      *error_message = assimp_error;
    }
    return false;
  }

  std::unordered_map<std::string, std::size_t> mesh_counters;
  for (auto& mesh_data : assimp_meshes) {
    SdfNode mesh_node;
    SetNodeIdentity(mesh_node, "mesh", "", &mesh_counters);
    mesh_node.vertices = std::move(mesh_data.vertices);
    mesh_node.indices = std::move(mesh_data.indices);
    mesh_node.has_uv = mesh_data.has_uv;
    mesh_node.texture = mesh_data.texture_uri;
    if (prefer_sdf_material_color) {
      mesh_node.color = sdf_color;
    } else if (mesh_data.has_color && HasNonZeroColor(mesh_data.color)) {
      mesh_node.color = mesh_data.color;
    } else {
      mesh_node.color = sdf_color;
    }
    node.children.push_back(std::move(mesh_node));
    any_mesh_loaded = true;
  }

  return !node.children.empty();
}

bool BuildLinkNode(const sdf::Link& link, const fs::path& sdf_dir,
                   SdfNode& node, std::string* error_message,
                   bool& any_mesh_loaded,
                   std::unordered_map<std::string, std::size_t>& counters) {
  SetNodeIdentity(node, "link", link.Name(), &counters);
  SetNodePose(node, link.RawPose());
  node.scale = Eigen::Vector3d::Ones();
  std::unordered_map<std::string, std::size_t> child_counters;

  for (uint64_t vi = 0; vi < link.VisualCount(); ++vi) {
    const sdf::Visual* visual = link.VisualByIndex(vi);
    if (!visual) {
      continue;
    }
    SdfNode visual_node;
    if (BuildVisualNode(*visual, sdf_dir, visual_node, error_message,
                        any_mesh_loaded, child_counters)) {
      node.children.push_back(std::move(visual_node));
    }
  }

  return !node.children.empty();
}

bool BuildModelNode(const sdf::Model& model, const fs::path& sdf_dir,
                    SdfNode& node, std::string* error_message,
                    bool& any_mesh_loaded,
                    std::unordered_map<std::string, std::size_t>& counters) {
  SetNodeIdentity(node, "model", model.Name(), &counters);
  SetNodePose(node, model.RawPose());
  node.scale = Eigen::Vector3d::Ones();
  std::unordered_map<std::string, std::size_t> child_counters;

  for (uint64_t li = 0; li < model.LinkCount(); ++li) {
    const sdf::Link* link = model.LinkByIndex(li);
    if (!link) {
      continue;
    }
    SdfNode link_node;
    if (BuildLinkNode(*link, sdf_dir, link_node, error_message,
                      any_mesh_loaded, child_counters)) {
      node.children.push_back(std::move(link_node));
    }
  }

  for (uint64_t mi = 0; mi < model.ModelCount(); ++mi) {
    const sdf::Model* nested = model.ModelByIndex(mi);
    if (!nested) {
      continue;
    }
    SdfNode nested_node;
    if (BuildModelNode(*nested, sdf_dir, nested_node, error_message,
                       any_mesh_loaded, child_counters)) {
      node.children.push_back(std::move(nested_node));
    }
  }

  return !node.children.empty();
}

void AppendModelMeshes(const sdf::Model& model, const fs::path& sdf_dir,
                       const Eigen::Affine3d& parent_transform,
                       std::vector<Vertex>& vertices,
                       std::vector<uint32_t>& indices,
                       std::string* error_message, bool& any_mesh_loaded) {
  const Eigen::Affine3d model_tf =
      parent_transform * PoseToEigen(model.RawPose());

  for (uint64_t li = 0; li < model.LinkCount(); ++li) {
    const sdf::Link* link = model.LinkByIndex(li);
    if (!link) {
      continue;
    }
    const Eigen::Affine3d link_tf = model_tf * PoseToEigen(link->RawPose());

    for (uint64_t vi = 0; vi < link->VisualCount(); ++vi) {
      const sdf::Visual* visual = link->VisualByIndex(vi);
      if (!visual) {
        continue;
      }
      const sdf::Geometry* geom = visual->Geom();
      if (!geom || geom->Type() != sdf::GeometryType::MESH) {
        continue;
      }
      const sdf::Mesh* mesh = geom->MeshShape();
      if (!mesh) {
        continue;
      }

      fs::path base_dir = sdf_dir;
      if (!mesh->FilePath().empty()) {
        base_dir = fs::path(mesh->FilePath()).parent_path();
      }

      const std::string resolved =
          ResolveMeshUri(mesh->Uri(), sdf_dir, base_dir, error_message);
      if (resolved.empty()) {
        if (error_message) {
          std::ostringstream oss;
          oss << "Failed to resolve mesh URI: " << mesh->Uri();
          *error_message = oss.str();
        }
        continue;
      }

      Eigen::Affine3d visual_tf = link_tf * PoseToEigen(visual->RawPose());
      visual_tf =
          visual_tf * Eigen::Scaling(mesh->Scale().X(), mesh->Scale().Y(),
                                     mesh->Scale().Z());

      std::string assimp_error;
      std::vector<AssimpMeshData> assimp_meshes;
      if (!LoadAssimpMeshes(resolved, assimp_meshes, &assimp_error)) {
        if (error_message) {
          *error_message = assimp_error;
        }
        continue;
      }

      for (const auto& mesh_data : assimp_meshes) {
        const uint32_t base_index = static_cast<uint32_t>(vertices.size());
        vertices.reserve(vertices.size() + mesh_data.vertices.size());
        for (const auto& v : mesh_data.vertices) {
          Eigen::Vector3d p =
              visual_tf * Eigen::Vector3d(static_cast<double>(v.x),
                                          static_cast<double>(v.y),
                                          static_cast<double>(v.z));
          vertices.push_back(Vertex{.x = static_cast<float>(p.x()),
                                    .y = static_cast<float>(p.y()),
                                    .z = static_cast<float>(p.z()),
                                    .u = v.u,
                                    .v = v.v});
        }
        indices.reserve(indices.size() + mesh_data.indices.size());
        for (const auto idx : mesh_data.indices) {
          indices.push_back(base_index + idx);
        }
        any_mesh_loaded = true;
      }
    }
  }

  for (uint64_t mi = 0; mi < model.ModelCount(); ++mi) {
    const sdf::Model* nested = model.ModelByIndex(mi);
    if (!nested) {
      continue;
    }
    AppendModelMeshes(*nested, sdf_dir, model_tf, vertices, indices,
                      error_message, any_mesh_loaded);
  }
}
#endif

}  // namespace

bool LoadSdfMeshes(const std::string& sdf_path, std::vector<Vertex>& vertices,
                   std::vector<uint32_t>& indices, std::string* error_message) {
#ifndef LIVISION_ENABLE_SDF
  if (error_message) {
    *error_message =
        "SDF support is disabled (LIVISION_ENABLE_SDF is not enabled).";
  }
  (void)sdf_path;
  (void)vertices;
  (void)indices;
  return false;
#else
  const fs::path sdf_file(sdf_path);
  const fs::path sdf_dir = sdf_file.parent_path();

  sdf::ParserConfig config;
  config.SetFindCallback([sdf_dir](const std::string& uri) -> std::string {
    const std::string resolved = ResolveMeshUri(uri, sdf_dir, sdf_dir, nullptr);
    if (!resolved.empty()) {
      return resolved;
    }
    return {};
  });

  sdf::Root root;
  sdf::Errors errors = root.Load(sdf_path, config);
  if (!errors.empty()) {
    if (error_message) {
      std::ostringstream oss;
      oss << "Failed to load SDF file: " << sdf_path;
      for (const auto& err : errors) {
        oss << "\n  - " << err.Message();
      }
      *error_message = oss.str();
    }
    return false;
  }

  std::vector<const sdf::Model*> models;
  if (const sdf::Model* model = root.Model()) {
    models.push_back(model);
  }

  for (uint64_t wi = 0; wi < root.WorldCount(); ++wi) {
    const sdf::World* world = root.WorldByIndex(wi);
    if (!world) {
      continue;
    }
    for (uint64_t mi = 0; mi < world->ModelCount(); ++mi) {
      const sdf::Model* model = world->ModelByIndex(mi);
      if (model) {
        models.push_back(model);
      }
    }
  }

  if (models.empty()) {
    if (error_message) {
      *error_message = "No models found in SDF file.";
    }
    return false;
  }

  bool any_mesh_loaded = false;
  for (const auto* model : models) {
    if (!model) {
      continue;
    }
    AppendModelMeshes(*model, sdf_dir, Eigen::Affine3d::Identity(), vertices,
                      indices, error_message, any_mesh_loaded);
  }

  if (!any_mesh_loaded) {
    if (error_message && error_message->empty()) {
      *error_message =
          "No mesh visuals found in SDF file or all meshes failed to load.";
    }
    return false;
  }

  return true;
#endif
}

bool LoadMeshFile(const std::string& mesh_path, std::vector<Vertex>& vertices,
                  std::vector<uint32_t>& indices, bool* has_uv,
                  std::string* texture_uri, std::string* error_message) {
#ifndef LIVISION_ENABLE_SDF
  if (error_message) {
    *error_message =
        "Assimp mesh loading is disabled (LIVISION_ENABLE_SDF is not "
        "enabled).";
  }
  (void)mesh_path;
  (void)vertices;
  (void)indices;
  if (has_uv) {
    *has_uv = false;
  }
  if (texture_uri) {
    texture_uri->clear();
  }
  return false;
#else
  std::vector<AssimpMeshData> assimp_meshes;
  std::string assimp_error;
  if (!LoadAssimpMeshes(mesh_path, assimp_meshes, &assimp_error)) {
    if (error_message) {
      *error_message = assimp_error;
    }
    return false;
  }

  vertices.clear();
  indices.clear();
  if (has_uv) {
    *has_uv = false;
  }
  if (texture_uri) {
    texture_uri->clear();
  }

  for (const auto& mesh_data : assimp_meshes) {
    const uint32_t base_index = static_cast<uint32_t>(vertices.size());
    vertices.reserve(vertices.size() + mesh_data.vertices.size());
    for (const auto& v : mesh_data.vertices) {
      vertices.push_back(v);
    }
    indices.reserve(indices.size() + mesh_data.indices.size());
    for (const auto idx : mesh_data.indices) {
      indices.push_back(base_index + idx);
    }
    if (has_uv && mesh_data.has_uv) {
      *has_uv = true;
    }
    if (texture_uri && texture_uri->empty() && !mesh_data.texture_uri.empty()) {
      *texture_uri = mesh_data.texture_uri;
    }
  }

  return !vertices.empty() && !indices.empty();
#endif
}

bool LoadMeshFileParts(const std::string& mesh_path,
                       std::vector<MeshPart>& parts,
                       std::string* error_message) {
#ifndef LIVISION_ENABLE_SDF
  if (error_message) {
    *error_message =
        "Assimp mesh loading is disabled (LIVISION_ENABLE_SDF is not "
        "enabled).";
  }
  (void)mesh_path;
  (void)parts;
  return false;
#else
  std::vector<AssimpMeshData> assimp_meshes;
  std::string assimp_error;
  if (!LoadAssimpMeshes(mesh_path, assimp_meshes, &assimp_error)) {
    if (error_message) {
      *error_message = assimp_error;
    }
    return false;
  }

  parts.clear();
  parts.reserve(assimp_meshes.size());
  for (auto& src : assimp_meshes) {
    MeshPart part;
    part.vertices = std::move(src.vertices);
    part.indices = std::move(src.indices);
    part.has_uv = src.has_uv;
    part.texture_uri = std::move(src.texture_uri);
    part.has_color = src.has_color;
    part.color = src.color;
    if (!part.vertices.empty() && !part.indices.empty()) {
      parts.push_back(std::move(part));
    }
  }
  return !parts.empty();
#endif
}

bool LoadSdfScene(const std::string& sdf_path, SdfNode& root,
                  std::string* error_message) {
#ifndef LIVISION_ENABLE_SDF
  if (error_message) {
    *error_message =
        "SDF support is disabled (LIVISION_ENABLE_SDF is not enabled).";
  }
  (void)sdf_path;
  (void)root;
  return false;
#else
  const fs::path sdf_file(sdf_path);
  const fs::path sdf_dir = sdf_file.parent_path();

  sdf::ParserConfig config;
  config.SetFindCallback([sdf_dir](const std::string& uri) -> std::string {
    const std::string resolved = ResolveMeshUri(uri, sdf_dir, sdf_dir, nullptr);
    if (!resolved.empty()) {
      return resolved;
    }
    return {};
  });

  sdf::Root sdf_root;
  sdf::Errors errors = sdf_root.Load(sdf_path, config);
  if (!errors.empty()) {
    if (error_message) {
      std::ostringstream oss;
      oss << "Failed to load SDF file: " << sdf_path;
      for (const auto& err : errors) {
        oss << "\n  - " << err.Message();
      }
      *error_message = oss.str();
    }
    return false;
  }

  root = SdfNode{};
  root.tag = "sdf";
  root.effective_name = "sdf";
  bool any_mesh_loaded = false;
  bool any_model_found = false;
  std::unordered_map<std::string, std::size_t> world_counters;

  if (const sdf::Model* model = sdf_root.Model()) {
    any_model_found = true;
    SdfNode world_node;
    SetNodeIdentity(world_node, "world", "", &world_counters);
    world_node.scale = Eigen::Vector3d::Ones();
    std::unordered_map<std::string, std::size_t> child_counters;
    SdfNode model_node;
    if (BuildModelNode(*model, sdf_dir, model_node, error_message,
                       any_mesh_loaded, child_counters)) {
      world_node.children.push_back(std::move(model_node));
    }
    if (!world_node.children.empty()) {
      root.children.push_back(std::move(world_node));
    }
  }

  for (uint64_t wi = 0; wi < sdf_root.WorldCount(); ++wi) {
    const sdf::World* world = sdf_root.WorldByIndex(wi);
    if (!world) {
      continue;
    }
    SdfNode world_node;
    SetNodeIdentity(world_node, "world", world->Name(), &world_counters);
    world_node.scale = Eigen::Vector3d::Ones();
    std::unordered_map<std::string, std::size_t> child_counters;
    for (uint64_t mi = 0; mi < world->ModelCount(); ++mi) {
      const sdf::Model* model = world->ModelByIndex(mi);
      if (!model) {
        continue;
      }
      any_model_found = true;
      SdfNode model_node;
      if (BuildModelNode(*model, sdf_dir, model_node, error_message,
                         any_mesh_loaded, child_counters)) {
        world_node.children.push_back(std::move(model_node));
      }
    }
    if (!world_node.children.empty()) {
      root.children.push_back(std::move(world_node));
    }
  }

  if (!any_model_found) {
    if (error_message) {
      *error_message = "No models found in SDF file.";
    }
    return false;
  }

  if (!any_mesh_loaded) {
    if (error_message && error_message->empty()) {
      *error_message =
          "No mesh visuals found in SDF file or all meshes failed to load.";
    }
    return false;
  }

  return true;
#endif
}

}  // namespace livision::internal::sdf_loader
