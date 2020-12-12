#ifndef GB_PHYSICS_COMPONENTS_H
#define GB_PHYSICS_COMPONENTS_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// External

// Project
#include "GbComponent.h"
#include "../containers/GbContainerExtensions.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
// Macros
/////////////////////////////////////////////////////////////////////////////////////////////
#ifdef GB_USE_DOUBLE
typedef double real_g;
#else
typedef float real_g;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SceneObject;
class RigidBody;
class CharacterController;
class CCTManager;
template <typename D, size_t size> class Vector;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/** @class RigidBodyComponent
*/
class RigidBodyComponent: public Component{
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    
    RigidBodyComponent();
    RigidBodyComponent(const RigidBodyComponent& comp);
    RigidBodyComponent(const std::shared_ptr<SceneObject>& object);
    ~RigidBodyComponent();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Gb Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "RigidBodyComponent"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::RigidBodyComponent"; }
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{    

    std::unique_ptr<RigidBody>& body() { return m_rigidBody; }

    /// @brief Max number of allowed components per scene object
    virtual int maxAllowed() const override { return 1; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{    

    /// @brief Update the physics for the current update frame
    void updateTransformFromPhysics();

    /// @brief Reinitialize the body with the latest settings
    void refreshBody();

    /// @brief Enable this component
    virtual void enable();

    /// @brief Disable this component
    virtual void disable();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friends 
    /// @{

    friend class SceneObject;

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{  

    void initializeDefault();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{    

    /// @brief The rigid body for this component
    std::unique_ptr<RigidBody> m_rigidBody = nullptr;

    /// @}

};
Q_DECLARE_METATYPE(RigidBodyComponent);



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class CharControlComponent
class CharControlComponent : public Component {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    CharControlComponent();
    CharControlComponent(const std::shared_ptr<SceneObject>& object);
    ~CharControlComponent();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Gb Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "CharControlComponent"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::CharControlComponent"; }
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{    

    const std::shared_ptr<CharacterController>& controller() const { return m_controller; }

    /// @brief Max number of allowed components per scene object
    virtual int maxAllowed() const override { return 1; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{    

    /// @brief Move the object
    /// @note Gravity needs to be applied manually, since character controllers are kinematic
    void move(const Vector<real_g, 3>& disp);
    void setGravity(const Vector<real_g, 3>& gravity);

    /// @brief Enable this component
    virtual void enable() override;

    /// @brief Disable this component
    virtual void disable() override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friends 
    /// @{

    friend class SceneObject;

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{  

    const std::shared_ptr<CCTManager>& cctManager() const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{    

    /// @brief The rigid body for this component
    std::shared_ptr<CharacterController> m_controller = nullptr;

    /// @}

};
Q_DECLARE_METATYPE(CharControlComponent);


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 
