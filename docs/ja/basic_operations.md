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

## スクリーンショットと録画

`Viewer` は現在のフレームを PNG で保存でき、ビューポートを動画として録画できます。
`capture_ui` を有効にしたままなら、コントロールパネルの **Capture** セクションから
ボタン 2 つで同じ操作ができます。

```cpp
// 次に描画されるフレームを PNG 保存。false で ImGui のオーバーレイを除外。
viewer->SaveScreenshot("scene.png", false);

// StopRecording() まで録画。".gif" なら GIF、それ以外は H.264。
viewer->StartRecording("scene.mp4", 30);
...
viewer->StopRecording();
```

画像は *次の* `SpinOnce()` の実行中に書き込まれるため、ファイルを読む前に
`SpinOnce()` をもう一度呼んでください。パスを空にすると、カレントディレクトリに
日時入りの名前で生成します。

録画はフレームを `ffmpeg` に流し込むので、`ffmpeg` がインストールされ `PATH` に
通っている必要があります。フレームは実時間に合わせて出力するため、描画が指定
フレームレートより遅くても動画は実速度で再生されます。エンコード中はフレーム
サイズを変更できないので、ウィンドウをリサイズすると録画は停止します。

### ウィンドウなしでの描画

ヘッドレスの Viewer も同じようにシーンを描画します（ImGui のオーバーレイのみ
描画しません）。スクリプトや CI から画像を生成できます。

```cpp
auto viewer = livision::Viewer::Instance({.headless = true,
                                          .width = 1280, .height = 720});
viewer->AddObject(...);
viewer->SpinOnce();                     // 描画
viewer->SaveScreenshot("out.png");
viewer->SpinOnce();                     // ここでファイルが書き出される
```
