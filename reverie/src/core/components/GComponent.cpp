///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GComponent.h"

// Standard Includes

// External

// Project
#include "../GCoreEngine.h"
#include "../scene/GSceneObject.h"
#include "../scene/GScenario.h"
#include "../scene/GScene.h"

#include "../components/GTransformComponent.h"
#include "../components/GComponent.h"
#include "../components/GScriptComponent.h"
#include "../components/GShaderComponent.h"
#include "../components/GCameraComponent.h"
#include "../components/GLightComponent.h"
#include "../components/GModelComponent.h"
#include "../components/GListenerComponent.h"
#include "../components/GCharControlComponent.h"
#include "../components/GRigidBodyComponent.h"
#include "../components/GCanvasComponent.h"
#include "../components/GAnimationComponent.h"
#include "../components/GCubeMapComponent.h"
#include "../components/GAudioSourceComponent.h"
#include "../components/GAudioListenerComponent.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Component
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Component::Constraints Component::GetRequirements(ComponentType type)
{
    if (TypeConstraints.find(type) != TypeConstraints.end()) {
        return TypeConstraints.at(type);
    }
    else {
        return Constraints();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Component* Component::create(const std::shared_ptr<SceneObject>& object, ComponentType type)
{
    // Instantiate/load component
    Component* component;
    switch (type) {
    case ComponentType::kPythonScript:
        component = new ScriptComponent(object);
        break;
    case ComponentType::kLight:
        component = new LightComponent(object);
        break;
    case ComponentType::kCamera:
        component = new CameraComponent(object);
        break;
    case ComponentType::kShader:
        component = new ShaderComponent(object);
        break;
    case ComponentType::kTransform:
        // Transform is not stored in components list, since it is required
        component = nullptr;
        break;
    case ComponentType::kModel:
        component = new ModelComponent(object);
        break;
    case ComponentType::kListener:
        component = new ListenerComponent(object);
        break;
    case ComponentType::kRigidBody:
        component = new RigidBodyComponent(object);
        break;
    case ComponentType::kCanvas:
        component = new CanvasComponent(object);
        break;
    case ComponentType::kCharacterController:
        component = new CharControlComponent(object);
        break;
    case ComponentType::kBoneAnimation:
        component = new BoneAnimationComponent(object);
        break;
    case ComponentType::kCubeMap:
    {
        component = new CubeMapComponent(object);
        break;
    }
    case ComponentType::kAudioSource:
    {
        component = new AudioSourceComponent(object);
        break;
    }
    case ComponentType::kAudioListener:
    {
        component = new AudioListenerComponent(object);
        break;
    }
    default:
#ifdef DEBUG_MODE
        throw("loadFromJson:: Error, this type of component is not implemented");
#endif
        break;
    }

    // TODO: Add this to component widget instead
    //component->addRequiredComponents();

    return component;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Component* Component::create(const std::shared_ptr<SceneObject>& object, const QJsonObject & json)
{
    ComponentType componentType = ComponentType(json.value("type").toInt());
    auto component = create(object, componentType);
    if (component) {
        // Component is nullptr if is transform type
        component->loadFromJson(json);
    }
    return component;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Component::Component(ComponentType type, bool isScene):
    m_type(type),
    m_componentFlags(SceneBehaviorFlag::kEnabled),
    m_isSceneComponent(isScene)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Component::Component(CoreEngine* engine, ComponentType type, bool isScene):
    m_type(type),
    m_componentFlags(SceneBehaviorFlag::kEnabled),
    m_isSceneComponent(isScene)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Component::Component(const Component & other) :
    m_sceneObject(other.m_sceneObject),
    m_scene(other.m_scene),
    m_type(other.m_type),
    m_componentFlags(other.m_componentFlags),
    m_isSceneComponent(other.m_isSceneComponent)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Component::Component(const std::shared_ptr<SceneObject>& object, ComponentType type) :
    Object(),
    m_sceneObject(object),
    m_scene(object->scene()),
    m_type(type),
    m_componentFlags(SceneBehaviorFlag::kEnabled),
    m_isSceneComponent(false)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Component::Component(Scene* object, ComponentType type) :
    Object(),
    m_scene(object),
    m_type(type),
    m_componentFlags(SceneBehaviorFlag::kEnabled),
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
    if (std::shared_ptr<SceneObject> object = m_sceneObject.lock()) {
        return object;
    }
    else {
        return nullptr;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Component::setSceneObject(const std::shared_ptr<SceneObject>& object) {
    if (m_isSceneComponent) throw("Error, cannot set scene object for scene component");
    m_sceneObject = object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Component::setScene(Scene* object) {
    if (!m_isSceneComponent) throw("Error, cannot set scene for scene object component");
    m_scene = object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Component::asJson(const SerializationContext& context) const
{
    QJsonObject object;
    object.insert("type", (int)m_type);
    object.insert("isEnabled", isEnabled());

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Component::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    QJsonObject object = json.toObject();
    if (object.contains("isEnabled")) {
        m_componentFlags.setFlag(SceneBehaviorFlag::kEnabled, object["isEnabled"].toBool());
    }

    if (sceneObject()) {
        // Need this check because might be a scene component
        toggle(isEnabled() && sceneObject()->isEnabled());
    }
    else {
        toggle(isEnabled());
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
tsl::robin_map<ComponentType, Component::Constraints> Component::TypeConstraints =
{ 
    // TODO: Enforce these in widget
    {ComponentType::kCanvas, {{ {ComponentType::kModel, false} }}}, // Only one material per object, so either have a canvas or model
    {ComponentType::kModel,  {{ {ComponentType::kCanvas, false} }}}, // Only one material per object, so either have a canvas or model
    {ComponentType::kBoneAnimation, {{ {ComponentType::kModel, true} }}}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing
