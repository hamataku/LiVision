#pragma once

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
   * @brief Add a child object and register this as its parent.
   */
  Container& AddObject(ObjectBase* object) {
    objects_.push_back(object);
    object->RegisterParentObject(this);
    return *this;
  }
  /**
   * @brief Get the list of child objects.
   */
  std::vector<ObjectBase*>& GetObjects() { return objects_; }
  /**
   * @brief Clear the list of child objects.
   */
  void ClearObjects() { objects_.clear(); }

 private:
  std::vector<ObjectBase*> objects_;
};

}  // namespace livision
