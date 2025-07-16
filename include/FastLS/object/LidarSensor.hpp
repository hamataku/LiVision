#pragma once

#include "FastLS/object/ObjectBase.hpp"

namespace fastls {

class LidarSensor : public ObjectBase {
 public:
  std::vector<glm::vec3>& GetPointClouds() { return points_; }
  glm::mat4& GetLastLidarMtx() { return last_lidar_mtx_; }
  float GetLidarRange() const { return lidar_range_; }
  void SetLidarRange(float range) { lidar_range_ = range; }

 private:
  std::vector<glm::vec3> points_;
  glm::mat4 last_lidar_mtx_ = glm::mat4(1.0F);
  float lidar_range_ = 50.0F;
};
}  // namespace fastls
