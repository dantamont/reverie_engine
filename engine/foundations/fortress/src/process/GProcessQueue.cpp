#include "fortress/process/GProcessQueue.h"


namespace rev {

bool CompareBySortingLayer::operator()(const std::shared_ptr<Process>& a, const std::shared_ptr<Process>& b) const
{
    const SortingLayers& layers  = a->queue()->sortingLayers();
    return layers.getLayerFromId(a->sortingLayerId()) < layers.getLayerFromId(b->sortingLayerId());
}

CompareBySortingLayer CompareBySortingLayer::s_compareBySortingLayer = CompareBySortingLayer();




// ProcessQueue

ProcessQueue::ProcessQueue()
{
    // Initialize sorting layer list 
    m_sortingLayers.addLayer(SortingLayer::s_defaultSortingLayer, 0);
}

ProcessQueue::~ProcessQueue()
{
}

void ProcessQueue::refreshProcessOrder()
{
    // Reorder process vectors
    std::sort(m_processes.begin(), m_processes.end(), CompareBySortingLayer::s_compareBySortingLayer);
    std::sort(m_processQueue.begin(), m_processQueue.end(), CompareBySortingLayer::s_compareBySortingLayer);
}

void ProcessQueue::onRemoveSortingLayer(size_t layerId)
{
    // Find all processes with given sorting layer and set to default
    const SortingLayer& defaultLayer = m_sortingLayers.getLayer(SortingLayer::s_defaultSortingLayer);
    for (const std::shared_ptr<Process>& process : m_processes) {
        if (process->sortingLayerId() == layerId) {
            process->setSortingLayer(defaultLayer);
        }
    }
    for (const std::shared_ptr<Process>& process : m_processQueue) {
        if (process->sortingLayerId() == layerId) {
            process->setSortingLayer(defaultLayer);
        }
    }

    refreshProcessOrder();
}

WeakProcessInterfacePtr ProcessQueue::attachProcess(StrongProcessInterfacePtr process, bool initialize)
{
    std::shared_ptr<Process> castedProcess = std::static_pointer_cast<Process>(process);
    m_processQueue.emplace_back(castedProcess);
    if (initialize) {
        process->onInit();
    }
    return WeakProcessInterfacePtr(process);
}

WeakProcessInterfacePtr ProcessQueue::reattachProcess(StrongProcessInterfacePtr process)
{
    /// @todo Unused and untested
    // Find process in process set
    ProcessVec::const_iterator iter =
        std::find_if(m_processes.begin(), m_processes.end(),
            [&](const std::shared_ptr<Process>& p) {
        return p->getUuid() == process->getUuid();
    });

#ifdef DEBUG_MODE
    throw std::runtime_error("Error, cannot reattach process that was never attached");
#endif

    std::shared_ptr<Process> castedProcess = std::static_pointer_cast<Process>(process);
    m_processes.erase(iter);
    m_processQueue.emplace_back(castedProcess);

    return WeakProcessInterfacePtr(process);
}

void ProcessQueue::abortAllProcesses(bool immediate)
{
    if (m_processes.size()) {
        std::vector<std::shared_ptr<Process>> processes = m_processes;
        ProcessVec::iterator it = processes.begin();
        while (it != processes.end())
        {
            abortProcess(*it, immediate);
            ++it;
        }
    }
}

void ProcessQueue::clearProcesses()
{
    // Abort all processes
    abortAllProcesses(true);

    // Clear process queues
    m_processes.clear();
}

void ProcessQueue::abortProcess(StrongProcessInterfacePtr process, bool immediate)
{
    if (process->isAlive())
    {
        process->setState(ProcessState::kAborted);
        if (immediate)
        {
            process->onAbort();

            // Remove from process set if not a threaded process
            ProcessVec::const_iterator iter = std::find_if(m_processes.begin(),
                m_processes.end(),
                [&](std::shared_ptr<Process> p) {
                return p->getUuid() == process->getUuid();
            });
            if (iter != m_processes.end()) {
                m_processes.erase(iter);
                return;
            }
        }
    }
}

void to_json(nlohmann::json& orJson, const ProcessQueue& korObject)
{
    // Add sorting layers to JSON
    orJson["sortingLayers"] = korObject.m_sortingLayers;
}

void from_json(const nlohmann::json& korJson, ProcessQueue& orObject)
{
    if (korJson.contains("sortingLayers")) {
        korJson["sortingLayers"].get_to(orObject.m_sortingLayers);
    }
}

} // End namespaces