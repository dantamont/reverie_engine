"""
Script for the ManyLights class
"""
import sys
import numpy as np
import random

# ==============================================================================
# Globals
# ==============================================================================

# ==============================================================================
# Classes
# ==============================================================================

class ManyLights(ScriptBehavior):
    """ Test behavior from which all scripted behaviors must inherit
    """
    def __init__(self, scene_object):
        """Create the behavior, and underlying C++ PythonBehavior object"""
        ScriptBehavior.__init__(self, scene_object)
        return

    # def initialize(self):
    #     """Initialize"""
    #     ScriptBehavior.initialize(self)

    #     # Create lights
    #     for idx in range(0, 200):
    #         # posX = random.randint(-200, 200)
    #         # posY = random.randint(0, 25)
    #         # posZ = random.randint(-150, 150)

    #         posX = random.randint(-800, 800)
    #         posY = random.randint(0, 20)
    #         posZ = random.randint(-160, 160)


    #         r = random.randint(0, 255)
    #         g = random.randint(0, 255)
    #         b = random.randint(0, 255)

    #         light_object = SceneObject(self.scene)
    #         new_light = LightComponent(light_object)
    #         new_light.set_diffuse_color((r, g, b))
    #         new_light.set_intensity(20)
    #         new_light.set_range(100)
    #         new_light.set_attenuations([1.0, 0.1, 0.1])
    #         light_object.transform().translation().set_local_pos(
    #             [posX, posY, posZ]
    #         )

    #     self.total_ms = 0

    #     return 2


    # def update(self, delta_ms: int):
    #     ScriptBehavior.update(self, delta_ms)
    #     return 2

    # def late_update(self, delta_ms: int):
    #     # Camera operations go here
    #     ScriptBehavior.late_update(self, delta_ms)
    #     # print("Performing late update")
    #     return 2

    # def fixed_update(self, delta_ms: int):
    #     ScriptBehavior.fixed_update(self, delta_ms)
    #     return 2