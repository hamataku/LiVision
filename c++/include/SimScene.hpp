#pragma once

#include "FastGL/SceneBase.hpp"
#include "FastGL/object/Box.hpp"

class SimScene : public fastgl::SceneBase {
 public:
  void Init() override { AddObject(&box0_); }
  void Update() override {}

 private:
  fastgl::Box box0_;
};