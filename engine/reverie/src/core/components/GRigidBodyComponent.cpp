///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GRigidBodyComponent.h"

// Standard Includes

// External

// Project
#include "GTransformComponent.h"
#include "../scene/GSceneObject.h"
#include "../scene/GScene.h"

#include "../geometry/GTransform.h"
#include "../GCoreEngine.h"
#include "../physics/GPhysicsActor.h"
#include "../physics/GPhysicsShape.h"
#include "../physics/GPhysicsMaterial.h"
#include "../physics/GPhysicsGeometry.h"
#include "../physics/GPhysicsManager.h"
#include "../physics/GPhysicsScene.h"
#include "../physics/GCharacterController.h"

#include "../readers/GJsonReader.h"
#include "../../view/components/GComponentWidgets.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RigidBodyComponent
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RigidBodyComponent::RigidBodyComponent() :
    Component(ComponentType::kRigidBody)
{
    throw("should never be called, for Qt metatype registration");
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RigidBodyComponent::RigidBodyComponent(const RigidBodyComponent & comp):
    Component(ComponentType::kRigidBody)
{
    m_rigidBody = std::make_unique<RigidBody>(comp.sceneObject().get());
    comp.sceneObject()->addComponent(this);
    loadFromJson(comp.asJson());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RigidBodyComponent::RigidBodyComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, ComponentType::kRigidBody)
{
    if (!object->scene()->physics()) {
        object->scene()->addPhysics();
    }

    m_rigidBody = std::make_unique<RigidBody>(object.get());
    sceneObject()->addComponent(this);
    initializeDefault();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RigidBodyComponent::~RigidBodyComponent()
{
    // Parent class destructor will handle children
    //delete m_rigidBody;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBodyComponent::updateTransformFromPhysics()
{
    // TODO: Add a warning for the case where rigid bodies are added to both
    // parent and child scene objects
    // TODO: Alternatively, fix children to parents as joints

    // Return if disabled
    if (!isEnabled()) { return; }

    // Update transform
    if (!m_rigidBody->m_actor) {
#ifdef DEBUG_MODE
        throw("actor not found");
#endif
    }

    physx::PxRigidActor* actor = m_rigidBody->as<physx::PxRigidActor>();
    physx::PxTransform physxTransform = actor->getGlobalPose();
    sceneObject()->transform().setPosition(PhysicsManager::toVector3(physxTransform.p), false);
    sceneObject()->transform().setRotation(PhysicsManager::toQuaternion(physxTransform.q), true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBodyComponent::refreshBody()
{
    if (m_rigidBody) {
        if (m_rigidBody->shapes().size() > 1){
            throw("Error, more than one shape not implemented");
        }
        m_rigidBody->initialize(sceneObject().get(),
            sceneObject()->transform(),
            m_rigidBody->shape(0).prefab());
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBodyComponent::enable()
{
    Component::enable();
    //body()->enableGravity();
    body()->setTransform(sceneObject()->transform());
    body()->setKinematic(body()->isKinematic(), false);
    body()->toggleQueries(true);
    body()->toggleContact(true);
    //body()->awaken();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBodyComponent::disable()
{
    Component::disable();
    //body()->disableGravity();
    //body()->setLinearVelocity(Vector3f(0.0, 0.0, 0.0));
    body()->setKinematic(true, false); // kinematic actors do not participate in dynamics
    body()->toggleQueries(false);
    body()->toggleContact(false);
    //body()->sleep();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue RigidBodyComponent::asJson(const SerializationContext& context) const
{
    QJsonObject object = Component::asJson(context).toObject();
    object.insert("body", m_rigidBody->asJson());

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBodyComponent::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    const QJsonObject& object = json.toObject();

    //if (!sceneObject()->hasParents()) {

    // Load rigid body from JSON
    if (object.contains("body")) {
        m_rigidBody->loadFromJson(object.value("body"));
    }

    Component::loadFromJson(json);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBodyComponent::initializeDefault()
{
    PhysicsShapePrefab* defaultShape = PhysicsManager::DefaultShape();
        
    // Load default parameters
    m_rigidBody->m_rigidType = RigidBody::kDynamic;
    m_rigidBody->initialize(sceneObject().get(), sceneObject()->transform(), *defaultShape);
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing