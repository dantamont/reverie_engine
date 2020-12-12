#include "GbPySceneObject.h"
#include "../../scene/GbScenario.h"
#include "../../scene/GbSceneObject.h"
#include "../../scene/GbScene.h"
#include "../../GbCoreEngine.h"
#include "GbPyEngine.h"

namespace Gb {


//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PyCoreEngine::PyInitialize(py::module_ & m)
{    
    // Initialize Engine class
    py::class_<CoreEngine>(m, "CoreEngine")
        //.def_property_readonly("scenario")
        //.def_property_readonly("input_handler")
        //.def_property_readonly("resource_cache")
        .def("__repr__", [](const CoreEngine* e) {return e->asQString().toStdString(); })
        ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
}