#pragma once

// External
#include "core/scripting/GPythonWrapper.h" // everything needed for embedding (needs to be included before Qt stuff pollutes slots)

// QT

// Internal
#include "fortress/process/GProcess.h"

namespace py = pybind11;

namespace rev {
class CoreEngine;
class ProcessManager;
class PythonBehaviorWrapper;
class PythonClassScript;
class ScriptComponent;

/// @class ScriptedProcess
/// @brief Represents a python-driven, scripted process
class ScriptedProcess : public Process {
public:
	/// @name Constructors/Destructor
	/// @{
    ScriptedProcess(ScriptComponent* component, ProcessManager* manager);
	~ScriptedProcess();
	/// @}

	/// @name Public Methods
	/// @{

    /// @brief Refresh the behavior associated with this process (e.g., on file resize)
    void refresh();

    /// @brief Functions to be overriden by process subclasses as needed
    virtual void onInit() override;
    virtual void onUpdate(double deltaSec) override;
    virtual void onLateUpdate(double deltaSec) override;
    virtual void onPostUpdate(double deltaSec) override;
    virtual void onFixedUpdate(double deltaSec) override;
    virtual void onSuccess() override;
    virtual void onFail() override;
    virtual void onAbort() override;
	
    /// @}

protected:
    /// @name Friends
    /// @{

    friend class ProcessManager;

    /// @}

    /// @name Protected Methods
    /// @{

    /// @brief Check that the component is active
    bool componentIsActive() const;

    /// @}

    /// @name Protected Members
    /// @{

    /// @brief The behavior driven by this component
    /// @details Deletion ~should~ be driven by the python side
    py::object m_behavior;

    /// @brief Component corresponding to the Python script
    ScriptComponent* m_component;

    /// @}
};

} // End namespaces
