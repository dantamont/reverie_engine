///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GbComponent.h"

// Standard Includes

// External

// Project
#include "../GbCoreEngine.h"
#include "../scene/GbSceneObject.h"
#include "../scene/GbScenario.h"
#include "../scene/GbScene.h"

#include "../components/GbTransformComponent.h"
#include "../components/GbComponent.h"
#include "../components/GbScriptComponent.h"
#include "../components/GbRendererComponent.h"
#include "../components/GbCamera.h"
#include "../components/GbLight.h"
#include "../components/GbModelComponent.h"
#include "../components/GbListenerComponent.h"
#include "../components/GbPhysicsComponents.h"
#include "../components/GbCanvasComponent.h"
#include "../components/GbAnimationComponent.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Component
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Component::Requirements Component::GetRequirements(ComponentType type)
{
    if (TypeRequirements.find(type) != TypeRequirements.end()) {
        return TypeRequirements.at(type);
    }
    else {
        return Requirements();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Component* Component::create(const std::shared_ptr<SceneObject>& object, ComponentType type)
{
    // Instantiate/load component
    Component* component;
    switch (type) {
    case Component::kPythonScript:
        component = new ScriptComponent(object);
        break;
    case Component::kLight:
        component = new Light(object);
        break;
    case Component::kCamera:
        component = new CameraComponent(object);
        break;
    case Component::kRenderer:
        component = new RendererComponent(object);
        break;
    case Component::kTransform:
        // Transform is not stored in components list, since it is required
        component = nullptr;
        break;
    case Component::kModel:
        component = new ModelComponent(object);
        break;
    case Component::kListener:
        component = new ListenerComponent(object);
        break;
    case Component::kRigidBody:
        component = new RigidBodyComponent(object);
        break;
    case Component::kCanvas:
        component = new CanvasComponent(object);
        break;
    case Component::kCharacterController:
        component = new CharControlComponent(object);
        break;
    case Component::kBoneAnimation:
        component = new BoneAnimationComponent(object);
        break;
    default:
#ifdef DEBUG_MODE
        throw("loadFromJson:: Error, this type of component is not implemented");
#endif
    }

    return component;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Component* Component::create(const std::shared_ptr<SceneObject>& object, const QJsonObject & json)
{
    Component::ComponentType componentType = Component::ComponentType(json.value("type").toInt());
    auto component = create(object, componentType);
    if(component)
        // Component is nullptr if is transform type
        component->loadFromJson(json);
    return component;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Component::Component(ComponentType type, bool isScene):
    m_engine(nullptr),
    m_type(type),
    m_isEnabled(true),
    m_isSceneComponent(isScene)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Component::Component(CoreEngine* engine, ComponentType type, bool isScene):
    m_engine(engine),
    m_type(type),
    m_isEnabled(true),
    m_isSceneComponent(isScene)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Component::Component(const Component & other) :
    m_object(other.m_object),
    m_engine(other.m_engine),
    m_type(other.m_type),
    m_isEnabled(other.m_isEnabled),
    m_isSceneComponent(other.m_isSceneComponent)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Component::Component(const std::shared_ptr<SceneObject>& object, ComponentType type) :
    Object(),
    m_object(object),
    m_engine(object->engine()),
    m_type(type),
    m_isEnabled(true),
    m_isSceneComponent(false)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Component::Component(const std::shared_ptr<Scene>& object, ComponentType type) :
    Object(),
    m_object(object),
    m_engine(object->scenario()->engine()),
    m_type(type),
    m_isEnabled(true),
    m_isSceneComponent(true)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Component::~Component()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Component::toggle(int enable_)
{
    if (enable_ > 0) {
        enable();
    }
    else {
        disable();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SceneObject> Component::sceneObject() const
{
    if (m_isSceneComponent) return nullptr;
    if (std::shared_ptr<Object> object = m_object.lock()) {
        return std::static_pointer_cast<SceneObject>(object);
    }
    else {
        return nullptr;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Scene> Component::scene() const
{
    if (!m_isSceneComponent) return nullptr;
    if (std::shared_ptr<Object> object = m_object.lock()) {
        return std::static_pointer_cast<Scene>(object);
    }
    else {
        return nullptr;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Component::setSceneObject(const std::shared_ptr<SceneObject>& object) {
    if (m_isSceneComponent) throw("Error, cannot set scene object for scene component");
    m_object = object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Component::setScene(std::shared_ptr<Scene> object) {
    if (!m_isSceneComponent) throw("Error, cannot set scene for scene object component");
    m_object = object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Component::asJson() const
{
    QJsonObject object;
    object.insert("type", m_type);
    object.insert("isEnabled", m_isEnabled);

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Component::loadFromJson(const QJsonValue & json)
{
    QJsonObject object = json.toObject();
    if (object.contains("isEnabled")) {
        m_isEnabled = object["isEnabled"].toBool();
    }

    toggle(m_isEnabled);

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::unordered_map<Component::ComponentType, Component::Requirements> Component::TypeRequirements =
{ 
    {kBoneAnimation, { {Component::kModel} }}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing
