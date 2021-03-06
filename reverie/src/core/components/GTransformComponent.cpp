///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GTransformComponent.h"

// Standard Includes

// External

// Project
#include "../GCoreEngine.h"
#include "../scene/GSceneObject.h"
#include "../scene/GScene.h"
#include "../scene/GScenario.h"
#include "../components/GCameraComponent.h"
#include "../components/GLightComponent.h"
#include "../components/GModelComponent.h"
#include "../components/GAnimationComponent.h"
#include <core/components/GCharControlComponent.h>
#include <core/components/GRigidBodyComponent.h>
#include "../components/GAudioSourceComponent.h"
#include "../components/GAudioListenerComponent.h"
#include "../physics/GPhysicsActor.h"
#include "../utils/GInterpolation.h"
#include "../rendering/buffers/GUniformBufferObject.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TransformComponent
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TransformComponent::TransformComponent() :
    Component(ComponentType::kTransform),
    Transform()
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TransformComponent::TransformComponent(const std::shared_ptr<SceneObject>& object) :
    Transform(),
    Component(object, ComponentType::kTransform)
{
    // Note: Cannot checkValidity with shared pointer in constructor.
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TransformComponent::~TransformComponent()
{
    // Parent class destructor will handle children
    //if (sceneObject()) {
    //    if (sceneObject()->getName().contains("cube")) {
    //        bool stop;
    //        stop = true;
    //    }
    //}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue TransformComponent::asJson(const SerializationContext& context) const
{
    QJsonObject object = Component::asJson(context).toObject();
    QJsonObject transformObject = Transform::asJson(context).toObject();

    for (const QString& key : transformObject.keys()) {
        object.insert(key, transformObject.value(key));
    }

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TransformComponent::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    Component::loadFromJson(json);
    Transform::loadFromJson(json);
    const QJsonObject& object = json.toObject();
    Q_UNUSED(object);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TransformComponent::initialize()
{
    // Set transform's parent if it has a scene object
    auto so = sceneObject();
    if (so) {
        if (so->parent()) {
            setParent(&so->parent()->transform());
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TransformComponent::set(const Transform & transform)
{
    Transform::operator=(transform);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TransformComponent::computeWorldMatrix()
{
    // Compute world matrix for this transform
    computeThisWorldMatrix();

    // If there is a scene object for the component (should always be the case)
    auto sceneObj = sceneObject();
    if (!sceneObj) {
        throw("Error, no scene object for transform component");
    }

    // Update view matrix if the scene object has a camera
    if (CameraComponent* cam = sceneObj->hasComponent<CameraComponent>(ComponentType::kCamera)) {
        cam->camera().updateViewMatrix(m_matrices.m_worldMatrix);

        // Update visible frustum bounds for the scene
        // TODO: Think up a more performant solution, maybe just set a flag to update view bounds in sim loop
        sceneObj->scene()->updateVisibleFrustumBounds();
    }

    // If the scene object has a light, update its position
    if (LightComponent* light = sceneObj->hasComponent<LightComponent>(ComponentType::kLight)) {
        light->setLightPosition(m_translation.getPosition());
    }

    // If has an audio source component, queue for move update
    if (sceneObj->hasComponent(ComponentType::kAudioSource)) {
        for (Component* comp : sceneObj->components()[(size_t)ComponentType::kAudioSource]) {
            AudioSourceComponent* audioComp = static_cast<AudioSourceComponent*>(comp);
            audioComp->queueMove();
        }
    }

    // If component has an audio listener, queue for move update
    if (auto* audioListener = sceneObj->hasComponent<AudioListenerComponent>(ComponentType::kAudioListener)) {
        audioListener->queueUpdate3d();
    }

    // Update the position of any rigid bodies on the object
    // TODO: What might work better is to set children as connected to parents with fixed joints
    if (auto* rigidBodyComp = sceneObj->hasComponent<RigidBodyComponent>(ComponentType::kRigidBody)) {
        rigidBodyComp->as<RigidBodyComponent>()->body()->setTransform(*static_cast<Transform*>(this));
    }

    // Update the bounding geometry for the scene object
    sceneObj->updateBounds(*this);


    // Update all child states
    for (const auto& child: m_children) {
        //auto childObject = std::static_pointer_cast<SceneObject>(childPair.second);
        //auto childTransform = childObject->transform();
        child->computeWorldMatrix();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing