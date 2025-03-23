#pragma once

#include "../SimLidar.hpp"
#include "ObjectBase.hpp"

namespace fastls {

class Lidar : public ObjectBase {
 public:
  void Init() final { sim_lidar.RegisterLidar(this); }
};
}  // namespace fastls
