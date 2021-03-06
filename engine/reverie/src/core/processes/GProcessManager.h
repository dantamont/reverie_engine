/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_PROCESS_MANAGER_H
#define GB_PROCESS_MANAGER_H

#include <set>

// QT
#include <QThreadPool>
#include <QMutex>
#include <QMutexLocker>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>

// Internal
#include "../../core/GManager.h"
#include "GProcess.h"
#include "GProcessQueue.h"
#include "GProcessThread.h"
#include "../containers/GSortingLayer.h"

namespace rev {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief ProcessManager class
class ProcessManager: public Manager, public ProcessQueue, public Serializable {
    Q_OBJECT
public:
    typedef std::vector<StrongProcessPtr> ProcessVec;

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    ProcessManager(CoreEngine* engine, size_t initialThreadCount=2);
	~ProcessManager();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @property ThreadCount
    int threadCount() const { return m_threadPool->activeThreadCount(); }

    /// @property MaxThreadCount
    int maxThreadCount() const { return m_threadPool->maxThreadCount(); }

    /// @property ThreadPool
    QThreadPool* getThreadPool() { return m_threadPool; }

    /// @property number of load processes
    unsigned int loadProcessCount();

    ProcessThread& animationThread() { return m_dedicatedThreads[(int)DedicatedThreadType::kAnimation]; }

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Updates all attached processes
    virtual void updateProcesses(unsigned long deltaMs) override;

    /// @brief Called on destruction, clears all processes
    virtual void clearAllProcesses() override;

    /// @brief Attach a process to the process manager
    virtual WeakProcessPtr attachProcess(StrongProcessPtr process, bool initialize=false); 

    /// @brief Reattach a process (e.g., when the run order has changed)
    virtual WeakProcessPtr reattachProcess(StrongProcessPtr process);
    
    /// @brief Aborts all processes
    virtual void abortAllProcesses(bool immediate) override;

    /// @brief Called after construction
    virtual void postConstruction() override final;

	/// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "ProcessManager"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "rev::ProcessManager"; }
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

public slots:

    /// @brief Delete the threaded process with the given UUID
    void deleteThreadedProcess(const Uuid& uuid);

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Initialize threads
    void initializeThreads(unsigned int threadCount);

    /// @brief Send log event across threads
    void logMessage(const GString& message, LogLevel logLevel = LogLevel::Info);

    /// @brief Abort a given process
    virtual void abortProcess(StrongProcessPtr process, bool immediate);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Mutex for getting thread process queue
    QMutex m_threadedProcessMutex;

    /// @brief All threaded processes
    ProcessVec m_threadedProcesses;

    /// @brief Dedicated process threads
    std::array<ProcessThread, (size_t)DedicatedThreadType::kNUM_DEDICATED_THREADS> m_dedicatedThreads;

    /// @brief Threadpool for managing threads for any threaded processes
    QThreadPool* m_threadPool;

    /// @brief Static counter for threads opened by process managers
    static unsigned int THREAD_COUNT;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif