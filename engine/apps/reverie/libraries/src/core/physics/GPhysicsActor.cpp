#include "core/physics/GPhysicsActor.h"

#include "core/GCoreEngine.h"
#include "core/scene/GScenario.h"
#include "core/scene/GScene.h"
#include "core/scene/GSceneObject.h"

#include "core/physics/GPhysicsManager.h"
#include "core/physics/GPhysicsShape.h"
#include "core/physics/GPhysicsShapePrefab.h"
#include "core/physics/GPhysicsGeometry.h"
#include "core/physics/GPhysicsMaterial.h"
#include "core/physics/GPhysicsManager.h"
#include "core/physics/GPhysicsScene.h"
#include "fortress/containers/math/GTransform.h"
#include "core/components/GTransformComponent.h"
#include "core/converters/GPhysxConverter.h"

#include "logging/GLogger.h"

namespace rev {


// Actor

PhysicsActor::PhysicsActor(ActorType type):
    m_actorType(type)
{
}

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

void PhysicsActor::enableGravity()
{
    m_actor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, false);
}

void PhysicsActor::disableGravity()
{
    m_actor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
}

void to_json(json& orJson, const PhysicsActor& korObject)
{
    ToJson<PhysicsBase>(orJson, korObject);
    orJson["actorType"] = int(korObject.m_actorType);
    if (korObject.sceneObject()) {
        orJson["sceneObject"] = korObject.sceneObject()->getName().c_str();
    }
}

void from_json(const json& korJson, PhysicsActor& orObject)
{
    FromJson<PhysicsBase>(korJson, orObject);
    
    orObject.m_actorType = PhysicsActor::ActorType(korJson.at("actorType").get<Int32_t>());
    if (orObject.m_actor) {
        if (korJson.contains("sceneObject")) {
            GString sceneName;
            korJson.at("sceneObject").get_to(sceneName);
            std::shared_ptr<SceneObject> so = SceneObject::getByName(sceneName);
            orObject.m_actor->userData = so.get();
        }
    }
}




// Rigid Body

RigidBody::RigidBody(SceneObject* so) :
    PhysicsActor(kRigidBody),
    m_isKinematic(true),
    m_rigidType(ERigidBodyType::eDynamic),
    m_density(1.0)
{
    initialize(so, so->transform(), *PhysicsManager::DefaultShape());
}

RigidBody::RigidBody(SceneObject* so,
    PhysicsShapePrefab* shape,
    GRigidBodyType rType,
    float density) :
    PhysicsActor(kRigidBody),
    m_isKinematic(false),
    m_rigidType(rType),
    m_density(density)
{
    initialize(so, so->transform(), *shape);
}

RigidBody::~RigidBody()
{
    //delete m_material;
    for (PhysicsShape* shape : m_shapes) {
        delete shape;
    }
}

void RigidBody::setDensity(float density)
{
    m_density = density;
    if (as<physx::PxRigidDynamic>()) {
        updateMassAndInertia();
    }
}


void RigidBody::clearShapes()
{
    //auto endIter = m_shapes.end();
    for (PhysicsShape* shape: m_shapes) {
        shape->detach();
        delete shape;
    }
    m_shapes.clear();
}

PhysicsShape & RigidBody::shape(int index)
{
    return *m_shapes[index];
}

void RigidBody::toggleContact(bool contact)
{
    for (PhysicsShape* shape: m_shapes) {
        shape->toggleContact(contact);
    }
}

void RigidBody::toggleQueries(bool isQueried)
{
    for (PhysicsShape* shape : m_shapes) {
        shape->toggleSceneQueries(isQueried);
    }
}

bool RigidBody::updateMassAndInertia()
{
    return physx::PxRigidBodyExt::updateMassAndInertia(
        *as<physx::PxRigidDynamic>(), m_density);
}

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

void RigidBody::addShape(PhysicsShapePrefab& prefab)
{
    auto shapeIter = std::find_if(m_shapes.begin(), m_shapes.end(),
        [&prefab](const PhysicsShape* s) {
        return s->prefab().getUuid() == prefab.getUuid();
    });

    if (shapeIter != m_shapes.end()) {
        Logger::Throw("Error, shape with the given prefab already found on rigid body");
    }

    // This was problematic with values stored, so made pointers
    Vec::EmplaceBack(m_shapes, new PhysicsShape(prefab, this));
}

//Transform RigidBody::getTransform() const
//{
//    Transform pose = Transform(as<physx::PxRigidActor>()->getGlobalPose());
//    return pose;
//}

void RigidBody::setTransform(const TransformInterface& t)
{
    auto* db = as<physx::PxRigidDynamic>();
    physx::PxTransform pt = PhysxConverter::ToPhysX(t);
    if (!db) {
        auto* sb = as<physx::PxRigidStatic>();
        if (!sb) {
            Logger::Throw("Error, body must be dynamic or static");
        }

        // This instantaneously changes the actor space to the given world space transformation
        // This is not recommended, since moving static actors incurs a performance penalty
        sb->setGlobalPose(pt, true); // awaken the actor
    }
    else if (m_isKinematic) {

        if (!rigidBodyFlags().testFlag(RigidBodyFlag::kKinematic)) {
            Logger::Throw("Error, actor is not kinematic");
        }

        // This is the preferred way to set global pose
        auto* db = as<physx::PxRigidDynamic>();

        if (!db) {
            Logger::Throw("Error, body is kinematic and not dynamic");
        }

        // Set kinematic target, to which the object will be moved by the next frame
        // Invalid if actor has not been added to a scene or if DisableSimulation flag is set
        db->setKinematicTarget(pt);
    }
    else {
        // This instantaneously changes the actor space to the given world space transformation
        // TODO: This is not recommended for simulating actual movement, 
        // since this body will not correctly interact with joints and dynamic bodies as it moves
        auto* b = as<physx::PxRigidBody>();
        b->setGlobalPose(pt, true); // awaken the actor
    }
}

void RigidBody::sleep()
{
    auto* db = as<physx::PxRigidDynamic>();
    if (db) {
        db->putToSleep();
    }
}

void RigidBody::awaken()
{
    auto* db = as<physx::PxRigidDynamic>();
    if (db) {
        db->wakeUp();
    }
}

void RigidBody::setAngularDamping(float damping)
{
    as<physx::PxRigidBody>()->setAngularDamping(damping);
}

void RigidBody::setLinearVelocity(const Vector3f & vel)
{
    as<physx::PxRigidBody>()->setLinearVelocity(physx::PxVec3(vel.x(), vel.y(), vel.z()));
}

void to_json(json& orJson, const RigidBody& korObject)
{
    ToJson<PhysicsActor>(orJson, korObject);
    json shapeNames = json::array();
    for (PhysicsShape* shape : korObject.m_shapes) {
        shapeNames.push_back(shape->prefab().getName().c_str());
    }
    orJson["shapes"] = shapeNames;
    orJson["rigidType"] = int(korObject.m_rigidType);
    orJson["density"] = korObject.m_density;
    orJson["isKinematic"] = korObject.m_isKinematic;
}

void from_json(const json& korJson, RigidBody& orObject)
{
    FromJson<PhysicsActor>(korJson, orObject);

    // Load in attributes
    PhysicsShapePrefab* shape;
    if (korJson.contains("shapes")) {
        const json& shapes = korJson["shapes"];
        if (shapes.size() > 1) {
            Logger::Throw("Error, have not implemented multiple shapes per rigid body");
        }
        for (auto shapeJson: shapes) {
            if (shapeJson.is_string()) {
                GString shapeName;
                shapeJson.get_to(shapeName);
                shape = PhysicsManager::Shape(shapeName);
            }
            else {
                // Legacy, now storing shape JSON in physics manager
                Logger::Throw("Unsupported");
                //shape = PhysicsShapePrefab::get(shapeJson);
            }
        }
    }

    orObject.m_rigidType = GRigidBodyType(korJson.at("rigidType").get<Int32_t>());
    korJson.at("density").get_to(orObject.m_density);
    korJson.at("isKinematic").get_to(orObject.m_isKinematic);

    // Create actor, first retrieving the scene object it belongs to
    SceneObject* so = nullptr;
    if (korJson.contains("sceneObject")) {
        GString sceneName;
        korJson.at("sceneObject").get_to(sceneName);
        so = SceneObject::getByName(sceneName).get();
    }

    orObject.initialize(so, orObject.sceneObject()->transform(), *shape);

}

void RigidBody::reinitialize()
{
    // TODO: Account for more than one shape
    if (!m_shapes.size()) {
        Logger::Throw("Error, no shapes to reinitialize rigid body");
    }
    initialize(sceneObject(), sceneObject()->transform(), m_shapes[0]->prefab());
}

void RigidBody::reinitialize(PhysicsShapePrefab & prefab)
{
    // TODO: Account for more than one shape
    initialize(sceneObject(), sceneObject()->transform(), prefab);
}

void RigidBody::initialize(SceneObject* so,
    const TransformInterface & transform,
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
    /// \see https://docs.nvidia.com/gameworks/content/gameworkslibrary/physx/guide/Manual/Geometry.html
    if (shapePrefab.geometry()->getType() == EPhysicsGeometryType::ePlane) {
        m_rigidType = ERigidBodyType::eStatic;
    }

    // Type-specific logic
    bool updated;
    physx::PxTransform pt = PhysxConverter::ToPhysX(transform);
    switch ((ERigidBodyType)m_rigidType) {
    case ERigidBodyType::eStatic:
        m_actor = PhysicsManager::Physics()->createRigidStatic(pt);

        // Make shape exclusive to this body
        addShape(shapePrefab);

        break;
    case ERigidBodyType::eDynamic: {
        m_actor = PhysicsManager::Physics()->createRigidDynamic(pt);

        // Make shape exclusive to this body
        addShape(shapePrefab);

        // Update inertia tensor so that dynamics can apply
        updated = updateMassAndInertia();

        // Set kinematic
        if (m_isKinematic) {
            setRigidBodyFlags(RigidBodyFlag::kKinematic);
        }

        if (!updated) {
            Logger::Throw("Error, failed to update mass and inertia properties");
        }
        break; 
    }
    case ERigidBodyType::eArticulationLink:
    default:
        Logger::Throw("Error, type of rigid body unrecognized");
        break;
    }

    // Associate each physics actor with its scene object
    m_actor->userData = so;

    // Add the wrapped actor to the physx scene
    so->scene()->physics()->addActor(this);
}








}