#include "GbPhysicsActor.h"

#include "../GbCoreEngine.h"
#include "../scene/GbScenario.h"
#include "../scene/GbScene.h"
#include "../scene/GbSceneObject.h"

#include "GbPhysicsManager.h"
#include "GbPhysicsShape.h"
#include "GbPhysicsGeometry.h"
#include "GbPhysicsMaterial.h"
#include "GbPhysicsManager.h"
#include "GbPhysicsScene.h"
#include "../geometry/GbTransform.h"
#include "../components/GbTransformComponent.h"

namespace Gb {
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// Actor
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsActor::PhysicsActor(const std::shared_ptr<SceneObject>& sc, ActorType type) :
    m_sceneObject(sc),
    m_actorType(type)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsActor::~PhysicsActor()
{
    // Note, associated shapes are released with the actor
    if (!m_actor) {
        return;
    }
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
QJsonValue PhysicsActor::asJson() const
{
    QJsonObject object = PhysicsBase::asJson().toObject();
    object.insert("actorType", int(m_actorType));
    if (sceneObject()) {
        object.insert("sceneObject", sceneObject()->getName());
    }

    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsActor::loadFromJson(const QJsonValue & json)
{
    PhysicsBase::loadFromJson(json);

    QJsonObject object = json.toObject();
    m_actorType = ActorType(object.value("actorType").toInt());
    if (object.contains("sceneObject")) {
        QString sceneName = object.value("sceneObject").toString();
        m_sceneObject = SceneObject::getByName(sceneName);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// Rigid Body
//////////////////////////////////////////////////////////////////////////////////////////////////
RigidBody::RigidBody(const std::shared_ptr<SceneObject>& so) :
    PhysicsActor(so, kRigidBody),
    m_isKinematic(true),
    m_rigidType(kDynamic),
    m_density(1.0)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
RigidBody::RigidBody(const std::shared_ptr<SceneObject>& so, 
    const Transform & transform,
    std::shared_ptr<PhysicsShapePrefab> shape,
    RigidType rType,
    float density) :
    PhysicsActor(so, kRigidBody),
    m_isKinematic(false),
    m_rigidType(rType),
    m_density(density)
{
    initialize(so, transform, *shape);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
RigidBody::RigidBody(const std::shared_ptr<SceneObject>& so,
    const Transform& transform,
    std::shared_ptr<PhysicsGeometry> geometry,
    std::shared_ptr<PhysicsMaterial> material,
    RigidType rType,
    float density):
    PhysicsActor(so, kRigidBody),
    m_isKinematic(false),
    m_rigidType(rType),
    m_density(density)
{
    auto shape = PhysicsShapePrefab::create(so->getName(), geometry, material);
    initialize(so, transform, *shape);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
RigidBody::~RigidBody()
{
    //delete m_material;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::clearShapes()
{
    auto endIter = m_shapes.end();
    for (auto iter = m_shapes.begin(); iter != endIter;) {
        const PhysicsShape& shape = iter->second;
        removeShape(shape);
        iter = m_shapes.erase(iter);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::removeShape(const PhysicsShape& shape)
{
    const QUuid& prefabID = shape.prefab().getUuid();
    if (!m_shapes.count(prefabID)) {
        throw("Error, rigid body does not have this shape to remove");
    }
    if(m_actor)
        rigidActor()->detachShape(*m_shapes[prefabID].m_pxShape, true);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::toggleContact(bool contact)
{
    for (std::pair<const Uuid, PhysicsShape>& shapePair: m_shapes) {
        PhysicsShape& shape = shapePair.second;
        shape.toggleContact(contact);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::toggleQueries(bool isQueried)
{
    for (std::pair<const Uuid, PhysicsShape>& shapePair : m_shapes) {
        PhysicsShape& shape = shapePair.second;
        shape.toggleSceneQueries(isQueried);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
bool RigidBody::updateMassAndInertia()
{
    return physx::PxRigidBodyExt::updateMassAndInertia(
        *dynamicBody(), m_density);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::setKinematic(bool isKinematic, bool setMember)
{
    if (dynamicBody()) {
        dynamicBody()->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, isKinematic);
    }

    if(setMember) m_isKinematic = isKinematic;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::addShape(const PhysicsShapePrefab& prefab)
{
    if (Map::HasKey(m_shapes, prefab.getUuid())) {
        throw("Error, rigid body already has the given shape");
    }
    Map::Emplace(m_shapes,
        prefab.getUuid(),
        PhysicsShape(prefab, prefab.createExclusive(*this)));
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Transform RigidBody::getTransform() const
{
    Transform pose = Transform(getRigidActor()->getGlobalPose());
    return pose;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::setTransform(const Transform& t)
{
    physx::PxRigidActor* b = rigidActor();
    b->setGlobalPose(t.asPhysX(), true); // awaken the actor
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::sleep()
{
    if (dynamicBody()) {
        dynamicBody()->putToSleep();
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::awaken()
{
    if (dynamicBody()) {
        dynamicBody()->wakeUp();
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::setAngularDamping(float damping)
{
    body()->setAngularDamping(damping);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::setLinearVelocity(const Vector3f & vel)
{
    body()->setLinearVelocity(physx::PxVec3(vel.x(), vel.y(), vel.z()));
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue RigidBody::asJson() const
{
    QJsonObject object =  PhysicsActor::asJson().toObject();
    QJsonArray shapeNames;
    for (const auto& shapePair : m_shapes) {
        shapeNames.append(shapePair.second.prefab().getName());
    }
    object.insert("shapes", shapeNames);
    object.insert("rigidType", int(m_rigidType));
    object.insert("density", m_density);
    object.insert("isKinematic", m_isKinematic);
        
    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::loadFromJson(const QJsonValue & json)
{
    PhysicsActor::loadFromJson(json);

    // Load in attributes
    QJsonObject object = json.toObject();
    std::shared_ptr<PhysicsShapePrefab> shape;
    if (object.contains("shape")) {
        shape = PhysicsShapePrefab::get(object.value("shape"));
    }
    else if (object.contains("shapes")) {
        QJsonArray shapes = object["shapes"].toArray();
        if (shapes.size() > 1) {
            throw("Error, have not implemented multiple shapes per rigid body");
        }
        for (auto shapeJson: shapes) {
            if (shapeJson.isString()) {
                shape = PhysicsShapePrefab::get(shapeJson.toString());
            }
            else {
                // Legacy, now storing shape JSON in physics manager
                shape = PhysicsShapePrefab::get(shapeJson);
            }
        }
    }
    else {
        // Load geometry
        std::shared_ptr<PhysicsGeometry> geometry = PhysicsGeometry::createGeometry(
            object.value("geometry").toObject());

        // Load materials
        QString materialName = object.value("material").toString();
        auto mtl = PhysicsManager::materials()[materialName];
        shape = PhysicsShapePrefab::get(sceneObject()->getName(),
            geometry, mtl);
    }

    m_rigidType = RigidType(object.value("rigidType").toInt());
    m_density = object.value("density").toDouble();
    m_isKinematic = object.value("isKinematic").toBool();

    // Create actor
    initialize(sceneObject(), 
        *static_cast<Transform*>(sceneObject()->transform().get()),
        *shape);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBody::initialize(const std::shared_ptr<SceneObject>& so,
    const Transform & transform,
    const PhysicsShapePrefab& shapePrefab)
{
    // TODO: Account for more than one shape per rigid actor

    // If there already is an actor, delete
    if (m_actor) {
        PX_RELEASE(m_actor);
    }

    // Clear shape instantiations
    clearShapes();

    // If geometry is planar, force a static body
    // See: https://docs.nvidia.com/gameworks/content/gameworkslibrary/physx/guide/Manual/Geometry.html
    if (shapePrefab.geometry()->getType() == PhysicsGeometry::kPlane) {
        m_rigidType = kStatic;
    }

    // Type-specific logic
    bool updated;
    switch (m_rigidType) {
    case kStatic:
        m_actor = PhysicsManager::physics()->createRigidStatic(transform.asPhysX());

        // Make shape exclusive to this body
        addShape(shapePrefab);

        break;
    case kDynamic: {
        m_actor = PhysicsManager::physics()->createRigidDynamic(transform.asPhysX());

        // Make shape exclusive to this body
        addShape(shapePrefab);

        // Update inertia tensor so that dynamics can apply
        updated = updateMassAndInertia();

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

    // Add the wrapped actor to the physx scene
    sceneObject()->scene()->physics()->addActor(this, so);
}






//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
}