#include "GPySkeletalAnimation.h"
#include "../../scene/GScenario.h"
#include "../../scene/GSceneObject.h"
#include "../../scene/GScene.h"
#include "../../GCoreEngine.h"

#include "../../components/GAnimationComponent.h"
#include "../../readers/GJsonReader.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PySkeletalAnimation::PyInitialize(py::module_ & m)
{
    py::class_<Motion>(m, "Motion")
        .def("__str__", [](const Motion& m) {JsonReader::ToString<std::string>(m.asJson()); })
        .def("__repr__", [](const Motion& m) {return m.asQString().toStdString(); })
        .def("move", [](Motion& m, const char* stateName) {
            //Object().logInfo("Moving motion to state " + GString(stateName));
            m.queueMove(stateName);
        })
        ;

    py::class_<BoneAnimationComponent, Component, std::unique_ptr<BoneAnimationComponent, py::nodelete>>(m, "SkeletalAnimation")
        .def(py::init<>(&PyComponent::Create<BoneAnimationComponent>))
        .def("__str__", [](const BoneAnimationComponent& b) {JsonReader::ToString<std::string>(b.asJson()); })
        .def("__repr__", [](const BoneAnimationComponent& b) {return b.asQString().toStdString(); })
        .def("get_motion", [](BoneAnimationComponent& b, const char* name) {return b.animationController().getMotion(name); }, py::return_value_policy::reference)
        ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
}