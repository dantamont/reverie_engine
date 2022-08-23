#include "core/scripting/py_wrappers/GPySceneObject.h"
#include "core/scene/GScenario.h"
#include "core/scene/GSceneObject.h"
#include "core/scene/GScene.h"
#include "core/GCoreEngine.h"
#include "fortress/json/GJson.h"

#include "core/components/GAnimationComponent.h"
#include "core/components/GAudioSourceComponent.h"
#include "core/components/GCameraComponent.h"
#include <core/components/GCharControlComponent.h>
#include <core/components/GRigidBodyComponent.h>
#include "core/components/GTransformComponent.h"

#include "ripple/network/messages/GScenarioJsonMessage.h"

namespace rev {

std::shared_ptr<SceneObject> PySceneObject::Create(Scene * scene)
{
    auto object = SceneObject::Create(scene);
    object->setScriptGenerated(true);
    
    /// @fixme Implement once path to message gateway is formalized
    // Queue message that scenario has changed
    //static GScenarioJsonMessage s_message;
    //json scenarioJson = *object->scene()->engine()->scenario();
    //s_message.setJsonBytes(GJson::ToBytes(scenarioJson));
    //queueMessageForSend();
    //
    return object;
}

std::shared_ptr<SceneObject> PySceneObject::Create(uint32_t id)
{
    std::shared_ptr<SceneObject> object = SceneObject::Get(id);
    //emit object->scene()->engine()->scenarioChanged();
    return object;
}

void PySceneObject::PyInitialize(pybind11::module_ & m)
{
    //py::class_<Serializable>(m, "Serializable")
    //    .def(py::init<>());
    //py::class_<Persistable>(m, "Persistable")
    //    .def(py::init<>());
    py::class_<DagNode<SceneObject>, std::shared_ptr<DagNode<SceneObject>> /* <- holder type */>(m, "SceneObjectDagNode");

    /// \see https://pybind11.readthedocs.io/en/stable/advanced/functions.html for return value policies
    // Initialize SceneObject class
    py::class_<SceneObject, std::shared_ptr<SceneObject> /* <- holder type */, SceneObjectDagNode//, Serializable, Persistable
    >(m, "SceneObject")
        //.def(py::init<>())
        .def(py::init<>(py::overload_cast<uint32_t>(&PySceneObject::Create)), py::return_value_policy::reference)
        .def(py::init<>(py::overload_cast<Scene*>(&PySceneObject::Create)), py::return_value_policy::reference)
        .def_property("name",
            [](const SceneObject& so) {return so.getName().c_str(); },
            &SceneObject::setName)
        .def("__str__", [](const SceneObject& so) {return GJson::ToString<std::string>(json{so}); })
        .def_property("parent",
            [](const SceneObject& so) {
                std::shared_ptr<SceneObject> p = so.parent();
                if (p) {
                    return so.parent();
                }
                else {
                    return std::shared_ptr<SceneObject>(nullptr);
                }
            },
            [](SceneObject& so, const std::shared_ptr<SceneObject>& parent) {
                so.setParent(parent);
            })
        .def("get_child", [](const SceneObject& so, const char* name) {return so.getChildByName(name); },
            py::return_value_policy::reference)
        .def_property_readonly("transform", [](const SceneObject& so) {return &so.transform(); }, py::return_value_policy::reference)
        .def_property_readonly("camera", [](const SceneObject& so) {return so.getComponent<CameraComponent>(ComponentType::kCamera); }, py::return_value_policy::reference)
        .def("audio_source", [](const SceneObject& so) {return so.getComponent<AudioSourceComponent>(ComponentType::kAudioSource); }, py::return_value_policy::reference)
        .def_property_readonly("char_controller", [](const SceneObject& so) {return so.getComponent<CharControlComponent>(ComponentType::kCharacterController); }, py::return_value_policy::reference)
        .def_property_readonly("scene", &SceneObject::scene, py::return_value_policy::reference)
        .def_property_readonly("skeletal_animation", [](const SceneObject& so) {return so.getComponent<BoneAnimationComponent>(ComponentType::kBoneAnimation); }, py::return_value_policy::reference)
        .def("destroy", &SceneObject::removeFromScene)
        ;
}



}