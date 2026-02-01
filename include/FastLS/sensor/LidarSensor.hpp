#pragma once

#include <vector>

#include "LiVision/ObjectBase.hpp"

namespace livision {

class LidarSensor : public ObjectBase {
 public:
  std::vector<glm::vec3>& GetPointClouds() { return points_; }
  float GetLidarRange() const { return lidar_range_; }
  void SetLidarRange(float range) { lidar_range_ = range; }

 private:
  std::vector<glm::vec3> points_;
  float lidar_range_ = 50.0F;
};
}  // namespace livision
