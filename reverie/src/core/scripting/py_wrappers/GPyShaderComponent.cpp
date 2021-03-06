#include "GPyShaderComponent.h"
#include "../GPythonAPI.h"
#include "../../scene/GScenario.h"
#include "../../scene/GSceneObject.h"
#include "../../scene/GScene.h"
#include "../../GCoreEngine.h"
#include "../../components/GShaderComponent.h"
#include "../../readers/GJsonReader.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PyShaderComponent::PyInitialize(py::module_ & m)
{
    // Initialize Light class
    py::class_<ShaderComponent, Component, std::unique_ptr<ShaderComponent, py::nodelete>>(m, "ShaderComponent")
        .def(py::init<>(&PyComponent::Create<ShaderComponent>))
        .def("__str__", [](const ShaderComponent& s) {JsonReader::ToString<std::string>(s.asJson()); })
        .def("__repr__", [](const ShaderComponent& s) {s.asQString().toStdString(); })
        ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
}