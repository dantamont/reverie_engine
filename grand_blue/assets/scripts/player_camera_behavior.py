"""
Script for the PlayerCameraBehavior class

TODO: Increment camera rotation based on current direction, not cumulative mouse
delta, which is causing jumps

TODO: Add min/max zoom
"""
import sys
import numpy as np

# ==============================================================================
# Globals
# ==============================================================================

# ==============================================================================
# Classes
# ==============================================================================

class PlayerCameraBehavior(ScriptBehavior):
    """ Tell camera what to do
    """

    DEG_TO_RAD = np.pi / 180.0

    # @property
    # def camera(self):
    #     return self.scene_object.camera()

    def __init__(self, scene_object):
        """Create the behavior, and underlying C++ PythonBehavior object"""
        ScriptBehavior.__init__(self, scene_object)
        return

    # def initialize(self):
    #     """Initialize"""
    #     ScriptBehavior.initialize(self)

    #     self.target = self.scene.get_scene_object("boi")
    #     scale = self.target.get_child_("boi_body").transform().scale().get_scale()[1]
    #     self.distance = 1000.0 * scale
    #     self.height = 500 * scale
    #     self.current_mouse_pos = Vector2(0.0, 0.0)
    #     self.sensitivity_x = 4.0
    #     self.sensitivity_y = 1.0
    #     self.y_angle_min = 0.0 * self.DEG_TO_RAD
    #     self.y_angle_max = 50.0 * self.DEG_TO_RAD

    #     # Initialize camera to look at target
    #     target = self.target.transform().pos() # Fix, make global
    #     self.transform.set_pos(target + Vector3(0, 0.0, -self.distance))
    #     self.camera.look_at(target)
    #     self.set_target_pos()

    #     return 2

    # def set_target_pos(self):
    #     self.target_pos = self.target.transform().pos() + self.height * Vector3.up()

    # def get_look_delta(self):
    #     return self.target_pos - self.transform.pos()

    # def update(self, delta_ms: int):
    #     ScriptBehavior.update(self, delta_ms)

    #     # Scale mouse movement by delta_ms so not frame-rate dependent
    #     self.current_mouse_pos += self.input.mouse_handler().mouse_delta() * (delta_ms / 20.0)
    #     self.current_mouse_pos.set_y(
    #         max(min(self.current_mouse_pos.y(), self.y_angle_max), self.y_angle_min))

    #     self.set_target_pos()
    #     # self.log_info("Camera: update")
    #     return 2

    # def late_update(self, delta_ms: int):
    #     # Camera operations go here, since we want to move camera after player move
    #     # See: https://www.kinematicsoup.com/news/2016/8/9/rrypp5tkubynjwxhxjzd42s3o034o8
    #     ScriptBehavior.late_update(self, delta_ms)

    #     target = self.target_pos

    #     # If mouse moved, move camera
    #     new_pos = self.transform.pos()
    #     move_camera = False
    #     if self.input.mouse_handler().moved():
    #         distance = self.get_look_delta().length()
    #         direction = Vector3(0, 0, -distance)
    #         # direction = Vector3(0, 0, -self.distance)

    #         mouse_y = self.current_mouse_pos.y() * self.sensitivity_y
    #         mouse_x = self.current_mouse_pos.x() * self.sensitivity_x
    #         rotation = Quaternion.from_euler_angles(mouse_y, mouse_x, 0.0)
            
    #         # Was lerping to smooth out motion
    #         tracking_speed = 0.4 # From 0-1
    #         new_pos = target + rotation * direction
    #         # new_pos = Vector3.lerp(self.transform.pos(), new_pos, tracking_speed)
    #         move_camera = True

    #     # If mouse scrolled, zoom camera
    #     if self.input.mouse_handler().scrolled():
    #         scroll_delta = self.input.mouse_handler().scroll_delta()
    #         # self.log_info("Scrolled: " + str(scroll_delta))
    #         zoom_amount = -0.5 * scroll_delta.y()
    #         new_pos = target + (new_pos - target) * (zoom_amount + 1.0)
    #         move_camera = True

    #     # Move camera if zoomed or moved mouse
    #     if move_camera:
    #         self.transform.set_pos(new_pos)

    #     # Track target and set zoom with look-at
    #     self.camera.look_at(target)

    #     return 2

    # def fixed_update(self, delta_ms: int):
    #     ScriptBehavior.fixed_update(self, delta_ms)

    #     return 2
