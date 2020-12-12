#include "GbPyShaderComponent.h"
#include "../GbPythonAPI.h"
#include "../../scene/GbScenario.h"
#include "../../scene/GbSceneObject.h"
#include "../../scene/GbScene.h"
#include "../../GbCoreEngine.h"
#include "../../components/GbShaderComponent.h"
#include "../../readers/GbJsonReader.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PyShaderComponent::PyInitialize(py::module_ & m)
{
    // Initialize Light class
    py::class_<ShaderComponent, Component>(m, "ShaderComponent")
        .def(py::init<>(&PyComponent::Create<ShaderComponent>))
        .def("__str__", [](const ShaderComponent& s) {JsonReader::ToGString(s.asJson()).toStdString(); })
        ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
}