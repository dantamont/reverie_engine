#include "GbProcessQueue.h"

#include "../GbCoreEngine.h"
#include "../events/GbEventManager.h"

#include <QApplication>


namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
bool CompareBySortingLayer::operator()(const StrongProcessPtr& a,
    const StrongProcessPtr& b) const
{
    return a->getSortingLayer() < b->getSortingLayer();
}

CompareBySortingLayer CompareBySortingLayer::s_compareBySortingLayer = CompareBySortingLayer();


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// ProcessQueue
/////////////////////////////////////////////////////////////////////////////////////////////
ProcessQueue::ProcessQueue():
    m_deltaMs(0)
{
    // Initialize sorting layer list 
    SortingLayer* defaultLayer = new SortingLayer();
    m_sortingLayers.emplace_back(defaultLayer);
}
/////////////////////////////////////////////////////////////////////////////////////////////
ProcessQueue::~ProcessQueue()
{
    // Delete all sorting layers
    for (SortingLayer* layer : m_sortingLayers) {
        delete layer;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ProcessQueue::refreshProcessOrder()
{
    // Reorder process vectors
    std::sort(m_processes.begin(), m_processes.end(), CompareBySortingLayer::s_compareBySortingLayer);
    std::sort(m_processQueue.begin(), m_processQueue.end(), CompareBySortingLayer::s_compareBySortingLayer);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ProcessQueue::addSortingLayer()
{
    SortingLayer* newLayer = new SortingLayer();
    newLayer->setName(newLayer->getUuid().createUniqueName("layer_"));
    m_sortingLayers.emplace_back(newLayer);
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ProcessQueue::hasSortingLayer(const GString& label) const
{
    bool hasLabel;
    if (getSortingLayer(label)) {
        hasLabel = true;
    }
    else {
        hasLabel = false;
    }

    return hasLabel;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ProcessQueue::removeSortingLayer(const GString& label)
{
    // Find all processes with sorting layer and set to default
    for (const auto& process : m_processes) {
        if (process->getSortingLayer().getName() == label) {
            process->setSortingLayer(getSortingLayer(DEFAULT_SORTING_LAYER));
        }
    }
    for (const auto& process : m_processQueue) {
        if (process->getSortingLayer().getName() == label) {
            process->setSortingLayer(getSortingLayer(DEFAULT_SORTING_LAYER));
        }
    }

    refreshProcessOrder();

    // Delete the sorting layer
    int idx;
    SortingLayer* layer = getSortingLayer(label, &idx);
    std::vector<SortingLayer*>::iterator iter = m_sortingLayers.begin() + idx;
#ifdef DEBUG_MODE
    if (!layer) {
        throw("Error, attempted to delete non-existent layer");
    }
#endif
    m_sortingLayers.erase(iter);
    delete layer;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ProcessQueue::updateProcesses(unsigned long deltaMs)
{
    m_deltaMs = deltaMs;

    if (!m_processes.size()) {
        return;
    }

    ProcessVec::iterator it = m_processes.begin();
    while (it != m_processes.end())
    {
        // Grab the next process
        StrongProcessPtr currentProcess = (*it);

        // Run process
        bool isDead = currentProcess->runProcess(deltaMs);

        // Only queue process to run again if it hasn't died
        if (!isDead) {
            m_processQueue.emplace_back(currentProcess);
        }
        // Process is destroyed if it is dead

        // Increment iterator 
        ++it;
    }

    // Swap process queue with iterable list
    m_processes.swap(m_processQueue);
    std::sort(m_processes.begin(), m_processes.end(), CompareBySortingLayer::s_compareBySortingLayer);
    m_processQueue.clear();

    return;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ProcessQueue::fixedUpdateProcesses(unsigned long deltaMs)
{
    m_deltaMs = deltaMs;

    ProcessVec::iterator it = m_processes.begin();
    while (it != m_processes.end())
    {
        // Grab the next process
        StrongProcessPtr currentProcess = (*it);

        // Run process fixed update for process
        bool isDead = currentProcess->runFixed(deltaMs);

        // Only queue process to run again if it hasn't died
        if (!isDead) {
            m_processQueue.emplace_back(currentProcess);
        }
        // Process is destroyed if it is dead

        // Increment iterator 
        ++it;
    }

    // Swap process queue with iterable list
    m_processes.swap(m_processQueue);
    std::sort(m_processes.begin(), m_processes.end(), CompareBySortingLayer::s_compareBySortingLayer);
    m_processQueue.clear();

    return;
}
/////////////////////////////////////////////////////////////////////////////////////////////
WeakProcessPtr ProcessQueue::attachProcess(StrongProcessPtr process, bool initialize)
{
    m_processQueue.emplace_back(process);
    if (initialize) {
        process->onInit();
    }
    return WeakProcessPtr(process);
}
/////////////////////////////////////////////////////////////////////////////////////////////
WeakProcessPtr ProcessQueue::reattachProcess(StrongProcessPtr process)
{
    // UNTESTED
    // Return if threaded process
    if (process->isThreaded()) {
        throw("Error, cannot safely reattach threaded process");
        return process;
    }

    // Find process in process set
    ProcessVec::const_iterator iter =
        std::find_if(m_processes.begin(), m_processes.end(),
            [&](const StrongProcessPtr& process) {
        return process->getUuid() == process->getUuid();
    });
    if (iter == m_processes.end()) {
        throw("Error, cannot reattach process that was never attached");
    }
    else {
        m_processes.erase(iter);
        m_processQueue.emplace_back(process);
    }

    return process;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ProcessQueue::abortAllProcesses(bool immediate)
{
    if (m_processes.size()) {
        auto processes = m_processes;
        ProcessVec::iterator it = processes.begin();
        while (it != processes.end())
        {
            abortProcess(*it, immediate);
            ++it;
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
SortingLayer * ProcessQueue::getSortingLayer(const GString & name, int* outIndex) const
{
    auto iter = std::find_if(m_sortingLayers.begin(), m_sortingLayers.end(),
        [name](SortingLayer* layer) {
        return layer->getName() == name;
    });

    if (iter == m_sortingLayers.end()) {
        if (outIndex) {
            *outIndex = -1;
        }
        return nullptr;
    }
    else {
        if (outIndex) {
            *outIndex = int(iter - m_sortingLayers.begin());
        }
        return *iter;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ProcessQueue::clearAllProcesses()
{
    // Abort all processes
    abortAllProcesses(true);

    // Clear process queues
    m_processes.clear();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ProcessQueue::abortProcess(StrongProcessPtr process, bool immediate)
{
    if (process->isAlive())
    {
        process->setState(Process::kAborted);
        if (immediate)
        {
            process->onAbort();

            // Remove from process set if not a threaded process
            ProcessVec::const_iterator iter = std::find_if(m_processes.begin(),
                m_processes.end(),
                [&](StrongProcessPtr p) {
                return p->getUuid() == process->getUuid();
            });
            if (iter != m_processes.end()) {
                m_processes.erase(iter);
                return;
            }
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ProcessQueue::logMessage(CoreEngine* core, const GString & message, LogLevel logLevel)
{
    QApplication::postEvent(core, 
        new LogEvent(namespaceName(), message, Process::GetThreadID(), logLevel));
}

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces