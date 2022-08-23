#include "core/scripting/py_wrappers/GPyShaderComponent.h"
#include "core/scripting/GPythonAPI.h"
#include "core/scene/GScenario.h"
#include "core/scene/GSceneObject.h"
#include "core/scene/GScene.h"
#include "core/GCoreEngine.h"
#include "core/components/GShaderComponent.h"
#include "fortress/json/GJson.h"

namespace rev {


void PyShaderComponent::PyInitialize(py::module_ & m)
{
    // Initialize Light class
    py::class_<ShaderComponent, Component, std::unique_ptr<ShaderComponent, py::nodelete>>(m, "ShaderComponent")
        .def(py::init<>(&PyComponent::Create<ShaderComponent>))
        .def("__str__", [](const ShaderComponent& s) {return GJson::ToString<std::string>(json{s}); })
        .def("__repr__", [](const ShaderComponent& s) {return GJson::ToString<std::string>(json{ s }); })
        ;
}


}