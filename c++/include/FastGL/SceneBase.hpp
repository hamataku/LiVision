#pragma once

#include "object/ObjectBase.hpp"
#include <vector>

namespace fastgl {

class SceneBase {
 public:
  void AddObject(ObjectBase* object) {
    object->Init();
    objects_.push_back(object);
  }

  void Draw(bgfx::ProgramHandle& program) {
    for (auto& object : objects_) {
      object->Draw(program);
    }
  }

  virtual void Update() {};
  virtual void Init() {};

 private:
  std::vector<ObjectBase*> objects_;
};

}  // namespace fastgl