#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <memory>
#include <utility>

#include "livision/Color.hpp"
#include "livision/MeshData.hpp"
#include "livision/Renderer.hpp"

namespace livision {

class ObjectBase {
 public:
  struct Params {
    Eigen::Vector3d pos = Eigen::Vector3d::Zero();
    Eigen::Vector3d scale = Eigen::Vector3d::Ones();
    Eigen::Quaterniond quat = Eigen::Quaterniond::Identity();
    Color color = color::white;
  };

  ObjectBase() = default;
  explicit ObjectBase(Params params) : params_(std::move(params)) {}

  ~ObjectBase() { DeInit(); }
  void Init();
  void Draw(Renderer& renderer);
  void DeInit();

  virtual void OnInit() {}
  virtual void OnDraw(Renderer& renderer);

  ObjectBase& SetParams(const Params& params) {
    params_ = params;
    return *this;
  }

  ObjectBase& SetPos(const Eigen::Vector3d& pos);
  ObjectBase& SetPos(double x, double y, double z);
  ObjectBase& SetScale(const Eigen::Vector3d& scale);
  ObjectBase& SetScale(double x, double y, double z);
  ObjectBase& SetQuatRotation(const Eigen::Quaterniond& q);
  ObjectBase& SetDegRotation(const Eigen::Vector3d& euler_deg);
  ObjectBase& SetVisible(bool visible);
  ObjectBase& SetColor(const Color& color);

  bool IsVisible() const;
  Eigen::Vector3d GetGlobalPos();

  void UpdateMatrix();
  void RegisterParentObject(ObjectBase* obj);

 protected:
  Eigen::Affine3d global_mtx_ = Eigen::Affine3d::Identity();
  Eigen::Affine3d local_mtx_ = Eigen::Affine3d::Identity();

  Params params_;

  bool local_mtx_changed_ = true;
  bool is_initialized_ = false;
  bool visible_ = true;
  ObjectBase* parent_object_ = nullptr;

  std::shared_ptr<MeshData> mesh_;
};
}  // namespace livision