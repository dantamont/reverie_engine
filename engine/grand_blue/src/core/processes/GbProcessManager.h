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
#include "../../core/GbManager.h"
#include "GbProcess.h"
#include "../containers/GbSortingLayer.h"

namespace Gb {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @struct CompareBySortingLayer
/// @brief Struct containing a comparator for sorting scene objects list by sorting layer
struct CompareBySortingLayer {
    bool operator()(const StrongProcessPtr& a, const StrongProcessPtr& b) const;
};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief ProcessManager class
class ProcessManager: public Manager, public Serializable {
    Q_OBJECT
public:
    typedef std::multiset<StrongProcessPtr, CompareBySortingLayer> ProcessSet;

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    ProcessManager(CoreEngine* engine, size_t initialThreadCount=2);
	~ProcessManager();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @property Sorting Layers
    const std::unordered_map<QString, SortingLayer*>& sortingLayers() const { return m_sortingLayers; }

    /// @property deltaMs
    unsigned long getDeltaMs() const { 
        return m_deltaMs;
    }
    void setDeltaMs(unsigned long delta) {
        m_deltaMs = delta;
    }

    /// @property ThreadCount
    int threadCount() const { return m_threadPool->activeThreadCount(); }

    /// @property MaxThreadCount
    int maxThreadCount() const { return m_threadPool->maxThreadCount(); }

    /// @property ThreadPool
    QThreadPool* getThreadPool() { return m_threadPool; }

    /// @property ProcessCount
    unsigned int getProcessCount() const { return m_processes.size(); }

    /// @property number of load processes
    unsigned int loadProcessCount();

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Called on destruction, clears all processes
    void clearAllProcesses();

    /// @brief Refresh order of processes to reflect sorting layer changes
    void refreshProcessOrder();

    /// @brief Add a new sorting layer
    void addSortingLayer();

    /// @brief Whether the sorting layer with the given label exists or not
    bool hasSortingLayer(const QString& label) const;

    /// @brief Remove a sorting layer
    void removeSortingLayer(const QString& label);

    /// @brief Updates all attached processes
    void updateProcesses(unsigned long deltaMs);

    /// @brief Fixed-updates all attached processes
    void fixedUpdateProcesses(unsigned long deltaMs);

    /// @brief Attach a process to the process manager
    WeakProcessPtr attachProcess(StrongProcessPtr process, bool initialize=false); 

    /// @brief Reattach a process (e.g., when the run order has changed)
    WeakProcessPtr reattachProcess(StrongProcessPtr process);
    
    /// @brief Aborts all processes
    void abortAllProcesses(bool immediate);

    /// @brief Called after construction
    virtual void postConstruction() override final;

	/// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "ProcessManager"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::ProcessManager"; }
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

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
    void logMessage(const QString& message, LogLevel logLevel = LogLevel::Info);

    /// @brief Abort a given process
    void abortProcess(StrongProcessPtr process, bool immediate);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Map of sorting layers
    std::unordered_map<QString, SortingLayer*> m_sortingLayers;

    /// @brief Whether or not to run the process manager
    bool m_runProcesses;

    /// @brief Time since last update loop
    unsigned long m_deltaMs;

    /// @brief Mutex for getting thread process queue
    QMutex m_threadedProcessMutex;

    /// @brief Queue for temporarily staging processes to run
    ProcessSet m_processQueue;

    /// @brief All unthreaded processes, iterated over in simulation loop
    ProcessSet m_processes;

    /// @brief All threaded processes
    std::vector<StrongProcessPtr> m_threadedProcesses;

    /// @brief Threadpool for managing threads for any threaded processes
    QThreadPool* m_threadPool;

    /// @brief Static counter for threads opened by process managers
    static unsigned int THREAD_COUNT;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif