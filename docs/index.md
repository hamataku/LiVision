# LiVision Documentation

LiVision is a lightweight C++ 3D visualizer built around bgfx, SDL2, ImGui, and ImPlot.

Documentation pages:

- **Quick Start**
- **Basic Operations**
- **Color Settings**
- **Object List**
- **Container**
- **Camera Controllers**
- **API Reference**

## Quick Start

1. Install dependencies and clone the repository with submodules:

```bash
sudo apt install -y libeigen3-dev libsdl2-dev
git clone https://github.com/hamataku/LiVision.git --recursive
cd LiVision
```

2. Configure and build:

```bash
./scripts/setup.sh
./scripts/build.sh
```

3. Run an example:

```bash
./build/basic/basic
```

## What You Can Visualize

- Primitive objects (`Plane`, `Box`, `Sphere`, `Cylinder`, `Cone`)
- Marker objects (`Arrow`, `Grid`, `Path`, `Odometry`, `DegeneracyIndicator`)
- Mesh/model objects (`Mesh`, `Model`)
- Text objects (`Text`)

## Current Ownership Model (Important)

- `Viewer::AddObject` accepts `std::shared_ptr<ObjectBase>`.
- `Container::AddObject` also accepts `std::shared_ptr<ObjectBase>`.
- Most objects support `ClassName::Instance(...)` factory helpers for consistent construction.
