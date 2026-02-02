#pragma once

#include <array>
#include <cassert>
#include <glm/gtx/quaternion.hpp>

#include "livision/ObjectBase.hpp"
#include "livision/obstacle/Cylinder.hpp"
#include "livision/obstacle/Sphere.hpp"
#include "livision/utils.hpp"

namespace livision {

class DegeneracyIndicator : public ObjectBase {
 public:
  DegeneracyIndicator() {
    for (auto& t_cyl : trans_cyl_) {
      t_cyl.SetColorSpec(livision::utils::yellow.Alpha(0.8))
          .SetSize(glm::dvec3(0.1, 0.1, 2));
    }
    rot_cyl_.SetColorSpec(livision::utils::blue.Alpha(0.8))
        .SetSize(glm::dvec3(1.5, 1.5, 0.01));
    rot_sphere_.SetColorSpec(livision::utils::blue.Alpha(0.8))
        .SetSize(glm::dvec3(1, 1, 1));
  }

  void Draw(bgfx::ProgramHandle& program) final {
    // draw trans degeneracy
    assert(degen_trans_.size() < 3 &&
           "DegeneracyIndicator: degen_trans_ exceeds 3");
    for (size_t i = 0; i < degen_trans_.size(); ++i) {
      glm::dvec3 z = glm::dvec3(0.0, 0.0, 1.0);
      glm::dvec3 v = glm::normalize(degen_trans_[i]);
      glm::dquat quat = glm::rotation(z, v);
      trans_cyl_[i].SetPos(pos_).SetQuatRotation(quat);
      trans_cyl_[i].UpdateMatrix();
      trans_cyl_[i].Draw(program);
    }

    // draw rot degeneracy
    assert(degen_rot_.size() < 3 &&
           "DegeneracyIndicator: degen_rot_ exceeds 3");
    if (degen_rot_.size() == 1) {
      glm::dvec3 z = glm::dvec3(0.0, 0.0, 1.0);
      glm::dvec3 v = glm::normalize(degen_rot_[0]);
      glm::dquat quat = glm::rotation(z, v);
      rot_cyl_.SetPos(pos_).SetQuatRotation(quat);
      rot_cyl_.UpdateMatrix();
      rot_cyl_.Draw(program);
    } else if (degen_rot_.size() >= 2) {
      rot_sphere_.SetPos(pos_);
      rot_sphere_.UpdateMatrix();
      rot_sphere_.Draw(program);
    }
  }

  DegeneracyIndicator& SetDegeneracyInfo(
      const std::vector<glm::dvec3>& degen_rot,
      const std::vector<glm::dvec3>& degen_trans, const glm::dvec3& pos) {
    degen_rot_ = degen_rot;
    degen_trans_ = degen_trans;
    pos_ = pos;
    return *this;
  }

 private:
  std::vector<glm::dvec3> degen_rot_;
  std::vector<glm::dvec3> degen_trans_;
  glm::dvec3 pos_;

  std::array<Cylinder, 3> trans_cyl_;
  Cylinder rot_cyl_;
  Sphere rot_sphere_;
};
}  // namespace livision
