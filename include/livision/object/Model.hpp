#pragma once

#include <memory>
#include <vector>

#include "livision/Container.hpp"
#include "livision/object/Mesh.hpp"

namespace livision {
namespace internal::sdf_loader {
struct SdfNode;
}

/**
 * @brief Renderable model loaded from files (SDF/mesh formats).
 * @ingroup object
 */
class Model : public Container {
 public:
  /**
   * @brief Construct with optional parameters.
   */
  explicit Model(Params params = Params()) : Container(std::move(params)) {}

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
  void SetFromFile(const std::string& path);

 private:
  void ClearChildren();
  void AddOwned(std::unique_ptr<ObjectBase> child);
  void BuildFromNode(const internal::sdf_loader::SdfNode& node);

  std::vector<std::unique_ptr<ObjectBase>> owned_children_;
};

}  // namespace livision
