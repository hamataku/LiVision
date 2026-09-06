"""Screenshots and video recording.

Run it and use the Capture section of the control panel, or let the script
drive it: it saves a PNG and records a short clip on its own.

Recording needs ffmpeg on PATH.
"""

import math

import livision as lv

viewer = lv.Viewer(width=960, height=540)
viewer.set_camera_controller(
    lv.KeyboardOrbitCamera(target=[0, -8, 4], yaw=-1.626, pitch=0.35)
)

viewer.add_object(lv.Plane(scale=[30.0, 30.0, 0.0], color=lv.color.light_gray))
viewer.add_object(lv.Grid(scale=[30.0, 30.0, 0.0]))

box = lv.Box(pos=[-3.0, 0.0, 1.0], scale=2.0, color=lv.color.rainbow_z)
drone = lv.Drone(scale=2.0)
viewer.add_object(box)
viewer.add_object(drone)


def ui():
    lv.imgui.text("recording" if viewer.is_recording() else "idle")
    if lv.imgui.button("Close"):
        viewer.close()


viewer.register_ui_callback(ui)

theta = 0.0
frame = 0
while viewer.spin_once():
    theta += 0.03
    frame += 1
    drone.set_pos(4.0 * math.cos(theta), 4.0 * math.sin(theta), 2.0)
    box.set_rad_rotation([0.0, 0.0, theta])

    if frame == 10:
        # include_ui=False renders this one frame without the ImGui overlay.
        print("screenshot ->", viewer.save_screenshot("scene.png", include_ui=False))
    if frame == 20:
        print("recording ->", viewer.start_recording("scene.mp4", fps=30))
    if frame == 200:
        viewer.stop_recording()
