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
   * @brief Add and co-own a child object via shared_ptr.
   */
  Container* AddObject(std::shared_ptr<ObjectBase> object) {
    if (!object) {
      return this;
    }
    object->RegisterParentObject(this);
    objects_.push_back(std::move(object));
    return this;
  }
  /**
   * @brief Get the list of child objects.
   */
  std::vector<std::shared_ptr<ObjectBase>>& GetObjects() { return objects_; }
  /**
   * @brief Get the list of child objects.
   */
  const std::vector<std::shared_ptr<ObjectBase>>& GetObjects() const {
    return objects_;
  }
  /**
   * @brief Clear the list of child objects.
   */
  void ClearObjects() { objects_.clear(); }

 private:
  std::vector<std::shared_ptr<ObjectBase>> objects_;
};

}  // namespace livision
