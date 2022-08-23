#include "core/processes/GProcessManager.h"
#include "fortress/process/GThreadedProcess.h"
#include "core/processes/GLoadProcess.h"

#include "core/GCoreEngine.h"
#include "core/events/GEventManager.h"
#include "core/scene/GScenario.h"

#include <QApplication>

#define USE_MULTITHREADING

namespace rev {


ProcessManager::ProcessManager(CoreEngine* engine, Uint32_t initialThreadCount):
    Manager(engine, "ProcessManager"),
    m_threadedProcessQueue(initialThreadCount)
{
    // Initialize threads
    initializeThreads(initialThreadCount);
}

ProcessManager::~ProcessManager()
{
}

void ProcessManager::postConstruction()
{
    /// @todo @fixme. This function isn't even called!
    // Initialize signal/slot connections
    connect(m_engine->eventManager(),
        SIGNAL(deleteThreadedProcess(const Uuid&)),
        SLOT(deleteThreadedProcess(const Uuid&)));
}

void to_json(json& orJson, const ProcessManager& korObject)
{
    to_json(orJson, korObject.m_processQueue);
}

void from_json(const json& korJson, ProcessManager& orObject)
{
    from_json(korJson, orObject.m_processQueue);
}

void ProcessManager::initializeThreads(unsigned int threadCount)
{
    // Set thread count in engine
    m_engine->THREAD_COUNT += m_threadedProcessQueue.threadPool().maxThreadCount();

    // Initialize dedicate threads
    // TODO: Create threads for things other than animations
    m_dedicatedThreads[(int)DedicatedThreadType::kAnimation].setName("Animation thread");
    m_dedicatedThreads[(int)DedicatedThreadType::kAnimation].initialize();
}

void ProcessManager::onUpdate(double deltaSec)
{
    // Update processes on the main thread
    m_processQueue.step<ProcessUpdateState::kUpdate>(deltaSec);

    // Check health of threaded processes
    updateThreadedProcesses(deltaSec);
}

void ProcessManager::onLateUpdate(double deltaSec)
{
    m_processQueue.step<ProcessUpdateState::kLateUpdate>(deltaSec);
}

void ProcessManager::onPostUpdate(double deltaSec)
{
    m_processQueue.step<ProcessUpdateState::kPostUpdate>(deltaSec);
}

void ProcessManager::onFixedUpdate(double deltaSec)
{
    m_processQueue.step<ProcessUpdateState::kFixed>(deltaSec);
}

void ProcessManager::updateThreadedProcesses(double deltaSec)
{
    m_threadedProcessQueue.checkProcesses(deltaSec);

    for (const ProcessQueueThread& thread: m_dedicatedThreads) {
        thread.exceptionMutex().lock();
        const std::exception_ptr& ex = thread.exception();
        if (ex) {
            try {
                std::rethrow_exception(ex);
            }
            catch (const std::exception& e) {
                std::string ex = e.what();
                Logger::LogError("Caught exception \"" + GString(ex.c_str()) + "\"\n");
                Logger::Throw(ex);
            }
            catch (...) {
                std::exception_ptr curr = std::current_exception();
                std::rethrow_exception(curr);
                //Logger::Throw("Unknown exception occurred");
            }
        }
        thread.exceptionMutex().unlock();
    }
}

void ProcessManager::clearProcesses()
{
    // Clear processes on dedicated threads
    for (ProcessQueueThread& thread : m_dedicatedThreads) {
        if (!thread.isRunning()) {
            continue;
        }
        {
            std::unique_lock lock(thread.m_processMutex);
            thread.abortAllProcesses(true);
            //thread.join();
        }
    }

    m_processQueue.clearProcesses();
    m_threadedProcessQueue.clearProcesses();
}

void ProcessManager::deleteThreadedProcess(const Uuid& uuid){
    m_threadedProcessQueue.deleteThreadedProcess(uuid);
}


} // End namespaces