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

    @property
    def camera(self):
        return self.scene_object.camera

    def __init__(self, scene_object):
        """Create the behavior, and underlying C++ PythonBehavior object"""
        ScriptBehavior.__init__(self, scene_object)
        return

    def initialize(self):
        """Initialize"""

        self.target = self.scene.get_scene_object("boi")
        scale = self.target.get_child("boi_body").transform.scale.vec[1]
        self.default_distance = 1000.0 * scale
        self.zoom_factor = 1.0
        self.camera_rotation = Quaternion(0, 0, 0, 1)
        self.height = 500 * scale
        self.current_mouse_pos = Vector2(0.0, 0.0)
        self.sensitivity_x = 4.0 # Negative to invert direction
        self.sensitivity_y = 1.0 # Negative to invert direction
        self.y_angle_min = 0.0 * self.DEG_TO_RAD
        self.y_angle_max = 50.0 * self.DEG_TO_RAD

        # Initialize camera to look at target
        target = self.target.transform.pos
        self.transform.pos = target + Vector3(0, 0.0, -self.default_distance)
        self.camera.look_at(target)
        self.set_target_pos()

    def set_target_pos(self):
        self.target_pos = self.target.transform.pos + self.height * Vector3.up()

    # def get_look_delta(self):
    #     return self.target_pos - self.transform.pos

    def zoom(self, scroll_delta_y):
        zoom_amount = -0.5 * scroll_delta_y # Invert direction
        self.zoom_factor *= zoom_amount + 1.0 # scale current zoom factor

    def update(self, delta_s: float):
        # Scale mouse movement by delta_s so not frame-rate dependent
        self.current_mouse_pos += self.input.mouse_handler.mouse_delta() * (delta_s * 50.0)
        self.current_mouse_pos.y = max(min(self.current_mouse_pos.y, self.y_angle_max), self.y_angle_min)

        self.set_target_pos()
        # self.log_info("Camera: update")

    def late_update(self, delta_s: float):
        # Camera operations go here, since we want to move camera after player move
        # See: https://www.kinematicsoup.com/news/2016/8/9/rrypp5tkubynjwxhxjzd42s3o034o8
        target = self.target_pos

        # If mouse moved, move camera
        new_pos = self.transform.pos
        move_camera = True

        # If mouse scrolled, zoom camera
        if self.input.mouse_handler.scrolled():
            scroll_delta = self.input.mouse_handler.scroll_delta()
            self.zoom(scroll_delta.y)
            # new_pos = target + (new_pos - target) * (zoom_amount + 1.0)
            # move_camera = True

        if self.input.mouse_handler.moved():
            mouse_y = self.current_mouse_pos.y * self.sensitivity_y
            mouse_x = self.current_mouse_pos.x * self.sensitivity_x
            self.camera_rotation = Quaternion.from_euler_angles(mouse_y, mouse_x, 0.0)
            
            # Was lerping to smooth out motion
            # tracking_speed = 0.4 # From 0-1
            # new_pos = Vector3.lerp(self.transform.pos(), new_pos, tracking_speed)
            # move_camera = True

        # Move camera if zoomed or moved mouse
        if move_camera:
            distance = self.default_distance * self.zoom_factor
            direction = Vector3(0, 0, -distance)
            new_pos = target + self.camera_rotation * direction
            self.transform.pos = new_pos

        # Track target and set zoom with look-at
        self.camera.look_at(target)