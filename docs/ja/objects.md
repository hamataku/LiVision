# オブジェクト一覧

## Primitives

- `Plane`
- `Box`
- `Sphere`
- `Cylinder`
- `Cone`

## Objects

- `Mesh`
- `Model`
- `Text`
- `Drone`

## Markers

- `Arrow`
- `Grid`
- `Path`
- `Odometry`
- `DegeneracyIndicator`
- `PointCloud`

## Model読み込み

```cpp
auto model = livision::Model::InstanceWithFile(
    "path/to/model.sdf",
    {.scale = {1.0, 1.0, 1.0}});
viewer->AddObject(model);
```

または:

```cpp
auto model = livision::Model::Instance();
model->SetFromFile("path/to/model.stl");
viewer->AddObject(model);
```
