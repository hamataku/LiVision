#pragma once

#include <memory>
#include <vector>

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

 private:
  std::vector<std::shared_ptr<ObjectBase>> objects_;
};

}  // namespace livision
