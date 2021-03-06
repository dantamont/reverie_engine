#ifndef GB_PHYSICS_JOINT_H
#define GB_PHYSICS_JOINT_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// External

// QT

// Internal
#include <core/physics/GPhysicsManager.h>

namespace rev {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class PhysicsActor;
class Transform;

//////////////////////////////////////////////////////////////////////////////////
// Macro Definitions
//////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Type Definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Implement constraints, constraint flags


/// @details localFrame0 transform such that the origin is the center of the joint, is relative to global pose (frame) of object
/// localFrame1 = same as above, but for the second object
enum class PhysicsJointType {
    kFixed = 0, // Positions and orientations of contraint frames are the same
    kSpherical, // The origins of the actors contraint frames are coincident
    kRevolute, // Remove all but one rotational degree of freedom between objects (like a door hinge)
    kPrismatic, // Prevents all rotational motion, but allows origin of actor1's frame to move freely along the x-acis of actor0's (like a stepper motor)
    kDistance, // Keeps the origins of the constraint frames within a certain radius of distance
    kD6, // Fixed by default, but individual DOFs can be unlocked to permit any combination of rotations/translations along XYZ axes
    kCustom // User-specified
};

enum class PhysicsJointActorIndex {
    kActor0 = physx::PxJointActorIndex::eACTOR0,
    kActor1,
    COUNT
};

// TODO: Implement breaking
/// @class PhysicsJoint
class PhysicsJoint: public Object, public Serializable {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}


    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    PhysicsJoint();
    virtual ~PhysicsJoint();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    virtual PhysicsJointType jointType() const = 0;

    const char* getName(const char* name) const { 
        checkJoint(); 
        return m_pJoint->getName(); 
    }
    void setName(const char* name) {
        checkJoint();
        return m_pJoint->setName(name);
    }

    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Return the underlying physx joint as the specified type
    template<typename T>
    T* as() const {
        static_assert(std::is_base_of_v<physx::PxJoint, T>, "Class must be a subclass of physx::PxActor");
        return dynamic_cast<T*>(m_joint);
    }

    /// @brief Set the actors for the joint
    void setActors(const PhysicsActor& a1, const PhysicsActor& a2);

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
    const char* className() const override { return "PhysicsJoint"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "rev::PhysicsJoint"; }

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{


    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Create a physx joint of the template type
    /// @details This template enforces the enum class type
    /// @note https://stackoverflow.com/questions/9400581/template-argument-deduction-with-strongly-typed-enumerations/47484461
    template<auto EnumVal, typename ...Types>
    physx::PxJoint* createJoint(const Types&... types) {
       return createJoint_impl<decltype(EnumVal), EnumVal>(types...);
    }
    template<typename EnumType, EnumType EnumValue, typename ...Types>
    physx::PxJoint* createJoint_impl(const Types&... types) {
        // Instantiate the physx joint of the correct type
        physx::PxJoint* joint;
        static_assert(std::is_same_v<EnumType, PhysicsJointType>);
        if constexpr (EnumValue == PhysicsJointType::kFixed) {
            joint = physx::PxFixedJointCreate(*PhysicsManager::Physics(), types...);
        }
        else if constexpr (EnumValue == PhysicsJointType::kSpherical) {
            joint = physx::PxSphericalJointCreate(*PhysicsManager::Physics(), types...);
        }
        else if constexpr (EnumValue == PhysicsJointType::kRevolute) {
            joint = physx::PxRevoluteJointCreate(*PhysicsManager::Physics(), types...);
        }
        else if constexpr (EnumValue == PhysicsJointType::kPrismatic) {
            joint = physx::PxPrismaticJointCreate(*PhysicsManager::Physics(), types...);
        }
        else if constexpr (EnumValue == PhysicsJointType::kDistance) {
            joint = physx::PxDistanceJointCreate(*PhysicsManager::Physics(), types...);
        }
        else if constexpr (EnumValue == PhysicsJointType::kD6) {
            joint = physx::PxD6JointCreate(*PhysicsManager::Physics(), types...);
        }
        else {
            throw("Error, unimplemented physics joint type");
        }

        // Associate the physx joint with this PhysicsJoint
        joint->userData = this;

        return joint;
    }

    bool checkJoint() const { if (!m_pJoint) throw("Error, no joint found"); }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    physx::PxJoint* m_pJoint;

    /// @}

};


/// @class FixedPhysicsJoint
class FixedPhysicsJoint : public PhysicsJoint {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}


    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    FixedPhysicsJoint();
    FixedPhysicsJoint(const PhysicsActor& a0, const Transform& localFrame0, const PhysicsActor& a1, const Transform& localFrame1);
    ~FixedPhysicsJoint();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    virtual PhysicsJointType jointType() const { return PhysicsJointType::kFixed; };

    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

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
    const char* className() const override { return "FixedPhysicsJoint"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "rev::FixedPhysicsJoint"; }

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{


    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    void initialize(const PhysicsActor& a0, const Transform& localFrame0, const PhysicsActor& a1, const Transform& localFrame1);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @}

};

//////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif