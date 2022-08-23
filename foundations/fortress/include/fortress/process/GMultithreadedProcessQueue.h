#pragma once

// Internal
#include "fortress/process/GProcessInterface.h"
#include "fortress/process/GProcessQueueInterface.h"
#include "fortress/thread/GThreadPool.h"

namespace rev {

class ThreadedProcess;

/// @brief MultithreadedProcessQueue class
class MultithreadedProcessQueue: public ProcessQueueInterface {
public:

    static const Uint32_t s_defaultThreadCount;

	/// @name Constructors/Destructor
	/// @{
    MultithreadedProcessQueue(Uint32_t initialThreadCount = s_defaultThreadCount);
	~MultithreadedProcessQueue();
	/// @}

    /// @name Properties
    /// @{

    /// @property ThreadPool
    ThreadPool& threadPool() { return m_threadPool; }

    /// @property ProcessCount
    size_t getProcessCount() const { return m_threadedProcesses.size(); }


    /// @}

	/// @name Public Methods
	/// @{

    /// @brief Check for failures in threaded processes
    void checkProcesses(double deltaSec);

    /// @brief Called on destruction, clears all processes
    void clearProcesses();

    /// @brief Attach a process to the process queue
    WeakProcessInterfacePtr attachProcess(StrongProcessInterfacePtr process, bool initialize=false) override;

    /// @brief Delete the threaded process with the given UUID
    /// @todo Call this once a threaded process is done playing
    void deleteThreadedProcess(const Uuid& uuid);

	/// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const MultithreadedProcessQueue& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, MultithreadedProcessQueue& orObject);


    /// @}

protected:
    /// @name Protected Methods
    /// @{

    /// @brief Abort a given process
    void abortProcess(std::shared_ptr<ThreadedProcess> process, bool immediate);

    /// @brief Aborts all processes
    void abortAllProcesses(bool immediate);

    /// @}

    /// @name Protected Members
    /// @{

    /// @brief Mutex for getting thread process queue
    std::mutex m_threadedProcessMutex;

    /// @brief All threaded processes
    std::vector<std::shared_ptr<ThreadedProcess>> m_threadedProcesses;

    /// @brief Threadpool for managing threads for any threaded processes
    ThreadPool m_threadPool;

    /// @}
};



} // End namespaces
