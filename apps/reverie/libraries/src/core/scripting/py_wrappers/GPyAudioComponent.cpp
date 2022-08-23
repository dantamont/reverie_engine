#include "core/scripting/py_wrappers/GPyAudioComponent.h"
#include "core/scripting/GPythonAPI.h"
#include "core/scene/GScenario.h"
#include "core/scene/GSceneObject.h"
#include "core/scene/GScene.h"
#include "core/GCoreEngine.h"
#include "core/components/GAudioSourceComponent.h"
#include "fortress/json/GJson.h"

namespace rev {


void PyAudioSourceComponent::PyInitialize(py::module_ & m)
{
    // Initialize Light class
    py::class_<AudioSourceComponent, Component, std::unique_ptr<AudioSourceComponent, py::nodelete>>(m, "AudioSource")
        .def(py::init<>(&PyComponent::Create<AudioSourceComponent>))
        .def("__str__", [](const AudioSourceComponent& s) {return GJson::ToString<std::string>(json{s}); })
        .def("__repr__", [](const AudioSourceComponent& s) {return GJson::ToString<std::string>(json{ s }); })
        .def("play", &AudioSourceComponent::queuePlay)
        ;
}


}