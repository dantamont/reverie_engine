#include "core/scripting/py_wrappers/GPyScenario.h"
#include "core/scene/GScenario.h"
#include "core/scene/GSceneObject.h"
#include "core/scene/GScene.h"
#include "core/GCoreEngine.h"

namespace rev {


void PyScenario::PyInitialize(py::module_ & m)
{
    // Initialize Scene class
    py::class_<Scenario, std::shared_ptr<Scenario>>(m, "Scenario")
        .def("__repr__", [](const Scenario& s) {return GJson::ToString<std::string>(json{ s }); })
        ;
}


}