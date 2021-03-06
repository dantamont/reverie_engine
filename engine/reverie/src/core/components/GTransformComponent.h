#ifndef GB_TRANSFORM_COMPONENT_H
#define GB_TRANSFORM_COMPONENT_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// External

// Project
#include "GComponent.h"
#include "../geometry/GTransform.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SceneObject;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/** @class TransformComponent
    @brief A component representing a transform, consisting of a translation, a rotation, and a scaling
*/
class TransformComponent: public Component, public Transform{
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Enum for which transforms to inherit from a parent state
    enum InheritanceType {
        kAll, // inherit entire parent world matrix
        kTranslation, // inherit only the translation of the world state (e.g. won't orbit parent)
        kPreserveOrientation // preserves the local orientation of this transform 
    };

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    
    TransformComponent();
    TransformComponent(const std::shared_ptr<SceneObject>& object);
    ~TransformComponent();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "TransformComponent"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "rev::TransformComponent"; }
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{    

    /// @brief Max number of allowed components per scene object
    virtual int maxAllowed() const override{ return 1; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{    

    /// @brief Set value from a transform
    void set(const Transform& transform);

    /// @brief Compute world matrix
    void computeWorldMatrix() override;

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

    friend class AbstractTransformComponent;
    friend class AffineComponent;
    friend class SceneObject;

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{  

    /// @brief Initialize this transform
    void initialize();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{    

    /// @}

};
Q_DECLARE_METATYPE(TransformComponent);


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 
