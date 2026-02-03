#pragma once

#include "livision/ObjectBase.hpp"

namespace livision {
class Container : public ObjectBase {
 public:
  using ObjectBase::ObjectBase;

  Container& AddObject(ObjectBase* object) {
    objects_.push_back(object);
    object->RegisterParentObject(this);
    return *this;
  }
  std::vector<ObjectBase*>& GetObjects() { return objects_; }

 private:
  std::vector<ObjectBase*> objects_;
};

}  // namespace livision