#include "GPyEngine.h"
#include "../../scene/GScenario.h"
#include "../../scene/GSceneObject.h"
#include "../../scene/GScene.h"
#include "../../GCoreEngine.h"
#include "../../resource/GResourceCache.h"

namespace rev {


//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PyCoreEngine::PyInitialize(py::module_ & m)
{    
    // Initialize Engine class
    py::class_<CoreEngine>(m, "CoreEngine")
        .def_property_readonly("scenario", &CoreEngine::scenario)
        .def_property_readonly("resource_cache", &CoreEngine::resourceCache)
        .def("__repr__", [](const CoreEngine* e) {return e->asQString().toStdString(); })
        ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
}