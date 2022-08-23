#include "fortress/process/GThreadedProcess.h"
#include "fortress/process/GMultithreadedProcessQueue.h"

namespace rev {


ThreadedProcess::ThreadedProcess(MultithreadedProcessQueue* queue):
    ProcessInterface(queue)
{
}

ThreadedProcess::~ThreadedProcess()
{
}

void ThreadedProcess::onUpdate(double)
{
}

void ThreadedProcess::onSuccess()
{
    // Emit signal to delete the process
}

void ThreadedProcess::run()
{
    // Start update loop timer
    m_timer.restart();

    // Initialize the process
    if (getState() == ProcessState::kUninitialized) {
        onInit();
    }

    while (getState() == ProcessState::kRunning) {
        // Update while still active

        // Get delta time
        double deltaSec = StopwatchTimer::ToSeconds<double>(m_timer.restart());

        // Perform update and late update
        onUpdate(deltaSec);
        onLateUpdate(deltaSec);
        onPostUpdate(deltaSec);

        // Check whether or not the process is finished
        checkFinished();
    }

    // Check whether or not the process is finished
    // Note, this must be called to properly terminate the process
    checkFinished();
}




} // End namespaces