///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GbTransformComponent.h"

// Standard Includes

// External

// Project
#include "../scene/GbSceneObject.h"
#include "../scene/GbScene.h"
#include "../scene/GbScenario.h"
#include "../components/GbCamera.h"
#include "../components/GbLight.h"
#include "../components/GbPhysicsComponents.h"
#include "../physics/GbPhysicsActor.h"
#include "../utils/GbInterpolation.h"
#include "../rendering/shaders/GbUniformBufferObject.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TransformComponent
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TransformComponent::TransformComponent() :
    Component(kTransform),
    Transform()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TransformComponent::TransformComponent(const std::shared_ptr<SceneObject>& object) :
    Transform(),
    Component(object, kTransform)
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
void TransformComponent::loadFromJson(const QJsonValue & json)
{
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
        m_worldMatrix = m_localMatrix;
    }
    else {
        m_worldMatrix.setToIdentity();
        switch (m_inheritanceType) {
        case Transform::kTranslation:
            m_worldMatrix.setTranslation(parentTransform()->worldTranslation().getPosition().asReal());
            //m_worldMatrix = parentTransform()->worldTranslation().getMatrix();
            break;
        case Transform::kAll:
        case Transform::kPreserveOrientation:
        default:
            m_worldMatrix = parentTransform()->worldMatrix();
            break;
        }

        // Construct model matrix for each inheritance type
        switch (m_inheritanceType) {
        case Transform::kPreserveOrientation:
        {
            // Obtain world translation by removing the rotation component of the world matrix
            Matrix4x4f localTranslation;
            localTranslation.setTranslation(m_translation.getPosition().asReal());
            Matrix4x4f translation = m_worldMatrix * localTranslation;
            translation = translation.getTranslationMatrix();

            // Compute world matrix that preserves original orientation of state w.r.t. inertial frame
            m_worldMatrix = translation * m_rotation.getMatrix() * m_scale.getMatrix();
        }
        case Transform::kTranslation:
        case Transform::kAll:
        default:
            // Compute world matrix
            m_worldMatrix *= m_localMatrix;
            break;
        }
    }

    // If there is a scene object for the component (should always be the case)
    auto sceneObj = sceneObject();
    if (sceneObj) {
        // Update view matrix if the scene object has a camera
        if (sceneObj->hasCamera()) {
            auto cam = sceneObj->camera();
            cam->camera().computeViewMatrix(m_worldMatrix);
        }

        // If the scene object has a light, update its position
        if (sceneObj->hasComponent(kLight)) {
            sceneObj->light()->setPosition(m_translation.getPosition().asReal());
        }

        // Update the position of any rigid bodies on the object
        if (sceneObj->hasComponent(Component::kRigidBody)) {
			std::vector<Component*>& rigidBodies = sceneObj->components()[Component::kRigidBody];
            for (auto& comp : rigidBodies) {
                RigidBodyComponent* rigidBodyComp = static_cast<RigidBodyComponent*>(comp);
                rigidBodyComp->body()->setTransform(*static_cast<Transform*>(this));
            }
        }
    }


    // Update all child states
    for (const std::pair<Uuid, Transform*>& childPair: m_children) {
        //auto childObject = std::static_pointer_cast<SceneObject>(childPair.second);
        //auto childTransform = childObject->transform();
        childPair.second->computeWorldMatrix();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing