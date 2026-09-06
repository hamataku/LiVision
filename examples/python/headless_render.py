"""Render a scene to a PNG without showing a window.

Useful for generating figures or for checking a scene from a script. The
screenshot is written during the frame that follows the request, so spin_once()
is called once more before the file is read.
"""

import os

import livision as lv

viewer = lv.Viewer(headless=True, width=1280, height=720)
viewer.set_camera_controller(
    lv.KeyboardOrbitCamera(target=[0, -9, 5], yaw=-1.626, pitch=0.418)
)

viewer.add_object(lv.Plane(scale=[30.0, 30.0, 0.0], color=lv.color.light_gray))
viewer.add_object(lv.Grid(scale=[30.0, 30.0, 0.0]))
viewer.add_object(lv.Box(pos=[-3.0, 0.0, 1.0], scale=2.0, color=lv.color.rainbow_z))
viewer.add_object(lv.Sphere(pos=[3.0, 0.0, 1.0], scale=2.0, color=lv.color.red))

viewer.spin_once()                      # render the scene
path = viewer.save_screenshot("headless.png")
viewer.spin_once()                      # the file is written here

print(path, os.path.getsize(path), "bytes")
