#pragma once

// Internal
#include "fortress/types/GLoadable.h"
#include "fortress/types/GNameable.h"
#include "fortress/types/GIdentifiable.h"

namespace rev {

class SceneObject;


/// @class Blueprint
/// @brief Class for managing cached scene object configurations
class Blueprint: public IdentifiableInterface, public NameableInterface  {
public:
    /// @name Constructors/Destructor
    /// @{

    Blueprint();
    Blueprint(const SceneObject& sceneObject);
    ~Blueprint();

    /// @}

    /// @name Properties
    /// @{

    const json& sceneObjectJson() const { return m_soJson; }

    /// @}

    /// @name Operators
    /// @{

    Blueprint& operator=(const Blueprint& other);

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const Blueprint& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, Blueprint& orObject);


    /// @}

protected:

    /// @name Protected Members
    /// @{

    /// @brief Json for the scene object prefab
    json m_soJson;

    /// @}
};


} // End namespaces
