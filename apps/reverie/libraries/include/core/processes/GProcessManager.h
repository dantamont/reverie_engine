#pragma once

// Internal
#include "core/GManager.h"
#include "fortress/process/GMultithreadedProcessQueue.h"
#include "fortress/process/GProcessQueue.h"
#include "fortress/process/GProcessQueueThread.h"

namespace rev {

class CoreEngine;

/// @brief Enum denoting dedicated thread types
enum class DedicatedThreadType {
    kAnimation = 0, // Animation thread
    kAudio, // TODO: Move audio thread from sound manager into process manager
    kNUM_DEDICATED_THREADS
};

/// @brief ProcessManager class
class ProcessManager: public Manager, public ProcessInterface {
    Q_OBJECT

public:

	/// @name Constructors/Destructor
	/// @{
    ProcessManager(CoreEngine* engine, Uint32_t initialThreadCount);
	~ProcessManager();
	/// @}

    /// @name Properties
    /// @{

    ProcessQueueThread& animationThread() { return m_dedicatedThreads[(int)DedicatedThreadType::kAnimation]; }

    ProcessQueue& processQueue() { return m_processQueue; }
    MultithreadedProcessQueue& threadedProcessQueue() { return m_threadedProcessQueue; }

    /// @}

	/// @name Public Methods
	/// @{

    virtual void onUpdate(double deltaSec) override final;
    virtual void onLateUpdate(double deltaSec) override final;
    virtual void onPostUpdate(double deltaSec) override final;
    virtual void onFixedUpdate(double deltaSec) override final;

    /// @brief Abort all processes
    void clearProcesses();

    /// @brief Called after construction
    virtual void postConstruction() override final;

	/// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const ProcessManager& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, ProcessManager& orObject);


    /// @}

public slots:

    /// @brief Delete the threaded process with the given UUID
    void deleteThreadedProcess(const Uuid& uuid);

protected:
    /// @name Protected Methods
    /// @{

    /// @brief Initialize threads
    void initializeThreads(unsigned int threadCount);

    /// @brief Update threaded processes
    void updateThreadedProcesses(double deltaSec);

    /// @}

    /// @name Protected Members
    /// @{

    ProcessQueue m_processQueue; ///< Queue of serialized processes on the main thread
    MultithreadedProcessQueue m_threadedProcessQueue; ///< Queue of processes on multiple threads

    /// @brief Dedicated process threads
    std::array<ProcessQueueThread, (size_t)DedicatedThreadType::kNUM_DEDICATED_THREADS> m_dedicatedThreads;

    /// @}
};



} // End namespaces
