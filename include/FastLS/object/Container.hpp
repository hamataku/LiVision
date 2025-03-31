#pragma once

#include "FastLS/object/ObjectBase.hpp"
#include "FastLS/sim/LidarSim.hpp"
namespace fastls {

class Container : public ObjectBase {
 public:
  Container& AddObject(ObjectBase* object) {
    object->SetForceVisible(IsForceVisible());
    objects_.push_back(object);
    object->RegisterParentObject(this);
    return *this;
  }

  std::vector<ObjectBase*>& GetObjects() { return objects_; }

 private:
  std::vector<ObjectBase*> objects_;
};

}  // namespace fastls