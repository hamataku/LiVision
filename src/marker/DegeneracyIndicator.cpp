#include "livision/marker/DegeneracyIndicator.hpp"

namespace livision {

void DegeneracyIndicator::OnInit() {
  for (auto& t_cyl : trans_cyl_) {
    t_cyl.SetColor(color::yellow.Alpha(0.8))
        ->SetScale(Eigen::Vector3d(0.1, 0.1, 2));
  }
  rot_cyl_.SetColor(color::blue.Alpha(0.8))
      ->SetScale(Eigen::Vector3d(1.5, 1.5, 0.01));
  rot_sphere_.SetColor(color::blue.Alpha(0.8))
      ->SetScale(Eigen::Vector3d(1, 1, 1));
}

void DegeneracyIndicator::OnDraw(Renderer& renderer) {
  // draw trans degeneracy
  assert(degen_trans_.size() < 3 &&
         "DegeneracyIndicator: degen_trans_ exceeds 3");
  for (size_t i = 0; i < degen_trans_.size(); ++i) {
    Eigen::Vector3d z = Eigen::Vector3d(0.0, 0.0, 1.0);
    Eigen::Vector3d v = degen_trans_[i].normalized();
    Eigen::Quaterniond quat =
        Eigen::Quaterniond::FromTwoVectors(z, v).normalized();
    trans_cyl_[i].SetPos(params_.pos)->SetQuatRotation(quat);
    trans_cyl_[i].UpdateMatrix(global_mtx_);
    trans_cyl_[i].OnDraw(renderer);
  }

  // draw rot degeneracy
  assert(degen_rot_.size() < 3 && "DegeneracyIndicator: degen_rot_ exceeds 3");
  if (degen_rot_.size() == 1) {
    Eigen::Vector3d z = Eigen::Vector3d(0.0, 0.0, 1.0);
    Eigen::Vector3d v = degen_rot_[0].normalized();
    Eigen::Quaterniond quat =
        Eigen::Quaterniond::FromTwoVectors(z, v).normalized();
    rot_cyl_.SetPos(params_.pos)->SetQuatRotation(quat);
    rot_cyl_.UpdateMatrix(global_mtx_);
    rot_cyl_.OnDraw(renderer);
  } else if (degen_rot_.size() >= 2) {
    rot_sphere_.SetPos(params_.pos);
    rot_sphere_.UpdateMatrix(global_mtx_);
    rot_sphere_.OnDraw(renderer);
  }
}

DegeneracyIndicator& DegeneracyIndicator::SetDegeneracyInfo(
    const std::vector<Eigen::Vector3d>& degen_trans,
    const std::vector<Eigen::Vector3d>& degen_rot) {
  degen_trans_ = degen_trans;
  degen_rot_ = degen_rot;
  return *this;
}

}  // namespace livision
