# LiVision Documentation

LiVision は bgfx / SDL2 / ImGui / ImPlot を使った軽量な C++ 3D ビジュアライザです。

ドキュメントページ:

- **クイックスタート**
- **基本操作**
- **色設定**
- **オブジェクト一覧**
- **コンテナ**
- **カメラ操作クラス**
- **API Reference**

## クイックスタート

1. 依存パッケージの導入とリポジトリ取得:

```bash
sudo apt install -y libeigen3-dev libsdl2-dev
git clone https://github.com/hamataku/LiVision.git --recursive
cd LiVision
```

2. 設定とビルド:

```bash
./scripts/setup.sh
./scripts/build.sh
```

3. サンプル実行:

```bash
./build/basic/basic
```

## 可視化できるもの

- プリミティブ (`Plane`, `Box`, `Sphere`, `Cylinder`, `Cone`)
- マーカー (`Arrow`, `Grid`, `Path`, `Odometry`, `DegeneracyIndicator`)
- メッシュ/モデル (`Mesh`, `Model`)
- テキスト (`Text`)

## 現在の所有権モデル（重要）

- `Viewer::AddObject` は `std::shared_ptr<ObjectBase>` を受け取ります。
- `Container::AddObject` も `std::shared_ptr<ObjectBase>` を受け取ります。
- 多くのオブジェクトは `ClassName::Instance(...)` ファクトリを持ちます。
