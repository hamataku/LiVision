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

## Pose Convention

`MouseOrbitCamera::SetTarget(x, y, z)` places the camera (it flies rather than
orbits, so the target is effectively the eye position), and
`SetYawPitch(yaw, pitch)` sets where it looks, in radians. The view direction is
`(cos(pitch) cos(yaw), cos(pitch) sin(yaw), sin(pitch))` in the world frame
(z up): yaw is measured from +x toward +y and positive pitch looks up.

```cpp
// Stand 10 m behind a point on +x and look at it.
camera->SetTarget(-10.0F, 0.0F, 2.0F)->SetYawPitch(0.0F, 0.0F);
```

## Implement Your Own Camera

Implement `CameraBase`:

- `HandleEvent(const SDL_Event&)`
- `Update(const CameraInputContext&)`

Then pass your implementation via `SetCameraController(...)`.
