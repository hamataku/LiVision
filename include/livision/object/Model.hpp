#pragma once

#include <memory>
#include <utility>

#include "livision/Container.hpp"

namespace livision {
namespace internal::sdf_loader {
struct SdfNode;
}

/**
 * @brief Renderable model loaded from files (SDF/mesh formats).
 * @ingroup object
 */
class Model : public Container, public SharedInstanceFactory<Model> {
 public:
  using Container::Container;

  struct LoadOptions {
    bool force_reload = false;
  };

  static Model::Ptr InstanceWithPath(const std::string& path,
                                     Params params = Params()) {
    return std::make_shared<Model>(path, std::move(params));
  }

  static Model::Ptr InstanceWithPath(const std::string& path, Params params,
                                     LoadOptions options) {
    return std::make_shared<Model>(path, std::move(params), options);
  }

  /**
   * @brief Construct from a file path (SDF/mesh formats).
   */
  explicit Model(const std::string& path, Params params = Params())
      : Container(std::move(params)) {
    SetFromFile(path, LoadOptions{});
  }

  Model(const std::string& path, Params params, LoadOptions options)
      : Container(std::move(params)) {
    SetFromFile(path, options);
  }

  /**
   * @brief Load model from a file. Supports SDF and mesh formats
   * (STL/DAE/OBJ, etc.).
   */
  Model* SetFromFile(const std::string& path) {
    return SetFromFile(path, LoadOptions{});
  }
  Model* SetFromFile(const std::string& path,
                     LoadOptions options);

 private:
  void AddOwned(std::shared_ptr<ObjectBase> child);
  void BuildFromNode(const internal::sdf_loader::SdfNode& node,
                     bool apply_self_transform = true);
};

}  // namespace livision
