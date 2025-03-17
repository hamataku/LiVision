#pragma once

#include "FastGL/SceneBase.hpp"
#include "FastGL/object/Box.hpp"

class SimScene : public fastgl::SceneBase {
 public:
  void Init() override {
    box0_.SetSize(glm::vec3(0.5F, 1.0F, 1.0F));
    AddObject(&box0_);
  }
  void Update() override {}

 private:
  fastgl::Box box0_;
};