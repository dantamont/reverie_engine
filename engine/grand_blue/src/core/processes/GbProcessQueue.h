/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_PROCESS_QUEUE_H
#define GB_PROCESS_QUEUE_H

#include <set>

// QT
#include <QThreadPool>
#include <QMutex>
#include <QMutexLocker>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>

// Internal
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

    static CompareBySortingLayer s_compareBySortingLayer;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Enum denoting dedicated thread types
enum class DedicatedThreadType {
    kAnimation = 0, // Animation thread
    kAudio, // TODO: Move audio thread from sound manager into process manager
    kNUM_DEDICATED_THREADS
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief ProcessQueue class
class ProcessQueue {
public:
    typedef std::vector<StrongProcessPtr> ProcessVec;

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    ProcessQueue();
	virtual ~ProcessQueue();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @property Sorting Layers
    const std::vector<SortingLayer*>& sortingLayers() const { return m_sortingLayers; }

    /// @property deltaMs
    unsigned long getDeltaMs() const { 
        return m_deltaMs;
    }
    void setDeltaMs(unsigned long delta) {
        m_deltaMs = delta;
    }

    /// @property ProcessCount
    unsigned int getProcessCount() const { return m_processes.size(); }

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Get the sorting layer with the given name
    SortingLayer* getSortingLayer(const GString& name, int* outIndex = nullptr) const;

    /// @brief Called on destruction, clears all processes
    virtual void clearAllProcesses();

    /// @brief Refresh order of processes to reflect sorting layer changes
    void refreshProcessOrder();

    /// @brief Add a new sorting layer
    void addSortingLayer();

    /// @brief Whether the sorting layer with the given label exists or not
    bool hasSortingLayer(const GString& label) const;

    /// @brief Remove a sorting layer
    void removeSortingLayer(const GString& label);

    /// @brief Updates all attached processes
    void updateProcesses(unsigned long deltaMs);

    /// @brief Fixed-updates all attached processes
    void fixedUpdateProcesses(unsigned long deltaMs);

    /// @brief Attach a process to the process manager
    virtual WeakProcessPtr attachProcess(StrongProcessPtr process, bool initialize=false); 

    /// @brief Reattach a process (e.g., when the run order has changed)
    virtual WeakProcessPtr reattachProcess(StrongProcessPtr process);
    
    /// @brief Aborts all processes
    virtual void abortAllProcesses(bool immediate);


	/// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "ProcessQueue"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::ProcessQueue"; }
    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Abort a given process
    virtual void abortProcess(StrongProcessPtr process, bool immediate);

    /// @brief Send log event across threads
    void logMessage(CoreEngine* core, const GString& message, LogLevel logLevel = LogLevel::Info);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Map of sorting layers
    std::vector<SortingLayer*> m_sortingLayers;

    /// @brief Time since last update loop
    unsigned long m_deltaMs;

    /// @brief Queue for temporarily staging processes to run
    ProcessVec m_processQueue;

    /// @brief All unthreaded processes, iterated over in simulation loop
    ProcessVec m_processes;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif