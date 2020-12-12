#include "GbPySceneObject.h"
#include "../../scene/GbScenario.h"
#include "../../scene/GbSceneObject.h"
#include "../../scene/GbScene.h"
#include "../../GbCoreEngine.h"
#include "../../readers/GbJsonReader.h"

#include "../../components/GbAnimationComponent.h"
#include "../../components/GbAudioSourceComponent.h"
#include "../../components/GbCameraComponent.h"
#include "../../components/GbPhysicsComponents.h"
#include "../../components/GbTransformComponent.h"

namespace Gb {
//////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SceneObject> PySceneObject::Create(Scene * scene)
{
    std::shared_ptr<Scene> scenePtr = scene->scenario()->getScene(scene->getUuid());
    auto object = SceneObject::create(scenePtr);
    object->setIsPythonGenerated(true);
        
    // Emit signal that scenario has changed
    emit object->engine()->scenarioChanged();
    
    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SceneObject> PySceneObject::Create(const char * uuidStr)
{
    std::shared_ptr<SceneObject> object = SceneObject::get(uuidStr);
    emit object->engine()->scenarioChanged();
    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PySceneObject::PyInitialize(pybind11::module_ & m)
{
    //py::class_<Serializable>(m, "Serializable")
    //    .def(py::init<>());
    //py::class_<Persistable>(m, "Persistable")
    //    .def(py::init<>());
    py::class_<DagNode<SceneObject>, std::shared_ptr<DagNode<SceneObject>> /* <- holder type */>(m, "SceneObjectDagNode");

    // Initialize SceneObject class
    py::class_<SceneObject, std::shared_ptr<SceneObject> /* <- holder type */, SceneObjectDagNode//, Serializable, Persistable
    >(m, "SceneObject")
        .def(py::init<>())
        .def(py::init<>(py::overload_cast<const char *>(&PySceneObject::Create)))
        .def(py::init<>(py::overload_cast<Scene*>(&PySceneObject::Create)))
        .def_property("name",
            [](const SceneObject& so) {return so.getName().c_str(); },
            &SceneObject::setName)
        .def("__str__", [](const SceneObject& so) {return JsonReader::ToGString(so.asJson()).toStdString(); })
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
        .def("get_child", [](const SceneObject& so, const char* name) {return so.getChildByName(name); })
        .def_property_readonly("transform", &SceneObject::transform)
        .def_property_readonly("camera", &SceneObject::camera)
        .def("audio_source", &SceneObject::audioSource)
        .def_property_readonly("char_controller", &SceneObject::characterController)
        .def_property_readonly("scene", &SceneObject::scene)
        .def("destroy", &SceneObject::removeFromScene)
        ;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
}