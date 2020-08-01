"""
Script for the BaseBehavior class
"""
from PythonQt.scriptedBehaviors import PythonBehavior
from PythonQt.scene import SceneObject
from PythonQt.components import (TransformComponent,
                                 LightComponent,
                                 CharControlComponent,
                                 CameraComponent,
                                 ShaderComponent)
from PythonQt.alg import (Quaternion, 
                          EulerAngles,
                          Vector2,
                          Vector3,
                          Vector4,
                          Matrix2x2,
                          Matrix3x3,
                          Matrix4x4)
# from abc import ABC

# ==============================================================================
# Globals
# ==============================================================================
DEBUG_MODE = True

# ==============================================================================
# Classes Aliases
# ==============================================================================

# ==============================================================================
# Classes
# ==============================================================================

class BaseBehavior(object):
    """ Base behavior from which all scripted behaviors must inherit
    
    For inheritance:
    https://stackoverflow.com/questions/2200912/inheritance-in-python-c-extension
    https://stackoverflow.com/questions/9040669/how-can-i-implement-a-c-class-in-python-to-be-called-by-c
    """

    @property
    def input(self):
        """Return input handler"""
        return self.engine.input_handler()

    @property
    def engine(self):
        """Return engine from associated scene object"""
        return self.scene_object.engine()

    @property
    def resources(self):
        """Return resource cache from the core engine"""
        return self.engine.resource_cache()

    @property
    def transform(self):
        """Return engine from associated scene object"""
        return self.scene_object.transform()

    @property
    def scene(self):
        """Return scene from associated scene object"""
        return self.scene_object.scene()

    # def __init__(self):
    #     """Create the behavior, and underlying C++ PythonBehavior object"""
    #     self._behavior = PythonBehavior()
    #     return

    def __init__(self, scene_object):
        """Create the behavior, and underlying C++ PythonBehavior object"""
        self._behavior = PythonBehavior()
        self.scene_object = scene_object
        return

    def initialize(self):
        """Initialize"""
        self._behavior.initialize()
        return 1

    def update(self, delta_ms: int):
        self._behavior.update(delta_ms)
        return 1

    def late_update(self, delta_ms: int):
        return 1

    def fixed_update(self, delta_ms: int):
        self._behavior.fixed_update(delta_ms)
        return 1

    def on_success(self):
        """What to do on successful completion of behavior"""
        self._behavior.on_success()
        return 1

    def on_fail(self):
        """What to do on behavior failure"""
        self._behavior.on_fail()
        return 1

    def on_abort(self):
        """What to do on behavior abort"""
        self._behavior.on_abort()
        return 1

    def log_info(self, info: str=""):
        """ Log given string as info """
        if DEBUG_MODE:
            print(info)
            return 1
        return 0