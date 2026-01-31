#pragma once

#include <cmath>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "FastLS/object/ObjectBase.hpp"
#include "FastLS/object/obstacle/Box.hpp"
#include "FastLS/utils.hpp"

namespace fastls {

template <class T = Box>
class PointCloud : public ObjectBase {
 public:
  PointCloud() { color_ = utils::blue; }

  enum class ColorMode { Uniform, HeightColor };

  PointCloud& SetColorMode(ColorMode mode) {
    color_mode_ = mode;
    return *this;
  }

  void Draw(bgfx::ProgramHandle& program) final {
    for (const auto& point : points_) {
      if (color_mode_ == ColorMode::Uniform) {
        obj_.SetColor(color_);
      } else {
        // Height-based color (10m cycle)
        double z = point.z;
        double h = std::fmod(std::abs(z), 10.0) / 10.0;  // 0〜1
        utils::Color rgb = HSVtoRGB(static_cast<float>(h), 1.0F, 1.0F);
        obj_.SetColor(rgb);
      }

      obj_.SetPos(glm::dvec3(point.x, point.y, point.z));
      obj_.SetSize(glm::dvec3(point.w, point.w, point.w));
      obj_.UpdateMatrix();
      obj_.ForceSetGlobalMatrix(global_mtx_ * obj_.GetGlobalMatrix());
      obj_.Draw(program);
    }
  }

  PointCloud& SetVoxelSize(float size) {
    voxel_size_ = size;
    return *this;
  }

  PointCloud& SetPoints(const std::vector<glm::vec3>& points) {
    points_.clear();
    for (const auto& p : points) {
      points_.emplace_back(p.x, p.y, p.z, voxel_size_);
    }
    return *this;
  }

  PointCloud& SetPoints(const std::vector<glm::vec4>& points) {
    points_ = points;
    return *this;
  }

  const std::vector<glm::vec4>& GetPoints() { return points_; }

 private:
  std::vector<glm::vec4> points_;
  double voxel_size_ = 0.12;
  T obj_;

  utils::Color HSVtoRGB(float h, float s, float v) {
    float c = v * s;
    float x = c * (1.0F - std::fabs(std::fmod(h * 6.0F, 2.0F) - 1.0F));
    float m = v - c;

    utils::Color color{0.0F, 0.0F, 0.0F, 1.0F};
    if (0.0F <= h && h < 1.0F / 6.0F) {
      color.r = c;
      color.g = x;
      color.b = 0.0F;
    } else if (1.0F / 6.0F <= h && h < 2.0F / 6.0F) {
      color.r = x;
      color.g = c;
      color.b = 0.0F;
    } else if (2.0F / 6.0F <= h && h < 3.0F / 6.0F) {
      color.r = 0.0F;
      color.g = c;
      color.b = x;
    } else if (3.0F / 6.0F <= h && h < 4.0F / 6.0F) {
      color.r = 0.0F;
      color.g = x;
      color.b = c;
    } else if (4.0F / 6.0F <= h && h < 5.0F / 6.0F) {
      color.r = x;
      color.g = 0.0F;
      color.b = c;
    } else {
      color.r = c;
      color.g = 0.0F;
      color.b = x;
    }
    color.r += m;
    color.g += m;
    color.b += m;

    return color;
  }

  ColorMode color_mode_ = ColorMode::Uniform;
};

}  // namespace fastls
