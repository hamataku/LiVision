// Python bindings for LiVision (pybind11).
//
// Naming follows PEP 8 (snake_case methods) while mirroring the C++ API.
// Objects are constructed with keyword arguments that map to
// ObjectBase::Params, e.g. Box(pos=[0, 0, 1], scale=2.0, color=color.red).
// Setters return self so calls can be chained like in C++.

#include <pybind11/eigen.h>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "livision/Container.hpp"
#include "livision/Viewer.hpp"
#include "livision/marker/Arrow.hpp"
#include "livision/marker/DegeneracyIndicator.hpp"
#include "livision/marker/Grid.hpp"
#include "livision/marker/Odometry.hpp"
#include "livision/marker/Path.hpp"
#include "livision/marker/PointCloud.hpp"
#include "livision/object/Drone.hpp"
#include "livision/object/Mesh.hpp"
#include "livision/object/Model.hpp"
#include "livision/object/Text.hpp"
#include "livision/object/primitives.hpp"

#ifndef LIVISION_VERSION
#define LIVISION_VERSION "0.0.0"
#endif

namespace py = pybind11;
using namespace livision;
using namespace pybind11::literals;

namespace {

// ---------------------------------------------------------------------------
// Conversion helpers
// ---------------------------------------------------------------------------

using DoubleArray =
    py::array_t<double, py::array::c_style | py::array::forcecast>;
using FloatArray = py::array_t<float, py::array::c_style | py::array::forcecast>;
using IndexArray =
    py::array_t<uint32_t, py::array::c_style | py::array::forcecast>;

// Quaternion is exchanged as [x, y, z, w] (scipy convention).
Eigen::Quaterniond QuatFromPy(const py::handle& obj) {
  const auto v = obj.cast<Eigen::Vector4d>();
  Eigen::Quaterniond q(v[3], v[0], v[1], v[2]);
  q.normalize();
  return q;
}

Eigen::Vector4d QuatToPy(const Eigen::Quaterniond& q) {
  return {q.x(), q.y(), q.z(), q.w()};
}

// Same convention as ObjectBase::SetRadRotation (Z * Y * X).
Eigen::Quaterniond QuatFromEulerRad(const Eigen::Vector3d& rad) {
  return Eigen::AngleAxisd(rad[2], Eigen::Vector3d::UnitZ()) *
         Eigen::AngleAxisd(rad[1], Eigen::Vector3d::UnitY()) *
         Eigen::AngleAxisd(rad[0], Eigen::Vector3d::UnitX());
}

bool IsNumber(const py::handle& v) {
  return py::isinstance<py::float_>(v) || py::isinstance<py::int_>(v);
}

Eigen::Vector3d ScaleFromPy(const py::handle& v) {
  if (IsNumber(v)) {
    const double s = v.cast<double>();
    return {s, s, s};
  }
  return v.cast<Eigen::Vector3d>();
}

// Applies one keyword to ObjectBase::Params. Returns false if unknown.
bool ApplyBaseParam(ObjectBase::Params& p, const std::string& key,
                    const py::handle& value) {
  if (key == "pos") {
    p.pos = value.cast<Eigen::Vector3d>();
  } else if (key == "scale") {
    p.scale = ScaleFromPy(value);
  } else if (key == "quat") {
    p.quat = QuatFromPy(value);
  } else if (key == "deg_rotation") {
    p.quat = QuatFromEulerRad(value.cast<Eigen::Vector3d>() * M_PI / 180.0);
  } else if (key == "rad_rotation") {
    p.quat = QuatFromEulerRad(value.cast<Eigen::Vector3d>());
  } else if (key == "color") {
    p.color = value.cast<Color>();
  } else if (key == "wire_color") {
    p.wire_color = value.cast<Color>();
  } else if (key == "texture") {
    p.texture = value.cast<std::string>();
  } else if (key == "name") {
    p.name = value.cast<std::string>();
  } else {
    return false;
  }
  return true;
}

[[noreturn]] void ThrowUnexpectedKeyword(const std::string& key) {
  throw py::type_error("unexpected keyword argument '" + key + "'");
}

ObjectBase::Params ParamsFromKwargs(const py::kwargs& kwargs) {
  ObjectBase::Params p;
  for (const auto& item : kwargs) {
    const auto key = py::str(item.first).cast<std::string>();
    if (!ApplyBaseParam(p, key, item.second)) {
      ThrowUnexpectedKeyword(key);
    }
  }
  return p;
}

Text::Params TextParamsFromKwargs(const py::kwargs& kwargs) {
  Text::Params p;
  for (const auto& item : kwargs) {
    const auto key = py::str(item.first).cast<std::string>();
    const auto& v = item.second;
    if (ApplyBaseParam(p, key, v)) {
      continue;
    }
    if (key == "text") {
      p.text = v.cast<std::string>();
    } else if (key == "height") {
      p.height = v.cast<float>();
    } else if (key == "font") {
      p.font = v.cast<std::string>();
    } else if (key == "facing_mode") {
      p.facing_mode = v.cast<TextFacingMode>();
    } else if (key == "depth_mode") {
      p.depth_mode = v.cast<TextDepthMode>();
    } else if (key == "align") {
      p.align = v.cast<TextAlign>();
    } else {
      ThrowUnexpectedKeyword(key);
    }
  }
  return p;
}

std::vector<Eigen::Vector3d> ToVector3List(const DoubleArray& arr,
                                           const char* what) {
  if (arr.ndim() != 2 || arr.shape(1) != 3) {
    throw py::value_error(std::string(what) + " must be an (N, 3) array");
  }
  auto r = arr.unchecked<2>();
  std::vector<Eigen::Vector3d> out;
  out.reserve(static_cast<size_t>(r.shape(0)));
  for (py::ssize_t i = 0; i < r.shape(0); ++i) {
    out.emplace_back(r(i, 0), r(i, 1), r(i, 2));
  }
  return out;
}

py::array_t<double> FromVector3List(const std::vector<Eigen::Vector3d>& v) {
  py::array_t<double> out({static_cast<py::ssize_t>(v.size()),
                           static_cast<py::ssize_t>(3)});
  auto w = out.mutable_unchecked<2>();
  for (size_t i = 0; i < v.size(); ++i) {
    w(static_cast<py::ssize_t>(i), 0) = v[i].x();
    w(static_cast<py::ssize_t>(i), 1) = v[i].y();
    w(static_cast<py::ssize_t>(i), 2) = v[i].z();
  }
  return out;
}

py::array_t<double> FromVector4List(const std::vector<Eigen::Vector4d>& v) {
  py::array_t<double> out({static_cast<py::ssize_t>(v.size()),
                           static_cast<py::ssize_t>(4)});
  auto w = out.mutable_unchecked<2>();
  for (size_t i = 0; i < v.size(); ++i) {
    for (py::ssize_t j = 0; j < 4; ++j) {
      w(static_cast<py::ssize_t>(i), j) = v[i][j];
    }
  }
  return out;
}

std::vector<Vertex> ToVertexList(const FloatArray& arr) {
  if (arr.ndim() != 2 || (arr.shape(1) != 3 && arr.shape(1) != 5)) {
    throw py::value_error(
        "vertices must be an (N, 3) array of xyz or an (N, 5) array of xyzuv");
  }
  auto r = arr.unchecked<2>();
  const bool has_uv = r.shape(1) == 5;
  std::vector<Vertex> out;
  out.reserve(static_cast<size_t>(r.shape(0)));
  for (py::ssize_t i = 0; i < r.shape(0); ++i) {
    Vertex v{r(i, 0), r(i, 1), r(i, 2)};
    if (has_uv) {
      v.u = r(i, 3);
      v.v = r(i, 4);
    }
    out.push_back(v);
  }
  return out;
}

std::vector<uint32_t> ToIndexList(const IndexArray& arr) {
  std::vector<uint32_t> out(static_cast<size_t>(arr.size()));
  const auto* data = arr.data();
  std::copy(data, data + arr.size(), out.begin());
  return out;
}

Color ColorFromSequence(const py::sequence& seq) {
  const auto n = py::len(seq);
  if (n != 3 && n != 4) {
    throw py::value_error("color must be (r, g, b) or (r, g, b, a)");
  }
  const float r = seq[0].cast<float>();
  const float g = seq[1].cast<float>();
  const float b = seq[2].cast<float>();
  const float a = n == 4 ? seq[3].cast<float>() : 1.0F;
  return Color(r, g, b, a);
}

// Chainable setter: runs `fn(self, args...)` and returns the same Python
// object. The argument list is deduced from the lambda's call operator so
// pybind11 sees a concrete signature.
template <class T, class Fn, class... Args>
auto ChainImpl(Fn fn, void (Fn::*)(T&, Args...) const) {
  return [fn](py::object self, Args... args) -> py::object {
    fn(self.cast<T&>(), args...);
    return self;
  };
}

template <class T, class Fn>
auto Chain(Fn fn) {
  return ChainImpl<T>(fn, &Fn::operator());
}

// ---------------------------------------------------------------------------
// Bindings per area
// ---------------------------------------------------------------------------

void BindLog(py::module_& m) {
  py::enum_<LogLevel>(m, "LogLevel")
      .value("Off", LogLevel::Off)
      .value("Error", LogLevel::Error)
      .value("Warn", LogLevel::Warn)
      .value("Info", LogLevel::Info)
      .value("Debug", LogLevel::Debug);
  m.def("set_log_level", &SetLogLevel, "level"_a,
        "Set the global log level.");
  m.def("get_log_level", &GetLogLevel, "Get the global log level.");
}

void BindColor(py::module_& m) {
  py::class_<Color> color(m, "Color");
  py::enum_<Color::ColorMode>(color, "ColorMode")
      .value("Fixed", Color::ColorMode::Fixed)
      .value("Rainbow", Color::ColorMode::Rainbow)
      .value("InVisible", Color::ColorMode::InVisible);

  color
      .def(py::init<float, float, float, float>(), "r"_a, "g"_a, "b"_a,
           "a"_a = 1.0F, "RGBA color, each channel in [0, 1].")
      .def(py::init([](const py::sequence& seq) {
             return ColorFromSequence(seq);
           }),
           "rgba"_a, "Color from an (r, g, b) or (r, g, b, a) sequence.")
      .def(py::init<const Color::ColorMode&>(), "mode"_a)
      .def_static(
          "rainbow",
          [](float r, float g, float b, const Eigen::Vector3d& direction,
             float a) { return Color(r, g, b, a, direction); },
          "r"_a, "g"_a, "b"_a, "direction"_a, "a"_a = 1.0F,
          "Rainbow color: hue shifts along `direction` (its length sets the "
          "period).")
      .def_property(
          "r", [](const Color& c) { return c.base[0]; },
          [](Color& c, float v) { c.base[0] = v; })
      .def_property(
          "g", [](const Color& c) { return c.base[1]; },
          [](Color& c, float v) { c.base[1] = v; })
      .def_property(
          "b", [](const Color& c) { return c.base[2]; },
          [](Color& c, float v) { c.base[2] = v; })
      .def_property(
          "a", [](const Color& c) { return c.base[3]; },
          [](Color& c, float v) { c.base[3] = v; })
      .def_readwrite("mode", &Color::mode)
      .def_readwrite("direction", &Color::direction)
      .def("red", &Color::Red, "r"_a, "Copy with the red channel replaced.")
      .def("green", &Color::Green, "g"_a,
           "Copy with the green channel replaced.")
      .def("blue", &Color::Blue, "b"_a, "Copy with the blue channel replaced.")
      .def("alpha", &Color::Alpha, "a"_a, "Copy with the alpha replaced.")
      .def("set_mode", &Color::SetMode, "mode"_a, "Copy with a new mode.")
      .def("set_direction", &Color::SetDirection, "direction"_a,
           "Copy with a new rainbow direction.")
      .def("__repr__", [](const Color& c) {
        std::string mode = c.mode == Color::ColorMode::Fixed     ? "Fixed"
                           : c.mode == Color::ColorMode::Rainbow ? "Rainbow"
                                                                 : "InVisible";
        return "Color(r=" + std::to_string(c.base[0]) +
               ", g=" + std::to_string(c.base[1]) +
               ", b=" + std::to_string(c.base[2]) +
               ", a=" + std::to_string(c.base[3]) + ", mode=" + mode + ")";
      });
  py::implicitly_convertible<py::tuple, Color>();
  py::implicitly_convertible<py::list, Color>();

  py::module_ cm = m.def_submodule("color", "Predefined colors.");
  cm.attr("transparent") = color::transparent;
  cm.attr("white") = color::white;
  cm.attr("black") = color::black;
  cm.attr("gray") = color::gray;
  cm.attr("light_gray") = color::light_gray;
  cm.attr("off_white") = color::off_white;
  cm.attr("dark_gray") = color::dark_gray;
  cm.attr("red") = color::red;
  cm.attr("green") = color::green;
  cm.attr("blue") = color::blue;
  cm.attr("yellow") = color::yellow;
  cm.attr("cyan") = color::cyan;
  cm.attr("magenta") = color::magenta;
  cm.attr("orange") = color::orange;
  cm.attr("teal") = color::teal;
  cm.attr("olive") = color::olive;
  cm.attr("violet") = color::violet;
  cm.attr("rose") = color::rose;
  cm.attr("sand") = color::sand;
  cm.attr("rainbow_x") = color::rainbow_x;
  cm.attr("rainbow_y") = color::rainbow_y;
  cm.attr("rainbow_z") = color::rainbow_z;
  cm.attr("palette") = color::color_palette;
}

void BindObjectBase(py::module_& m) {
  py::class_<ObjectBase, std::shared_ptr<ObjectBase>>(m, "ObjectBase")
      .def("set_pos",
           Chain<ObjectBase>([](ObjectBase& o, const Eigen::Vector3d& p) {
             o.SetPos(p);
           }),
           "pos"_a, "Set local position [x, y, z].")
      .def("set_pos",
           Chain<ObjectBase>([](ObjectBase& o, double x, double y, double z) {
             o.SetPos(x, y, z);
           }),
           "x"_a, "y"_a, "z"_a)
      .def("set_scale",
           Chain<ObjectBase>([](ObjectBase& o, const py::handle& s) {
             o.SetScale(ScaleFromPy(s));
           }),
           "scale"_a, "Set scale; a scalar applies to all axes.")
      .def("set_scale",
           Chain<ObjectBase>([](ObjectBase& o, double x, double y, double z) {
             o.SetScale(x, y, z);
           }),
           "x"_a, "y"_a, "z"_a)
      .def("set_quat_rotation",
           Chain<ObjectBase>([](ObjectBase& o, const py::handle& q) {
             o.SetQuatRotation(QuatFromPy(q));
           }),
           "quat"_a, "Set rotation from a quaternion [x, y, z, w].")
      .def("set_deg_rotation",
           Chain<ObjectBase>([](ObjectBase& o, const Eigen::Vector3d& e) {
             o.SetDegRotation(e);
           }),
           "euler_deg"_a, "Set rotation from XYZ Euler angles in degrees.")
      .def("set_rad_rotation",
           Chain<ObjectBase>([](ObjectBase& o, const Eigen::Vector3d& e) {
             o.SetRadRotation(e);
           }),
           "euler_rad"_a, "Set rotation from XYZ Euler angles in radians.")
      .def("set_visible",
           Chain<ObjectBase>([](ObjectBase& o, bool v) { o.SetVisible(v); }),
           "visible"_a)
      .def("set_color",
           Chain<ObjectBase>([](ObjectBase& o, const Color& c) {
             o.SetColor(c);
           }),
           "color"_a)
      .def("set_wire_color",
           Chain<ObjectBase>([](ObjectBase& o, const Color& c) {
             o.SetWireColor(c);
           }),
           "color"_a)
      .def("set_texture",
           Chain<ObjectBase>([](ObjectBase& o, const std::string& t) {
             o.SetTexture(t);
           }),
           "path"_a)
      .def("clear_texture",
           Chain<ObjectBase>([](ObjectBase& o) { o.ClearTexture(); }))
      .def("set_name",
           Chain<ObjectBase>([](ObjectBase& o, const std::string& n) {
             o.SetName(n);
           }),
           "name"_a)
      .def("set_global_matrix",
           Chain<ObjectBase>([](ObjectBase& o, const Eigen::Matrix4d& m) {
             o.SetGlobalMatrix(Eigen::Affine3d(m));
           }),
           "matrix"_a, "Override the global 4x4 transform.")
      .def("is_visible", &ObjectBase::IsVisible)
      .def("get_global_pos", &ObjectBase::GetGlobalPos)
      .def("get_global_matrix",
           [](const ObjectBase& o) -> Eigen::Matrix4d {
             return o.GetGlobalMatrix().matrix();
           })
      .def("get_name", &ObjectBase::GetName)
      .def_property_readonly("name", &ObjectBase::GetName)
      .def("__repr__", [](py::object self) {
        const auto& o = self.cast<const ObjectBase&>();
        const std::string type =
            py::str(self.get_type().attr("__name__")).cast<std::string>();
        return "<livision." + type + " name='" + o.GetName() + "'>";
      });
}

void BindContainer(py::module_& m) {
  py::class_<Container, ObjectBase, std::shared_ptr<Container>>(m,
                                                                  "Container")
      .def(py::init([](const py::kwargs& kw) {
             return std::make_shared<Container>(ParamsFromKwargs(kw));
           }),
           "Group of objects moved together.")
      .def("add_object",
           Chain<Container>([](Container& c, std::shared_ptr<ObjectBase> o) {
             c.AddObject(std::move(o));
           }),
           "object"_a)
      .def("get_objects",
           [](Container& c) { return c.GetObjects(); })
      .def("clear_objects", &Container::ClearObjects)
      .def("get_child", &Container::GetChild, "name"_a)
      .def("get_children", &Container::GetChildren, "name"_a)
      .def("get_child_container", &Container::GetChildContainer, "name"_a)
      .def("get_by_path", &Container::GetByPath, "path"_a)
      .def("get_container_by_path", &Container::GetContainerByPath, "path"_a)
      .def("dump_tree", &Container::DumpTree)
      .def("print_tree", &Container::PrintTree, "level"_a = LogLevel::Info);
}

template <class T>
void BindPrimitive(py::module_& m, const char* name, const char* doc) {
  py::class_<T, ObjectBase, std::shared_ptr<T>>(m, name)
      .def(py::init([](const py::kwargs& kw) {
             return std::make_shared<T>(ParamsFromKwargs(kw));
           }),
           doc);
}

void BindObjects(py::module_& m) {
  BindPrimitive<Box>(m, "Box", "Unit cube centered at the origin.");
  BindPrimitive<Sphere>(m, "Sphere", "Unit-diameter sphere.");
  BindPrimitive<Cylinder>(m, "Cylinder",
                          "Unit-diameter, unit-height cylinder along Z.");
  BindPrimitive<Cone>(m, "Cone", "Unit-diameter, unit-height cone along +Z.");
  BindPrimitive<Plane>(m, "Plane", "Unit square in the XY plane.");

  py::class_<Mesh, ObjectBase, std::shared_ptr<Mesh>>(m, "Mesh")
      .def(py::init([](const FloatArray& vertices, const IndexArray& indices,
                       const py::kwargs& kw) {
             auto mesh = std::make_shared<Mesh>(ParamsFromKwargs(kw));
             auto verts = ToVertexList(vertices);
             const bool has_uv = vertices.shape(1) == 5;
             mesh->SetMeshData(verts, ToIndexList(indices), has_uv);
             return mesh;
           }),
           "vertices"_a, "indices"_a,
           "Triangle mesh. `vertices` is (N, 3) xyz or (N, 5) xyzuv; "
           "`indices` is a flat or (M, 3) array of triangle indices.")
      .def(
          "set_mesh_data",
          [](Mesh& mesh, const FloatArray& vertices,
             const IndexArray& indices) {
            mesh.SetMeshData(ToVertexList(vertices), ToIndexList(indices),
                             vertices.shape(1) == 5);
          },
          "vertices"_a, "indices"_a);

  py::class_<Model, Container, std::shared_ptr<Model>>(m, "Model")
      .def(py::init([](const std::string& path, py::kwargs kw) {
             Model::LoadOptions options;
             if (kw.contains("force_reload")) {
               options.force_reload = kw["force_reload"].cast<bool>();
               kw.attr("pop")("force_reload");
             }
             return std::make_shared<Model>(path, ParamsFromKwargs(kw),
                                            options);
           }),
           "path"_a,
           "Load a model file (STL/OBJ/DAE/... via assimp, or an SDF). "
           "Pass force_reload=True to bypass the mesh cache.")
      .def(
          "set_from_file",
          Chain<Model>([](Model& model, const std::string& path,
                          bool force_reload) {
            model.SetFromFile(path, Model::LoadOptions{force_reload});
          }),
          "path"_a, "force_reload"_a = false);

  py::enum_<TextFacingMode>(m, "TextFacingMode")
      .value("Billboard", TextFacingMode::Billboard)
      .value("Fixed", TextFacingMode::Fixed);
  py::enum_<TextDepthMode>(m, "TextDepthMode")
      .value("DepthTest", TextDepthMode::DepthTest)
      .value("AlwaysVisible", TextDepthMode::AlwaysVisible);
  py::enum_<TextAlign>(m, "TextAlign")
      .value("Left", TextAlign::Left)
      .value("Center", TextAlign::Center)
      .value("Right", TextAlign::Right);

  py::class_<Text, ObjectBase, std::shared_ptr<Text>>(m, "Text")
      .def(py::init([](const py::kwargs& kw) {
             return std::make_shared<Text>(TextParamsFromKwargs(kw));
           }),
           "3D text. Extra keywords: text, height, font, facing_mode, "
           "depth_mode, align.")
      .def("set_text",
           Chain<Text>([](Text& t, const std::string& s) { t.SetText(s); }),
           "text"_a)
      .def("set_height",
           Chain<Text>([](Text& t, float h) { t.SetHeight(h); }), "height"_a)
      .def("set_font",
           Chain<Text>([](Text& t, const std::string& f) { t.SetFont(f); }),
           "font_path"_a)
      .def("set_facing_mode",
           Chain<Text>([](Text& t, TextFacingMode v) { t.SetFacingMode(v); }),
           "mode"_a)
      .def("set_depth_mode",
           Chain<Text>([](Text& t, TextDepthMode v) { t.SetDepthMode(v); }),
           "mode"_a)
      .def("set_align",
           Chain<Text>([](Text& t, TextAlign v) { t.SetAlign(v); }),
           "align"_a);

  py::class_<Drone, Container, std::shared_ptr<Drone>>(m, "Drone")
      .def(py::init([](const py::kwargs& kw) {
             return std::make_shared<Drone>(ParamsFromKwargs(kw));
           }),
           "Simple quadrotor model built from primitives.");
}

void BindMarkers(py::module_& m) {
  py::class_<Grid, ObjectBase, std::shared_ptr<Grid>>(m, "Grid")
      .def(py::init([](const py::kwargs& kw) {
             return std::make_shared<Grid>(ParamsFromKwargs(kw));
           }),
           "Grid lines in the XY plane; `scale` sets the extent.")
      .def("set_resolution",
           Chain<Grid>([](Grid& g, double r) { g.SetResolution(r); }),
           "resolution"_a);

  py::class_<Arrow::ArrowParams>(m, "ArrowParams")
      .def(py::init([](const Eigen::Vector3d& from, const Eigen::Vector3d& to,
                       double head_length, double head_radius,
                       double body_radius) {
             Arrow::ArrowParams p;
             p.from_ = from;
             p.to_ = to;
             p.head_length_ = head_length;
             p.head_radius_ = head_radius;
             p.body_radius_ = body_radius;
             return p;
           }),
           "from_pos"_a = Eigen::Vector3d::Zero(),
           "to_pos"_a = Eigen::Vector3d::Zero(), "head_length"_a = 0.3,
           "head_radius"_a = 0.06, "body_radius"_a = 0.03)
      .def_readwrite("from_pos", &Arrow::ArrowParams::from_)
      .def_readwrite("to_pos", &Arrow::ArrowParams::to_)
      .def_readwrite("head_length", &Arrow::ArrowParams::head_length_)
      .def_readwrite("head_radius", &Arrow::ArrowParams::head_radius_)
      .def_readwrite("body_radius", &Arrow::ArrowParams::body_radius_);

  py::class_<Arrow, ObjectBase, std::shared_ptr<Arrow>>(m, "Arrow")
      .def(py::init([](const py::kwargs& kw) {
             return std::make_shared<Arrow>(ParamsFromKwargs(kw));
           }),
           "Arrow from one point to another.")
      .def("set_from_to",
           Chain<Arrow>([](Arrow& a, const Eigen::Vector3d& from,
                           const Eigen::Vector3d& to) { a.SetFromTo(from, to); }),
           "from_pos"_a, "to_pos"_a)
      .def("set_arrow_params",
           Chain<Arrow>([](Arrow& a, const Arrow::ArrowParams& p) {
             a.SetArrowParams(p);
           }),
           "params"_a)
      .def("set_head_length",
           Chain<Arrow>([](Arrow& a, double v) { a.SetHeadLength(v); }),
           "length"_a)
      .def("set_head_radius",
           Chain<Arrow>([](Arrow& a, double v) { a.SetHeadRadius(v); }),
           "radius"_a)
      .def("set_body_radius",
           Chain<Arrow>([](Arrow& a, double v) { a.SetBodyRadius(v); }),
           "radius"_a);

  py::class_<Path, ObjectBase, std::shared_ptr<Path>>(m, "Path")
      .def(py::init([](const py::kwargs& kw) {
             return std::make_shared<Path>(ParamsFromKwargs(kw));
           }),
           "Polyline drawn as cylinders, optionally with spheres at "
           "vertices.")
      .def("set_path",
           Chain<Path>([](Path& p, const DoubleArray& pts) {
             p.SetPath(ToVector3List(pts, "path"));
           }),
           "points"_a, "Set the polyline from an (N, 3) array.")
      .def("get_path",
           [](Path& p) { return FromVector3List(p.GetPath()); })
      .def("set_path_width",
           Chain<Path>([](Path& p, double w) { p.SetPathWidth(w); }),
           "width"_a)
      .def("set_sphere_visible",
           Chain<Path>([](Path& p, bool v) { p.SetSphereVisible(v); }),
           "visible"_a)
      .def("set_sphere_size",
           Chain<Path>([](Path& p, double s) { p.SetSphereSize(s); }),
           "size"_a);

  using BoxCloud = PointCloud<Box>;
  py::class_<BoxCloud, ObjectBase, std::shared_ptr<BoxCloud>>(m, "PointCloud")
      .def(py::init([](const py::kwargs& kw) {
             return std::make_shared<BoxCloud>(ParamsFromKwargs(kw));
           }),
           "Instanced point cloud; each point is drawn as a small cube.")
      .def(
          "set_points",
          Chain<BoxCloud>([](BoxCloud& pc, const DoubleArray& arr) {
            if (arr.ndim() != 2 || (arr.shape(1) != 3 && arr.shape(1) != 4)) {
              throw py::value_error(
                  "points must be an (N, 3) array of xyz or an (N, 4) array "
                  "of xyz + size");
            }
            auto r = arr.unchecked<2>();
            if (r.shape(1) == 3) {
              pc.SetPoints(ToVector3List(arr, "points"));
            } else {
              std::vector<Eigen::Vector4d> pts;
              pts.reserve(static_cast<size_t>(r.shape(0)));
              for (py::ssize_t i = 0; i < r.shape(0); ++i) {
                pts.emplace_back(r(i, 0), r(i, 1), r(i, 2), r(i, 3));
              }
              pc.SetPoints(pts);
            }
          }),
          "points"_a,
          "Set points from an (N, 3) array (uniform size) or an (N, 4) array "
          "with per-point size in the last column.")
      .def("set_size",
           Chain<BoxCloud>([](BoxCloud& pc, double s) { pc.SetSize(s); }),
           "size"_a, "Uniform point size used by (N, 3) set_points.")
      .def("get_points",
           [](BoxCloud& pc) { return FromVector4List(pc.GetPoints()); });

  py::class_<Odometry, Container, std::shared_ptr<Odometry>>(m, "Odometry")
      .def(py::init([](const py::kwargs& kw) {
             return std::make_shared<Odometry>(ParamsFromKwargs(kw));
           }),
           "XYZ axis triad (red, green, blue arrows).")
      .def("set_arrow_params",
           Chain<Odometry>([](Odometry& o, const Arrow::ArrowParams& p) {
             o.SetArrowParams(p);
           }),
           "params"_a)
      .def("set_head_length",
           Chain<Odometry>([](Odometry& o, double v) { o.SetHeadLength(v); }),
           "length"_a)
      .def("set_head_radius",
           Chain<Odometry>([](Odometry& o, double v) { o.SetHeadRadius(v); }),
           "radius"_a)
      .def("set_body_radius",
           Chain<Odometry>([](Odometry& o, double v) { o.SetBodyRadius(v); }),
           "radius"_a);

  py::class_<DegeneracyIndicator, ObjectBase,
             std::shared_ptr<DegeneracyIndicator>>(m, "DegeneracyIndicator")
      .def(py::init([](const py::kwargs& kw) {
             return std::make_shared<DegeneracyIndicator>(ParamsFromKwargs(kw));
           }))
      .def("set_degeneracy_info",
           Chain<DegeneracyIndicator>([](DegeneracyIndicator& d,
                                         const DoubleArray& trans,
                                         const DoubleArray& rot) {
             d.SetDegeneracyInfo(ToVector3List(trans, "degen_trans"),
                                 ToVector3List(rot, "degen_rot"));
           }),
           "degen_trans"_a, "degen_rot"_a,
           "Degenerate translation / rotation directions, each (N, 3).");
}

template <class T>
std::shared_ptr<T> MakeOrbitCamera(const py::object& target,
                                   const py::object& yaw,
                                   const py::object& pitch) {
  auto cam = std::make_shared<T>();
  if (!target.is_none()) {
    const auto t = target.cast<Eigen::Vector3d>();
    cam->SetTarget(static_cast<float>(t.x()), static_cast<float>(t.y()),
                   static_cast<float>(t.z()));
  }
  if (!yaw.is_none() || !pitch.is_none()) {
    const float y = yaw.is_none() ? cam->GetYaw() : yaw.template cast<float>();
    const float p =
        pitch.is_none() ? cam->GetPitch() : pitch.template cast<float>();
    cam->SetYawPitch(y, p);
  }
  return cam;
}

// Viewer takes ownership through unique_ptr, which pybind11 cannot transfer
// from a Python-held object, so a fresh controller of the same kind and pose
// is made.
std::unique_ptr<CameraBase> MakeCameraLike(const CameraBase& camera) {
  const auto* orbit = dynamic_cast<const MouseOrbitCamera*>(&camera);
  if (orbit == nullptr) {
    throw py::type_error("unsupported camera controller type");
  }
  std::unique_ptr<MouseOrbitCamera> fresh;
  if (dynamic_cast<const KeyboardOrbitCamera*>(&camera) != nullptr) {
    fresh = std::make_unique<KeyboardOrbitCamera>();
  } else {
    fresh = std::make_unique<MouseOrbitCamera>();
  }
  const auto t = orbit->GetTarget();
  fresh->SetTarget(t[0], t[1], t[2]);
  fresh->SetYawPitch(orbit->GetYaw(), orbit->GetPitch());
  return fresh;
}

void BindCamera(py::module_& m) {
  py::class_<CameraBase, std::shared_ptr<CameraBase>>(m, "CameraBase");
  py::class_<MouseOrbitCamera, CameraBase, std::shared_ptr<MouseOrbitCamera>>
      mouse(m, "MouseOrbitCamera");
  mouse
      .def(py::init([](const py::object& target, const py::object& yaw,
                       const py::object& pitch) {
             return MakeOrbitCamera<MouseOrbitCamera>(target, yaw, pitch);
           }),
           "target"_a = py::none(), "yaw"_a = py::none(),
           "pitch"_a = py::none(),
           "Mouse-driven fly camera (default). Optional initial pose: "
           "target [x, y, z], yaw and pitch in radians.")
      .def("set_target",
           Chain<MouseOrbitCamera>([](MouseOrbitCamera& c,
                                      const Eigen::Vector3d& t) {
             c.SetTarget(static_cast<float>(t.x()), static_cast<float>(t.y()),
                         static_cast<float>(t.z()));
           }),
           "target"_a)
      .def("set_yaw_pitch",
           Chain<MouseOrbitCamera>([](MouseOrbitCamera& c, float yaw,
                                      float pitch) { c.SetYawPitch(yaw, pitch); }),
           "yaw"_a, "pitch"_a)
      .def("get_target",
           [](const MouseOrbitCamera& c) {
             const auto t = c.GetTarget();
             return Eigen::Vector3d(t[0], t[1], t[2]);
           })
      .def("get_yaw", &MouseOrbitCamera::GetYaw)
      .def("get_pitch", &MouseOrbitCamera::GetPitch);
  py::class_<KeyboardOrbitCamera, MouseOrbitCamera,
             std::shared_ptr<KeyboardOrbitCamera>>(m, "KeyboardOrbitCamera")
      .def(py::init([](const py::object& target, const py::object& yaw,
                       const py::object& pitch) {
             return MakeOrbitCamera<KeyboardOrbitCamera>(target, yaw, pitch);
           }),
           "target"_a = py::none(), "yaw"_a = py::none(),
           "pitch"_a = py::none(),
           "Fly camera with additional WASD-style keys. Same pose keywords "
           "as MouseOrbitCamera.");
}

void BindViewer(py::module_& m) {
  py::class_<Viewer, std::unique_ptr<Viewer>>(m, "Viewer")
      .def(py::init([](bool headless, bool vsync, int width, int height,
                       const Color& background, LogLevel log_level,
                       bool capture_ui) {
             ViewerConfig cfg;
             cfg.headless = headless;
             cfg.vsync = vsync;
             cfg.width = width;
             cfg.height = height;
             cfg.background = background;
             cfg.log_level = log_level;
             cfg.capture_ui = capture_ui;
             return std::make_unique<Viewer>(cfg);
           }),
           "headless"_a = false, "vsync"_a = true, "width"_a = 1280,
           "height"_a = 720, "background"_a = color::light_gray,
           "log_level"_a = LogLevel::Info, "capture_ui"_a = true,
           "Create the window and renderer. Only one Viewer may exist at a "
           "time.")
      .def("spin_once", &Viewer::SpinOnce,
           py::call_guard<py::gil_scoped_release>(),
           "Process events and render one frame. Returns False once the "
           "window is closed.")
      .def("close", &Viewer::Close, "Request the window to close.")
      .def("save_screenshot", &Viewer::SaveScreenshot, "path"_a = "",
           "include_ui"_a = true,
           "Request a PNG of the next rendered frame and return its path. "
           "The file is written during the following spin_once(), so call "
           "spin_once() once more before reading it. Pass include_ui=False "
           "for a render without the ImGui overlay.")
      .def("start_recording", &Viewer::StartRecording, "path"_a = "",
           "fps"_a = 30,
           "Start recording frames to a video file and return its path. "
           "Requires ffmpeg on PATH. A '.gif' extension writes a GIF, "
           "anything else H.264. Output is paced on the wall clock so the "
           "video plays at real speed.")
      .def("stop_recording", &Viewer::StopRecording,
           "Stop recording and finish writing the file.")
      .def("is_recording", &Viewer::IsRecording)
      .def("add_object", &Viewer::AddObject, "object"_a,
           "Add a top-level object to the scene.")
      .def("register_ui_callback", &Viewer::RegisterUICallback, "callback"_a,
           "Register a function called every frame inside the ImGui frame; "
           "use livision.imgui.* inside it.")
      .def(
          "set_camera_controller",
          [](Viewer& v, const CameraBase& camera) {
            v.SetCameraController(MakeCameraLike(camera));
          },
          "camera"_a,
          "Replace the camera controller, e.g. with KeyboardOrbitCamera().")
      .def("__enter__", [](py::object self) { return self; })
      .def("__exit__", [](Viewer& v, const py::object&, const py::object&,
                          const py::object&) { v.Close(); });
}

// Minimal ImGui subset for UI callbacks. Widgets that modify a value return
// (changed, value) so Python callers can keep their own state.
void BindImGui(py::module_& m) {
  py::module_ im = m.def_submodule(
      "imgui", "Small ImGui subset usable inside register_ui_callback.");
  im.def(
      "begin",
      [](const std::string& name) { return ImGui::Begin(name.c_str()); },
      "name"_a, "Begin a window; always pair with end().");
  im.def("end", []() { ImGui::End(); });
  im.def(
      "text", [](const std::string& s) { ImGui::TextUnformatted(s.c_str()); },
      "text"_a);
  im.def(
      "button",
      [](const std::string& label) { return ImGui::Button(label.c_str()); },
      "label"_a, "Returns True on the frame the button is clicked.");
  im.def(
      "checkbox",
      [](const std::string& label, bool value) {
        const bool changed = ImGui::Checkbox(label.c_str(), &value);
        return py::make_tuple(changed, value);
      },
      "label"_a, "value"_a, "Returns (changed, value).");
  im.def(
      "slider_float",
      [](const std::string& label, float value, float v_min, float v_max) {
        const bool changed =
            ImGui::SliderFloat(label.c_str(), &value, v_min, v_max);
        return py::make_tuple(changed, value);
      },
      "label"_a, "value"_a, "min"_a, "max"_a, "Returns (changed, value).");
  im.def(
      "slider_int",
      [](const std::string& label, int value, int v_min, int v_max) {
        const bool changed =
            ImGui::SliderInt(label.c_str(), &value, v_min, v_max);
        return py::make_tuple(changed, value);
      },
      "label"_a, "value"_a, "min"_a, "max"_a, "Returns (changed, value).");
  im.def(
      "input_float",
      [](const std::string& label, float value) {
        const bool changed = ImGui::InputFloat(label.c_str(), &value);
        return py::make_tuple(changed, value);
      },
      "label"_a, "value"_a, "Returns (changed, value).");
  im.def(
      "input_text",
      [](const std::string& label, const std::string& value) {
        char buf[1024];
        std::snprintf(buf, sizeof(buf), "%s", value.c_str());
        const bool changed = ImGui::InputText(label.c_str(), buf, sizeof(buf));
        return py::make_tuple(changed, std::string(buf));
      },
      "label"_a, "value"_a, "Returns (changed, value).");
  im.def("same_line", []() { ImGui::SameLine(); });
  im.def("separator", []() { ImGui::Separator(); });
  im.def(
      "is_key_down",
      [](int key) { return ImGui::IsKeyDown(static_cast<ImGuiKey>(key)); },
      "key"_a, "Key is an ImGuiKey value, e.g. livision.imgui.Key.Q.");
  im.def("want_capture_keyboard",
         []() { return ImGui::GetIO().WantCaptureKeyboard; });
  im.def("want_capture_mouse",
         []() { return ImGui::GetIO().WantCaptureMouse; });

  py::enum_<ImGuiKey>(im, "Key")
      .value("Space", ImGuiKey_Space)
      .value("Enter", ImGuiKey_Enter)
      .value("Escape", ImGuiKey_Escape)
      .value("LeftArrow", ImGuiKey_LeftArrow)
      .value("RightArrow", ImGuiKey_RightArrow)
      .value("UpArrow", ImGuiKey_UpArrow)
      .value("DownArrow", ImGuiKey_DownArrow)
      .value("A", ImGuiKey_A)
      .value("D", ImGuiKey_D)
      .value("E", ImGuiKey_E)
      .value("Q", ImGuiKey_Q)
      .value("R", ImGuiKey_R)
      .value("S", ImGuiKey_S)
      .value("W", ImGuiKey_W);
}

}  // namespace

PYBIND11_MODULE(_livision, m) {
  m.doc() = "LiVision: lightweight 3D visualizer for rapid prototyping.";
  m.attr("__version__") = LIVISION_VERSION;

  BindLog(m);
  BindColor(m);
  BindObjectBase(m);
  BindContainer(m);
  BindObjects(m);
  BindMarkers(m);
  BindCamera(m);
  BindViewer(m);
  BindImGui(m);
}
