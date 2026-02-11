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

## 独自実装

`CameraBase` を継承し、以下を実装します:

- `HandleEvent(const SDL_Event&)`
- `Update(const CameraInputContext&)`

その後 `SetCameraController(...)` で設定します。
