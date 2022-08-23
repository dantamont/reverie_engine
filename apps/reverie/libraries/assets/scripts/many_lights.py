"""
Script for the ManyLights class
"""
import sys
import numpy as np
import random

from reverie.components import Light
# ==============================================================================
# Globals
# ==============================================================================

# ==============================================================================
# Classes
# ==============================================================================

class ManyLights(ScriptBehavior):
    """ Script for generating a lot of lights
    """

    def initialize(self):
        """Initialize"""
        # Create lights
        for idx in range(0, 200):
            posX = random.randint(-800, 800)
            posY = random.randint(0, 20)
            posZ = random.randint(-160, 160)


            r = random.randint(0, 255)
            g = random.randint(0, 255)
            b = random.randint(0, 255)

            light_object = SceneObject(self.scene)
            new_light = Light(light_object)
            new_light.diffuse_color = (r, g, b)
            new_light.intensity = 20
            new_light.range = 100
            new_light.attenuations = [1.0, 0.1, 0.1]
            # light_object.transform
            light_object.transform.local_pos = [posX, posY, posZ]
        self.total_ms = 0


    def update(self, delta_s: float):
        pass

    def late_update(self, delta_s: float):
        pass

    def fixed_update(self, delta_s: float):
        pass