#include "GPyAudioComponent.h"
#include "../GPythonAPI.h"
#include "../../scene/GScenario.h"
#include "../../scene/GSceneObject.h"
#include "../../scene/GScene.h"
#include "../../GCoreEngine.h"
#include "../../components/GAudioSourceComponent.h"
#include "../../readers/GJsonReader.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PyAudioSourceComponent::PyInitialize(py::module_ & m)
{
    // Initialize Light class
    py::class_<AudioSourceComponent, Component, std::unique_ptr<AudioSourceComponent, py::nodelete>>(m, "AudioSource")
        .def(py::init<>(&PyComponent::Create<AudioSourceComponent>))
        .def("__str__", [](const AudioSourceComponent& s) {return JsonReader::ToString<std::string>(s.asJson()); })
        .def("__repr__", [](const AudioSourceComponent& s) {return s.asQString().toStdString(); })
        .def("play", &AudioSourceComponent::queuePlay)
        ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
}