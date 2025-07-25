#pragma once

#include <SDL3/SDL_events.h>

#include "object/Container.hpp"
#include "object/ObjectBase.hpp"

namespace fastls {

class Container;
class SceneBase {
 public:
  void AddObject(ObjectBase* object) {
    object->Init();
    auto* container = dynamic_cast<Container*>(object);
    if (container) {
      for (auto* obj : container->GetObjects()) {
        obj->Init();
        AddObject(obj);
      }
    } else {
      objects_.push_back(object);
    }
  }

  void AddMeshList() {
    for (auto* object : objects_) {
      object->UpdateMatrix();
    }
    for (auto& object : objects_) {
      if (object->IsLidarVisible()) object->AddMeshList();
    }
  }

  void UpdateMatrix() {
    for (auto* object : objects_) {
      object->UpdateMatrix();
    }
  }

  void Draw(bgfx::ProgramHandle& program, bool visible_only) {
    for (auto* object : objects_) {
      if (visible_only) {
        if (object->IsVisible() && object->IsForceVisible())
          object->Draw(program);
      } else {
        if (object->IsVisible()) object->Draw(program);
      }
    }
  }

  void SetHeadless(bool headless) { headless_ = headless; }

  virtual void Update() {};
  virtual void Init() {};
  virtual bool CameraControl(float* view) {
    (void)view;
    return false;
  };
  virtual void EventHandler(SDL_Event event) {};
  virtual void GuiCustomize() {};

  void ExitRequest() { is_exit_requested_ = true; }
  bool IsExitRequested() const { return is_exit_requested_; }

 protected:
  bool headless_ = false;

 private:
  std::vector<ObjectBase*> objects_;
  bool is_exit_requested_ = false;
};

}  // namespace fastls