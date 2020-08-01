/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_SCRIPTED_PROCESS_H
#define GB_SCRIPTED_PROCESS_H
// External
#include "../../third_party/pythonqt/PythonQt.h"

// QT

// Internal
#include "GbProcess.h"

namespace Gb {
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

    /// @brief Whether the process is threaded as well
    virtual bool isThreaded() const override { return false; }
	
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "ScriptedProcess"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::ScriptedProcess"; }
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

    /// @brief Return main python module
    PythonQtObjectPtr mainContext() const {
        return PythonQt::self()->getMainModule();
    }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief The behavior driven by this component
    /// @details Deletion ~should~ be driven by the python side
    /// @note Can remove with clearVariable
    /// See: https://sourceforge.net/p/pythonqt/discussion/631393/thread/04acf1a9/
    /// https://sourceforge.net/p/pythonqt/discussion/631393/thread/3198439b36/
    /// For importing modules, finding callables from C++:
    /// https://sourceforge.net/p/pythonqt/discussion/631393/thread/fe9f9001/
    /// https://sourceforge.net/p/pythonqt/discussion/631393/thread/6a851191/
    PythonQtObjectPtr m_behavior;
    //PythonBehaviorWrapper* m_behaviorWrapper;

    /// @brief Component corresponding to the Python script
    ScriptComponent* m_component;

    /// @brief Whether update ran during the current loop or not
    bool m_ranUpdate;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif