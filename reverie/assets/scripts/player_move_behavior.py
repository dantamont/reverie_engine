"""
Script for the PlayerMoveBehavior class
"""
import sys
import numpy as np
from enum import Enum
# ==============================================================================
# Globals
# ==============================================================================


# ==============================================================================
# Classes
# ==============================================================================


class PlayerAction(Enum):
    none = -1
    idle = 0
    walk = 1
    run = 2

class PlayerMoveBehavior(ScriptBehavior):
    """ Tell player how to move
    """

    @property
    def is_idle(self):
        return self.locomotion_action is PlayerAction.idle

    def __init__(self, scene_object):
        """Create the behavior, and underlying C++ PythonBehavior object"""
        ScriptBehavior.__init__(self, scene_object)
        self.locomotion_action = PlayerAction.none

    def initialize(self):
        """Initialize"""
        # Initialize list of player actions
        self.actions = []

        # Get camera from related object
        self.camera = self.scene.get_scene_object("camera").camera
        self.body = self.scene_object.get_child("boi_body")
        if self.camera is None:
            raise("Error, camera not found")
        if self.body is None:
            raise("Error, body not found")


        # Animation setup
        self.walk_anim_name = "human armature|walk"
        self.idle_anim_name = "human armature|idle"
        loco_motion = "locomotion"
        self.skeletal_animation = self.body.skeletal_animation
        self.locomotion = self.skeletal_animation.get_motion(loco_motion)
        if self.skeletal_animation is None:
            raise("Error, skeletal animation not found")


    def update(self, delta_ms: int):

        pos = self.transform.local_pos
        held_up = self.input.key_handler.is_held("w")
        held_right = self.input.key_handler.is_held("d")
        held_left = self.input.key_handler.is_held("a")
        held_down = self.input.key_handler.is_held("s")

        # Get move direction
        direction = Vector3()
        if held_up:
            direction += self.camera.forward()
        if held_down:
            direction += -self.camera.forward()
        if held_right:
            direction += self.camera.right()
        if held_left:
            direction += -self.camera.right()

        if direction.length_squared():
            # If character moved

            # Project direction onto ground and normalize
            direction = direction - Vector3.up() * ((direction.dot(Vector3.up()))/(Vector3.up().length()))
            direction.normalize()

            # Rotate character to face move direction
            lookSpeed = 1.0 * delta_ms / 100.0
            current_quat = self.body.transform.rotation.quaternion
            target_quat = Quaternion.from_direction(direction, Vector3.up())
            lookQuat = Quaternion.slerp(current_quat, target_quat, lookSpeed)
            lookQuat.normalize()
            self.body.transform.set_rotation(lookQuat)

            # Move character
            speed = delta_ms / 10.0
            direction *= speed
            # self.controller.move(direction)
            self.transform.local_pos += direction

            # If is idle, need to trigger walking animation
            if self.is_idle:
                self.locomotion_action = PlayerAction.walk
                self.log_info("Switching to walk")
                self.locomotion.move(self.walk_anim_name)
        else:
            # If the character did not move
            if not self.is_idle:
                # If is not idle, need to trigger idle animation
                self.log_info("Switching to idle")
                self.locomotion_action = PlayerAction.idle
                self.locomotion.move(self.idle_anim_name)



