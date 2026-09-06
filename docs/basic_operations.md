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
sphere->SetColor(livision::color::cyan)->SetWireColor(livision::color::black);
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

## Screenshots and recording

`Viewer` can save a PNG of the current frame and record the viewport to a
video. When `capture_ui` is left on, a **Capture** section in the control
panel offers the same thing with two buttons.

```cpp
// PNG of the next rendered frame. Pass false to leave out the ImGui overlay.
viewer->SaveScreenshot("scene.png", false);

// Record until StopRecording(). ".gif" writes a GIF, anything else H.264.
viewer->StartRecording("scene.mp4", 30);
...
viewer->StopRecording();
```

The image is written while the *next* `SpinOnce()` runs, so call `SpinOnce()`
once more before reading the file. Passing an empty path generates a
timestamped name in the current directory.

Recording streams frames to `ffmpeg`, which must be installed and on `PATH`.
Frames are paced on the wall clock, so the video plays at real speed even when
the scene renders slower than the requested frame rate. Resizing the window
stops the recording, because the encoder cannot change frame size mid-stream.

### Rendering without a window

A headless viewer renders the scene the same way, just without the ImGui
overlay, so it can produce images from a script or from CI:

```cpp
auto viewer = livision::Viewer::Instance({.headless = true,
                                          .width = 1280, .height = 720});
viewer->AddObject(...);
viewer->SpinOnce();                     // render
viewer->SaveScreenshot("out.png");
viewer->SpinOnce();                     // the file is written here
```
