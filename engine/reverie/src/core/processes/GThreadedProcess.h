/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_THREADED_PROCESS_H
#define GB_THREADED_PROCESS_H

// std
#include <exception>
#include <stdexcept>

// QT
//#include <QThread>

// Internal
#include "GProcess.h"

namespace rev {
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

    std::mutex& exceptionMutex() { return m_exceptionLock; }
    const std::exception_ptr& exception() const { return m_exceptionPtr; }

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
	
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "ThreadedProcess"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "rev::ThreadedProcess"; }
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

    mutable std::mutex m_exceptionLock;
    std::exception_ptr m_exceptionPtr = nullptr;
    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif