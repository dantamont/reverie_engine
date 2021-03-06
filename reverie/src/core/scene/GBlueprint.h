/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_BLUEPRINT_H
#define GB_BLUEPRINT_H

// Internal
#include <core/mixins/GLoadable.h>
#include <core/mixins/GNameable.h>

namespace rev {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class SceneObject;

/////////////////////////////////////////////////////////////////////////////////////////////
// Type definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class Blueprint
/// @brief Class for managing cached scene object configurations
class Blueprint: public Identifiable, public Nameable, public Serializable  {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    Blueprint();
    Blueprint(const SceneObject& sceneObject);
    ~Blueprint();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    const QJsonObject& sceneObjectJson() const { return m_soJson; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    Blueprint& operator=(const Blueprint& other);

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

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{


    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Json for the scene object prefab
    QJsonObject m_soJson;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif