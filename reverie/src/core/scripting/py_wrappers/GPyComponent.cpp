#include "GPyComponent.h"
#include "../../scene/GScenario.h"
#include "../../scene/GScene.h"
#include "../../scene/GSceneObject.h"
#include "../../GCoreEngine.h"
#include "../../components/GComponent.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PyComponent::PyInitialize(py::module_ & m)
{
    // Initialize Light class
    // Note, MUST be deallocated from the C++ side or there will be a memory leak
    py::class_<Component, std::unique_ptr<Component, py::nodelete>>(m, "Component")
        .def("scene_object", &Component::sceneObject)
        .def("scene", &Component::scene)
        ;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
}