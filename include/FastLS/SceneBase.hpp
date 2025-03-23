#pragma once

#include <vector>

#include "object/ObjectBase.hpp"

namespace fastls {

class SceneBase {
 public:
  void AddObject(ObjectBase* object) {
    object->Init();
    objects_.push_back(object);
  }

  void AddMeshList() {
    for (auto& object : objects_) {
      if (object->IsLidarVisible()) object->AddMeshList();
    }
  }

  void Draw(bgfx::ProgramHandle& program) {
    for (auto& object : objects_) {
      if (object->IsVisible()) object->Draw(program);
    }
  }

  virtual void Update(){};
  virtual void Init(){};

 private:
  std::vector<ObjectBase*> objects_;
};

}  // namespace fastls