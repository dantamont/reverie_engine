"""
Script for the TestBehavior class
"""
# import base_behavior
# from PythonQt.components import Transform
import sys
# sys.path.append("C:/Users/dante/Documents/Projects/grand-blue-engine/third_party/Python38-32/Lib/site-packages") 
import numpy as np

# ==============================================================================
# Globals
# ==============================================================================

# ==============================================================================
# Classes
# ==============================================================================

class CircleBehavior(ScriptBehavior):
    """ Test behavior from which all scripted behaviors must inherit
    """
    def __init__(self, scene_object):
        """Create the behavior, and underlying C++ PythonBehavior object"""
        ScriptBehavior.__init__(self, scene_object)
        return

    # def initialize(self):
    #     """Initialize"""
    #     ScriptBehavior.initialize(self)
    #     self.total_ms = 0
    #     return 2


    # def update(self, delta_ms: int):
    #     ScriptBehavior.update(self, delta_ms)

    #     self.total_ms += delta_ms
    #     return 2

    # def late_update(self, delta_ms: int):
    #     # Camera operations go here
    #     ScriptBehavior.late_update(self, delta_ms)
    #     return 2

    # def fixed_update(self, delta_ms: int):
    #     ScriptBehavior.fixed_update(self, delta_ms)

    #     # Test moving an object in a circle
    #     # self.move_in_circle(self.transform, 0.1, (1, 0, 0))
    #     self.move_in_circle(self.transform, 
    #                         0.002*delta_ms, 
    #                        (1, 0, 0),
    #                        circle_radius=20)

    #     return 2


    # def move_in_circle(self, 
    #                    transform: TransformComponent, 
    #                    speed: float,
    #                    center: tuple = (0.0, 0.0, 0.0),
    #                    up: tuple = (0.0, 1.0, 0.0),
    #                    circle_radius: float = 1):
    #     """Move transform in a circle
        
    #     Args:
    #         speed: Rate at which to move transform
    #     """
    #     pos_vec = transform.translation().local_pos()
    #     from_center = pos_vec - center
    #     up_vec = Vector3(up)
    #     dir_ = -1.0 * from_center.cross(up_vec)
    #     offset = dir_.normalized() * speed * circle_radius
    #     transform.translation().set_local_pos(pos_vec + offset)
    #     # print(transform.translation().local_pos())

    #     # X := originX + cos(angle)*radius;
    #     # Y := originY + sin(angle)*radius;