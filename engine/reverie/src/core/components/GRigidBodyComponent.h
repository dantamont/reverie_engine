#ifndef GB_RIGID_BODY_COMPONENT_H
#define GB_RIGID_BODY_COMPONENT_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// External

// Project
#include "GComponent.h"
#include "../containers/GContainerExtensions.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

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
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "RigidBodyComponent"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "rev::RigidBodyComponent"; }
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
    QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 
