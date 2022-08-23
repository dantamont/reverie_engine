#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <future>

// std
#include <exception>
#include <stdexcept>

// Internal
#include "fortress/types/GNameable.h"
#include "fortress/process/GProcessQueue.h"


namespace rev {

/// @class ProcessQueueThread
/// @brief Represents a process queue on a dedicated thread
class ProcessQueueThread: public ProcessQueue, public NameableInterface {
public:
    /// @name Static
    /// @{
    /// @}

	/// @name Constructors/Destructor
	/// @{

    ProcessQueueThread();
    ~ProcessQueueThread();

	/// @}

    /// @name Properties
    /// @{

    bool isRunning() const { return m_isRunning.load(); }

    std::mutex& exceptionMutex() const { return m_exceptionLock; }
    const std::exception_ptr& exception() const { return m_exceptionPtr; }

    /// @}

	/// @name Public Methods
	/// @{

    /// @brief The update loop for the thread
    void update();

    /// @brief Stop thread
    void join();

    /// @brief Attach a process to the process manager
    virtual WeakProcessInterfacePtr attachProcess(StrongProcessInterfacePtr process, bool initialize = false) override final;

    /// @}

protected:
    friend class ProcessManager;

    /// @name Protected Methods
    /// @{

    void initialize();

    /// @}

    /// @name Protected Members
    /// @{

    std::shared_mutex m_processMutex; ///< Need to control access to processes to avoid a race condition

    /// @details How much time (sec) must pass before a fixed update event is called
    double m_fixedUpdateInterval; ///< Fixed update interval

    double m_minUpdateIntervalSec; ///< Minimum time between updates for the thread, in seconds

    double m_fixedDeltaTime; ///< Time-keeping for fixed updates

    std::atomic<bool> m_isRunning; ///< Whether or not the threaded process is still running

    std::thread m_thread; ///< The thread where all the magic happens

    std::future<void> m_future; ///< Future for the thread task. Currently unused, but would be a way to return a value on thread completion.

    mutable std::mutex m_exceptionLock;
    std::exception_ptr m_exceptionPtr = nullptr;

    /// @}
};


} /// End rev namespace
