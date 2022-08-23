#include "fortress/process/GProcessQueueThread.h"
#include "fortress/time/GStopwatchTimer.h"

namespace rev {

ProcessQueueThread::ProcessQueueThread():
    ProcessQueue(),
    m_isRunning(false),
    m_fixedDeltaTime(0),
    m_fixedUpdateInterval(0.016), // in sec
    m_minUpdateIntervalSec(5.0e-3)
{
}

ProcessQueueThread::~ProcessQueueThread()
{
    join();
}

void ProcessQueueThread::initialize() 
{
    // Initialize thread from a packaged task
    m_isRunning.store(true);
    std::packaged_task<void()> task(std::bind(&ProcessQueueThread::update, this)); // Wrap the update function
    m_future = task.get_future(); // Get future from the task
    m_thread = std::thread(std::move(task)); // Launch the task on a thread
    //m_thread = std::thread(&ProcessQueueThread::update, this); // Simpler, but no future
}

typedef std::chrono::steady_clock clock;
void ProcessQueueThread::update()
{
    try {
        StopwatchTimer timer;
        double deltaSec = 0;
        while (m_isRunning.load()) {
            // Restart timer
            timer.restart();

            // Update processes
            {
                std::unique_lock lock(m_processMutex);

                //// Fixed update loop 
                m_fixedDeltaTime += deltaSec;
                while (m_fixedDeltaTime > m_fixedUpdateInterval) {
                    // Perform fixed updates
                    step<ProcessUpdateState::kFixed>(m_fixedUpdateInterval);
                    m_fixedDeltaTime -= m_fixedUpdateInterval;
                }

                //// Updates
                step<ProcessUpdateState::kUpdate>(deltaSec);
                step<ProcessUpdateState::kLateUpdate>(deltaSec);
                step<ProcessUpdateState::kPostUpdate>(deltaSec);
            }

            // Set elapsed time
            deltaSec = timer.getElapsed<double>();

            // If not enough time has passed until next loop, sleep
            double sleepForSec = m_minUpdateIntervalSec - deltaSec;
            if (sleepForSec > 0) {
                std::this_thread::sleep_for(std::chrono::duration<double>(sleepForSec));
                deltaSec = timer.getElapsed<double>();
            }
        }
    }
    catch (const std::exception& e) {
        std::string err = e.what();
        std::cerr << (err.c_str());
        m_exceptionLock.lock();
        m_exceptionPtr = std::current_exception();
        m_exceptionLock.unlock();
    }
    catch (...) {
        m_exceptionLock.lock();
        m_exceptionPtr = std::current_exception();
        m_exceptionLock.unlock();
    }
}

void ProcessQueueThread::join()
{
    m_isRunning.store(false); // Stop the update loop
    if (m_thread.joinable()) {
        // Check if thread is already joined
        m_thread.join(); // Join the thread
    }
}

WeakProcessInterfacePtr ProcessQueueThread::attachProcess(StrongProcessInterfacePtr process, bool initialize)
{
    std::unique_lock lock(m_processMutex);
    return ProcessQueue::attachProcess(process, initialize);
}



} // End namespaces