"""
Script for the PlayerMoveBehavior class
"""
import sys
import numpy as np

# ==============================================================================
# Globals
# ==============================================================================

# ==============================================================================
# Classes
# ==============================================================================

class PlayerMoveBehavior(ScriptBehavior):
    """ Tell camera what to do
    """

    # @property
    # def controller(self):
    #     return self.scene_object.character_controller()

    def __init__(self, scene_object):
        """Create the behavior, and underlying C++ PythonBehavior object"""
        ScriptBehavior.__init__(self, scene_object)
        return

    # def initialize(self):
    #     """Initialize"""
    #     ScriptBehavior.initialize(self)

    #     # Get camera from related object
    #     self.camera = self.scene.get_scene_object("camera").camera()
    #     self.body = self.scene_object.get_child("boi_body")
    #     if self.camera is None:
    #         return 0
    #     if self.body is None:
    #         return 0
    #     return 2


    # def update(self, delta_ms: int):
    #     ScriptBehavior.update(self, delta_ms)

    #     pos = self.transform.translation().local_pos()
    #     held_up = self.input.key_handler().is_held("w")
    #     held_right = self.input.key_handler().is_held("d")
    #     held_left = self.input.key_handler().is_held("a")
    #     held_down = self.input.key_handler().is_held("s")

    #     # Get move direction
    #     direction = Vector3()
    #     if held_up:
    #         direction += self.camera.forward()
    #     if held_down:
    #         direction += -self.camera.forward()
    #     if held_right:
    #         direction += self.camera.right()
    #     if held_left:
    #         direction += -self.camera.right()

    #     # If character moved
    #     if direction.length_squared():
    #         # Project direction onto ground and normalize
    #         direction = direction - Vector3.up() * ((direction.dot(Vector3.up()))/(Vector3.up().length()))
    #         direction.normalize()

    #         # Rotate character to face move direction
    #         lookSpeed = 1.0 * delta_ms / 100.0
    #         current_quat = self.body.transform().rotation().quaternion()
    #         target_quat = Quaternion.from_direction(direction, Vector3.up())
    #         # self.log_info(str(direction))
    #         # self.log_info(str(target_quat))
    #         # self.log_info("-----")
    #         lookQuat = Quaternion.slerp(current_quat, target_quat, lookSpeed)
    #         lookQuat.normalize()
    #         self.body.transform().rotation().set(lookQuat)

    #         # Move character
    #         speed = delta_ms / 10.0
    #         direction *= speed
    #         # self.controller.move(direction)
    #         self.transform.translation().set_local_pos(self.transform.translation().local_pos() + direction)

    #     # self.log_info("Player: update")
    #     return 2

    # # def late_update(self, delta_ms: int):
    # #     # Camera operations go here
    # #     ScriptBehavior.late_update(self, delta_ms)
    # #     # print("Performing late update")

    # #     self.log_info("Player: late_update")
    # #     return 2

    # def fixed_update(self, delta_ms: int):
    #     ScriptBehavior.fixed_update(self, delta_ms)

    #     return 2
