#ifndef GB_PHYSICS_ACTOR_H
#define GB_PHYSICS_ACTOR_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// External

// QT

// Internal
#include "GbPhysics.h"

namespace Gb {

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
    PhysicsActor(const std::shared_ptr<SceneObject>& sceneObject, ActorType type = kAbstract);
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

    void enableGravity();
    void disableGravity();
    
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PhysicsActor"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PhysicsActor"; }

    /// @}

protected:

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{


    /// @brief Obtain pointer to this actor's scene
    inline std::shared_ptr<SceneObject> sceneObject() const {
        if (const std::shared_ptr<SceneObject>& so = m_sceneObject.lock()) {
            return so;
        }
        else {
            return nullptr;
        }
    }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Weak pointer to the scene that this actor belongs to
    std::weak_ptr<SceneObject> m_sceneObject;

    /// @brief Pointer to the actor wrpaped by this class
    physx::PxActor* m_actor = nullptr;

    /// @brief Type of actor
    ActorType m_actorType;

    /// @}

};


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
    RigidBody(const std::shared_ptr<SceneObject>& so);
    RigidBody(const std::shared_ptr<SceneObject>& so, 
        const Transform& transform,
        std::shared_ptr<PhysicsShapePrefab> shape,
        RigidType rType, 
        float density = 1.0);
    RigidBody(const std::shared_ptr<SceneObject>& so, 
        const Transform& transform, 
        std::shared_ptr<PhysicsGeometry> geometry,
        std::shared_ptr<PhysicsMaterial> material,
        RigidType rType,
        float density = 1.0);
    ~RigidBody();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    float density() const { return m_density; }
    void setDensity(float density) { 
        m_density = density;
        if(dynamicBody())
            updateMassAndInertia();
    }

    /// @brief The type of rigid body
    /// @details Setting this requires a call to the reinitialize routine in order to take effect on
    /// the underlying PhysX body
    RigidType rigidType() const { return m_rigidType; }
    void setRigidType(RigidType type) { m_rigidType = type; }

    std::vector<PhysicsShape>& shapes() { return m_shapes; }

    /// @brief Whether the body is kinematic or not
    bool isKinematic() const { return m_isKinematic; }

    physx::PxRigidActor* getRigidActor() const { return m_actor->is<physx::PxRigidActor>(); }
    physx::PxRigidActor* rigidActor() { return m_actor->is<physx::PxRigidActor>(); }

    physx::PxRigidBody* getBody() const { return m_actor->is<physx::PxRigidBody>(); }
    physx::PxRigidBody* body() { return m_actor->is<physx::PxRigidBody>(); }

    physx::PxRigidDynamic* dynamicBody() { return m_actor->is<physx::PxRigidDynamic>(); }

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
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "RigidBody"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::RigidBody"; }

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
    void initialize(const std::shared_ptr<SceneObject>& so,
        const Transform& transform, PhysicsShapePrefab& prefab);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Shape instantiations
    /// @details Can also be obtained with PxRigidActor::getShapes
    std::vector<PhysicsShape> m_shapes;

    /// @brief The type of rigid body, e.g. static vs dynamic
    RigidType m_rigidType;

    /// @brief Whether or not the object is kinematic
    /// @details If the object is kinematic, it may not participate in simulation collision, only for triggers
    /// or scene queries of moving objects under animation control
    // TODO: Implement
    bool m_isKinematic;

    float m_density;

    /// @}

};



//////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif