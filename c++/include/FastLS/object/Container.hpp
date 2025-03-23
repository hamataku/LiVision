#pragma once

#include "../SimLidar.hpp"
#include "ObjectBase.hpp"

namespace fastls {

class Container : public ObjectBase {
 public:
  Container& AddObject(ObjectBase* object) {
    object->Init();
    objects_.push_back(object);
    return *this;
  }

  void Draw(bgfx::ProgramHandle& program) final {
    CalcMtx();  // コンテナの変換行列を更新

    for (auto& object : objects_) {
      if (!object->IsVisible()) continue;

      // オブジェクトの元の変換行列を取得
      glm::mat4 orig_mtx = object->GetLocalMatrix();

      // コンテナの変換を適用した新しい行列を設定
      object->SetWorldMatrix(mtx_ * orig_mtx, true);

      object->Draw(program);

      // 元の行列に戻す
      object->SetWorldMatrix(orig_mtx);
    }
  }

 private:
  std::vector<ObjectBase*> objects_;
};

}  // namespace fastls