#include "GProcessQueue.h"

#include "../GCoreEngine.h"
#include "../events/GEventManager.h"
#include <core/processes/GThreadedProcess.h>

#include <QApplication>


namespace rev {
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
    m_sortingLayers.addLayer(DEFAULT_SORTING_LAYER, 0);
}
/////////////////////////////////////////////////////////////////////////////////////////////
ProcessQueue::~ProcessQueue()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ProcessQueue::refreshProcessOrder()
{
    // Reorder process vectors
    std::sort(m_processes.begin(), m_processes.end(), CompareBySortingLayer::s_compareBySortingLayer);
    std::sort(m_processQueue.begin(), m_processQueue.end(), CompareBySortingLayer::s_compareBySortingLayer);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ProcessQueue::onRemoveSortingLayer(size_t layerId)
{
    // Find all processes with sorting layer and set to default
    SortingLayer* defaultLayer = m_sortingLayers.getLayer(DEFAULT_SORTING_LAYER);
    for (const auto& process : m_processes) {
        if (process->getSortingLayer().id() == layerId) {
            process->setSortingLayer(defaultLayer);
        }
    }
    for (const auto& process : m_processQueue) {
        if (process->getSortingLayer().id() == layerId) {
            process->setSortingLayer(defaultLayer);
        }
    }

    refreshProcessOrder();
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
    if (process->as<ThreadedProcess>()) {
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