#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "object/Utils.hpp"

namespace fastgl {

class RayCast {
 public:
  utils::PosVertex CalcMul(const utils::PosVertex* _vec, const float* _mat) {
    utils::PosVertex result;
    result.x =
        _vec->x * _mat[0] + _vec->y * _mat[4] + _vec->z * _mat[8] + _mat[12];
    result.y =
        _vec->x * _mat[1] + _vec->y * _mat[5] + _vec->z * _mat[9] + _mat[13];
    result.z =
        _vec->x * _mat[2] + _vec->y * _mat[6] + _vec->z * _mat[10] + _mat[14];
    return result;
  }

  void AddMeshLists(const utils::PosVertex* vertices, float* mtx,
                    const uint16_t* indices, uint32_t num_vertices,
                    uint32_t num_indices) {
    for (uint32_t i = 0; i < num_vertices; ++i) {
      utils::PosVertex result = CalcMul(&vertices[i], mtx);
      mesh_vertices_.push_back(result);
    }
    for (uint32_t i = 0; i < num_indices; ++i) {
      mesh_indices_.push_back(mesh_index_ + indices[i]);
    }
    mesh_index_ += num_vertices;
  }

  void GetPointCloud(std::vector<glm::vec3>& points, const glm::vec3& origin,
                     const std::vector<glm::vec3>& direction) {
    for (auto ray_dir : direction) {
      ray_dir = glm::normalize(ray_dir);  // レイの方向を正規化
      bool is_hit = false;
      float min_distance = std::numeric_limits<float>::max();
      glm::vec3 min_point;

      for (size_t j = 0; j < mesh_indices_.size(); j += 3) {
        // 三角形の頂点を取得

        glm::vec3 v0 = glm::vec3(mesh_vertices_[mesh_indices_[j]].x,
                                 mesh_vertices_[mesh_indices_[j]].y,
                                 mesh_vertices_[mesh_indices_[j]].z);
        glm::vec3 v1 = glm::vec3(mesh_vertices_[mesh_indices_[j + 1]].x,
                                 mesh_vertices_[mesh_indices_[j + 1]].y,
                                 mesh_vertices_[mesh_indices_[j + 1]].z);
        glm::vec3 v2 = glm::vec3(mesh_vertices_[mesh_indices_[j + 2]].x,
                                 mesh_vertices_[mesh_indices_[j + 2]].y,
                                 mesh_vertices_[mesh_indices_[j + 2]].z);

        // Möller-Trumbore法による交差判定
        glm::vec3 e1 = v1 - v0;
        glm::vec3 e2 = v2 - v0;
        glm::vec3 h = glm::cross(ray_dir, e2);
        float a = glm::dot(e1, h);

        if (std::abs(a) < std::numeric_limits<float>::epsilon()) {
          continue;  // レイが三角形と平行
        }

        float f = 1.0F / a;
        glm::vec3 s = origin - v0;
        float u = f * glm::dot(s, h);
        if (u < 0.0F || u > 1.0F) {
          continue;
        }

        glm::vec3 q = glm::cross(s, e1);
        float v = f * glm::dot(ray_dir, q);
        if (v < 0.0F || u + v > 1.0F) {
          continue;
        }

        float t = f * glm::dot(e2, q);
        if (t > std::numeric_limits<float>::epsilon()) {  // t > 0 なら交差
          if (t < min_distance) {
            min_distance = t;
            min_point = origin + ray_dir * t;
            is_hit = true;
          }
        }
      }
      if (is_hit) {
        points.push_back(min_point);
      }
    }
  }

 private:
  std::vector<utils::PosVertex> mesh_vertices_;
  std::vector<uint16_t> mesh_indices_;
  uint16_t mesh_index_ = 0;
};

inline RayCast ray_cast;
}  // namespace fastgl