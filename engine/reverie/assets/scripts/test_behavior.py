"""
Script for the TestBehavior class
"""
import sys
import numpy as np

from reverie.components import Camera
# ==============================================================================
# Globals
# ==============================================================================

# ==============================================================================
# Classes
# ==============================================================================

class TestBehavior(ScriptBehavior):
    """ Test behavior
    """
    def __init__(self, scene_object):
        """Create the behavior, and underlying C++ PythonBehavior object"""
        ScriptBehavior.__init__(self, scene_object)
        return

    def initialize(self):
        """Initialize"""

        # Create light
        light_object = SceneObject(self.scene)
        # self.new_light = Light(light_object)
        # self.new_light.set_color((0, 255, 0))
        # self.new_light.set_intensity(1)

        # Create camera
        # light_object.transform().translation().set_local_pos([0, 5, 20])
        # print(light_object.transform().translation().local_pos())
        self.new_camera = Camera(light_object)
        self.new_camera.set_depth(-1)
        self.new_camera.set_viewport(-0.7, -0.7, 0.3, 0.3)
        # self.new_camera.set_fov(70)
        self.total_ms = 0

    def update(self, delta_ms: int):
        pass
        # ScriptBehavior.update(self, delta_ms)
        # self.log_info(self.scene_object.name)
        # self.log_info(self.scene.name)
        # so = self.scene.get_scene_object(self.scene_object.name)
        # self.log_info(so.name)
        # self.log_info(str(self.engine))
        # self.log_info(str(self.resources))
        # self.log_info(str(self.input))
        # self.log_info(str(self.transform))
        # self.log_info("----------------")

        # # Shader program
        # sp = self.resources.get_shader_program("basic")
        # self.log_info(sp.name)
        # self.log_info(str(sp))
        # self.log_info("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~")
        

        # Vector
        # vec = Vector3(1, 2, 3)
        # vec *= 2
        # vec -= 2
        # vec += 10
        # vec = -vec
        # self.log_info(str(vec))

        # # Matrix
        # mat = Matrix4x4()
        # mat = mat.transpose_multiply(mat)
        # self.log_info(str(mat))
        # mat2 = Matrix4x4([[1,0,0,0], [0,2,0,0], [0,0,3,0], [0,0,0,4]])
        # mat2 = mat2.inversed()
        # self.log_info(str(mat2))

    #     self.total_ms += delta_ms
    #     # effects_shader = self.resources.get_shader_program("effects")
    #     # rim_color = effects_shader.get_uniform("rimColor")
    #     # new_color = [(1 + np.sin(i*self.total_ms/1000.0))/2.0
    #     #     for i, c in enumerate(rim_color)]
    #     # effects_shader.set_uniform("rimColor", new_color)
    #     # effects_shader.set_uniform("rimColor", [0, 1, 0])

    #     # Test vec2
    #     # print(Vector2(1, 1) + Vector2(2, 2))

    #     # Test Matrices
    #     # print(Matrix2x2([[1, 0], [0, 1]]) + Matrix2x2([[1, 0], [0, 1]]))
    #     # print(Matrix3x3([[1, 0, 0], [0, 1, 0], [0, 0, 1]]) + 
    #     #     Matrix3x3([[1, 0, 0], [0, 1, 0], [0, 0, 1]]))
    #     # print(Matrix4x4([[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]]) + 
    #     #     Matrix4x4([[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]]))

    #     # Test engine access
    #     pos = self.transform.translation().local_pos()
    #     held_up = self.input.key_handler().is_held("up")
    #     held_right = self.input.key_handler().is_held("right")
    #     held_left = self.input.key_handler().is_held("left")
    #     held_down = self.input.key_handler().is_held("down")
    #     if held_up:
    #         print("Held up!")
    #         self.transform.translation().set_local_pos(
    #             pos + Vector3([0, 0, 0.1]))
    #         pos = self.transform.translation().local_pos()
    #     if held_down:
    #         print("Held down!")
    #         self.transform.translation().set_local_pos(pos +
    #          Vector3([0, 0, -0.1]))
    #         pos = self.transform.translation().local_pos()
    #     if held_right:
    #         print("Held right!")
    #         self.transform.translation().set_local_pos(pos +
    #          Vector3([0.1, 0, 0.0]))
    #         pos = self.transform.translation().local_pos()
    #     if held_left:
    #         print("Held left!")
    #         self.transform.translation().set_local_pos(pos +
    #          Vector3([-0.1, 0, 0.0]))
    #         pos = self.transform.translation().local_pos()

    #     if self.input.mouse_handler().moved():
    #         # print(self.input.mouse_handler().screen_pos())
    #         pass

    #     return 2

    # def late_update(self, delta_ms: int):
    #     # Camera operations go here
    #     ScriptBehavior.late_update(self, delta_ms)
    #     # print("Performing late update")
    #     return 2

    def fixed_update(self, delta_ms: int):

        # Set transform
        # array = Transform()
        # t_json = self.transform.as_json_str()
        # t_list = self.transform.translation().local_pos()
        # t_list  = t_list + 0.01
        # self.log_info(self.scene_object.get_name())

        # Test wrappers
        # t = self.transform
        # self.transform.translation().set_local_pos(t_list)
        # self.log_info(str(t))

        # q = Quaternion(0.0, 0.0, 0.0, 0.0)
        # q = Quaternion()
        q = Quaternion(0.0, 0.1, 0.1, 1.0)
        q2 = q * q
        # print(str(q2))
        # q *= q
        # print(q2)

        # Set rotation
        rotation = EulerAngles(0.0, 0.0005, 0.0, [0, 1, 2], 0)
        self.transform.add_rotation(rotation)

        # Set scale
        # s = self.transform.scale().get_scale()
        # s = [0.999 * e for e in s]
        # self.transform.scale().set_scale(s)

        # Test access to engine
        e = self.engine
        # print(str(self.scene_object))
        sc = e.scenario

        # print(Quaternion.__dict__)
        # self.log_info(str(q))

        # Test vectors
        # vec = Vector3()
        # v2 = Vector3() + Vector3([1, 2, 1])
        # v3 = Vector3() - Vector3([1, 2, 1])

        # Test moving an object in a circle
        # self.move_in_circle(self.transform, 0.1, (1, 0, 0))
        # self.move_in_circle(self.new_light.scene_object().transform(), 
        #                     0.01*delta_ms, 
        #                    (1, 0, 0),
        #                    circle_radius=2)


    # def move_in_circle(self, 
    #                    transform: TransformComponent, 
    #                    speed: float,
    #                    center: tuple = (0.0, 0.0, 0.0),
    #                    up: tuple = (0.0, 0.0, 1.0),
    #                    circle_radius: float = 1):
    #     """Move transform in a circle
        
    #     Args:
    #         speed: Rate at which to move transform
    #     """
    #     pos_vec = Vector3(transform.translation().local_pos())
    #     from_center = pos_vec - center
    #     up_vec = Vector3(up)
    #     dir_ = -1.0 * from_center.cross(up_vec)
    #     offset = dir_.normalized() * speed * circle_radius
    #     transform.translation().set_local_pos(pos_vec + offset)
    #     # print(transform.translation().local_pos())

    #     # X := originX + cos(angle)*radius;
    #     # Y := originY + sin(angle)*radius;