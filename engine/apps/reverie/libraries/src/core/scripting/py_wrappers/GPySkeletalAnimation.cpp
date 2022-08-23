#include "core/scripting/py_wrappers/GPySkeletalAnimation.h"
#include "core/scene/GScenario.h"
#include "core/scene/GSceneObject.h"
#include "core/scene/GScene.h"
#include "core/GCoreEngine.h"

#include "core/components/GAnimationComponent.h"
#include "fortress/json/GJson.h"

namespace rev {


void PySkeletalAnimation::PyInitialize(py::module_ & m)
{
    py::class_<Motion>(m, "Motion")
        .def("__str__", [](const Motion& m) {GJson::ToString<std::string>(json{m}); })
        .def("__repr__", [](const Motion& m) {return GJson::ToString<std::string>(json{ m }); })
        .def("move", [](Motion& m, const char* stateName) {
            //Object().logInfo("Moving motion to state " + GString(stateName));
            m.queueMove(stateName);
        })
        ;

    py::class_<BoneAnimationComponent, Component, std::unique_ptr<BoneAnimationComponent, py::nodelete>>(m, "SkeletalAnimation")
        .def(py::init<>(&PyComponent::Create<BoneAnimationComponent>))
        .def("__str__", [](const BoneAnimationComponent& b) {GJson::ToString<std::string>(json{b}); })
        .def("__repr__", [](const BoneAnimationComponent& b) { return GJson::ToString<std::string>(json{ b }); })
        .def("get_motion", [](BoneAnimationComponent& b, const char* name) {return b.animationController().getMotion(name); }, py::return_value_policy::reference)
        ;
}


}