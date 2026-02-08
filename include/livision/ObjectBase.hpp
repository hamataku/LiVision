#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <memory>
#include <utility>

#include "livision/Color.hpp"
#include "livision/MeshBuffer.hpp"
#include "livision/Renderer.hpp"

namespace livision {

/**
 * @brief Base class for renderable objects.
 */
class ObjectBase {
 public:
  /**
   * @brief Common transform and appearance parameters.
   */
  struct Params {
    Eigen::Vector3d pos = Eigen::Vector3d::Zero();
    Eigen::Vector3d scale = Eigen::Vector3d::Ones();
    Eigen::Quaterniond quat = Eigen::Quaterniond::Identity();
    Color color = color::white;
    Color wire_color = color::invisible;
  };

  /**
   * @brief Default construct.
   */
  ObjectBase() = default;
  /**
   * @brief Construct with initial parameters.
   */
  explicit ObjectBase(Params params) : params_(std::move(params)) {}

  /**
   * @brief Destroy object and release resources.
   */
  ~ObjectBase() { DeInit(); }
  /**
   * @brief Initialize graphics resources.
   */
  void Init();
  /**
   * @brief Deinitialize graphics resources.
   */
  void DeInit();

  /**
   * @brief Called during initialization.
   */
  virtual void OnInit() {}
  /**
   * @brief Called during draw submission.
   */
  virtual void OnDraw(Renderer& renderer);

  /**
   * @brief Set all parameters.
   */
  ObjectBase& SetParams(const Params& params);

  /**
   * @brief Set position.
   */
  ObjectBase& SetPos(const Eigen::Vector3d& pos);
  /**
   * @brief Set position from components.
   */
  ObjectBase& SetPos(double x, double y, double z);
  /**
   * @brief Set scale.
   */
  ObjectBase& SetScale(const Eigen::Vector3d& scale);
  /**
   * @brief Set scale from components.
   */
  ObjectBase& SetScale(double x, double y, double z);
  /**
   * @brief Set rotation from quaternion.
   */
  ObjectBase& SetQuatRotation(const Eigen::Quaterniond& q);
  /**
   * @brief Set rotation from Euler degrees.
   */
  ObjectBase& SetDegRotation(const Eigen::Vector3d& euler_deg);
  /**
   * @brief Set rotation from Euler radians.
   */
  ObjectBase& SetRadRotation(const Eigen::Vector3d& euler_rad);
  /**
   * @brief Set visibility flag.
   */
  ObjectBase& SetVisible(bool visible);
  /**
   * @brief Set base color.
   */
  ObjectBase& SetColor(const Color& color);
  /**
   * @brief Set wireframe color.
   */
  ObjectBase& SetWireColor(const Color& color);

  /**
   * @brief Override global transform matrix.
   */
  ObjectBase& SetGlobalMatrix(const Eigen::Affine3d& mtx);

  /**
   * @brief Get visibility flag.
   */
  bool IsVisible() const;
  /**
   * @brief Get global position.
   */
  Eigen::Vector3d GetGlobalPos();
  /**
   * @brief Get global transform matrix.
   */
  Eigen::Affine3d GetGlobalMatrix() const;

  /**
   * @brief Update cached transform matrices.
   */
  void UpdateMatrix();
  /**
   * @brief Attach to a parent object for hierarchical transforms.
   */
  void RegisterParentObject(ObjectBase* obj);

  /**
   * @brief Access the mesh buffer.
   */
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