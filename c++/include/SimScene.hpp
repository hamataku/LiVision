#pragma once

#include "FastGL/SceneBase.hpp"
#include "FastGL/object/Box.hpp"
#include "FastGL/object/Plane.hpp"

class SimScene : public fastgl::SceneBase {
 public:
  void Init() override {
    box0_.SetSize(glm::vec3(0.5F, 1.0F, 1.0F));
    AddObject(&box0_);

    plane0_.SetSize(glm::vec2(10.0F, 10.0F));
    AddObject(&plane0_);
  }
  void Update() override {}

 private:
  fastgl::Box box0_;
  fastgl::Plane plane0_;
};