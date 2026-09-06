"""LiVision: lightweight 3D visualizer for rapid prototyping.

Typical use::

    import livision as lv

    viewer = lv.Viewer()
    viewer.add_object(lv.Grid(scale=[20, 20, 0]))
    box = lv.Box(pos=[0, 0, 1], scale=2.0, color=lv.color.rainbow_z)
    viewer.add_object(box)
    while viewer.spin_once():
        pass
"""

import os as _os
import sys as _sys

# Shaders bundled in the wheel take priority over any system install so the
# extension always finds shaders matching its own version.
_shader_dir = _os.path.join(_os.path.dirname(_os.path.abspath(__file__)), "shaders")
if _os.path.isdir(_shader_dir):
    _existing = _os.environ.get("LIVISION_SHADER_PATHS", "")
    _os.environ["LIVISION_SHADER_PATHS"] = (
        _shader_dir if not _existing else _shader_dir + _os.pathsep + _existing
    )

from ._livision import *  # noqa: E402,F401,F403
from ._livision import __version__, color, imgui  # noqa: E402

# Make `import livision.color` / `import livision.imgui` work.
_sys.modules[__name__ + ".color"] = color
_sys.modules[__name__ + ".imgui"] = imgui

del _os, _sys, _shader_dir
