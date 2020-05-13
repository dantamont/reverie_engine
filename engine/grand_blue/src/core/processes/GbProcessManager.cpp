#include "GbProcessManager.h"
#include "GbThreadedProcess.h"
#include "GbLoadProcess.h"

#include "../GbCoreEngine.h"
#include "../events/GbEventManager.h"
#include "../scene/GbScenario.h"

#include <QApplication>

#define USE_MULTITHREADING

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
bool CompareBySortingLayer::operator()(const StrongProcessPtr& a,
    const StrongProcessPtr& b) const
{
    return a->getSortingLayer() < b->getSortingLayer();
}
/////////////////////////////////////////////////////////////////////////////////////////////
ProcessManager::ProcessManager(CoreEngine* engine, size_t initialThreadCount):
    Manager(engine, "ProcessManager"),
    m_runProcesses(false),
    m_deltaMs(0),
    m_threadPool(new QThreadPool())
{
    // Initialize sorting layer list 
    SortingLayer* defaultLayer = new SortingLayer();
    m_sortingLayers.emplace(defaultLayer->m_label, defaultLayer);

    // Initialize threads
    initializeThreads(initialThreadCount);
}
/////////////////////////////////////////////////////////////////////////////////////////////
ProcessManager::~ProcessManager()
{
    // Delete all sorting layers
    for (const std::pair<QString, SortingLayer*> layerPair : m_sortingLayers) {
        delete layerPair.second;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ProcessManager::refreshProcessOrder()
{
    // Reorder process sets
    ProcessSet tempSet;
    for (const auto& process : m_processes) {
        tempSet.insert(process);
    }
    m_processes = tempSet;

    tempSet.clear();
    for (const auto& process : m_processQueue) {
        tempSet.insert(process);
    }
    m_processQueue = tempSet;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ProcessManager::addSortingLayer()
{
    SortingLayer* newLayer = new SortingLayer();
    newLayer->m_label = newLayer->getUuid().createUniqueName("layer_");
    m_sortingLayers[newLayer->m_label] = newLayer;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ProcessManager::hasSortingLayer(const QString& label) const
{
    bool hasLabel;
    if (m_engine->processManager()->sortingLayers().find(label) !=
        m_engine->processManager()->sortingLayers().end()) {
        hasLabel = true;
    }
    else {
        hasLabel = false;
    }

    return hasLabel;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ProcessManager::removeSortingLayer(const QString& label)
{
    // Find all processes with sorting layer and set to default
    for (const auto& process : m_processes) {
        if (process->getSortingLayer().m_label == label) {
            process->setSortingLayer(m_sortingLayers["default"]);
        }
    }
    for (const auto& process : m_processQueue) {
        if (process->getSortingLayer().m_label == label) {
            process->setSortingLayer(m_sortingLayers["default"]);
        }
    }

    refreshProcessOrder();

    // Delete the sorting layer
    SortingLayer* layer = m_sortingLayers[label];
    m_sortingLayers.erase(label);
    delete layer;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ProcessManager::updateProcesses(unsigned long deltaMs)
{
    m_deltaMs = deltaMs;

    ProcessSet::iterator it = m_processes.begin();
    while (it != m_processes.end())
    {
        // Grab the next process
        StrongProcessPtr currentProcess = (*it);

        // Run process
        bool isDead = currentProcess->runProcess(deltaMs);

        // Only queue process to run again if it hasn't died
        if (!isDead) {
            m_processQueue.emplace(currentProcess);
        }
        // Process is destroyed if it is dead

        // Increment iterator 
        ++it;
    }

    // Swap process queue with iterable list
    m_processes.swap(m_processQueue);
    m_processQueue.clear();

    return;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ProcessManager::fixedUpdateProcesses(unsigned long deltaMs)
{
    m_deltaMs = deltaMs;

    ProcessSet::iterator it = m_processes.begin();
    while (it != m_processes.end())
    {
        // Grab the next process
        StrongProcessPtr currentProcess = (*it);

        // Run process fixed update for process
        bool isDead = currentProcess->runFixed(deltaMs);

        // Only queue process to run again if it hasn't died
        if (!isDead) {
            m_processQueue.emplace(currentProcess);
        }
        // Process is destroyed if it is dead

        // Increment iterator 
        ++it;
    }

    // Swap process queue with iterable list
    m_processes.swap(m_processQueue);
    m_processQueue.clear();

    return;
}
/////////////////////////////////////////////////////////////////////////////////////////////
WeakProcessPtr ProcessManager::attachProcess(StrongProcessPtr process, bool initialize)
{
    // Handle threaded vs unthreaded processes
    if (!process->isThreaded()) {
        // Add unthreaded process to process list
        m_processQueue.emplace(process);
        if (initialize)
            process->onInit();
    }
    else {
        // Add threaded process to thread pool queue
        auto threadedProcess = std::dynamic_pointer_cast<ThreadedProcess>(process);
        Vec::EmplaceBack(m_threadedProcesses, threadedProcess);

        //int count = m_threadPool->activeThreadCount();
        //int maxCount = m_threadPool->maxThreadCount();
#ifndef USE_MULTITHREADING
        threadedProcess->run();
#else
        auto future = QtConcurrent::run(m_threadPool, std::bind(&ThreadedProcess::run, threadedProcess));
#endif
        //future.waitForFinished();

        //m_threadPool->start(threadedProcess.get());
    }
    return WeakProcessPtr(process);
}
/////////////////////////////////////////////////////////////////////////////////////////////
WeakProcessPtr ProcessManager::reattachProcess(StrongProcessPtr process)
{
    // Return if threaded process
    if (process->isThreaded()) {
#ifdef DEBUG_MODE
        logError("Error, cannot safely reattach threaded process, nothing done");
#endif
        return process;
    }

    // Find process in process set
    ProcessSet::const_iterator iter =
        std::find_if(m_processes.begin(), m_processes.end(),
            [&](const StrongProcessPtr& process) {
        return process->getUuid() == process->getUuid();
    });
    if (iter == m_processes.end()) {
#ifdef DEBUG_MODE
        throw("Error, cannot reattach process that was never attached");
#else
        attachProcess(process);
#endif
    }
    else {
        m_processQueue.erase(iter);
        m_processQueue.emplace(process);
    }

    return process;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ProcessManager::abortAllProcesses(bool immediate)
{
    if (m_processes.size()) {
        auto processes = m_processes;
        ProcessSet::iterator it = processes.begin();
        while (it != processes.end())
        {
            abortProcess(*it, immediate);
            ++it;
        }
    }

    if (m_threadedProcesses.size()) {
        std::vector<StrongProcessPtr> threadedProcesses = m_threadedProcesses;
        std::vector<StrongProcessPtr>::iterator thrit = threadedProcesses.begin();
        while (thrit != threadedProcesses.end())
        {
            abortProcess(*thrit, immediate);
            ++thrit;
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ProcessManager::postConstruction()
{
    // Initialize signal/slot connections
    deferredSenderConnect("EventManager",
        SIGNAL(deleteThreadedProcess(const Uuid&)),
        SLOT(deleteThreadedProcess(const Uuid&)));
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue ProcessManager::asJson() const
{
    QJsonObject json;

    // Add sorting layers to JSON
    QJsonArray sortingLayers;
    for (const std::pair<QString, SortingLayer*> layerPair : m_sortingLayers) {
        sortingLayers.append(layerPair.second->asJson());
    }
    json.insert("sortingLayers", sortingLayers);

    return json;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ProcessManager::loadFromJson(const QJsonValue & json)
{
    const QJsonObject& object = json.toObject();
    if (object.contains("sortingLayers")) {
        const QJsonArray& sortingLayers = object.value("sortingLayers").toArray();
        for (const auto& layerJson : sortingLayers) {
            SortingLayer* newLayer = new SortingLayer(layerJson);
            m_sortingLayers.emplace(newLayer->m_label, newLayer);
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ProcessManager::initializeThreads(unsigned int threadCount)
{
    // Set new threads to not expire
    m_threadPool->setExpiryTimeout(-1);

    // Create threads in thread pool
    for (size_t i = 0; i < threadCount; i++) {
        m_threadPool->reserveThread();
    }

#ifdef DEBUG_MODE
    int maxAllowableThreads = m_threadPool->maxThreadCount();
    int currentCount = m_threadPool->activeThreadCount();
    if (currentCount > maxAllowableThreads) {
        logWarning("Warning, thread count exceeds optimum number of threads");
    }
#endif

    THREAD_COUNT += threadCount;
    m_engine->THREAD_COUNT += threadCount;
}
/////////////////////////////////////////////////////////////////////////////////////////////
unsigned int ProcessManager::loadProcessCount() const
{
    int count = 0;
    for (StrongProcessPtr ptr : m_threadedProcesses) {
        if (QString(ptr->className()).contains("LoadProcess")) {
            count++;
        }
    }

    return count;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ProcessManager::clearAllProcesses()
{
    // Abort all processes
    abortAllProcesses(true);

    // Clear process queues
    m_processes.clear();
    m_threadedProcesses.clear();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ProcessManager::deleteThreadedProcess(const Uuid& uuid){
    auto iter = std::find_if(m_threadedProcesses.begin(), m_threadedProcesses.end(),
        [&](const std::shared_ptr<Process>& process) {
        if (!process) { 
            return true;
        }
        else {
            return process->getUuid() == uuid;
        }
    });
    if (iter != m_threadedProcesses.end()) {
        m_threadedProcesses.erase(iter);
    }
    else {
#ifdef DEBUG_MODE
        logWarning("Warning, did not find threaded process to delete");
#endif
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ProcessManager::logMessage(const QString & message, LogLevel logLevel)
{
    QApplication::postEvent(m_engine,
        new LogEvent(namespaceName(), message, Process::getThreadID(), logLevel));
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ProcessManager::abortProcess(StrongProcessPtr process, bool immediate)
{
    if (process->isAlive())
    {
        process->setState(Process::kAborted);
        if (immediate)
        {
            process->onAbort();

            // Remove from process set if not a threaded process
            ProcessSet::const_iterator iter = std::find_if(m_processes.begin(),
                m_processes.end(),
                [&](StrongProcessPtr p) {
                return p->getUuid() == process->getUuid();
            });
            if (iter != m_processes.end()) {
                m_processes.erase(iter);
            }

            // Remove from threaded process list if threaded
            std::vector<StrongProcessPtr>::const_iterator titer = std::find_if(m_threadedProcesses.begin(),
                m_threadedProcesses.end(),
                [&](StrongProcessPtr p) {
                return p->getUuid() == process->getUuid();
            });
            if (titer != m_threadedProcesses.end()) {
                m_threadedProcesses.erase(titer);
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
unsigned int ProcessManager::THREAD_COUNT = 0;




/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces