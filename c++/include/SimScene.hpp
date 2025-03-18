#pragma once

#include <array>

#include "FastGL/RayCast.hpp"
#include "FastGL/SceneBase.hpp"
#include "FastGL/object/Box.hpp"
#include "FastGL/object/Plane.hpp"
#include "FastGL/object/PointCloud.hpp"

class SimScene : public fastgl::SceneBase {
 public:
  void Init() override {
    std::array<float, 4> color{0.3F, 0.3F, 0.3F, 0.1F};

    box0_.SetSize(glm::vec3(0.1F, 10.0F, 5.0F));
    box0_.SetPos(glm::vec3(5.0F, 0.0F, 2.5F));
    box0_.SetColor(color);
    AddObject(&box0_);

    box1_.SetSize(glm::vec3(0.1F, 10.0F, 5.0F));
    box1_.SetPos(glm::vec3(-5.0F, 0.0F, 2.5F));
    box1_.SetColor(color);
    AddObject(&box1_);

    box2_.SetSize(glm::vec3(10.0F, 0.1F, 5.0F));
    box2_.SetPos(glm::vec3(0.0F, 5.0F, 2.5F));
    box2_.SetColor(color);
    AddObject(&box2_);

    box3_.SetSize(glm::vec3(10.0F, 0.1F, 5.0F));
    box3_.SetPos(glm::vec3(0.0F, -5.0F, 2.5F));
    box3_.SetColor(color);
    AddObject(&box3_);

    plane0_.SetSize(glm::vec2(10.0F, 10.0F));
    AddObject(&plane0_);

    AddObject(&point_cloud0_);

    for (int i = 0; i < 360; i += 3) {
      for (int j = 0; j < 60; j += 2) {
        float rad_yaw = glm::radians(static_cast<float>(i));
        float rad_pitch = glm::radians(static_cast<float>(j));
        direction_.emplace_back(std::cos(rad_yaw), std::sin(rad_yaw),
                                std::sin(rad_pitch));
      }
    }
  }
  void Update() override {
    std::vector<glm::vec3> points;
    const glm::vec3 origin{0.0F, 4.0F, 0.3F};

    fastgl::ray_cast.GetPointCloud(points, origin, direction_);
    point_cloud0_.SetPoints(points);
  }

 private:
  fastgl::Box box0_;
  fastgl::Box box1_;
  fastgl::Box box2_;
  fastgl::Box box3_;
  fastgl::Plane plane0_;
  fastgl::PointCloud point_cloud0_;
  std::vector<glm::vec3> direction_;
};