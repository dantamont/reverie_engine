#ifndef GB_PHYSICS_ACTOR_H
#define GB_PHYSICS_ACTOR_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// External

// QT

// Internal
#include "GPhysics.h"
#include <core/containers/GFlags.h>

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class SceneObject;
class Transform;
class PhysicsGeometry;
class PhysicsMaterial;
class PhysicsShapePrefab;
class PhysicsScene;
class PhysicsShape;
class CoreEngine;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

// TODO: Use this
enum class PhysicsActorFlag {
    kVisualization = physx::PxActorFlag::eVISUALIZATION, // Enable debug renderer for this actor
    kDisableGravity = physx::PxActorFlag::eDISABLE_GRAVITY, // Disable gravity for this actor
    kSendSleepNotifies = physx::PxActorFlag::eSEND_SLEEP_NOTIFIES, // Enables the sending of PxSimulationEventCallback::onWake() and PxSimulationEventCallback::onSleep() notify events. 
    kDisableSimulation = physx::PxActorFlag::eDISABLE_SIMULATION // Dosab;es simulation for the actor, only supported by PxRigidStatic and PxRigidDynamic
};
MAKE_BITWISE(PhysicsActorFlag);
MAKE_FLAGS_EXT(PhysicsActorFlag, PhysicsActorFlags, physx::PxU8)

/// @class PhysicsActor
/// @brief Class representing a base physics object
/// See: https://docs.nvidia.com/gameworks/content/gameworkslibrary/physx/apireference/files/classPxActor.html
class PhysicsActor : public PhysicsBase {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum ActorType {
        kAbstract,
        kRigidBody
    };

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    PhysicsActor(ActorType type = kAbstract);
    virtual ~PhysicsActor();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief Return the scene of this actor
    std::shared_ptr<PhysicsScene> physicsScene() const;

    /// @brief Return the type of actor
    ActorType getType() const { return m_actorType; }

    /// @brief Return actor
    physx::PxActor* actor() { return m_actor; }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Return the encapsulated actor as the specified type
    template<typename T>
    T* as() const {
        static_assert(std::is_base_of_v<physx::PxActor, T>, "Class must be a subclass of physx::PxActor");
        return m_actor->is<T>();
    }

    void enableGravity();
    void disableGravity();
    
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PhysicsActor"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "rev::PhysicsActor"; }

    /// @}

protected:

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{


    /// @brief Obtain pointer to this actor's scene
    inline SceneObject* sceneObject() const {
        if (!m_actor) {
            return nullptr;
        }
        else {
            return (SceneObject*)m_actor->userData;
        }
    }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Pointer to the actor wrpaped by this class
    physx::PxActor* m_actor = nullptr;

    /// @brief Type of actor
    ActorType m_actorType;

    /// @}

};

// TODO: Store these instead of m_isKinematic, or even better, don't store flags here at all, just access them
enum class RigidBodyFlag {
    kKinematic = physx::PxRigidBodyFlag::eKINEMATIC,
    kUseKinematicTargetForSceneQueries = physx::PxRigidBodyFlag::eUSE_KINEMATIC_TARGET_FOR_SCENE_QUERIES,
    kEnableCCD = physx::PxRigidBodyFlag::eENABLE_CCD, // Enables swept integration for the actor
    kEnableCCDFriction = physx::PxRigidBodyFlag::eENABLE_CCD_FRICTION, // Enable friction in swept integration
    kEnablePoseIntegrationPReview = physx::PxRigidBodyFlag::eENABLE_POSE_INTEGRATION_PREVIEW,
    kEnableSpeculativeCCD = physx::PxRigidBodyFlag::eENABLE_SPECULATIVE_CCD,
    kEnableCCDMaxContactImpulse = physx::PxRigidBodyFlag::eENABLE_CCD_MAX_CONTACT_IMPULSE,
    kRetainAccelerations = physx::PxRigidBodyFlag::eRETAIN_ACCELERATIONS
};
MAKE_BITWISE(RigidBodyFlag);
MAKE_FLAGS_EXT(RigidBodyFlag, RigidBodyFlags, physx::PxU8)

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class RigidBody
/// @brief Class representing a rigid body
class RigidBody : public PhysicsActor {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    
    enum RigidType {
        kStatic = 0,
        kDynamic,
        kArticulationLink // TODO: Implement
    };

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    RigidBody(SceneObject* so);
    RigidBody(SceneObject* so,
        PhysicsShapePrefab* shape,
        RigidType rType, 
        float density = 1.0);
    ~RigidBody();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    RigidBodyFlags rigidBodyFlags() const {
        return (RigidBodyFlags)as<physx::PxRigidDynamic>()->getRigidBodyFlags();
    }
    void setRigidBodyFlags(RigidBodyFlags flags) {
        as<physx::PxRigidDynamic>()->setRigidBodyFlags((physx::PxRigidBodyFlags)flags);
    }

    float density() const { return m_density; }
    void setDensity(float density);

    /// @brief The type of rigid body
    /// @details Setting this requires a call to the reinitialize routine in order to take effect on
    /// the underlying PhysX body
    RigidType rigidType() const { return m_rigidType; }
    void setRigidType(RigidType type) { m_rigidType = type; }

    std::vector<PhysicsShape*>& shapes() { return m_shapes; }

    /// @brief Whether the body is kinematic or not
    bool isKinematic() const { return m_isKinematic; }

    /// @brief Clears shapes from the rigid body
    /// @details Does not need to be called on destruction, detachment is automatic
    void clearShapes();

    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Return shape at index
    PhysicsShape& shape(int index = 0);

    /// @brief Toggle for scene queries
    void toggleContact(bool contact);

    /// @brief Toggle for contact
    void toggleQueries(bool isQueried);

    bool updateMassAndInertia();

    void setKinematic(bool isKinematic, bool setMember);

    /// @brief Add a shape to the rigid body
    void addShape(PhysicsShapePrefab& prefab);

    /// @brief Get the transform of the rigid body
    Transform getTransform() const;

    /// @brief Set the transform of the rigid bdy
    void setTransform(const Transform& t);

    /// @brief Send the body to sleep
    void sleep();

    /// @brief Awaken the body
    void awaken();

    void setAngularDamping(float damping);

    void setLinearVelocity(const Vector3f& vel);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "RigidBody"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "rev::RigidBody"; }

    void reinitialize();
    void reinitialize(PhysicsShapePrefab& prefab);

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class RigidBodyComponent;
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Initialize the rigid body
    void initialize(SceneObject* so, const Transform& transform, PhysicsShapePrefab& prefab);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Shape instantiations
    /// @details Can also be obtained with PxRigidActor::getShapes
    std::vector<PhysicsShape*> m_shapes;

    /// @brief The type of rigid body, e.g. static vs dynamic
    RigidType m_rigidType;

    /// @brief Whether or not the object is kinematic
    /// @details If the object is kinematic, it may not participate in simulation collision, only for triggers
    /// or scene queries of moving objects under animation control
    bool m_isKinematic;

    float m_density;

    /// @}

};



//////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif