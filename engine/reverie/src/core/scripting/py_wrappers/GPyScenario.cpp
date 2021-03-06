#include "GPyScenario.h"
#include "../../scene/GScenario.h"
#include "../../scene/GSceneObject.h"
#include "../../scene/GScene.h"
#include "../../GCoreEngine.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PyScenario::PyInitialize(py::module_ & m)
{
    // Initialize Scene class
    py::class_<Scenario, std::shared_ptr<Scenario>>(m, "Scenario")
        .def("__repr__", [](const Scenario& s) {return s.asQString().toStdString(); })
        ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
}