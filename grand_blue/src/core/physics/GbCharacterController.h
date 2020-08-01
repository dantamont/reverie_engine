#ifndef GB_CHARACTER_CONTROLLER_H
#define GB_CHARACTER_CONTROLLER_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// External
#include <characterkinematic\PxControllerManager.h>
#include <characterkinematic\PxController.h>

// QT

// Internal
#include "GbPhysicsManager.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class SceneObject;
class Scene;
class Transform;
class PhysicsGeometry;
class PhysicsMaterial;
class PhysicsShapePrefab;
class PhysicsScene;
class PhysicsShape;
class CoreEngine;
class CCTManager;
/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class ControllerFilters
// TODO: Implement
class ControllerFilters : public physx::PxControllerFilters {
public:
    ControllerFilters();
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @struct ControllerDescription
class ControllerDescription : public PhysicsBase {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum ControllerType {
        kBox,
        kCapsule
    };

    static std::shared_ptr<ControllerDescription> create(const QJsonValue& json);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Destructor
    /// @{

    ControllerDescription(ControllerType type);
    virtual ~ControllerDescription();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual std::shared_ptr<physx::PxControllerDesc> toPhysX() const = 0;

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
    const char* className() const override { return "ControllerDescription"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::ControllerDescription"; }

    /// @}


    //--------------------------------------------------------------------------------------------
    /// @name  Members
    /// @{

    Vector3 m_initialPosition = {0.0, 0.0, 0.0}; // The position of the character
    Vector3g m_upDirection = {0.0f, 1.0f, 0.0f}; // Specifies the 'up' direction
    float m_slopeLimit= 0.707f; // The maximum slope which the character can walk up (in radians)
    float m_invisibleWallHeight = 0.0f; // Height of invisible walls created around non-walkable triangles, e.g. those defined by slope limit
    
    /// @details The 'maxJumpHeight' variable is used to extend the size of the collision volume downward. This way, all the non-walkable triangles 
    /// are properly found by the collision queries and it becomes impossible to 'jump over' invisible walls.
    float m_maxJumpHeight = 0.0f; // Maximum height a jumping character can reach. Only used if invisible walls are created
    float m_contactOffset = 0.1f; // The contact offset used by the controller, used to avoid numerical precision issues. Specifies a skin around the object within which contacts will be generated
    float m_stepOffset = 0.5f; // Defines the maximum height of an obstacle which the character can climb
    float m_density = 10.0f; // Density of the underlying kinematic actor
    float m_scaleCoeff = 0.99f; // Uniform scaling for the kinematic actor under the hood, should be a bit smaller than one
    float m_volumeGrowth = 1.5f; // The amount of space around the controller that can be cached to improve performance, ideally between 1.0f and 2.0f

    /// @brief Specifies a user report callback.
    // This report callback is called when the character collides with shapes and other characters.
    // Setting this to NULL disables the callback.
    physx::PxUserControllerHitReport* m_reportCallback = nullptr;

    /// @brief Specifies a user behavior callback.
    // This behavior callback is called to customize the controller's behavior w.r.t. touched shapes.
    // Setting this to NULL disables the callback.
    physx::PxControllerBehaviorCallback* m_behaviorCallback = nullptr;

    /// @brief Use a deletion listener to get informed about released objects and clear internal caches if needed.
    // If a character controller registers a deletion listener, it will get informed about released
    // objects. That allows the controller to invalidate cached data that connects to a released object.
    // If a deletion listener is not registered,
    // PxController::invalidateCache has to be called manually after objects have been released.
    bool m_registerDeletionListener = true;

    /// @brief The material for the actor associated with the controller.
    /// @details The controller internally creates a rigid body actor.This parameter specifies the material of the actor.
    std::shared_ptr<PhysicsMaterial> m_material = nullptr;

    /// @brief 0 just prevents climbing, 1 also forces slidiing
    int m_unwalkableMode = 0;

    /// @brief User specified data associated with the controller
    void* m_userData = nullptr;

    ControllerType m_type;

    /// @}

protected:

    /// @brief Load attributes into the given physx description
    void loadIntoPhysx(std::shared_ptr<physx::PxControllerDesc> pxDesc) const;
};
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class BoxControllerDescription
/// @brief Class representing a box controller description
class BoxControllerDescription : public ControllerDescription {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Destructor
    /// @{
    BoxControllerDescription();
    virtual ~BoxControllerDescription();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual std::shared_ptr<physx::PxControllerDesc> toPhysX() const override;

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
    const char* className() const override { return "BoxControllerDescription"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::BoxControllerDescription"; }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Members
    /// @{

    float m_halfHeight = 0.5;
    float m_halfSideExtent = 1.0;
    float m_halfForwardExtent = 0.5;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class CapsuleControllerDescription
/// @brief Class representing a box controller description
class CapsuleControllerDescription : public ControllerDescription {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Destructor
    /// @{
    CapsuleControllerDescription();
    virtual ~CapsuleControllerDescription();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual std::shared_ptr<physx::PxControllerDesc> toPhysX() const override;

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
    const char* className() const override { return "CapsuleControllerDescription"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::CapsuleControllerDescription"; }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Members
    /// @{

    float m_radius = 1.0;
    float m_height = 1.0;
    size_t m_climbingMode = 0; //0 is easy, 1 is contrained, which limites according to step offset

    /// @}

};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class CharacterController
/// @brief Class representing a PhysX character controller
/// See: https://gameworksdocs.nvidia.com/PhysX/4.0/documentation/PhysXGuide/Manual/CharacterControllers.html
class CharacterController : public PhysicsBase {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum ControllerType {
        kBox = physx::PxControllerShapeType::eBOX,
        kCapsule = physx::PxControllerShapeType::eCAPSULE
    };

    enum CollisionType
    {
        kCollisionSides = (1 << 0),	//!< Character is colliding to the sides.
        kCollisionUp = (1 << 1),	//!< Character has collision above.
        kCollisionDown = (1 << 2)	//!< Character has collision below.
    };

    enum UnwalkableMode {
        kPreventClimbing = (1 << 0),
        kForceSliding = (1 << 1)
    };
    typedef QFlags<UnwalkableMode> WalkableFlags;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Destructor
    /// @{
    virtual ~CharacterController();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    float heightOffset() const {
        return m_heightOffset;
    }
    void setHeightOffset(float offset) { m_heightOffset = offset; }

    /// @brief Optional value to set that enforces gravity on character controller
    const Vector3g& getGravity() const { return m_gravity; }
    void setGravity(const Vector3g& g) {
        m_gravity = g;
    }

    const Vector3g& getFallVelocity() const { return m_fallVelocity; }
    void setFallVelocity(const Vector3g& v) { m_fallVelocity = v; }

    float getTerminalVelocity() const { return m_terminalVelocity; }

    /// @brief Whether or not the controller is grounded
    bool isGrounded() const { return m_isGrounded; }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Either a box or a capsule controller
    ControllerType getType() const {
        return ControllerType(m_controller->getType());
    }

    /// @brief Move the controller, while checking for collisions
    /// @details Elapsed time is time elapsed since the last move call
    // TODO: Implement obstacles
    QFlags<CharacterController::CollisionType> move(const Vector3g& displacement);
    QFlags<CharacterController::CollisionType> move(const Vector3& displacement);
    QFlags<CharacterController::CollisionType> move(const Vector3& displacement,
        const ControllerFilters& filters, const physx::PxObstacleContext* obstacles = nullptr);

    /// @brief Position of the centroid off the controller
    Vector3 getPosition() const {
        const physx::PxExtendedVec3& pos = m_controller->getPosition();
        return Vector3(pos.x, pos.y, pos.z);
    }

    /// @brief "Teleports" the CCT, use move to check for collisions
    bool setPosition(const Vector3& pos) {
        return m_controller->setPosition({ pos.x(), pos.y(), pos.z() });
    }

    /// @brief "Teleports" the CCT, and sets initial position, use move to check for collisions
    bool setInitialPosition(const Vector3& pos) {
        m_description->m_initialPosition = pos;
        return m_controller->setPosition({ pos.x(), pos.y(), pos.z() });
    }

    void updateFallVelocity(float dt);

    /// @brief Get position at the bottom-center of the controller
    /// @details Note, takes the contact offset into account
    Vector3 getFootPosition() const {
        const physx::PxExtendedVec3& footPos = m_controller->getFootPosition();
        return Vector3(footPos.x, footPos.y, footPos.z);
    }

    bool setFootPosition(const Vector3& footPos) {
        return m_controller->setFootPosition({ footPos.x(), footPos.y(), footPos.z() });
    }

    /// @brief Returns the rigid body actor associated with this controller
    // TODO: Use userData attribute of actor to identify as a character controller
    physx::PxRigidDynamic* getActor() const {
        return m_controller->getActor();
    }

    /// @brief Return the physx scene associated with this controller
    physx::PxScene* getPxScene() const {
        return m_controller->getScene();
    }

    /// @brief How large a distance the CCT can climb over (for boxes).  For capsules, it will be a bit larger than this
    float getStepOffset() const { return m_controller->getStepOffset(); }
    void setStepOffset(float offset) { m_controller->setStepOffset(offset); }

    /// @brief specifies how a CCT interacts with non-walkable parts.
    /// @details This is only used when slopeLimit is non zero. It is currently enabled for static actors only, and not supported for spheres or capsules.
    void setUnwalkableMode(const WalkableFlags& flags) {
        if (flags.testFlag(kForceSliding))
            m_controller->setNonWalkableMode(physx::PxControllerNonWalkableMode::ePREVENT_CLIMBING_AND_FORCE_SLIDING);
        else
            m_controller->setNonWalkableMode(physx::PxControllerNonWalkableMode::ePREVENT_CLIMBING);
    }

    WalkableFlags getUnwalkableMode() const {
        WalkableFlags flags;
        size_t mode = (size_t)m_controller->getNonWalkableMode();
        flags.setFlag(kPreventClimbing, true);
        if (mode) {
            flags.setFlag(kForceSliding, true);
        }
        return flags;
    }

    /// @brief An additional offset for what constitutes contact with the controller
    float getContactOffset() const {
        return m_controller->getContactOffset();
    }
    void setContactOffset(float offset) { m_controller->setContactOffset(offset); }

    /// @brief Can be set every frame, allowing the character to navigate on spherical worlds
    /// @details Note that this effectively rotates the controller, which may cause it to clip through geometry
    Vector3g getUpDirection() const {
        physx::PxVec3 vec = m_controller->getUpDirection();
        return Vector3g(vec.x, vec.y, vec.z);
    }
    void setUpDirection(const Vector3g& vec) {
        m_controller->setUpDirection({ vec.x(), vec.y(), vec.z() });
    }

    /// @brief Retrieve the slope limit for the controller
    /// @details This limit cannot be set at runtime.  Changes will have no effect
    float getSlopeLimit() const { return m_controller->getSlopeLimit(); }

    /// @brief Resizes the controller to the given height, moving the center position and preserving the bottom position
    void resize(float height) { m_controller->resize(height); }

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
    const char* className() const override { return "CharacterController"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::CharacterController"; }

    /// @}

protected:
    friend class CCTManager;
    friend class PhysicsManager;

    //--------------------------------------------------------------------------------------------
    /// @name Constructors
    /// @{

    CharacterController(std::shared_ptr<ControllerDescription> desc,
        const std::shared_ptr<SceneObject>& sceneObject);

    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    CoreEngine* getEngine() const;
    const std::shared_ptr<CCTManager>& getManager() const;

    /// @brief Obtain pointer to this actor's scene
    inline std::shared_ptr<SceneObject> sceneObject() const {
        if (std::shared_ptr<SceneObject> so = m_sceneObject.lock()) {
            return so;
        }
        else {
            return nullptr;
        }
    }

    void initialize();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Last time a move was called
    float m_lastTimeInSecs = 0;

    /// @brief The minimum distance to consider, anything less will not move the character
    float m_minDistance = 1e-6f;

    /// @brief Weak pointer to the scene that this actor belongs to
    std::weak_ptr<SceneObject> m_sceneObject;

    physx::PxController* m_controller;

    /// @brief The description used to initialize this controller
    std::shared_ptr<ControllerDescription> m_description;

    bool m_isGrounded = false;

    /// @brief The offset from the scene object height that this capsule has
    float m_heightOffset = 0.0f;

    /// @brief Enforces gravitational acceleration on the controller
    Vector3g m_gravity = { 0.0f, -9.81f, 0.0f };
    Vector3g m_fallVelocity = { 0.0f, 0.0f, 0.0f };

    /// @brief Terminal velocity
    float m_terminalVelocity = 55.0f; // m/s

    /// @}

};
typedef QFlags<CharacterController::UnwalkableMode> WalkableFlags;
typedef QFlags<CharacterController::CollisionType> ControllerCollisionFlags;
Q_DECLARE_OPERATORS_FOR_FLAGS(ControllerCollisionFlags)


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class BoxController
/// @brief Class representing a box character controller
class BoxController : public CharacterController {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    float getHalfHeight() const { return controller()->getHalfHeight(); }
    bool setHalfHeight(float hh) { 
        description()->m_halfHeight = hh;
        return controller()->setHalfHeight(hh); 
    }

    float getHalfSideExtent() const { return controller()->getHalfSideExtent(); }
    bool setHalfSideExtent(float hs) { 
        description()->m_halfSideExtent = hs;
        return controller()->setHalfSideExtent(hs);
    }

    float getHalfForwardExtent() const { return controller()->getHalfForwardExtent(); }
    bool setHalfForwardExtent(float hf) { 
        description()->m_halfForwardExtent = hf;
        return controller()->setHalfForwardExtent(hf); 
    }

    /// @}

protected:
    friend class CCTManager;

    //--------------------------------------------------------------------------------------------
    /// @name Constructor
    /// @{

    BoxController(std::shared_ptr<BoxControllerDescription> desc,
        const std::shared_ptr<SceneObject>& sceneObject);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    std::shared_ptr<BoxControllerDescription> description() const {
        return std::static_pointer_cast<BoxControllerDescription>(m_description);
    }

    physx::PxBoxController* controller() const {
        return static_cast<physx::PxBoxController*>(m_controller);
    }

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class CapsuleController
/// @brief Class representing a capsule character controller
/*
A capsule character controller.

The capsule is defined as a position, a vertical height, and a radius. The height is the distance between the two sphere centers at the end of the capsule. In other words:

p = pos (returned by controller)
h = height
r = radius

p = center of capsule
top sphere center = p.y + h*0.5
bottom sphere center = p.y - h*0.5
top capsule point = p.y + h*0.5 + r
bottom capsule point = p.y - h*0.5 - r
*/
class CapsuleController : public CharacterController {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum ClimbingMode {
        kEasy,
        kConstrained
    };

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    float getRadius() const { return controller()->getRadius(); }
    bool setRadius(float radius) { 
        description()->m_radius = radius;
        return controller()->setRadius(radius);
    }

    /// @brief Setting the height will not preserve the bottom position of the capsule, the center will remain
    float getHeight() const { return controller()->getHeight(); }
    bool setHeight(float height) { 
        description()->m_height = height;
        return controller()->setHeight(height);
    }

    ClimbingMode getClimbingMode() const {
        return ClimbingMode(controller()->getClimbingMode());
    }
    bool setClimbingMode(ClimbingMode mode) {
        controller()->setClimbingMode(physx::PxCapsuleClimbingMode::Enum(mode));
    }

    /// @}

protected:
    friend class CCTManager;


    //--------------------------------------------------------------------------------------------
    /// @name Constructor
    /// @{

    CapsuleController(std::shared_ptr<CapsuleControllerDescription> desc,
        const std::shared_ptr<SceneObject>& sceneObject);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    std::shared_ptr<CapsuleControllerDescription> description() const {
        return std::static_pointer_cast<CapsuleControllerDescription>(m_description);
    }

    physx::PxCapsuleController* controller() const {
        return static_cast<physx::PxCapsuleController*>(m_controller);
    }

    /// @}
};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class CCTManager
/// @brief Class representing a Character Controller Manager
/// See: https://gameworksdocs.nvidia.com/PhysX/4.0/documentation/PhysXGuide/Manual/CharacterControllers.html
class CCTManager : public PhysicsBase {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    CCTManager(PhysicsScene& scene);
    virtual ~CCTManager();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    std::unordered_map<physx::PxController*, std::shared_ptr<CharacterController>>& controllers() {
        return m_controllers;
    }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Initialize a character controller
    std::shared_ptr<CharacterController> createController(const QJsonValue& json);
    std::shared_ptr<CharacterController> createController(const std::shared_ptr<ControllerDescription>& desc,
        const std::shared_ptr<SceneObject>& sceneObject);


    uint32_t getNumControllers() const { return m_controllerManager->getNbControllers(); }
    const std::shared_ptr<CharacterController>& getController(uint32_t idx) {
        return m_controllers[m_controllerManager->getController(idx)];
    }

    void removeController(const CharacterController& controller);

    /// @brief Clears all controllers from the manager
    void clearControllers() { 
        m_controllerManager->purgeControllers(); 
        m_controllers.clear();
    }

    /// @brief If true, tries to correct character controller overlap with static geometry
    void toggleOverlapRecovery(bool flag) { m_controllerManager->setOverlapRecoveryModule(flag); }

    /// @brief Can disable precise collision tests, generally when overlap recovery is toggled on
    void togglePreciseSweeps(bool flag) { m_controllerManager->setPreciseSweeps(flag); }

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
    const char* className() const override { return "CharacterController"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::CharacterController"; }

    /// @}

protected:
    friend class CharacterController;

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{


    /// @brief Obtain pointer to this actor's scene
    inline std::shared_ptr<Scene> scene() const {
        if (std::shared_ptr<Scene> scn = m_scene.lock()) {
            return scn;
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
    std::weak_ptr<Scene> m_scene;

    physx::PxControllerManager* m_controllerManager;

    /// @brief Map of physx controllers and their corresponding wrapped character controllers
    std::unordered_map<physx::PxController*, std::shared_ptr<CharacterController>> m_controllers;

    /// @}

};





//////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif