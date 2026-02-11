# Basic Operations

## Create a Viewer Instance

```cpp
auto viewer = livision::Viewer::Instance({
    .headless = false,
    .vsync = true,
    .width = 1280,
    .height = 720,
});
```

## Register Objects

`AddObject` accepts only `std::shared_ptr<ObjectBase>`.

```cpp
auto sphere = livision::Sphere::Instance(
    {.pos = {0.0, 0.0, 1.0}, .scale = {2.0, 2.0, 2.0}});
viewer->AddObject(sphere);
```

## Register ImGui Callback

```cpp
viewer->RegisterUICallback([&]() {
  if (ImGui::Button("Close")) {
    viewer->Close();
  }
});
```

## Main Loop

```cpp
while (viewer->SpinOnce()) {
  // update object states here
}
```
