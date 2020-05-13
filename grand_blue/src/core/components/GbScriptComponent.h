#ifndef GB_SCRIPT_COMPONENT_H
#define GB_SCRIPT_COMPONENT_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// Qt
#include <QString>

// Project
#include "GbComponent.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

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

    /// @brief Return the python class script used by this component
    std::shared_ptr<PythonClassScript> script() { return m_script; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name GB object Properties
    /// @{
    /// @property className
    const char* className() const override { return "ScriptComponent"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::ScriptComponent"; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

protected:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Path to the python script for this component
    QString m_path;

    /// @brief The python script for this component
    std::shared_ptr<PythonClassScript> m_script;

    /// @brief The process associated with this script component
    std::shared_ptr<ScriptedProcess> m_scriptProcess;

    /// @}


};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 
