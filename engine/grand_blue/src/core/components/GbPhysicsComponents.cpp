///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GbPhysicsComponents.h"

// Standard Includes

// External

// Project
#include "GbTransformComponent.h"
#include "../scene/GbSceneObject.h"
#include "../scene/GbScene.h"

#include "../geometry/GbTransform.h"
#include "../GbCoreEngine.h"
#include "../physics/GbPhysicsActor.h"
#include "../physics/GbPhysicsShape.h"
#include "../physics/GbPhysicsMaterial.h"
#include "../physics/GbPhysicsGeometry.h"
#include "../physics/GbPhysicsManager.h"
#include "../physics/GbPhysicsScene.h"
#include "../physics/GbCharacterController.h"

#include "../readers/GbJsonReader.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RigidBodyComponent
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RigidBodyComponent::RigidBodyComponent() :
    Component(kRigidBody)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RigidBodyComponent::RigidBodyComponent(const RigidBodyComponent & comp):
    Component(kRigidBody)
{
    m_rigidBody = std::make_unique<RigidBody>(comp.sceneObject());
    comp.sceneObject()->addComponent(this);
    loadFromJson(comp.asJson());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RigidBodyComponent::RigidBodyComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, kRigidBody)
{
    if (!object->scene()->physics()) {
        object->scene()->addPhysics();
    }

    m_rigidBody = std::make_unique<RigidBody>(object);
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

    // Return if disabled
    if (!m_isEnabled) return;

    // Update transform
    Transform physicsTransform = m_rigidBody->getTransform();
    sceneObject()->transform()->translation().setPosition(physicsTransform.translation().getPosition(), false);
    sceneObject()->transform()->rotation().setRotation(physicsTransform.getRotationQuaternion());

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBodyComponent::enable()
{
    Component::enable();
    //body()->enableGravity();
    body()->setTransform(*sceneObject()->transform());
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
QJsonValue RigidBodyComponent::asJson() const
{
    QJsonObject object = Component::asJson().toObject();
    if (m_rigidBody) {
        object.insert("body", m_rigidBody->asJson());
    }

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBodyComponent::loadFromJson(const QJsonValue & json)
{
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
    static QString defaultShapeKey("defaultShape");
    static QString defaultMaterialKey("defaultMaterial");

    std::shared_ptr<PhysicsShapePrefab> defaultShape = nullptr;
    std::shared_ptr<PhysicsMaterial> defaultMaterial;
    if (Map::HasKey(PhysicsManager::shapes(), defaultShapeKey)) {
        defaultShape = PhysicsManager::shapes().at(defaultShapeKey);
    }
    else {
        if (Map::HasKey(PhysicsManager::materials(), defaultMaterialKey)) {
            defaultMaterial = PhysicsManager::materials().at(defaultMaterialKey);
        }
        else {
            defaultMaterial = PhysicsMaterial::create(defaultMaterialKey,
                0.5f, 0.5f, 0.2f);
        }
        defaultShape = PhysicsShapePrefab::create(defaultShapeKey,
            std::make_shared<BoxGeometry>(),
            defaultMaterial
        );
    }
        
    // Load default parameters
    m_rigidBody->m_rigidType = RigidBody::kDynamic;
    m_rigidBody->initialize(sceneObject(), *sceneObject()->transform(), *defaultShape);
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CharControlComponent
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CharControlComponent::CharControlComponent() :
    Component(kCharacterController)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CharControlComponent::CharControlComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, kCharacterController)
{
    if (!object->scene()->physics()) {
        object->scene()->addPhysics();
    }

    object->addComponent(this);

    /// Create a basic capsule controller
    auto desc = std::make_shared<CapsuleControllerDescription>();
    desc->m_material = PhysicsManager::materials()["defaultMaterial"];
    desc->m_radius = 1.0;
    desc->m_height = 1.0;
    m_controller = cctManager()->createController(desc, sceneObject());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CharControlComponent::~CharControlComponent()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CharControlComponent::move(const Vector3 & disp)
{
    m_controller->move(disp);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CharControlComponent::setGravity(const Vector3g& gravity)
{
    m_controller->setGravity(gravity);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CharControlComponent::enable()
{
    Component::enable();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CharControlComponent::disable()
{
    Component::disable();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue CharControlComponent::asJson() const
{
    QJsonObject object = Component::asJson().toObject();
    if (m_controller) {
        object.insert("controller", m_controller->asJson());
    }

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CharControlComponent::loadFromJson(const QJsonValue & json)
{
    const QJsonObject& object = json.toObject();

    if (m_controller) {
        // Remove old controller from controller manager
        cctManager()->removeController(*m_controller);
    }

    // Load character controller from JSON
    if (object.contains("controller")) {
        m_controller = cctManager()->createController(object.value("controller"));
    }

    Component::loadFromJson(json);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const std::shared_ptr<CCTManager>& CharControlComponent::cctManager() const
{
    return sceneObject()->scene()->physics()->cctManager();
}





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing