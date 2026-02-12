#pragma once

#include <memory>
#include <string>
#include <vector>

#include "livision/Log.hpp"
#include "livision/ObjectBase.hpp"

namespace livision {
/**
 * @brief Object that holds and manages child objects.
 */
class Container : public ObjectBase {
 public:
  using ObjectBase::ObjectBase;

  /**
   * @brief Called during initialization.
   */
  void OnInit() final;
  /**
   * @brief Called during deinitialization.
   */
  void OnDeInit() final;
  /**
   * @brief Called during draw submission.
   */
  void OnDraw(Renderer& renderer) final;
  /**
   * @brief Update cached transform matrices.
   */
  void UpdateMatrix(const Eigen::Affine3d& parent_mtx) final;

  /**
   * @brief Add and co-own a child object via shared_ptr.
   */
  Container* AddObject(std::shared_ptr<ObjectBase> object);
  /**
   * @brief Get the list of child objects.
   */
  std::vector<std::shared_ptr<ObjectBase>>& GetObjects();
  /**
   * @brief Get the list of child objects.
   */
  const std::vector<std::shared_ptr<ObjectBase>>& GetObjects() const;
  /**
   * @brief Clear the list of child objects.
   */
  void ClearObjects();
  /**
   * @brief Get the first direct child that matches the name.
   * @return Matching child, or nullptr when not found.
   */
  std::shared_ptr<ObjectBase> GetChild(const std::string& name) const;
  /**
   * @brief Get all direct children that match the name.
   */
  std::vector<std::shared_ptr<ObjectBase>> GetChildren(
      const std::string& name) const;
  /**
   * @brief Get the first direct child as Container when possible.
   * @return Matching child as Container, or nullptr when not found / not a
   * Container.
   */
  std::shared_ptr<Container> GetChildContainer(const std::string& name) const;
  /**
   * @brief Resolve a '/' separated relative path from this container.
   * @return Resolved object, or nullptr when not found.
   */
  std::shared_ptr<ObjectBase> GetByPath(const std::string& path) const;
  /**
   * @brief Resolve a '/' separated relative path as Container.
   * @return Resolved Container, or nullptr when not found / not a Container.
   */
  std::shared_ptr<Container> GetContainerByPath(const std::string& path) const;
  /**
   * @brief Dump this container hierarchy as a text tree.
   */
  std::string DumpTree() const;
  /**
   * @brief Print this container hierarchy to log output.
   */
  void PrintTree(LogLevel level = LogLevel::Info) const;

 private:
  std::vector<std::shared_ptr<ObjectBase>> objects_;
};

}  // namespace livision
