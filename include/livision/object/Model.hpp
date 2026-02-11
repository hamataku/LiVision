#pragma once

#include <memory>
#include <utility>
#include <vector>

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

  static Model::Ptr InstanceWithFile(const std::string& path,
                                     Params params = Params()) {
    return std::make_shared<Model>(path, std::move(params));
  }

  /**
   * @brief Construct from a file path (SDF/mesh formats).
   */
  explicit Model(const std::string& path, Params params = Params())
      : Container(std::move(params)) {
    SetFromFile(path);
  }

  /**
   * @brief Load model from a file. Supports SDF and mesh formats
   * (STL/DAE/OBJ, etc.).
   */
 Model* SetFromFile(const std::string& path);

 private:
  void AddOwned(std::shared_ptr<ObjectBase> child);
  void BuildFromNode(const internal::sdf_loader::SdfNode& node);
};

}  // namespace livision
