# 基本操作

## ビューワーインスタンスの作成

```cpp
auto viewer = livision::Viewer::Instance({
    .headless = false,
    .vsync = true,
    .width = 1280,
    .height = 720,
});
```

## オブジェクトの登録

`AddObject` は `std::shared_ptr<ObjectBase>` のみ受け取ります。

```cpp
auto sphere = livision::Sphere::Instance(
    {.pos = {0.0, 0.0, 1.0}, .scale = {2.0, 2.0, 2.0}});
sphere->SetColor(livision::color::cyan)->SetWireColor(livision::color::black);
viewer->AddObject(sphere);
```

## ImGuiコールバックの登録

```cpp
viewer->RegisterUICallback([&]() {
  if (ImGui::Button("Close")) {
    viewer->Close();
  }
});
```

## メインループ

```cpp
while (viewer->SpinOnce()) {
  // ここでオブジェクト状態を更新
}
```
