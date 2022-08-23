#include "core/scripting/py_wrappers/GPyCamera.h"
#include "core/scene/GScenario.h"
#include "core/scene/GSceneObject.h"
#include "core/scene/GScene.h"
#include "core/GCoreEngine.h"

#include "core/components/GCameraComponent.h"
#include "core/components/GTransformComponent.h"
#include "fortress/json/GJson.h"
#include "core/scripting/py_wrappers/GPyMousePicker.h"

namespace rev {


void PyCamera::PyInitialize(py::module_ & m)
{
    using namespace pybind11::literals;

    // Initialize MousePicker wrapper
    PyMousePicker::PyInitialize(m);

    // Initialize Camera wrapper
    py::class_<CameraComponent, Component, std::unique_ptr<CameraComponent, py::nodelete>>(m, "Camera")
        .def(py::init<>(&PyComponent::Create<CameraComponent>))
        .def("__str__", [](const CameraComponent& c) {return GJson::ToString<std::string>(json{c}); })
        .def("__repr__", [](const CameraComponent& c) {return GJson::ToString<std::string>(json{ c }); })
        .def("rotate_about_point", 
            [](CameraComponent& c, const Vector3& point, const Vector2& mouse_delta, double speed_factor = 1.0) {
                c.camera().rotateAboutPoint(point, mouse_delta, speed_factor);
            },
            "point"_a, "mouse_delta"_a, "speed_factor"_a = 1.0 // Default argument
        )
        .def("look_at",
            [](CameraComponent& c, const Transform& transform, const Vector3& up = Vector3::Up()) {
                c.camera().setLookAt(c.camera().eye(), transform.getPosition(), up);
            },
            "transform"_a, "up"_a = Vector3::Up() // Default argument
            )
        .def("look_at",
            [](CameraComponent& c, const Vector3& pos, const Vector3& up = Vector3::Up()) {
                c.camera().setLookAt(c.camera().eye(), pos, up);
            },
            "pos"_a, "up"_a = Vector3::Up() // Default argument
            )
        .def("forward", [](const CameraComponent& c) {
                // We want forward to mean the look-at direction, which is the opposite of getForwardVec's convention
                return -c.camera().getForwardVec();
            })
        .def("right", [](const CameraComponent& c) {return c.camera().getRightVec(); })
        .def("up", [](const CameraComponent& c) {return c.camera().getUpVec(); })
        .def("set_viewport",
            [](CameraComponent& c, double x = 0, double y = 0, double w = 1.0, double h = 1.0) {
                c.camera().viewport().m_xn = x;
                c.camera().viewport().m_yn = y;
                c.camera().viewport().m_width = w;
                c.camera().viewport().m_height = h;
            },
            "x"_a = 0.0, "y"_a = 0.0, "w"_a = 1.0, "h"_a = 1.0
            )
        .def("set_depth",
            [](CameraComponent& self, int depth = 0) {
                self.camera().viewport().setDepth(depth);
            },
            "depth"_a = 0
            )
        .def("set_fov",
            [](CameraComponent& self, double fov_deg) {
                self.camera().renderProjection().setFOV(fov_deg);
            },
            "fov_deg"_a = 90.0
            )
        .def_property_readonly("mouse_picker", 
            [](CameraComponent& self) {
                return self.camera().mousePicker();
            })
        ;
}


}