#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <Eigen/Geometry>

#include "livision/Color.hpp"
#include "livision/Vertex.hpp"

namespace livision::internal::sdf_loader {

// Load all mesh visuals from an SDF file and append them into vertices/indices.
// Returns true on success, false on failure. When false, error_message (if
// provided) is filled with a human-readable description.
bool LoadSdfMeshes(const std::string& sdf_path, std::vector<Vertex>& vertices,
                   std::vector<uint32_t>& indices,
                   std::string* error_message = nullptr);

// Load a mesh file with assimp (STL/DAE/OBJ, etc.) and merge submeshes.
bool LoadMeshFile(const std::string& mesh_path, std::vector<Vertex>& vertices,
                  std::vector<uint32_t>& indices,
                  std::string* error_message = nullptr);

struct SdfNode {
  enum class PrimitiveType { None, Box, Sphere, Cylinder, Cone, Plane };

  Eigen::Vector3d pos = Eigen::Vector3d::Zero();
  Eigen::Quaterniond rot = Eigen::Quaterniond::Identity();
  Eigen::Vector3d scale = Eigen::Vector3d::Ones();
  Color color = color::white;
  PrimitiveType primitive = PrimitiveType::None;
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  std::vector<SdfNode> children;

  bool HasMesh() const { return !vertices.empty() && !indices.empty(); }
  bool HasPrimitive() const { return primitive != PrimitiveType::None; }
};

// Load an SDF file into a node hierarchy. Each node stores local transform and
// optional mesh data (for visuals). Colors are taken from material diffuse or
// ambient values (textures are ignored).
bool LoadSdfScene(const std::string& sdf_path, SdfNode& root,
                  std::string* error_message = nullptr);

}  // namespace livision::internal::sdf_loader
