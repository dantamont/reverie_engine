#include "core/scripting/py_wrappers/GPyEngine.h"
#include "core/scene/GScenario.h"
#include "core/scene/GSceneObject.h"
#include "core/scene/GScene.h"
#include "core/GCoreEngine.h"
#include "core/resource/GResourceCache.h"

namespace rev {



void PyCoreEngine::PyInitialize(py::module_ & m)
{    
    // Initialize Engine class
    py::class_<CoreEngine>(m, "CoreEngine")
        .def_property_readonly("scenario", &CoreEngine::scenario)
        .def("__repr__", [](const CoreEngine* e) {return "CoreEngine"; })
        ;
}


}