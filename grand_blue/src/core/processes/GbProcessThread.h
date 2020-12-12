/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_PROCESS_THREAD_H
#define GB_PROCESS_THREAD_H

#include <memory>
#include <thread>
#include <mutex>
#include <shared_mutex>

// QT

// Internal
#include "../GbObject.h"
#include "GbProcessQueue.h"


namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Typedefs
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class ProcessThread
/// @brief Represents a thread-friendly process
class ProcessThread: public Gb::Object, public ProcessQueue {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{

    ProcessThread();
    ~ProcessThread();

	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    bool isRunning() const { return m_isRunning; }


    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Initialize thread with a function and its arguments
    //template<typename Delegate, typename... Params>
    //void initialize(Delegate func, Params... params) {
    //    m_thread = std::thread(&Delegate, params...);
    //}

    /// @brief The update loop for the thread
    void update();

    /// @brief Stop thread
    void join() {
        m_isRunning = false;
        m_thread.join();
    }

    /// @brief Attach a process to the process manager
    virtual WeakProcessPtr attachProcess(StrongProcessPtr process, bool initialize = false);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "ProcessThread"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::ProcessThread"; }
    /// @}


protected:
    friend class ProcessManager;

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    void initialize();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Need to control access to processes to avoid a race condition
    std::shared_mutex m_processMutex;

    /// @brief Fixed update interval
    /// @details How much time (ms) must pass before a fixed update event is called
    size_t m_fixedUpdateInterval;

    /// @brief Number of nanoseconds that have elapsed since last loop iteration, if loop took less than 1ms
    //double m_nsecCount;

    /// @brief Minimum time between updates for the thread, in milliseconds
    double m_minUpdateIntervalMSec;

    /// @brief Total elapsed time since start of simulation in msec (for querying)
    //quint64 m_elapsedTime;

    /// @brief Time-keeping for updates
    //QElapsedTimer m_updateTimer;

    /// @brief Time-keeping for fixed updates
    double m_fixedDeltaTime;

    bool m_isRunning;

    /// @brief Mutexes for each type of operation supported in the thread
    //std::vector<std::shared_mutex> m_mutexes;

    /// @brief The thread where all the magic happens
    std::thread m_thread;


    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif