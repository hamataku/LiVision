"""Minimal LiVision example: a grid, a sphere, and an ImGui slider."""

import math

import livision as lv

viewer = lv.Viewer(width=1280, height=720)

state = {"theta": 0.0}


def ui():
    changed, state["theta"] = lv.imgui.slider_float(
        "theta", state["theta"], 0.0, 2.0 * math.pi
    )
    if lv.imgui.button("Close"):
        viewer.close()


viewer.register_ui_callback(ui)

viewer.add_object(lv.Grid(scale=[15.0, 15.0, 0.0]))

sphere = lv.Sphere(scale=2.0, color=lv.color.rainbow_z, wire_color=lv.color.black)
viewer.add_object(sphere)

while viewer.spin_once():
    sphere.set_pos(3.0 * math.cos(state["theta"]), 3.0 * math.sin(state["theta"]), 1.0)
