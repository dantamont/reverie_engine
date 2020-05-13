/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_THREADED_PROCESS_H
#define GB_THREADED_PROCESS_H

// QT
#include <QThread>

// Internal
#include "GbProcess.h"

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class ProcessManager;

/////////////////////////////////////////////////////////////////////////////////////////////
// Typedefs
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class Process
/// @brief Represents a thread-friendly process
class ThreadedProcess: public Process {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    ThreadedProcess(CoreEngine* engine, ProcessManager* manager);
	virtual ~ThreadedProcess();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Functions to be overriden by process subclasses as needed
    virtual void onInit() override{ m_state = kRunning; }
    virtual void onUpdate(unsigned long deltaMs);
    virtual void onSuccess() override;
    virtual void onFail() override { }
    virtual void onAbort() override{}

    /// @brief Required override for QRunnable for threaded process
    virtual void run();

    virtual bool isThreaded() const override { return true; }
	
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "ThreadedProcess"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::ThreadedProcess"; }
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
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{
    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif