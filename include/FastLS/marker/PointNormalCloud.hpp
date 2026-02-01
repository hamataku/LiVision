#pragma once

#include <glm/gtx/quaternion.hpp>
#include <vector>

#include "LiVision/ObjectBase.hpp"
#include "LiVision/obstacle/Box.hpp"

namespace livision {

template <class T = Box>
class PointNormalCloud : public ObjectBase {
 public:
  void Draw(bgfx::ProgramHandle& program) final {
    obj_.SetColorSpec(color_spec_);
    for (size_t i = 0; i < points_.size(); ++i) {
      const auto& point = points_[i];
      // draw point
      obj_.SetPos(glm::dvec3(point.x, point.y, point.z));
      obj_.SetSize(glm::dvec3(voxel_size_, voxel_size_, voxel_size_));
      obj_.UpdateMatrix();
      obj_.Draw(program);

      // draw normal
      if (i < normals_.size()) {
        const auto& normal = normals_[i];
        glm::dvec3 box_center = glm::dvec3(point.x, point.y, point.z) +
                                glm::dvec3(normal) * (normal_length_ / 2.0);
        glm::dvec3 z = glm::dvec3(0.0, 0.0, 1.0);
        glm::dvec3 v = glm::normalize(glm::dvec3(normal));
        glm::dquat quat = glm::rotation(z, v);
        normal_obj_.SetColorSpec(utils::red)
            .SetSize(glm::dvec3(0.01, 0.01, normal_length_))
            .SetPos(box_center)
            .SetQuatRotation(quat);
        normal_obj_.UpdateMatrix();
        normal_obj_.Draw(program);
      }
    }
  }
  PointNormalCloud& SetVoxelSize(float size) {
    voxel_size_ = size;
    return *this;
  }
  PointNormalCloud& SetPoints(const std::vector<glm::vec3>& points,
                              const std::vector<glm::vec3>& normals) {
    points_ = points;
    normals_ = normals;
    return *this;
  }

 private:
  std::vector<glm::vec3> points_;   // x,y,z
  std::vector<glm::vec3> normals_;  // nx,ny,nz
  double voxel_size_ = 0.12;        // Default voxel size
  double normal_length_ = 0.4;      // Default normal length
  T obj_;
  Box normal_obj_;
};
}  // namespace livision
