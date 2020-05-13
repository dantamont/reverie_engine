/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_PROCESS_H
#define GB_PROCESS_H

#include <memory>

// QT

// Internal
#include "../GbObject.h"
#include "../events/GbLogEvent.h"
#include "../containers/GbSortingLayer.h"

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class Process;
class CoreEngine;
class ProcessManager;

/////////////////////////////////////////////////////////////////////////////////////////////
// Typedefs
/////////////////////////////////////////////////////////////////////////////////////////////
typedef std::shared_ptr<Process> StrongProcessPtr;
typedef std::weak_ptr<Process> WeakProcessPtr;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class Process
/// @brief Represents a thread-friendly process
class Process: public Gb::Object {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum State {
        kUninitialized = 0, // Created, but not running
        kRemoved, // Removed from the process list, but not destroyed, e.g. if running process is parented to another process
        kRunning, // Living processes
        kPaused, // Initialized, but paused
        // Dead processes
        kSucceeded, // Completed successfully
        kFailed, // Failed to complete
        kAborted // Aborted, may not have started
    };

    /// @brief Return ID for a thread
    static QString getThreadID();

    /// @brief Determine whether this current thread is the main thread or not
    static bool isMainThread();

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    Process(CoreEngine* engine, ProcessManager* manager);
    Process(CoreEngine* engine, SortingLayer* sortLayer, ProcessManager* manager);
	virtual ~Process();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief The order of the process
    const SortingLayer& getSortingLayer() const;
    void setSortingLayer(SortingLayer* layer) { m_sortingLayer = layer; }

    /// @brief The state of the process
    State getState(void) const { return m_state; }
    void setState(const State& state) { m_state = state; }

    /// @brief Wrappers on process state to get current status
    bool isAlive(void) const { return (m_state == kRunning || m_state == kPaused); }
    bool isDead(void) const { return (m_state == kSucceeded || m_state == kFailed || m_state == kAborted); }
    bool isRemoved(void) const { return (m_state == kRemoved); }
    bool isPaused(void) const { return m_state == kPaused; }
    bool isAborted(void) const { return m_state == kAborted; }

    virtual bool isThreaded() const { return false; }

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Functions to be overriden by process subclasses as needed
    virtual void onInit() { m_state = kRunning; }
    virtual void onUpdate(unsigned long deltaMs) = 0;
    virtual void onFixedUpdate(unsigned long deltaMs) { Q_UNUSED(deltaMs); }
    virtual void onSuccess() { }
    virtual void onFail() { }
    virtual void onAbort(){}

    /// @brief Run the process update loop
    /// @details Returns true if the process has died, else false
    virtual bool runProcess(unsigned long deltaMs);

    /// @brief Run the process fixed update loop
    /// @details Returns true if the process has died, else false
    virtual bool runFixed(unsigned long deltaMs);

    /// @brief Functions for ending the process.
    inline void succeed(void);
    inline void fail(void);
    inline void abort(void);

    /// @brief pause
    inline void pause(void);
    inline void unPause(void);

    // child functions
    inline void attachChild(StrongProcessPtr pChild);
    StrongProcessPtr removeChild(void);  // releases ownership of the child
    StrongProcessPtr peekChild(void) { return m_child; }  // doesn't release ownership of the child
	
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "Process"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::Process"; }
    /// @}


protected:
    friend class ProcessManager;

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Check if the process is finished
    bool checkFinished();

    /// @brief Send log event across threads
    void logMessage(const QString& message, LogLevel logLevel = LogLevel::Info);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief The current state of the process
    State m_state;

    /// @brief Sorting layer of the process
    SortingLayer* m_sortingLayer;
    
    /// @brief The child process, if any
    StrongProcessPtr m_child;

    /// @brief Pointer to the core engine
    CoreEngine* m_engine;

    /// @brief Pointer to the manager for this process
    ProcessManager* m_processManager;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
// Inline Functions
/////////////////////////////////////////////////////////////////////////////////////////////
inline void Process::succeed(void)
{
    assert(m_state == kRunning || m_state ==kPaused);
    m_state = kSucceeded;
}

inline void Process::fail(void)
{
    assert(m_state == kRunning || m_state == kPaused);
    m_state = kFailed;
}

inline void Process::abort(void)
{
    assert(m_state == kRunning || m_state == kPaused || m_state == kUninitialized);
    m_state = kAborted;
}

inline void Process::attachChild(StrongProcessPtr child)
{
    if (m_child)
        m_child->attachChild(child);
    else
        m_child = child;
}

inline void Process::pause(void)
{
    if (m_state == kRunning)
        m_state = kPaused;
    else {
#ifdef DEBUG_MODE
        logWarning("Attempting to pause a process that isn't running");
#endif
    }
}

inline void Process::unPause(void)
{
    if (m_state == kPaused)
        m_state = kRunning;
    else {
#ifdef DEBUG_MODE
        logWarning("Attempting to unpause a process that isn't paused");
#endif
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif