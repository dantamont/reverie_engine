/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_SCRIPTED_PROCESS_H
#define GB_SCRIPTED_PROCESS_H
// External
#include "../scripting/GPythonWrapper.h" // everything needed for embedding (needs to be included before Qt stuff pollutes slots)

// QT

// Internal
#include "GProcess.h"

namespace py = pybind11;

namespace rev {
/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class ProcessManager;
class PythonBehaviorWrapper;
class PythonClassScript;
class ScriptComponent;

/////////////////////////////////////////////////////////////////////////////////////////////
// Typedefs
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class ScriptedProcess
/// @brief Represents a python-driven, scripted process
class ScriptedProcess : public Process {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    ScriptedProcess(CoreEngine* engine, 
        ScriptComponent* component,
        ProcessManager* manager);
	~ScriptedProcess();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Refresh the behavior associated with this process (e.g., on file resize)
    void refresh();

    /// @brief Functions to be overriden by process subclasses as needed
    virtual void onInit() override;
    virtual void onUpdate(unsigned long deltaMs) override;
    virtual void onFixedUpdate(unsigned long deltaMs) override;
    virtual void onSuccess() override;
    virtual void onFail() override;
    virtual void onAbort() override;
	
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "ScriptedProcess"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "rev::ScriptedProcess"; }
    /// @}


protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class ProcessManager;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Check that the component is active
    bool componentIsActive() const;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief The behavior driven by this component
    /// @details Deletion ~should~ be driven by the python side
    py::object m_behavior;

    /// @brief Component corresponding to the Python script
    ScriptComponent* m_component;

    /// @brief Whether update ran during the current loop or not
    bool m_ranUpdate;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif