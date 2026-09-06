# Python

LiVision ships Python bindings (`livision` package, built with pybind11).
The Python API mirrors the C++ one: the same objects, markers, containers,
colors, and a small ImGui subset for UI callbacks.

## Installation

LiVision is published on PyPI as a source distribution, so `pip` compiles it
on install. That takes a few minutes the first time and needs the same build
dependencies as the C++ library.

```bash
sudo apt install -y build-essential cmake pkg-config \
  libsdl2-dev libeigen3-dev libsdformat14-dev libassimp-dev libcurl4-openssl-dev \
  python3-dev

pip install livision
```

To build from a checkout instead:

```bash
git clone https://github.com/hamataku/LiVision.git --recursive
cd LiVision
pip install .
```

The wheel bundles `liblivision` and the compiled shaders, so nothing else
needs to be installed system-wide. The runtime shared libraries
(`libsdl2`, `libsdformat14`, `libassimp`) must be present.

For development against an existing CMake build, enable the bindings and
point `PYTHONPATH` at the build tree instead:

```bash
cmake .. -DLIVISION_BUILD_PYTHON=ON
make -j
export PYTHONPATH=$PWD/python
```

## Sample Program

```python
import math
import livision as lv

viewer = lv.Viewer(width=1280, height=720)

state = {"theta": 0.0}

def ui():
    _, state["theta"] = lv.imgui.slider_float("theta", state["theta"], 0.0, 2.0 * math.pi)
    if lv.imgui.button("Close"):
        viewer.close()

viewer.register_ui_callback(ui)

viewer.add_object(lv.Grid(scale=[15.0, 15.0, 0.0]))

sphere = lv.Sphere(scale=2.0, color=lv.color.rainbow_z, wire_color=lv.color.black)
viewer.add_object(sphere)

while viewer.spin_once():
    sphere.set_rad_rotation([0.0, 0.0, state["theta"]])
```

## Mapping from C++

| C++ | Python |
| --- | --- |
| `Box::Instance({.pos = {0, 0, 1}, .scale = {2, 2, 2}, .color = color::red})` | `lv.Box(pos=[0, 0, 1], scale=2.0, color=lv.color.red)` |
| `obj->SetPos(x, y, z)->SetColor(c)` | `obj.set_pos(x, y, z).set_color(c)` |
| `obj->SetDegRotation({90, 0, 0})` | `obj.set_deg_rotation([90, 0, 0])` |
| `obj->SetQuatRotation(q)` | `obj.set_quat_rotation([x, y, z, w])` |
| `Model::InstanceWithPath(path, params)` | `lv.Model(path, **params)` |
| `Text::Instance({.text = "hi", .height = 0.5})` | `lv.Text(text="hi", height=0.5)` |
| `pc->SetPoints(std::vector<Eigen::Vector3d>)` | `pc.set_points(np.ndarray (N, 3))` |
| `path->SetPath(...)` | `path.set_path(np.ndarray (N, 3))` |
| `viewer->RegisterUICallback(fn)` | `viewer.register_ui_callback(fn)` |
| `viewer->SetCameraController(std::make_unique<KeyboardOrbitCamera>())` | `viewer.set_camera_controller(lv.KeyboardOrbitCamera())` |

Constructor keywords accepted by every object: `pos`, `scale` (vector or
scalar), `quat` (`[x, y, z, w]`), `deg_rotation`, `rad_rotation`, `color`,
`wire_color`, `texture`, `name`. Colors also accept plain tuples such as
`(1.0, 0.0, 0.0)` or `(1.0, 0.0, 0.0, 0.5)`.

Setters return the object itself so calls chain the same way as in C++.

## Objects and markers

`Box`, `Sphere`, `Cylinder`, `Cone`, `Plane`, `Mesh`, `Model`, `Text`,
`Drone`, `Container`, `Grid`, `Arrow`, `Path`, `PointCloud`, `Odometry`,
`DegeneracyIndicator`. See [Object List](objects.md) for what each draws.

`Mesh` takes numpy arrays: `vertices` as `(N, 3)` xyz or `(N, 5)` xyzuv
(`float32`), and `indices` as a flat or `(M, 3)` integer array.

## ImGui subset

Inside the UI callback, `lv.imgui` provides `begin`/`end`, `text`, `button`,
`checkbox`, `slider_float`, `slider_int`, `input_float`, `input_text`,
`same_line`, `separator`, `is_key_down`, `want_capture_keyboard`, and
`want_capture_mouse`. Widgets that edit a value return `(changed, value)`.

## Camera

`MouseOrbitCamera` and `KeyboardOrbitCamera` accept an initial pose:

```python
viewer.set_camera_controller(lv.KeyboardOrbitCamera(target=[0, -9, 5], yaw=-1.6, pitch=0.4))
```

`target` is the camera position, and `yaw`/`pitch` are in radians.

## Screenshots and recording

```python
# PNG of the next rendered frame; include_ui=False leaves out the overlay.
viewer.save_screenshot("scene.png", include_ui=False)

# Record until stop_recording(). ".gif" writes a GIF, anything else H.264.
viewer.start_recording("scene.mp4", fps=30)
...
viewer.stop_recording()
```

The file is written during the following `spin_once()`, so call it once more
before reading the image. An empty path produces a timestamped name in the
current directory. Recording needs `ffmpeg` on `PATH`.

`Viewer(headless=True)` renders the scene without a window, which makes it a
convenient way to turn a script into an image:

```python
viewer = lv.Viewer(headless=True, width=1280, height=720)
viewer.add_object(lv.Box(scale=2.0, color=lv.color.red))
viewer.spin_once()
viewer.save_screenshot("out.png")
viewer.spin_once()
```

See `examples/python/capture.py` and `examples/python/headless_render.py`.
