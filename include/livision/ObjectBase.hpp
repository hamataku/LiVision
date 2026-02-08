#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <memory>
#include <utility>

#include "livision/Color.hpp"
#include "livision/MeshBuffer.hpp"
#include "livision/Renderer.hpp"

namespace livision {

class ObjectBase {
 public:
  struct Params {
    Eigen::Vector3d pos = Eigen::Vector3d::Zero();
    Eigen::Vector3d scale = Eigen::Vector3d::Ones();
    Eigen::Quaterniond quat = Eigen::Quaterniond::Identity();
    Color color = color::white;
    Color wire_color = color::invisible;
  };

  ObjectBase() = default;
  explicit ObjectBase(Params params) : params_(std::move(params)) {}

  ~ObjectBase() { DeInit(); }
  void Init();
  void DeInit();

  virtual void OnInit() {}
  virtual void OnDraw(Renderer& renderer);

  ObjectBase& SetParams(const Params& params);

  ObjectBase& SetPos(const Eigen::Vector3d& pos);
  ObjectBase& SetPos(double x, double y, double z);
  ObjectBase& SetScale(const Eigen::Vector3d& scale);
  ObjectBase& SetScale(double x, double y, double z);
  ObjectBase& SetQuatRotation(const Eigen::Quaterniond& q);
  ObjectBase& SetDegRotation(const Eigen::Vector3d& euler_deg);
  ObjectBase& SetRadRotation(const Eigen::Vector3d& euler_rad);
  ObjectBase& SetVisible(bool visible);
  ObjectBase& SetColor(const Color& color);
  ObjectBase& SetWireColor(const Color& color);

  ObjectBase& SetGlobalMatrix(const Eigen::Affine3d& mtx);

  bool IsVisible() const;
  Eigen::Vector3d GetGlobalPos();
  Eigen::Affine3d GetGlobalMatrix() const;

  void UpdateMatrix();
  void RegisterParentObject(ObjectBase* obj);

  std::shared_ptr<MeshBuffer>& GetMeshBuffer() { return mesh_buf_; }

 protected:
  Eigen::Affine3d global_mtx_ = Eigen::Affine3d::Identity();
  Eigen::Affine3d local_mtx_ = Eigen::Affine3d::Identity();

  Params params_;

  bool local_mtx_changed_ = true;
  bool is_initialized_ = false;
  bool visible_ = true;
  ObjectBase* parent_object_ = nullptr;

  std::shared_ptr<MeshBuffer> mesh_buf_;
};
}  // namespace livision