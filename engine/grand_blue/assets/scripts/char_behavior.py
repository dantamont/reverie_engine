"""
Script for the CharBehavior class
"""
import sys
import numpy as np

# ==============================================================================
# Globals
# ==============================================================================

# ==============================================================================
# Classes
# ==============================================================================

class CharBehavior(BaseBehavior):
    """ Test behavior from which all scripted behaviors must inherit
    """
    def __init__(self, scene_object):
        """Create the behavior, and underlying C++ PythonBehavior object"""
        BaseBehavior.__init__(self, scene_object)
        return

    def initialize(self):
        """Initialize"""
        BaseBehavior.initialize(self)
        self.char_controller = CharControlComponent(self.scene_object)
        self.char_controller.set_height(10)
        self.char_controller.set_radius(2.5)
        self.char_controller.set_initial_position(Vector3([0, 10, 0]))
        self.char_controller.set_height_offset(7.7)

        return 2


    def update(self, delta_ms: int):
        BaseBehavior.update(self, delta_ms)
        return 2

    def late_update(self, delta_ms: int):
        # Camera operations go here
        BaseBehavior.late_update(self, delta_ms)
        return 2

    def fixed_update(self, delta_ms: int):
        BaseBehavior.fixed_update(self, delta_ms)

        # Test inputs
        pos = self.transform.translation().get_position()
        held_up = self.input.key_handler().is_held("w")
        held_right = self.input.key_handler().is_held("d")
        held_left = self.input.key_handler().is_held("a")
        held_down = self.input.key_handler().is_held("s")
        held_space = self.input.key_handler().is_held(" ")
        held_x = self.input.key_handler().is_held("x")
        move_dir = Vector3()
        if held_up:
            move_dir += Vector3(0, 0, -0.1)
            # print(self.char_controller.as_json_str())
        if held_down:
            move_dir += Vector3(0, 0, 0.1)
        if held_right:
            move_dir += Vector3(0.1, 0, 0.0)
        if held_left:
            move_dir += Vector3(-0.1, 0, 0.0)
        if held_space:
            move_dir += Vector3(0, 0.1, 0.0)
        if held_x:
            move_dir += Vector3(0, -0.1, 0.0)
        if self.input.mouse_handler().moved():
            # print(self.input.mouse_handler().screen_pos())
            pass
        if move_dir.length() > 0:
            self.char_controller.move(move_dir)
            move_dir.normalize()

            # Look in the direction of character movement
            # https://answers.unity.com/questions/803365/make-the-player-face-his-movement-direction.html
            curr_rot = self.transform.rotation().get_quaternion()
            rot = Quaternion.look_rotation(move_dir, Vector3(0, 1, 0))
            rot = Quaternion.slerp(curr_rot, rot, 0.15)
            self.transform.rotation().set_rotation(rot)

        return 2