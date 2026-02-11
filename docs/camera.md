# Camera Controller Classes

`Viewer` supports replacing the camera controller at runtime.

## Classes

- `CameraBase`: abstract interface
- `MouseOrbitCamera`: default controller
- `KeyboardOrbitCamera`: orbit controller with keyboard support

## Set a Camera Controller

```cpp
viewer->SetCameraController(std::make_unique<livision::KeyboardOrbitCamera>());
```

## Implement Your Own Camera

Implement `CameraBase`:

- `HandleEvent(const SDL_Event&)`
- `Update(const CameraInputContext&)`

Then pass your implementation via `SetCameraController(...)`.
