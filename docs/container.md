# Container

`Container` is a hierarchical object that owns child objects with `std::shared_ptr`.

## Core API

- `Container::AddObject(std::shared_ptr<ObjectBase>)`
- `Container::GetObjects()`
- `Container::ClearObjects()`

## Behavior

- Added children automatically register the parent transform.
- Child lifetimes are co-owned by the container.
- Clearing a container releases child ownership.

## Typical Container-based Classes

- `Model`
- `Drone`
- `Odometry`
