#pragma once

// std
#include <set>
#include <atomic>

// Internal
#include "fortress/process/GProcess.h"
#include "fortress/process/GProcessQueueInterface.h"
#include "fortress/containers/GSortingLayer.h"
#include "fortress/layer/framework/GFlags.h"

namespace rev {

/// @struct CompareBySortingLayer
/// @brief Struct containing a comparator for sorting scene objects list by sorting layer
struct CompareBySortingLayer {
    friend class Process;

    bool operator()(const std::shared_ptr<Process>& a, const std::shared_ptr<Process>& b) const;

    static CompareBySortingLayer s_compareBySortingLayer;
};

/// @brief ProcessQueue class
class ProcessQueue: public ProcessQueueInterface {
private:
    typedef std::vector<std::shared_ptr<Process>> ProcessVec;
public:
	/// @name Constructors/Destructor
	/// @{
    ProcessQueue();
	virtual ~ProcessQueue();
	/// @}

    /// @name Properties
    /// @{

    /// @property Sorting Layers
    const SortingLayers& sortingLayers() const { return m_sortingLayers; }
    SortingLayers& sortingLayers() { return m_sortingLayers; }

    /// @property ProcessCount
    size_t getProcessCount() const { return m_processes.size(); }

    /// @}

	/// @name Public Methods
	/// @{

    /// @brief Called on destruction, clears all processes
    virtual void clearProcesses();

    /// @brief Refresh order of processes to reflect sorting layer changes
    void refreshProcessOrder();

    /// @brief Remove a sorting layer
    void onRemoveSortingLayer(size_t layerId);

    /// @brief Updates all attached processes
    template<ProcessUpdateState UpdateState>
    void step(double deltaSec) {
        if constexpr (ProcessUpdateState::kUpdate == UpdateState ||
            ProcessUpdateState::kLateUpdate == UpdateState) {
            for (const std::shared_ptr<Process>& process : m_processes)
            {
                process->step<UpdateState>(deltaSec);
            }
        }
        else if constexpr (ProcessUpdateState::kPostUpdate == UpdateState ||
            ProcessUpdateState::kFixed == UpdateState ) {

            // @note Since fixed updates and post updates happen at the end of their respective
            // loops, they must update the list of processes for the next iteration
            for (const std::shared_ptr<Process>& process : m_processes)
            {
                // Update process
                bool isDead = process->step<UpdateState>(deltaSec);

                // Only queue process to run again if it hasn't died
                if (!isDead) {
                    m_processQueue.emplace_back(process);
                }
            }

            // Swap process queue with iterable list
            m_processes.swap(m_processQueue);
            std::sort(m_processes.begin(), m_processes.end(), CompareBySortingLayer::s_compareBySortingLayer);
            m_processQueue.clear();
        }
        else {
            assert(false && "Unrecognized update state");
        }
    }

    /// @brief Attach a process to the process manager
    virtual WeakProcessInterfacePtr attachProcess(StrongProcessInterfacePtr process, bool initialize=false) override;

    /// @brief Reattach a process (e.g., when the run order has changed)
    virtual WeakProcessInterfacePtr reattachProcess(StrongProcessInterfacePtr process);

	/// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const ProcessQueue& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, ProcessQueue& orObject);


    /// @}

protected:
    /// @name Protected Methods
    /// @{

    /// @brief Aborts all processes
    virtual void abortAllProcesses(bool immediate);

    /// @brief Abort a given process
    virtual void abortProcess(StrongProcessInterfacePtr process, bool immediate);

    /// @}

    /// @name Protected Members
    /// @{

    /// @brief Map of sorting layers
    SortingLayers m_sortingLayers;

    /// @brief Queue for temporarily staging processes to run
    ProcessVec m_processQueue;

    /// @brief All unthreaded processes, iterated over in simulation loop
    ProcessVec m_processes;

    /// @}
};


} // End namespaces
