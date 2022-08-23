#pragma once

// std
#include <exception>
#include <stdexcept>

// Internal
#include "fortress/time/GStopwatchTimer.h"
#include "GProcessInterface.h"

namespace rev {

class MultithreadedProcessQueue;

/// @class Process
/// @brief Represents a thread-friendly process
class ThreadedProcess: public ProcessInterface {
public:
    /// @name Constructors/Destructor
	/// @{
    ThreadedProcess(MultithreadedProcessQueue* queue);
	virtual ~ThreadedProcess();
	/// @}

    /// @name Properties
    /// @{

    std::mutex& exceptionMutex() { return m_exceptionLock; }
    const std::exception_ptr& exception() const { return m_exceptionPtr; }

    /// @}

	/// @name Public Methods
	/// @{

    /// @brief Functions to be overriden by process subclasses as needed
    virtual void onInit() override{ m_state.store(ProcessState::kRunning); }
    virtual void onUpdate(double deltaSec) override;
    virtual void onSuccess() override;
    virtual void onFail() override { }
    virtual void onAbort() override{}

    /// @brief Required override for QRunnable for threaded process
    virtual void run();
	
    /// @}

protected:

    /// @name Protected Members
    /// @{

    /// @brief The timer for the threaded process's update loop
    StopwatchTimer m_timer;

    /// @brief For exception handling
    mutable std::mutex m_exceptionLock;
    std::exception_ptr m_exceptionPtr = nullptr;
    /// @}
};


} // End namespaces
