#pragma once

#include "../SimLidar.hpp"
#include "ObjectBase.hpp"
namespace fastls {

class Container : public ObjectBase {
 public:
  Container& AddObject(ObjectBase* object) {
    objects_.push_back(object);
    object->RegisterParentObject(this);
    return *this;
  }

  std::vector<ObjectBase*>& GetObjects() { return objects_; }

 private:
  std::vector<ObjectBase*> objects_;
};

}  // namespace fastls