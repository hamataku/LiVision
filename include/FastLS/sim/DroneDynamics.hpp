#pragma once
#include <Eigen/Dense>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "FastLS/Settings.hpp"
#include "FastLS/object/ObjectBase.hpp"
#include "FastLS/sim/PIDController.hpp"

namespace fastls {

class DroneDynamics {
 public:
  using PIDGains = PIDController::PIDGains;

  explicit DroneDynamics(ObjectBase* object) : object_(object) {
    pos_pid_.SetGains(PIDGains{2.0, 0.0, 1.0});  // 位置制御ゲイン
    vel_pid_.SetGains(PIDGains{2.0, 0.0, 0.0});  // 速度制御ゲイン
    yaw_pid_.SetGains(PIDGains{1.0, 0.0, 0.0});  // ヨー角制御ゲイン
  }

  void Init(const glm::vec3& initial_pos, float initial_yaw) {
    pos_ = ToEigen(initial_pos);
    yaw_ = initial_yaw;
    pitch_ = 0.0;  // 追加
    roll_ = 0.0;   // 追加
    vel_ = Eigen::Vector3d::Zero();
    acc_ = Eigen::Vector3d::Zero();
    target_pos_ = pos_;
    target_vel_ = Eigen::Vector3d::Zero();
    target_yaw_ = initial_yaw;
    ResetPIDIntegrals();
  }

  void Update() { UpdateDynamics(); }

  // 制御インターフェース
  void SetTargetPos(const glm::vec3& pos) {
    target_pos_ = ToEigen(pos);
    control_mode_ = ControlMode::kPosition;
  }

  void SetTargetVel(const glm::vec3& vel) {
    target_vel_ = ToEigen(vel);
    control_mode_ = ControlMode::kVelocity;
  }

  void SetTargetYaw(float yaw) { target_yaw_ = yaw; }

  // ゲイン設定
  void SetPositionPIDGains(const PIDGains& gains) { pos_pid_.SetGains(gains); }
  void SetVelocityPIDGains(const PIDGains& gains) { vel_pid_.SetGains(gains); }
  void SetYawPIDGains(const PIDGains& gains) { yaw_pid_.SetGains(gains); }

  // 制約設定
  void SetMaxVel(float max_vel) { max_vel_ = max_vel; }
  void SetMaxAcc(float max_acc) { max_acc_ = max_acc; }
  void SetMaxYawRate(float max_yaw_rate) { max_yaw_rate_ = max_yaw_rate; }

  // 状態取得
  glm::vec3 GetPos() const { return ToGlm(pos_); }
  glm::vec3 GetVel() const { return ToGlm(vel_); }
  float GetYaw() const { return yaw_; }

 private:
  // glm-Eigen変換ヘルパー関数
  static Eigen::Vector3d ToEigen(const glm::vec3& v) {
    return Eigen::Vector3d(v.x, v.y, v.z);
  }

  static glm::vec3 ToGlm(const Eigen::Vector3d& v) {
    return glm::vec3(static_cast<float>(v.x()), static_cast<float>(v.y()),
                     static_cast<float>(v.z()));
  }

  // 制御モード
  enum class ControlMode {
    kPosition,  // k prefix for enum values
    kVelocity
  };

  void UpdateDynamics() {
    // 1. 位置・速度制御（PID出力は重力補償を含まない）
    Eigen::Vector3d desired_acc;
    if (control_mode_ == ControlMode::kPosition) {
      Eigen::Vector3d desired_vel = UpdatePositionPID();
      desired_vel = ApplyVelocityLimit(desired_vel);
      desired_acc = UpdateVelocityPID(desired_vel);
    } else {
      desired_acc = UpdateVelocityPID(target_vel_);
    }
    desired_acc = ApplyAccelerationLimit(desired_acc);

    // 2. 推力計算（重力補償を加える）
    // PID出力に重力補償を加えて、機体が必要とする加速度を計算
    Eigen::Vector3d total_desired_acc = desired_acc - kGravityVec_;
    thrust_ = total_desired_acc.norm();

    // 3. 姿勢計算
    // ヨー角のPID制御
    double yaw_rate = UpdateYawPID();
    yaw_rate = std::clamp(yaw_rate, -max_yaw_rate_, max_yaw_rate_);
    yaw_ += yaw_rate * settings::common_dt;

    // 推力方向から姿勢を計算
    Eigen::Vector3d thrust_dir = total_desired_acc.normalized();
    Eigen::Vector3d y_axis =
        Eigen::AngleAxisd(yaw_, kZAxis_) * Eigen::Vector3d::UnitY();
    Eigen::Vector3d x_axis = y_axis.cross(thrust_dir).normalized();
    Eigen::Vector3d y_axis_corrected = thrust_dir.cross(x_axis);

    // 回転行列の構築
    Eigen::Matrix3d rotation;
    rotation.col(0) = x_axis;
    rotation.col(1) = y_axis_corrected;
    rotation.col(2) = thrust_dir;

    // オイラー角の抽出
    double target_pitch = std::atan2(
        -rotation(2, 0), std::sqrt((rotation(2, 1) * rotation(2, 1)) +
                                   (rotation(2, 2) * rotation(2, 2))));
    double target_roll = std::atan2(rotation(2, 1), rotation(2, 2));

    // 姿勢の滑らかな遷移
    static constexpr double kAttitudeSmoothing = 0.2;  // 値が小さいほど滑らか
    pitch_ += (target_pitch - pitch_) * kAttitudeSmoothing;
    roll_ += (target_roll - roll_) * kAttitudeSmoothing;

    // 4. 実際の加速度計算（推力による加速度から重力の影響を引く）
    acc_ = thrust_ * thrust_dir + kGravityVec_;

    // 空気抵抗の追加
    Eigen::Vector3d drag_force =
        -kDragCoefficient * vel_.cwiseProduct(vel_.cwiseAbs());
    acc_ += drag_force;  // 空気抵抗による加速度を追加

    acc_ = ApplyAccelerationLimit(acc_);

    // 5. 状態更新
    vel_ += acc_ * settings::common_dt;
    vel_ = ApplyVelocityLimit(vel_);
    pos_ += vel_ * settings::common_dt;

    // 6. オブジェクト更新
    object_->SetPos(ToGlm(pos_));
    object_->SetRadRotation(glm::vec3(static_cast<float>(roll_),
                                      static_cast<float>(pitch_),
                                      static_cast<float>(yaw_)));
  }

  Eigen::Vector3d UpdatePositionPID() {
    return pos_pid_.Update(target_pos_ - pos_);
  }

  Eigen::Vector3d UpdateVelocityPID(const Eigen::Vector3d& desired_vel) {
    return vel_pid_.Update(desired_vel - vel_);
  }

  double UpdateYawPID() {
    double yaw_error = target_yaw_ - yaw_;
    // 角度の正規化（-π からπ の範囲に）
    yaw_error = std::atan2(std::sin(yaw_error), std::cos(yaw_error));
    return yaw_pid_.Update(yaw_error);
  }

  void ResetPIDIntegrals() {
    pos_pid_.Reset();
    vel_pid_.Reset();
    yaw_pid_.Reset();
  }

  Eigen::Vector3d ApplyVelocityLimit(const Eigen::Vector3d& vel) const {
    const double vel_norm = vel.norm();
    if (vel_norm > max_vel_) {
      return vel * (max_vel_ / vel_norm);
    }
    return vel;
  }

  Eigen::Vector3d ApplyAccelerationLimit(const Eigen::Vector3d& acc) const {
    const double acc_norm = acc.norm();
    if (acc_norm > max_acc_) {
      return acc * (max_acc_ / acc_norm);
    }
    return acc;
  }

  // メンバ変数
  ObjectBase* object_ = nullptr;

  // 定数
  static constexpr double kGravity = 9.81;
  static constexpr double kDragCoefficient = 0.5;  // 空気抵抗係数
  const Eigen::Vector3d kGravityVec_ =
      Eigen::Vector3d(0, 0, -kGravity);  // 下向きに変更
  const Eigen::Vector3d kZAxis_ = Eigen::Vector3d(0, 0, 1);

  // 制御モード
  ControlMode control_mode_ = ControlMode::kPosition;

  // PIDコントローラー
  PIDController pos_pid_;
  PIDController vel_pid_;
  PIDController yaw_pid_;

  // 制約値
  double max_vel_ = 2.0;
  double max_acc_ = 1.0;
  double max_yaw_rate_ = 0.5;

  // 状態変数
  Eigen::Vector3d pos_ = Eigen::Vector3d::Zero();
  Eigen::Vector3d vel_ = Eigen::Vector3d::Zero();
  Eigen::Vector3d acc_ = Eigen::Vector3d::Zero();
  double yaw_ = 0.0;
  double pitch_ = 0.0;  // 追加
  double roll_ = 0.0;   // 追加
  double thrust_ = kGravity;

  // 目標値
  Eigen::Vector3d target_pos_ = Eigen::Vector3d::Zero();
  Eigen::Vector3d target_vel_ = Eigen::Vector3d::Zero();
  double target_yaw_ = 0.0;
};

}  // namespace fastls
