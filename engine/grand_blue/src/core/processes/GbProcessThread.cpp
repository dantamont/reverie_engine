#include "GbProcessThread.h"
#include "GbProcessManager.h"
#include "../GbCoreEngine.h"

// Qt
#include <QApplication>

#include "../time/GbTimer.h"

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
ProcessThread::ProcessThread():
    ProcessQueue(),
    m_isRunning(false),
    m_fixedDeltaTime(0),
    m_fixedUpdateInterval(16), // in ms
    //m_elapsedTime(0),
    m_minUpdateIntervalMSec(5.0)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
ProcessThread::~ProcessThread()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ProcessThread::initialize() 
{
    // Assert that fixed update interval be nonzero
    //if (m_fixedUpdateInterval < 0) {
    //    throw("Fixed update interval must be greater than zero");
    //}

    // Initialize update timer, which just keep track of elapsed time
    //m_nsecCount = 0;
    //m_updateTimer.restart();

    // Initialize thread
    m_isRunning = true;
    m_thread = std::thread(&ProcessThread::update, this);
}
/////////////////////////////////////////////////////////////////////////////////////////////
typedef std::chrono::steady_clock clock;
void ProcessThread::update()
{
    // TODO: Don't use a while loop to hang: 
    // See: https://stackoverflow.com/questions/26593839/calling-functions-at-timed-intervals-using-threads
    clock::time_point callEnd = clock::now();
    double minUpdateIntervalSec = m_minUpdateIntervalMSec * 1e-3;
    while (m_isRunning) {
        // Get delta time since last loop
        clock::time_point callStart = clock::now();
        clock::duration sinceLastCall = callStart - callEnd;

        double deltaMs = Timer::ToSeconds<double>(sinceLastCall) * 1e3;
        //m_nsecCount -= deltaMs * 1e6;

        // Update cached elapsed times
        //m_elapsedTime += deltaMs;

        // Fixed update loop 
        m_fixedDeltaTime += deltaMs;
        while (m_fixedDeltaTime > m_fixedUpdateInterval) {
            // Perform fixed updates
            fixedUpdateProcesses(m_fixedUpdateInterval);
            m_fixedDeltaTime -= m_fixedUpdateInterval;
        }

        // Run processes
        m_processMutex.lock();
        updateProcesses((unsigned long)deltaMs);

        // Perform late update 
        updateProcesses((unsigned long)deltaMs);
        m_processMutex.unlock();

        // Denote call has ended
        callEnd = clock::now();

        // If not enough time has passed until next loop, sleep
        clock::duration callDuration = callEnd - callStart;
        double sleepForSec = minUpdateIntervalSec - Timer::ToSeconds<double>(callDuration);
        if (sleepForSec > 0) {
            std::this_thread::sleep_for(std::chrono::duration<double>(sleepForSec));
        }

        //logThreadMessage(CoreEngine::engines().begin()->second, ("Running " 
            //+ GString(", sleeping for ") + GString::FromNumber(sleepForSec*1e3) + "ms").c_str());
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
WeakProcessPtr ProcessThread::attachProcess(StrongProcessPtr process, bool initialize)
{
    std::unique_lock lock(m_processMutex);
    return ProcessQueue::attachProcess(process, initialize);
}


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces