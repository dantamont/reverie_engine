
// Includes

#include "core/components/GRigidBodyComponent.h"

// Standard Includes

// External

// Project
#include "core/components/GTransformComponent.h"
#include "core/scene/GSceneObject.h"
#include "core/scene/GScene.h"

#include "fortress/containers/math/GTransform.h"
#include "core/GCoreEngine.h"
#include "core/physics/GPhysicsActor.h"
#include "core/physics/GPhysicsShape.h"
#include "core/physics/GPhysicsMaterial.h"
#include "core/physics/GPhysicsGeometry.h"
#include "core/physics/GPhysicsManager.h"
#include "core/physics/GPhysicsScene.h"
#include "core/physics/GCharacterController.h"
#include "core/converters/GPhysxConverter.h"

#include "fortress/json/GJson.h"
#include "geppetto/qt/widgets/components/GComponentWidget.h"
#include "logging/GLogger.h"

// Namespace Definitions

namespace rev {



// RigidBodyComponent

RigidBodyComponent::RigidBodyComponent() :
    Component(ComponentType::kRigidBody)
{
    Logger::Throw("should never be called, for Qt metatype registration");
}

RigidBodyComponent::RigidBodyComponent(const RigidBodyComponent & comp):
    Component(ComponentType::kRigidBody)
{
    m_rigidBody = std::make_unique<RigidBody>(comp.sceneObject().get());
    comp.sceneObject()->setComponent(this);
    json otherJson{ comp };
    otherJson.get_to(*this);
}

RigidBodyComponent::RigidBodyComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, ComponentType::kRigidBody)
{
    if (!object->scene()->physics()) {
        object->scene()->addPhysics();
    }

    m_rigidBody = std::make_unique<RigidBody>(object.get());
    sceneObject()->setComponent(this);
    initializeDefault();
}

RigidBodyComponent::~RigidBodyComponent()
{
    // Parent class destructor will handle children
    //delete m_rigidBody;
}

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
        Logger::Throw("actor not found");
#endif
    }

    physx::PxRigidActor* actor = m_rigidBody->as<physx::PxRigidActor>();
    physx::PxTransform physxTransform = actor->getGlobalPose();
    sceneObject()->transform().setPosition(PhysxConverter::FromPhysX(physxTransform.p), false);
    sceneObject()->transform().setRotation(PhysxConverter::FromPhysX(physxTransform.q), true);
}

void RigidBodyComponent::refreshBody()
{
    if (m_rigidBody) {
        if (m_rigidBody->shapes().size() > 1){
            Logger::Throw("Error, more than one shape not implemented");
        }
        m_rigidBody->initialize(sceneObject().get(),
            sceneObject()->transform(),
            m_rigidBody->shape(0).prefab());
    }
}

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

void to_json(json& orJson, const RigidBodyComponent& korObject)
{
    ToJson<Component>(orJson, korObject);
    orJson["body"] = *korObject.m_rigidBody;
}

void from_json(const json& korJson, RigidBodyComponent& orObject)
{
    // Load rigid body from JSON
    if (korJson.contains("body")) {
        korJson.at("body").get_to(*orObject.m_rigidBody);
    }

    FromJson<Component>(korJson, orObject);
}

void RigidBodyComponent::initializeDefault()
{
    PhysicsShapePrefab* defaultShape = PhysicsManager::DefaultShape();
        
    // Load default parameters
    m_rigidBody->m_rigidType = ERigidBodyType::eDynamic;
    m_rigidBody->initialize(sceneObject().get(), sceneObject()->transform(), *defaultShape);
}





} // end namespacing