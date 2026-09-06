"""Tour of the objects available from Python (mirrors examples/objects)."""

import math
import os

import numpy as np

import livision as lv

viewer = lv.Viewer(width=1280, height=720)


def ui():
    if lv.imgui.button("Close") or (
        not lv.imgui.want_capture_keyboard() and lv.imgui.is_key_down(lv.imgui.Key.Q)
    ):
        viewer.close()


viewer.register_ui_callback(ui)

# Floor
viewer.add_object(lv.Plane(scale=[40.0, 40.0, 0.0], color=lv.color.light_gray))
viewer.add_object(lv.Grid(scale=[40.0, 40.0, 0.0]).set_resolution(1.0))

# Mesh from a file (STL/OBJ/DAE via assimp, or SDF)
here = os.path.dirname(os.path.abspath(__file__))
bunny = lv.Model(
    os.path.join(here, "..", "objects", "bunny.stl"),
    pos=[0.0, 0.0, -2.0],
    scale=50.0,
    color=lv.color.rainbow_z,
    wire_color=lv.color.black,
    deg_rotation=[90.0, 0.0, 0.0],
)
viewer.add_object(bunny)

# Primitives
box = lv.Box(scale=2.0, color=lv.color.rainbow_z)
cone = lv.Cone(scale=[1.5, 1.5, 3.0], color=lv.color.green, wire_color=lv.color.black)
cylinder = lv.Cylinder(scale=[1.5, 1.5, 3.0], color=lv.color.orange)
sphere = lv.Sphere(scale=2.0, color=lv.color.red, wire_color=lv.color.black)
for obj in (box, cone, cylinder, sphere):
    viewer.add_object(obj)

# Custom mesh from numpy: a single-sided quad
quad = lv.Mesh(
    np.array([[-1, 0, 0], [1, 0, 0], [-1, 0, 2], [1, 0, 2]], dtype=np.float32),
    np.array([[0, 1, 2], [1, 3, 2]], dtype=np.uint32),
    pos=[0.0, 8.0, 0.0],
    color=lv.color.yellow,
)
viewer.add_object(quad)

# Container with a drone and an attached label
drone = lv.Drone()
label = lv.Text(text="Drone", pos=[0.0, 0.0, 0.8], height=0.5,
                color=lv.color.red, align=lv.TextAlign.Center)
drone.add_object(label)
viewer.add_object(drone)

# Markers
arrow = lv.Arrow(color=lv.color.yellow).set_arrow_params(
    lv.ArrowParams(from_pos=[0, 0, 0], to_pos=[0, 0, 5],
                   head_length=1.0, head_radius=0.2, body_radius=0.1)
)
viewer.add_object(arrow)
viewer.add_object(lv.Odometry())

xs = np.arange(-10.0, 10.0, 0.1)
path_pts = np.stack([xs, np.sin(xs) + 2.0, np.cos(xs) + 5.0], axis=1)
viewer.add_object(lv.Path(color=lv.color.rainbow_x).set_path(path_pts).set_path_width(0.1))

gx, gy = np.meshgrid(np.linspace(4, 10, 40), np.linspace(4, 10, 40))
cloud_pts = np.stack([gx.ravel(), gy.ravel(), 0.5 + 0.5 * np.sin(gx.ravel() * 2) * np.cos(gy.ravel() * 2)], axis=1)
viewer.add_object(lv.PointCloud(color=lv.color.magenta).set_size(0.12).set_points(cloud_pts))

theta = 0.0
while viewer.spin_once():
    theta += 0.01
    q = math.pi / 4
    drone.set_pos(6 * math.cos(theta), 6 * math.sin(theta), 2.0)
    box.set_pos(6 * math.cos(theta + q), 6 * math.sin(theta + q), 2.0)
    cone.set_pos(6 * math.cos(theta + 2 * q), 6 * math.sin(theta + 2 * q), 2.0)
    cylinder.set_pos(6 * math.cos(theta + 3 * q), 6 * math.sin(theta + 3 * q), 2.0)
    sphere.set_pos(6 * math.cos(theta + 4 * q), 6 * math.sin(theta + 4 * q), 2.0)
    arrow.set_from_to(
        [6 * math.cos(theta + 4 * q), 6 * math.sin(theta + 4 * q), 2.0],
        [6 * math.cos(theta + 5 * q), 6 * math.sin(theta + 5 * q), 2.0],
    )
