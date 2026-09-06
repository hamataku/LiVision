# Python

LiVision には Python バインディング（`livision` パッケージ、pybind11 製）が付属します。
Python API は C++ API と同じ構成で、オブジェクト、マーカー、コンテナ、色に加えて、
UI コールバック用の小さな ImGui サブセットを提供します。

## インストール

LiVision は PyPI にソース配布として公開しているため、`pip` がインストール時に
ビルドします。初回は数分かかり、C++ ライブラリと同じビルド依存が必要です。

```bash
sudo apt install -y build-essential cmake pkg-config \
  libsdl2-dev libeigen3-dev libsdformat14-dev libassimp-dev libcurl4-openssl-dev \
  python3-dev

pip install livision
```

チェックアウトからビルドする場合は次のとおりです。

```bash
git clone https://github.com/hamataku/LiVision.git --recursive
cd LiVision
pip install .
```

ホイールには `liblivision` とコンパイル済みシェーダが同梱されるため、
システムへの追加インストールは不要です。ランタイムの共有ライブラリ
（`libsdl2`、`libsdformat14`、`libassimp`）は必要です。

既存の CMake ビルドで開発する場合は、バインディングを有効にして
ビルドツリーを `PYTHONPATH` に指定します。

```bash
cmake .. -DLIVISION_BUILD_PYTHON=ON
make -j
export PYTHONPATH=$PWD/python
```

## サンプルプログラム

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

## C++ との対応

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

すべてのオブジェクトのコンストラクタが受け付けるキーワード: `pos`、`scale`
（ベクトルまたはスカラー）、`quat`（`[x, y, z, w]`）、`deg_rotation`、
`rad_rotation`、`color`、`wire_color`、`texture`、`name`。
色は `(1.0, 0.0, 0.0)` や `(1.0, 0.0, 0.0, 0.5)` のようなタプルでも指定できます。

セッターは自身を返すので、C++ と同様にメソッドチェーンできます。

## オブジェクトとマーカー

`Box`、`Sphere`、`Cylinder`、`Cone`、`Plane`、`Mesh`、`Model`、`Text`、
`Drone`、`Container`、`Grid`、`Arrow`、`Path`、`PointCloud`、`Odometry`、
`DegeneracyIndicator`。各オブジェクトの内容は [オブジェクト一覧](objects.md) を参照してください。

`Mesh` は numpy 配列を受け取ります。`vertices` は `(N, 3)` の xyz または
`(N, 5)` の xyzuv（`float32`）、`indices` はフラットまたは `(M, 3)` の整数配列です。

## ImGui サブセット

UI コールバック内で `lv.imgui` の `begin`/`end`、`text`、`button`、
`checkbox`、`slider_float`、`slider_int`、`input_float`、`input_text`、
`same_line`、`separator`、`is_key_down`、`want_capture_keyboard`、
`want_capture_mouse` が使えます。値を編集するウィジェットは `(changed, value)` を返します。

## カメラ

`MouseOrbitCamera` と `KeyboardOrbitCamera` は初期姿勢を受け取れます。

```python
viewer.set_camera_controller(lv.KeyboardOrbitCamera(target=[0, -9, 5], yaw=-1.6, pitch=0.4))
```

`target` はカメラ位置、`yaw`/`pitch` はラジアンです。

## スクリーンショットと録画

```python
# 次のフレームを PNG 保存。include_ui=False でオーバーレイを除外。
viewer.save_screenshot("scene.png", include_ui=False)

# stop_recording() まで録画。".gif" なら GIF、それ以外は H.264。
viewer.start_recording("scene.mp4", fps=30)
...
viewer.stop_recording()
```

ファイルは次の `spin_once()` の実行中に書き込まれるので、画像を読む前にもう一度
呼んでください。パスを空にするとカレントディレクトリに日時入りの名前で生成します。
録画には `PATH` の通った `ffmpeg` が必要です。

`Viewer(headless=True)` はウィンドウなしでシーンを描画するため、スクリプトから
画像を生成する用途に向いています。

```python
viewer = lv.Viewer(headless=True, width=1280, height=720)
viewer.add_object(lv.Box(scale=2.0, color=lv.color.red))
viewer.spin_once()
viewer.save_screenshot("out.png")
viewer.spin_once()
```

`examples/python/capture.py` と `examples/python/headless_render.py` を参照してください。
