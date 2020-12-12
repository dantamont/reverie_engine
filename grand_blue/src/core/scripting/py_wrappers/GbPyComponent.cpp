#include "GbPyComponent.h"
#include "../../scene/GbScenario.h"
#include "../../scene/GbScene.h"
#include "../../scene/GbSceneObject.h"
#include "../../GbCoreEngine.h"
#include "../../components/GbComponent.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PyComponent::PyInitialize(py::module_ & m)
{
    // Initialize Light class
    py::class_<Component>(m, "Component")
        .def("scene_object", &Component::sceneObject)
        .def("scene", &Component::scene)
        ;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
}