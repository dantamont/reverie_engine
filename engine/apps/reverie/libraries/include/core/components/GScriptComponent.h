#ifndef GB_SCRIPT_COMPONENT_H
#define GB_SCRIPT_COMPONENT_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// Qt
#include <QString>

// Project
#include "GComponent.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class PythonClassScript;
class ScriptedProcess;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** @class ScriptComponent
    @brief  A python script to be attached to a SceneObject
*/
class ScriptComponent: public Component {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static Methods
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    ScriptComponent(const std::shared_ptr<SceneObject>& object);
    ~ScriptComponent();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Reset and resize the script associated with this component
    void reset();

    /// @brief Set the script file and checkValidity behavior
    void initializeBehavior(const QString& filepath);

    /// @brief Enable the behavior of this script component
    virtual void enable() override;

    /// @brief Disable the behavior of this script component
    virtual void disable() override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief The process associated with the script component
    std::shared_ptr<ScriptedProcess> process() { return m_scriptProcess; }

    /// @brief Return the python-class script used by this component
    PythonClassScript* script() { return m_script; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const ScriptComponent& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, ScriptComponent& orObject);


    /// @}

protected:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Path to the python script for this component
    QString m_path;

    /// @brief The python script for this component
    PythonClassScript* m_script;

    /// @brief The process associated with this script component
    std::shared_ptr<ScriptedProcess> m_scriptProcess;

    /// @}


};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 
