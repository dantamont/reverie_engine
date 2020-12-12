#include "GbPyScenario.h"
#include "../../scene/GbScenario.h"
#include "../../scene/GbSceneObject.h"
#include "../../scene/GbScene.h"
#include "../../GbCoreEngine.h"

namespace Gb {

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