// Includes
#include "core/components/GTransformComponent.h"

// Standard Includes

// External

// Project
#include "core/GCoreEngine.h"
#include "core/scene/GSceneObject.h"
#include "core/scene/GScene.h"
#include "core/scene/GScenario.h"
#include "core/components/GCameraComponent.h"
#include "core/components/GLightComponent.h"
#include "core/components/GModelComponent.h"
#include "core/components/GAnimationComponent.h"
#include <core/components/GCharControlComponent.h>
#include <core/components/GRigidBodyComponent.h>
#include "core/components/GAudioSourceComponent.h"
#include "core/components/GAudioListenerComponent.h"
#include "core/physics/GPhysicsActor.h"
#include "fortress/math/GInterpolation.h"
#include "core/rendering/buffers/GUniformBufferObject.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"


// Namespace Definitions

namespace rev {

TransformComponent::TransformComponent() :
    IndexedTransform(),
    Component(ComponentType::kTransform)
{
}

TransformComponent::TransformComponent(const std::shared_ptr<SceneObject>& object, std::vector<Matrix4x4>& worldMatrixVec, Uint32_t index) :
    IndexedTransform(worldMatrixVec, index),
    Component(object, ComponentType::kTransform)
{
    // Note: Cannot checkValidity with shared pointer in constructor.
    initialize();
}

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

void to_json(json& orJson, const TransformComponent& korObject)
{
    orJson = json::object();
    ToJson<Component>(orJson, korObject);
    ToJson<IndexedTransform>(orJson, korObject);
}

void from_json(const json& korJson, TransformComponent& orObject)
{
    FromJson<Component>(korJson, orObject);
    FromJson<IndexedTransform>(korJson, orObject);
}

void TransformComponent::initialize()
{
    auto so = sceneObject();
    if (so) {
        // Set transform's parent if it has a scene object
        if (so->parent()) {
            setParent(&so->parent()->transform());
        }
    }
}

void TransformComponent::set(const IndexedTransform& transform)
{
    IndexedTransform::operator=(transform);
}

void TransformComponent::computeWorldMatrix()
{
    // Compute world matrix for this transform
    computeThisWorldMatrix();

    // If there is a scene object for the component (should always be the case)
    auto sceneObj = sceneObject();
    if (!sceneObj) {
        Logger::Throw("Error, no scene object for transform component");
    }

    // Update view matrix if the scene object has a camera
    if (CameraComponent* cam = sceneObj->getComponent<CameraComponent>(ComponentType::kCamera)) {
        cam->camera().updateViewMatrix(m_matrices.worldMatrix());

        // Update visible frustum bounds for the scene
        // TODO: Think up a more performant solution, maybe just set a flag to update view bounds in sim loop
        sceneObj->scene()->updateVisibleFrustumBounds();
    }

    // If the scene object has a light, update its position
    if (LightComponent* light = sceneObj->getComponent<LightComponent>(ComponentType::kLight)) {
        light->setLightPosition(m_translation.getPosition());
    }

    // If has an audio source component, queue for move update
    AudioSourceComponent* audioComp = sceneObj->getComponent<AudioSourceComponent>(ComponentType::kAudioSource);
    if (audioComp) {
        audioComp->queueMove();
    }

    // If component has an audio listener, queue for move update
    if (auto* audioListener = sceneObj->getComponent<AudioListenerComponent>(ComponentType::kAudioListener)) {
        audioListener->queueUpdate3d();
    }

    // Update the position of any rigid bodies on the object
    // TODO: What might work better is to set children as connected to parents with fixed joints
    if (auto* rigidBodyComp = sceneObj->getComponent<RigidBodyComponent>(ComponentType::kRigidBody)) {
        rigidBodyComp->as<RigidBodyComponent>()->body()->setTransform(*this);
    }

    // Set the world matrix uniform
    RenderContext& context = sceneObj->scene()->engine()->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();
    m_uniforms.m_worldMatrix.setValue(m_matrices.worldMatrix(), uc);

    // Update the bounding geometry for the scene object
    sceneObj->updateBounds(*this);

    // Set flag that scene object was moved
    sceneObj->setMoved(true);

    // Update all child states
    for (const auto& child: m_children) {
        //auto childObject = std::static_pointer_cast<SceneObject>(childPair.second);
        //auto childTransform = childObject->transform();
        child->computeWorldMatrix();
    }
}


} // end namespacing