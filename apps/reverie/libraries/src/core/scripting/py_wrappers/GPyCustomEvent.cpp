#include "core/scripting/py_wrappers/GPyCustomEvent.h"
#include "fortress/json/GJson.h"
#include "core/scripting/GPythonAPI.h"

namespace rev {


void PyCustomEvent::PyInitialize(pybind11::module_ & m)
{

    // Initialize ScriptBehavior class, with trampoline helper
    py::class_<CustomEvent>(m, "CustomEvent")
        .def(py::init<>())
        .def("type", &CustomEvent::type)
        .def_property_readonly("data", [](const CustomEvent& s) {return PythonAPI::Instance().toPyDict(s.data()); })
        ;
}



}