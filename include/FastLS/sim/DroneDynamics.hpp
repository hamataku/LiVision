#pragma once
#include <Eigen/Dense>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "FastLS/object/ObjectBase.hpp"

namespace fastls {

class DroneDynamics {
 public:
  // PIDゲイン構造体（先頭に移動）
  struct PIDGains {
    double p = 1.0;
    double i = 0.0;
    double d = 0.0;
  };

  explicit DroneDynamics(ObjectBase* object, float dt)
      : object_(object), dt_(dt) {
    SetPositionPIDGains(PIDGains{2.0, 0.0, 1.0});  // 位置制御ゲイン
    SetVelocityPIDGains(PIDGains{2.0, 0.0, 0.0});  // 速度制御ゲイン
    SetYawPIDGains(PIDGains{1.0, 0.0, 0.0});       // ヨー角制御ゲイン
  }

  void Init(const glm::vec3& initial_pos, float initial_yaw) {
    pos_ = ToEigen(initial_pos);
    yaw_ = initial_yaw;
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
  void SetPositionPIDGains(const PIDGains& gains) { pos_gains_ = gains; }
  void SetVelocityPIDGains(const PIDGains& gains) { vel_gains_ = gains; }
  void SetYawPIDGains(const PIDGains& gains) { yaw_gains_ = gains; }

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
    yaw_ += yaw_rate * dt_;

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
    double pitch = std::atan2(-rotation(2, 0),
                              std::sqrt((rotation(2, 1) * rotation(2, 1)) +
                                        (rotation(2, 2) * rotation(2, 2))));
    double roll = std::atan2(rotation(2, 1), rotation(2, 2));

    // 4. 実際の加速度計算（推力による加速度から重力の影響を引く）
    acc_ = thrust_ * thrust_dir +
           kGravityVec_;  // ここでは+を使用（推力と重力の合力）
    acc_ = ApplyAccelerationLimit(acc_);

    // 5. 状態更新
    vel_ += acc_ * dt_;
    vel_ = ApplyVelocityLimit(vel_);
    pos_ += vel_ * dt_;

    // 6. オブジェクト更新
    object_->SetPos(ToGlm(pos_));
    object_->SetRadRotation(glm::vec3(static_cast<float>(roll),
                                      static_cast<float>(pitch),
                                      static_cast<float>(yaw_)));
  }

  Eigen::Vector3d UpdatePositionPID() {
    Eigen::Vector3d pos_error = target_pos_ - pos_;
    pos_integral_ += pos_error * dt_;
    Eigen::Vector3d pos_derivative = (pos_error - prev_pos_error_) / dt_;
    prev_pos_error_ = pos_error;

    return (pos_gains_.p * pos_error) + (pos_gains_.i * pos_integral_) +
           (pos_gains_.d * pos_derivative);
  }

  Eigen::Vector3d UpdateVelocityPID(const Eigen::Vector3d& desired_vel) {
    Eigen::Vector3d vel_error = desired_vel - vel_;
    vel_integral_ += vel_error * dt_;
    Eigen::Vector3d vel_derivative = (vel_error - prev_vel_error_) / dt_;
    prev_vel_error_ = vel_error;

    return (vel_gains_.p * vel_error) + (vel_gains_.i * vel_integral_) +
           (vel_gains_.d * vel_derivative);
  }

  double UpdateYawPID() {
    double yaw_error = target_yaw_ - yaw_;
    // 角度の正規化（-π からπ の範囲に）
    yaw_error = std::atan2(std::sin(yaw_error), std::cos(yaw_error));
    yaw_integral_ += yaw_error * dt_;
    double yaw_derivative = (yaw_error - prev_yaw_error_) / dt_;
    prev_yaw_error_ = yaw_error;

    return (yaw_gains_.p * yaw_error) + (yaw_gains_.i * yaw_integral_) +
           (yaw_gains_.d * yaw_derivative);
  }

  void ResetPIDIntegrals() {
    pos_integral_ = Eigen::Vector3d::Zero();
    vel_integral_ = Eigen::Vector3d::Zero();
    yaw_integral_ = 0.0;
    prev_pos_error_ = Eigen::Vector3d::Zero();
    prev_vel_error_ = Eigen::Vector3d::Zero();
    prev_yaw_error_ = 0.0;
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
  double dt_;

  // 定数
  static constexpr double kGravity = 9.81;
  const Eigen::Vector3d kGravityVec_ =
      Eigen::Vector3d(0, 0, -kGravity);  // 下向きに変更
  const Eigen::Vector3d kZAxis_ = Eigen::Vector3d(0, 0, 1);

  // 制御モード
  ControlMode control_mode_ = ControlMode::kPosition;

  // 制御パラメータ
  PIDGains pos_gains_;
  PIDGains vel_gains_;
  PIDGains yaw_gains_;

  // 制約値
  double max_vel_ = 2.0;
  double max_acc_ = 1.0;
  double max_yaw_rate_ = 0.5;

  // 状態変数
  Eigen::Vector3d pos_ = Eigen::Vector3d::Zero();
  Eigen::Vector3d vel_ = Eigen::Vector3d::Zero();
  Eigen::Vector3d acc_ = Eigen::Vector3d::Zero();
  double yaw_ = 0.0;
  double thrust_ = kGravity;

  // 目標値
  Eigen::Vector3d target_pos_ = Eigen::Vector3d::Zero();
  Eigen::Vector3d target_vel_ = Eigen::Vector3d::Zero();
  double target_yaw_ = 0.0;

  // PID制御用の状態
  Eigen::Vector3d pos_integral_ = Eigen::Vector3d::Zero();
  Eigen::Vector3d vel_integral_ = Eigen::Vector3d::Zero();
  double yaw_integral_ = 0.0;
  Eigen::Vector3d prev_pos_error_ = Eigen::Vector3d::Zero();
  Eigen::Vector3d prev_vel_error_ = Eigen::Vector3d::Zero();
  double prev_yaw_error_ = 0.0;
};

}  // namespace fastls
