///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GbTransformComponent.h"

// Standard Includes

// External

// Project
#include "../GbCoreEngine.h"
#include "../scene/GbSceneObject.h"
#include "../scene/GbScene.h"
#include "../scene/GbScenario.h"
#include "../components/GbCameraComponent.h"
#include "../components/GbLightComponent.h"
#include "../components/GbModelComponent.h"
#include "../components/GbAnimationComponent.h"
#include "../components/GbPhysicsComponents.h"
#include "../components/GbAudioSourceComponent.h"
#include "../components/GbAudioListenerComponent.h"
#include "../physics/GbPhysicsActor.h"
#include "../utils/GbInterpolation.h"
#include "../rendering/buffers/GbUniformBufferObject.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TransformComponent
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TransformComponent::TransformComponent() :
    Component(ComponentType::kTransform),
    Transform()
{
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
QJsonValue TransformComponent::asJson() const
{
    QJsonObject object = Component::asJson().toObject();
    QJsonObject transformObject = Transform::asJson().toObject();

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
    Transform::initialize();

    // Set transform's parent if it has a scene object
    auto so = sceneObject();
    if (so) {
        if (so->parent()) {
            setParent(so->parent()->transform().get());
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
    // Compute local matrix
    computeLocalMatrix();

    // Initialize model matrix using model matrices of parent as a base
    if (!m_parent) {
        m_matrices.m_worldMatrix = m_matrices.m_localMatrix;
    }
    else {
        m_matrices.m_worldMatrix.setToIdentity();
        switch (m_inheritanceType) {
        case Transform::kTranslation:
            m_matrices.m_worldMatrix.setTranslation(parentTransform()->worldTranslation().getPosition().asReal());
            //m_worldMatrix = parentTransform()->worldTranslation().getMatrix();
            break;
        case Transform::kAll:
        case Transform::kPreserveOrientation:
        default:
            m_matrices.m_worldMatrix = parentTransform()->worldMatrix();
            break;
        }

        // Construct model matrix for each inheritance type
        switch (m_inheritanceType) {
        case Transform::kPreserveOrientation:
        {
            // Obtain world translation by removing the rotation component of the world matrix
            Matrix4x4 localTranslation;
            localTranslation.setTranslation(m_translation.getPosition().asReal());
            Matrix4x4 translation = m_matrices.m_worldMatrix * localTranslation;
            translation = translation.getTranslationMatrix();

            // Compute world matrix that preserves original orientation of state w.r.t. inertial frame
            m_matrices.m_worldMatrix = translation * m_rotation.getMatrix() * m_scale.getMatrix();
        }
        case Transform::kTranslation:
        case Transform::kAll:
        default:
            // Compute world matrix
            m_matrices.m_worldMatrix *= m_matrices.m_localMatrix;
            break;
        }
    }

    // If there is a scene object for the component (should always be the case)
    auto sceneObj = sceneObject();
    if (!sceneObj) {
        throw("Error, no scene object for transform component");
    }

    // Update view matrix if the scene object has a camera
    if (sceneObj->hasCamera()) {
        CameraComponent* cam = sceneObj->camera();
        cam->camera().updateViewMatrix(m_matrices.m_worldMatrix);

        // Update visible frustum bounds for the scene
        // TODO: Think up a more performant solution, maybe just set a flag to update view bounds in sim loop
        sceneObj->scene()->updateVisibleFrustumBounds();
    }

    // If the scene object has a light, update its position
    if (sceneObj->hasComponent(ComponentType::kLight)) {
        sceneObj->light()->setLightPosition(m_translation.getPosition());
    }

    // If has an audio source component, queue for move update
    if (sceneObj->hasComponent(ComponentType::kAudioSource)) {
        for (Component* comp : sceneObj->components()[(size_t)ComponentType::kAudioSource]) {
            AudioSourceComponent* audioComp = static_cast<AudioSourceComponent*>(comp);
            audioComp->queueMove();
        }
    }

    // If component has an audio listener, queue for move update
    if (sceneObj->hasComponent(ComponentType::kAudioListener)) {
        AudioListenerComponent* audioListener = static_cast<AudioListenerComponent*>(
            sceneObj->components()[(size_t)ComponentType::kAudioListener][0]);
        audioListener->queueUpdate3d();
    }

    // Update the position of any rigid bodies on the object
    if (sceneObj->hasComponent(ComponentType::kRigidBody)) {
		std::vector<Component*>& rigidBodies = sceneObj->components()[(int)Component::ComponentType::kRigidBody];
        for (auto& comp : rigidBodies) {
            RigidBodyComponent* rigidBodyComp = static_cast<RigidBodyComponent*>(comp);
            rigidBodyComp->body()->setTransform(*static_cast<Transform*>(this));
        }
    }

    // Update the bounding geometry for any models on this object
    if (sceneObj->hasComponent(ComponentType::kModel)) 
    {
        if (sceneObj->hasComponent(ComponentType::kBoneAnimation)) {
            // Use skeleton to set bounding box if there is an animation component
            BoneAnimationComponent* animComp = sceneObj->boneAnimationComponent();
            animComp->updateBounds(*this);
        }
        else {
            // Use model chunks to set bounding boxes if there's no animation component
            ModelComponent* modelComp = sceneObj->modelComponent();
            modelComp->updateBounds(*this);
        }
    }

    // Update all child states
    for (const auto& child: m_children) {
        //auto childObject = std::static_pointer_cast<SceneObject>(childPair.second);
        //auto childTransform = childObject->transform();
        child->computeWorldMatrix();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing