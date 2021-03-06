#include "GPhysicsActor.h"

#include "../GCoreEngine.h"
#include "../scene/GScenario.h"
#include "../scene/GScene.h"
#include "../scene/GSceneObject.h"

#include "GPhysicsManager.h"
#include "GPhysicsShape.h"
#include "GPhysicsShapePrefab.h"
#include "GPhysicsGeometry.h"
#include "GPhysicsMaterial.h"
#include "GPhysicsManager.h"
#include "GPhysicsScene.h"
#include "../geometry/GTransform.h"
#include "../components/GTransformComponent.h"

namespace rev {
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// Actor
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsActor::PhysicsActor(ActorType type):
    m_actorType(type)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsActor::~PhysicsActor()
{
    // Note, associated shapes are released with the actor
    if (m_actor) {
        if (m_actor->getScene()) {
            if (physicsScene()) {
                // If scene still exists, remove from map
                physicsScene()->removeActor(this);
            }
            else {
                // Otherwise, just remove from physics scene directly
                m_actor->getScene()->removeActor(*m_actor);
            }
        }
        PX_RELEASE(m_actor);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<PhysicsScene> PhysicsActor::physicsScene() const
{
    auto so = sceneObject();
    if (so) {
        return sceneObject()->scene()->physics();
    }
    else {
        return nullptr;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsActor::enableGravity()
{
    m_actor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, false);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsActor::disableGravity()
{
    m_actor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue PhysicsActor::asJson(const SerializationContext& context) const
{
    QJsonObject object = PhysicsBase::asJson(context).toObject();
    object.insert("actorType", int(m_actorType));
    if (sceneObject()) {
        object.insert("sceneObject", sceneObject()->getName().c_str());
    }

    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsActor::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    PhysicsBase::loadFromJson(json);

    QJsonObject object = json.toObject();
    m_actorType = ActorType(object.value("actorType").toInt());
    if (m_actor) {
        if (object.contains("sceneObject")) {
            GString sceneName = object.value("sceneObject").toString();
            std::shared_ptr<SceneObject> so = SceneObject::getByName(sceneName);
            m_actor->userData = so.get();
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// Rigid Body
//////////////////////////////////////////////////////////////////////////////////////////////////
RigidBody::RigidBody(SceneObject* so) :
    PhysicsActor(kRigidBody),
    m_isKinematic(true),
    m_rigidType(kDynamic),
    m_density(1.0)
{
    initialize(so, so->transform(), *PhysicsManager::DefaultShape());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
RigidBody::RigidBody(SceneObject* so,
    PhysicsShapePrefab* shape,
    RigidType rType,
    float density) :
    PhysicsActor(kRigidBody),
    m_isKinematic(false),
    m_rigidType(rType),
    m_density(density)
{
    initialize(so, so->transform(), *shape);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
RigidBody::~RigidBody()
{
    //delete m_material;
    for (PhysicsShape* shape : m_shapes) {
        delete shape;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::setDensity(float density)
{
    m_density = density;
    if (as<physx::PxRigidDynamic>()) {
        updateMassAndInertia();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::clearShapes()
{
    //auto endIter = m_shapes.end();
    for (PhysicsShape* shape: m_shapes) {
        shape->detach();
        delete shape;
    }
    m_shapes.clear();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsShape & RigidBody::shape(int index)
{
    return *m_shapes[index];
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::toggleContact(bool contact)
{
    for (PhysicsShape* shape: m_shapes) {
        shape->toggleContact(contact);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::toggleQueries(bool isQueried)
{
    for (PhysicsShape* shape : m_shapes) {
        shape->toggleSceneQueries(isQueried);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
bool RigidBody::updateMassAndInertia()
{
    return physx::PxRigidBodyExt::updateMassAndInertia(
        *as<physx::PxRigidDynamic>(), m_density);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::setKinematic(bool isKinematic, bool setMember)
{
    auto* db = as<physx::PxRigidDynamic>();
    if (db) {
        db->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, isKinematic);
    }

    if (setMember) {
        m_isKinematic = isKinematic;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::addShape(PhysicsShapePrefab& prefab)
{
    auto shapeIter = std::find_if(m_shapes.begin(), m_shapes.end(),
        [&prefab](const PhysicsShape* s) {
        return s->prefab().getUuid() == prefab.getUuid();
    });

    if (shapeIter != m_shapes.end()) {
        throw("Error, shape with the given prefab already found on rigid body");
    }

    // This was problematic with values stored, so made pointers
    Vec::EmplaceBack(m_shapes, new PhysicsShape(prefab, this));
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Transform RigidBody::getTransform() const
{
    Transform pose = Transform(as<physx::PxRigidActor>()->getGlobalPose());
    return pose;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::setTransform(const Transform& t)
{
    auto* db = as<physx::PxRigidDynamic>();
    if (!db) {
        auto* sb = as<physx::PxRigidStatic>();
        if (!sb) {
            throw("Error, body must be dynamic or static");
        }

        // This instantaneously changes the actor space to the given world space transformation
        // This is not recommended, since moving static actors incurs a performance penalty
        sb->setGlobalPose(t.asPhysX(), true); // awaken the actor
    }
    else if (m_isKinematic) {

        if (!rigidBodyFlags().testFlag(RigidBodyFlag::kKinematic)) {
            throw("Error, actor is not kinematic");
        }

        // This is the preferred way to set global pose
        auto* db = as<physx::PxRigidDynamic>();

        if (!db) {
            throw("Error, body is kinematic and not dynamic");
        }

        // Set kinematic target, to which the object will be moved by the next frame
        // Invalid if actor has not been added to a scene or if DisableSimulation flag is set
        db->setKinematicTarget(t.asPhysX());
    }
    else {
        // This instantaneously changes the actor space to the given world space transformation
        // TODO: This is not recommended for simulating actual movement, 
        // since this body will not correctly interact with joints and dynamic bodies as it moves
        auto* b = as<physx::PxRigidBody>();
        b->setGlobalPose(t.asPhysX(), true); // awaken the actor
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::sleep()
{
    auto* db = as<physx::PxRigidDynamic>();
    if (db) {
        db->putToSleep();
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::awaken()
{
    auto* db = as<physx::PxRigidDynamic>();
    if (db) {
        db->wakeUp();
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::setAngularDamping(float damping)
{
    as<physx::PxRigidBody>()->setAngularDamping(damping);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::setLinearVelocity(const Vector3f & vel)
{
    as<physx::PxRigidBody>()->setLinearVelocity(physx::PxVec3(vel.x(), vel.y(), vel.z()));
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue RigidBody::asJson(const SerializationContext& context) const
{
    QJsonObject object =  PhysicsActor::asJson(context).toObject();
    QJsonArray shapeNames;
    for (PhysicsShape* shape : m_shapes) {
        shapeNames.append(shape->prefab().getName().c_str());
    }
    object.insert("shapes", shapeNames);
    object.insert("rigidType", int(m_rigidType));
    object.insert("density", m_density);
    object.insert("isKinematic", m_isKinematic);
        
    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    PhysicsActor::loadFromJson(json);

    // Load in attributes
    QJsonObject object = json.toObject();
    PhysicsShapePrefab* shape;
    if (object.contains("shapes")) {
        QJsonArray shapes = object["shapes"].toArray();
        if (shapes.size() > 1) {
            throw("Error, have not implemented multiple shapes per rigid body");
        }
        for (auto shapeJson: shapes) {
            if (shapeJson.isString()) {
                GString shapeName = GString(shapeJson.toString());
                shape = PhysicsManager::Shape(shapeName);
            }
            else {
                // Legacy, now storing shape JSON in physics manager
                throw("Unsupported");
                //shape = PhysicsShapePrefab::get(shapeJson);
            }
        }
    }

    m_rigidType = RigidType(object.value("rigidType").toInt());
    m_density = object.value("density").toDouble();
    m_isKinematic = object.value("isKinematic").toBool();

    // Create actor, first retrieving the scene object it belongs to
    SceneObject* so = nullptr;
    if (object.contains("sceneObject")) {
        GString sceneName = object.value("sceneObject").toString();
        so = SceneObject::getByName(sceneName).get();
    }

    initialize(so, sceneObject()->transform(), *shape);

}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::reinitialize()
{
    // TODO: Account for more than one shape
    if (!m_shapes.size()) {
        throw("Error, no shapes to reinitialize rigid body");
    }
    initialize(sceneObject(), sceneObject()->transform(), m_shapes[0]->prefab());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::reinitialize(PhysicsShapePrefab & prefab)
{
    // TODO: Account for more than one shape
    initialize(sceneObject(), sceneObject()->transform(), prefab);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::initialize(SceneObject* so,
    const Transform & transform,
    PhysicsShapePrefab& shapePrefab)
{
    // TODO: Account for more than one shape per rigid actor

    // Clear shape instantiations
    // Must do this before clearing the actor
    clearShapes();

    // If there already is an actor, delete
    if (m_actor) {
        PX_RELEASE(m_actor);
    }

    // If geometry is planar, force a static body
    // See: https://docs.nvidia.com/gameworks/content/gameworkslibrary/physx/guide/Manual/Geometry.html
    if (shapePrefab.geometry()->getType() == PhysicsGeometry::kPlane) {
        m_rigidType = kStatic;
    }

    // Type-specific logic
    bool updated;
    switch (m_rigidType) {
    case kStatic:
        m_actor = PhysicsManager::Physics()->createRigidStatic(transform.asPhysX());

        // Make shape exclusive to this body
        addShape(shapePrefab);

        break;
    case kDynamic: {
        m_actor = PhysicsManager::Physics()->createRigidDynamic(transform.asPhysX());

        // Make shape exclusive to this body
        addShape(shapePrefab);

        // Update inertia tensor so that dynamics can apply
        updated = updateMassAndInertia();

        // Set kinematic
        if (m_isKinematic) {
            setRigidBodyFlags(RigidBodyFlag::kKinematic);
        }

        if (!updated) {
            throw("Error, failed to update mass and inertia properties");
        }
        break; 
    }
    case kArticulationLink:
    default:
        throw("Error, type of rigid body unrecognized");
        break;
    }

    // Associate each physics actor with its scene object
    m_actor->userData = so;

    // Add the wrapped actor to the physx scene
    so->scene()->physics()->addActor(this);
}






//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
}