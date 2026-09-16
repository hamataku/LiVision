# カメラ操作クラス

`Viewer` はカメラコントローラを差し替え可能です。

## クラス

- `CameraBase`: 抽象インターフェース
- `MouseOrbitCamera`: デフォルト実装
- `KeyboardOrbitCamera`: キーボード対応のオービット実装

## カメラ設定

```cpp
viewer->SetCameraController(std::make_unique<livision::KeyboardOrbitCamera>());
```

## 姿勢の約束

`MouseOrbitCamera::SetTarget(x, y, z)` はカメラを置く位置です (オービットではなく
飛行するカメラなので、target が実質的に視点位置になります)。
`SetYawPitch(yaw, pitch)` は向きをラジアンで指定します。視線方向はワールド座標 (z 上向き) で
`(cos(pitch) cos(yaw), cos(pitch) sin(yaw), sin(pitch))` です。yaw は +x から +y へ測り、
pitch は正で上向きです。

```cpp
// +x 上の点の 10 m 手前に立ってその点を見る
camera->SetTarget(-10.0F, 0.0F, 2.0F)->SetYawPitch(0.0F, 0.0F);
```

## 独自実装

`CameraBase` を継承し、以下を実装します:

- `HandleEvent(const SDL_Event&)`
- `Update(const CameraInputContext&)`

その後 `SetCameraController(...)` で設定します。
