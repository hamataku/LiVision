# コンテナ

`Container` は子オブジェクトを `std::shared_ptr` で保持する階層オブジェクトです。

## 基本API

- `Container::AddObject(std::shared_ptr<ObjectBase>)`
- `Container::GetObjects()`
- `Container::ClearObjects()`

## 挙動

- 追加した子には親変換が自動で適用されます。
- 子オブジェクトの生存期間はコンテナが共同所有します。
- `ClearObjects()` で子の保持を解除します。

## Containerベースの代表クラス

- `Model`
- `Drone`
- `Odometry`
