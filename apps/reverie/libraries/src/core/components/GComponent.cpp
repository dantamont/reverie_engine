
// Includes

#include "core/components/GComponent.h"

// Standard Includes

// External

// Project
#include "core/GCoreEngine.h"
#include "core/scene/GSceneObject.h"
#include "core/scene/GScenario.h"
#include "core/scene/GScene.h"

#include "core/components/GAnimationComponent.h"
#include "core/components/GAudioListenerComponent.h"
#include "core/components/GAudioSourceComponent.h"
#include "core/components/GCanvasComponent.h"
#include "core/components/GCameraComponent.h"
#include "core/components/GCharControlComponent.h"
#include "core/components/GComponent.h"
#include "core/components/GCubeMapComponent.h"
#include "core/components/GLightComponent.h"
#include "core/components/GListenerComponent.h"
#include "core/components/GModelComponent.h"
#include "core/components/GPhysicsSceneComponent.h"
#include "core/components/GRigidBodyComponent.h"
#include "core/components/GScriptComponent.h"
#include "core/components/GShaderComponent.h"
#include "core/components/GTransformComponent.h"

namespace rev {

Component* Component::Create(Scene& scene, const json& json)
{
    ComponentType type = ComponentType(json["type"].get<Int32_t>());
    switch (type) {
    case ComponentType::kPhysicsScene:
        return new PhysicsSceneComponent(&scene, json);
    }
    return nullptr;
}

Component* Component::create(const std::shared_ptr<SceneObject>& object, ComponentType type)
{
    /// @todo Get rid of this and any other switch statements depending on component type
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
        Logger::Throw("loadFromJson:: Error, this type of component is not implemented");
#endif
        break;
    }

    // TODO: Add this to component widget instead
    //component->addRequiredComponents();

    return component;
}

Component* Component::create(const std::shared_ptr<SceneObject>& object, const json& json)
{
    ComponentType componentType = ComponentType(json.at("type").get<Int32_t>());
    Component* component = create(object, componentType);

    // Load component from json
    switch (componentType) {
    case ComponentType::kPythonScript:
        FromJson<ScriptComponent>(json, *component);
        break;
    case ComponentType::kLight:
        FromJson<LightComponent>(json, *component);
        break;
    case ComponentType::kCamera:
        FromJson<CameraComponent>(json, *component);
        break;
    case ComponentType::kShader:
        FromJson<ShaderComponent>(json, *component);
        break;
    case ComponentType::kTransform:
        // Transform is not stored in components list, since it is required
        break;
    case ComponentType::kModel:
        FromJson<ModelComponent>(json, *component);
        break;
    case ComponentType::kListener:
        FromJson<ListenerComponent>(json, *component);
        break;
    case ComponentType::kRigidBody:
        FromJson<RigidBodyComponent>(json, *component);
        break;
    case ComponentType::kCanvas:
        FromJson<CanvasComponent>(json, *component);
        break;
    case ComponentType::kCharacterController:
        FromJson<CharControlComponent>(json, *component);
        break;
    case ComponentType::kBoneAnimation:
        FromJson<BoneAnimationComponent>(json, *component);
        break;
    case ComponentType::kCubeMap:
        FromJson<CubeMapComponent>(json, *component);
        break;
    case ComponentType::kAudioSource:
        FromJson<AudioSourceComponent>(json, *component);
        break;
    case ComponentType::kAudioListener:
        FromJson<AudioListenerComponent>(json, *component);
        break;
    default:
#ifdef DEBUG_MODE
        Logger::Throw("loadFromJson:: Error, this type of component is not implemented");
#endif
        break;
    }

    return component;
}

Component::Component(ComponentType type, bool isScene):
    m_type(type),
    m_componentFlags(SceneBehaviorFlag::kEnabled),
    m_isSceneComponent(isScene)
{
}

Component::Component(CoreEngine* engine, ComponentType type, bool isScene):
    m_type(type),
    m_componentFlags(SceneBehaviorFlag::kEnabled),
    m_isSceneComponent(isScene)
{
}

Component::Component(const Component & other) :
    m_sceneObject(other.m_sceneObject),
    m_scene(other.m_scene),
    m_type(other.m_type),
    m_componentFlags(other.m_componentFlags),
    m_isSceneComponent(other.m_isSceneComponent)
{
}

Component::Component(const std::shared_ptr<SceneObject>& object, ComponentType type) :
    m_sceneObject(object),
    m_scene(object->scene()),
    m_type(type),
    m_componentFlags(SceneBehaviorFlag::kEnabled),
    m_isSceneComponent(false)
{
}

Component::Component(Scene* object, ComponentType type) :
    m_scene(object),
    m_type(type),
    m_componentFlags(SceneBehaviorFlag::kEnabled),
    m_isSceneComponent(true)
{
}

Component::~Component()
{
}

json Component::toJson() const
{
    const ComponentType& componentType = m_type;
    json outJson = json::object();

    // Load component from json
    switch (componentType) {
    case ComponentType::kPythonScript:
        ToJson<ScriptComponent>(outJson, *this);
        break;
    case ComponentType::kLight:
        ToJson<LightComponent>(outJson, *this);
        break;
    case ComponentType::kCamera:
        ToJson<CameraComponent>(outJson, *this);
        break;
    case ComponentType::kShader:
        ToJson<ShaderComponent>(outJson, *this);
        break;
    case ComponentType::kTransform:
        // Transform is not stored in components list, since it is required
        break;
    case ComponentType::kModel:
        ToJson<ModelComponent>(outJson, *this);
        break;
    case ComponentType::kListener:
        ToJson<ListenerComponent>(outJson, *this);
        break;
    case ComponentType::kRigidBody:
        ToJson<RigidBodyComponent>(outJson, *this);
        break;
    case ComponentType::kCanvas:
        ToJson<CanvasComponent>(outJson, *this);
        break;
    case ComponentType::kCharacterController:
        ToJson<CharControlComponent>(outJson, *this);
        break;
    case ComponentType::kBoneAnimation:
        ToJson<BoneAnimationComponent>(outJson, *this);
        break;
    case ComponentType::kCubeMap:
        ToJson<CubeMapComponent>(outJson, *this);
        break;
    case ComponentType::kAudioSource:
        ToJson<AudioSourceComponent>(outJson, *this);
        break;
    case ComponentType::kAudioListener:
        ToJson<AudioListenerComponent>(outJson, *this);
        break;
    default:
#ifdef DEBUG_MODE
        Logger::Throw("toJson:: Error, this type of component is not implemented");
#endif
        break;
    }

    return outJson;
}

void Component::toggle(int enable_)
{
    if (enable_ > 0) {
        enable();
    }
    else {
        disable();
    }
}

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

void Component::setSceneObject(const std::shared_ptr<SceneObject>& object) {
    if (m_isSceneComponent) Logger::Throw("Error, cannot set scene object for scene component");
    m_sceneObject = object;
}

void Component::setScene(Scene* object) {
    if (!m_isSceneComponent) Logger::Throw("Error, cannot set scene for scene object component");
    m_scene = object;
}

void to_json(json& orJson, const Component& korObject)
{
    orJson["type"] = (int)korObject.m_type;
    orJson["isEnabled"] = korObject.isEnabled();
    orJson["id"] = korObject.getUuid(); ///< Only used for widgets, not from_json
}

void from_json(const json& korJson, Component& orObject)
{
    if (korJson.contains("isEnabled")) {
        orObject.m_componentFlags.setFlag(SceneBehaviorFlag::kEnabled, korJson.at("isEnabled").get<bool>());
    }

    if (orObject.sceneObject()) {
        // Need this check because might be a scene component
        orObject.toggle(orObject.isEnabled() && orObject.sceneObject()->isEnabled());
    }
    else {
        orObject.toggle(orObject.isEnabled());
    }
}



} // end namespacing
