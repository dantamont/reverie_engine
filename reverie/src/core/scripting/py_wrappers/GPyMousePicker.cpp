#include "GPyMousePicker.h"
#include "../../GCoreEngine.h"
//#include "../../input/GInputHandler.h"
#include <core/geometry/GMousePicking.h>
#include <core/scene/GSceneObject.h>
#include <core/mixins/GRenderable.h>

namespace rev {
void PyMousePicker::PyInitialize(py::module_ & m)
{
    // Mouse Picker
    py::class_<MousePicker>(m, "MousePicker")
        .def("__repr__", [](const MousePicker& m) {return "MousePicker"; })
        .def("is_mouse_over", py::overload_cast<SceneObject*>(&MousePicker::isMouseOver, py::const_))
        .def("is_mouse_over", py::overload_cast<Renderable*>(&MousePicker::isMouseOver, py::const_))
        ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
}