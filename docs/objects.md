# Object List

## Primitives

- `Plane`
- `Box`
- `Sphere`
- `Cylinder`
- `Cone`

## Scene Objects

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

## Model Loading

```cpp
auto model = livision::Model::InstanceWithPath(
    "path/to/model.sdf",
    {.scale = {1.0, 1.0, 1.0}});
viewer->AddObject(model);
```

or

```cpp
auto model = livision::Model::Instance();
model->SetFromFile("path/to/model.stl");
viewer->AddObject(model);
```
