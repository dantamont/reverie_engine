#include "core/scripting/py_wrappers/GPyComponent.h"
#include "core/scene/GScenario.h"
#include "core/scene/GScene.h"
#include "core/scene/GSceneObject.h"
#include "core/GCoreEngine.h"
#include "core/components/GComponent.h"

namespace rev {


void PyComponent::PyInitialize(py::module_ & m)
{
    // Initialize Light class
    // Note, MUST be deallocated from the C++ side or there will be a memory leak
    py::class_<Component, std::unique_ptr<Component, py::nodelete>>(m, "Component")
        .def("scene_object", &Component::sceneObject)
        .def("scene", &Component::scene)
        ;
}



}