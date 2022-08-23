#pragma once

#include <memory>
#include <atomic>

// Internal
#include "fortress/GGlobal.h"
#include "fortress/types/GIdentifiable.h"

namespace rev {

class ProcessQueueInterface;
class ProcessInterface;

typedef std::shared_ptr<ProcessInterface> StrongProcessInterfacePtr;
typedef std::weak_ptr<ProcessInterface> WeakProcessInterfacePtr;

/// @brief The possible states of a process
enum class ProcessState {
    kUninitialized = 0, // Created, but not running
    kRemoved, // Removed from the process list, but not destroyed, e.g. if running process is parented to another process
    kRunning, // Living processes
    kPaused, // Initialized, but paused
    // Dead processes
    kSucceeded, // Completed successfully
    kFailed, // Failed to complete
    kAborted // Aborted, may not have started
};

/// @brief The possible update states for a process
enum class ProcessUpdateState {
    kFixed,       // Updates at a fixed rate, separate from other update calls
    kUpdate,      // The main update loop
    kLateUpdate,  // Performed after all update calls
    kPostUpdate   // Performed after all late update calls (intended for rendering)
};


/// @class Process
/// @brief Represents a thread-friendly process
class ProcessInterface: public IdentifiableInterface {
public:
    /// @name Static
    /// @{
    /// @}

	/// @name Constructors/Destructor
	/// @{
    ProcessInterface();
    ProcessInterface(ProcessQueueInterface* queue);
	virtual ~ProcessInterface();
	/// @}

    /// @name Properties
    /// @{

    /// @brief The state of the process
    ProcessState getState(void) const { return m_state.load(); }
    void setState(const ProcessState& state) { m_state.store(state); }

    /// @brief Wrappers on process state to get current status
    bool isAlive(void) const { 
        ProcessState state = getState();
        return (state == ProcessState::kRunning || state == ProcessState::kPaused);
    }
    bool isDead(void) const { 
        ProcessState state = getState();
        return (state == ProcessState::kSucceeded || state == ProcessState::kFailed || state == ProcessState::kAborted);
    }
    bool isRemoved(void) const { return (m_state.load() == ProcessState::kRemoved); }
    bool isPaused(void) const { return m_state.load() == ProcessState::kPaused; }
    bool isAborted(void) const { return m_state.load() == ProcessState::kAborted; }

    template<typename T>
    T* as() {
        static_assert(std::is_base_of_v<Process, T>, "Error, can only convert to a process type");
        return dynamic_cast<T*>(this);
    }

    /// @}

	/// @name Public Methods
	/// @{

    /// @brief Functions to be overriden by process subclasses as needed
    virtual void onInit() { m_state.store(ProcessState::kRunning); }
    virtual void onUpdate(double deltaSec) = 0;
    virtual void onLateUpdate(double deltaSec) = 0;
    virtual void onPostUpdate(double deltaSec) = 0;
    virtual void onFixedUpdate(double) {}
    virtual void onSuccess() { }
    virtual void onFail() { }
    virtual void onAbort(){}

    /// @brief Run the process update loop
    /// @details Returns true if the process has died, else false
    template<ProcessUpdateState UpdateState>
    bool step(double deltaSec) {
        bool dead = false;

        // Ensure that process is initialized
        if constexpr (UpdateState == ProcessUpdateState::kUpdate ||
            UpdateState == ProcessUpdateState::kFixed) {
            if (getState() == ProcessState::kUninitialized) {
                onInit();
            }
        }

        // Perform update if the process is running
        bool isRunning = (getState() == ProcessState::kRunning);
        if (isRunning) {
            if constexpr (UpdateState == ProcessUpdateState::kUpdate) {
                onUpdate(deltaSec);
            }
            else if constexpr (UpdateState == ProcessUpdateState::kLateUpdate) {
                onLateUpdate(deltaSec);
            }
            else if constexpr (UpdateState == ProcessUpdateState::kPostUpdate) {
                onPostUpdate(deltaSec);
            }
            else if constexpr (UpdateState == ProcessUpdateState::kFixed) {
                onFixedUpdate(deltaSec);
            }
            else {
                assert(false && "Unrecognized update type");
            }
        }

        // Check if the process is dead
        // This is only necessary once per loop, so after a fixed update or postUpdate call
        if constexpr (UpdateState == ProcessUpdateState::kFixed ||
            UpdateState == ProcessUpdateState::kPostUpdate) {
            dead = checkFinished();
        }

        return dead;
    }

    /// @brief Functions for ending the process.
    inline void succeed(void);
    inline void fail(void);
    inline void abort(void);

    /// @brief pause
    inline void pause(void);
    inline void unPause(void);

    // child functions
    inline void attachChild(StrongProcessInterfacePtr pChild);
    StrongProcessInterfacePtr removeChild(void);  // releases ownership of the child
    StrongProcessInterfacePtr getChild(void) const { return m_child; }  // doesn't release ownership of the child
	
    /// @}

protected:
    /// @name Protected Methods
    /// @{

    /// @brief Check if the process is finished
    bool checkFinished();

    /// @}

    /// @name Protected Members
    /// @{

    /// @brief The current state of the process
    std::atomic<ProcessState> m_state;
    
    /// @brief The child process, if any
    StrongProcessInterfacePtr m_child;

    /// @brief Pointer to the queue for this process
    ProcessQueueInterface* m_processQueue;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
// Inline Functions
/////////////////////////////////////////////////////////////////////////////////////////////
inline void ProcessInterface::succeed(void)
{
    ProcessState state = getState();
    assert(state == ProcessState::kRunning || state == ProcessState::kPaused);
    m_state.store(ProcessState::kSucceeded);
}

inline void ProcessInterface::fail(void)
{
    ProcessState state = getState();
    assert(state == ProcessState::kRunning || state == ProcessState::kPaused);
    m_state.store(ProcessState::kFailed);
}

inline void ProcessInterface::abort(void)
{
    ProcessState state = getState();
    assert(state == ProcessState::kRunning || state == ProcessState::kPaused || state == ProcessState::kUninitialized);
    m_state.store(ProcessState::kAborted);
}

inline void ProcessInterface::attachChild(StrongProcessInterfacePtr child)
{
    if (m_child) {
        m_child->attachChild(child);
    }
    else {
        m_child = child;
    }
}

inline void ProcessInterface::pause(void)
{
    assert(getState() == ProcessState::kRunning && "Process must be running to pause");
    m_state.store(ProcessState::kPaused);
}

inline void ProcessInterface::unPause(void)
{
    assert(getState() == ProcessState::kPaused && "Attempting to unpause a process that isn't paused");
    m_state.store(ProcessState::kRunning);
}


} // End rev namespaces