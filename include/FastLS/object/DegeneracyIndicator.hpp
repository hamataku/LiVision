#pragma once

#include <glm/gtx/quaternion.hpp>

#include "FastLS/object/Cylinder.hpp"
#include "FastLS/object/ObjectBase.hpp"
#include "FastLS/utils.hpp"

namespace fastls {

class DegeneracyIndicator : public ObjectBase {
 public:
  DegeneracyIndicator() { color_ = utils::yellow; }
  virtual ~DegeneracyIndicator() = default;

  void Draw(bgfx::ProgramHandle& program) final {
    cyl_.SetColor(color_);
    if (dim_ == 1) {
      // 1 dim degen
      glm::dvec3 z = glm::dvec3(0.0, 0.0, 1.0);
      glm::dvec3 v = glm::normalize(dir_);
      glm::dquat quat = glm::rotation(z, v);
      cyl_.SetVisible(true)
          .SetSize(glm::dvec3(0.2, 0.2, 3))
          .SetPos(pos_)
          .SetQuatRotation(quat);
      cyl_.UpdateMatrix();
      cyl_.Draw(program);
    } else if (dim_ == 2) {
      // 2 dim degen
      glm::dvec3 z = glm::dvec3(0.0, 0.0, 1.0);
      glm::dvec3 v = glm::normalize(dir_);
      glm::dquat quat = glm::rotation(z, v);
      cyl_.SetVisible(true)
          .SetSize(glm::dvec3(2.0, 2.0, 0.01))
          .SetPos(pos_)
          .SetQuatRotation(quat);
      cyl_.UpdateMatrix();
      cyl_.Draw(program);
    } else if (dim_ == 3) {
      // 3 dim degen
      cyl_.SetVisible(true)
          .SetSize(glm::dvec3(1.0, 1.0, 1.0))
          .SetPos(pos_)
          .SetRadRotation(glm::dvec3(0, 0, 0));
      cyl_.UpdateMatrix();
      cyl_.Draw(program);
    }
  }

  DegeneracyIndicator& SetDegeneracyInfo(int dim, const glm::dvec3& dir,
                                         const glm::dvec3& pos) {
    dim_ = dim;
    dir_ = dir;
    pos_ = pos;
    return *this;
  }

 private:
  int dim_;
  glm::dvec3 dir_;
  glm::dvec3 pos_;
  Cylinder cyl_;
};
}  // namespace fastls
