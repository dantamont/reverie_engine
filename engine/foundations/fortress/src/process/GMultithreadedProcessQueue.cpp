#include "fortress/process/GMultithreadedProcessQueue.h"

#include <algorithm>

#include "fortress/containers/GContainerExtensions.h"
#include "fortress/process/GThreadedProcess.h"

#define USE_MULTITHREADING

namespace rev {

const Uint32_t MultithreadedProcessQueue::s_defaultThreadCount = std::max((Uint32_t)(std::thread::hardware_concurrency() - 2), (Uint32_t)2);

MultithreadedProcessQueue::MultithreadedProcessQueue(Uint32_t initialThreadCount):
    m_threadPool(initialThreadCount)
{
}

MultithreadedProcessQueue::~MultithreadedProcessQueue()
{
}

WeakProcessInterfacePtr MultithreadedProcessQueue::attachProcess(StrongProcessInterfacePtr process, bool initialize)
{
    std::shared_ptr<ThreadedProcess> threadedProcess = std::static_pointer_cast<ThreadedProcess>(process);

    m_threadedProcessMutex.lock();

    // Add threaded process to thread pool queue
    Vec::EmplaceBack(m_threadedProcesses, threadedProcess);
    m_threadedProcessMutex.unlock();

    //int count = m_threadPool->activeThreadCount();
    //int maxCount = m_threadPool->maxThreadCount();
#ifndef USE_MULTITHREADING
    threadedProcess->run();
#else
    std::future<void> future = m_threadPool.addTask(&ThreadedProcess::run, threadedProcess);
    G_UNUSED(future);
#endif
    //future.waitForFinished();

    //m_threadPool->start(threadedProcess.get());

    return WeakProcessInterfacePtr(threadedProcess);
}

void MultithreadedProcessQueue::abortAllProcesses(bool immediate)
{
    // Clear processes
    {
        std::unique_lock lock(m_threadedProcessMutex);

        if (m_threadedProcesses.size()) {

            std::vector<std::shared_ptr<ThreadedProcess>> threadedProcesses = m_threadedProcesses;
            std::vector<std::shared_ptr<ThreadedProcess>>::iterator thrit = threadedProcesses.begin();
            while (thrit != threadedProcesses.end())
            {
                abortProcess(*thrit, immediate);
                ++thrit;
            }
        }
    }
}

void to_json(json&, const MultithreadedProcessQueue&)
{
}

void from_json(const json&, MultithreadedProcessQueue&)
{
}

void MultithreadedProcessQueue::checkProcesses(double deltaSec)
{
    {
        // Check that threaded processes haven't failed
        std::unique_lock tpLock(m_threadedProcessMutex);
        for (const std::shared_ptr<ThreadedProcess>& process : m_threadedProcesses) {
            std::shared_ptr<ThreadedProcess> threadedProcess = std::static_pointer_cast<ThreadedProcess>(process);
            threadedProcess->exceptionMutex().lock();
            const std::exception_ptr& ex = threadedProcess->exception();
            if (ex) {
                try {
                    std::rethrow_exception(ex);
                }
                catch (const std::exception& e) {
                    std::string ex = e.what();
                    std::cerr << ("Caught exception \"" + GString(ex.c_str()) + "\"\n");
                    throw std::runtime_error(ex);
                }
                catch (...) {
                    std::exception_ptr curr = std::current_exception();
                    std::rethrow_exception(curr);
                }
            }
            threadedProcess->exceptionMutex().unlock();
        }
    }
}

void MultithreadedProcessQueue::clearProcesses()
{
    // Abort all processes
    abortAllProcesses(true);

    // Clear process queues
    std::unique_lock lock(m_threadedProcessMutex);
    m_threadedProcesses.clear();
}

void MultithreadedProcessQueue::deleteThreadedProcess(const Uuid& uuid){

    std::unique_lock lock(m_threadedProcessMutex);

    auto iter = std::find_if(m_threadedProcesses.begin(), m_threadedProcesses.end(),
        [&](const std::shared_ptr<ThreadedProcess>& process) {
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
        std::cout << ("Warning, did not find threaded process to delete");
#endif
    }
}

void MultithreadedProcessQueue::abortProcess(std::shared_ptr<ThreadedProcess> process, bool immediate)
{
    if (process->isAlive())
    {
        process->setState(ProcessState::kAborted);
        if (immediate)
        {
            process->onAbort();

            // Throw any exceptions
            const std::exception_ptr& ex = process->exception();
            if (ex) {
                std::rethrow_exception(ex);
            }

            // Remove from threaded process list if threaded
            // Mutex should already be locked
            std::vector<std::shared_ptr<ThreadedProcess>>::const_iterator titer = std::find_if(m_threadedProcesses.begin(),
                m_threadedProcesses.end(),
                [&](std::shared_ptr<ThreadedProcess> p) {
                    return p->getUuid() == process->getUuid();
                });
            if (titer != m_threadedProcesses.end()) {
                m_threadedProcesses.erase(titer);
            }
        }
    }
}




} // End namespaces