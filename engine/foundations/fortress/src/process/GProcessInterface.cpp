#include "fortress/process/GProcessInterface.h"
#include "fortress/process/GProcessQueueInterface.h"

namespace rev {

ProcessInterface::ProcessInterface() :
    m_child(nullptr),
    m_state(ProcessState::kUninitialized),
    m_processQueue(nullptr)
{
}

ProcessInterface::ProcessInterface(ProcessQueueInterface* queue) :
    m_child(nullptr),
    m_state(ProcessState::kUninitialized),
    m_processQueue(queue)
{
}

ProcessInterface::~ProcessInterface()
{
}

StrongProcessInterfacePtr ProcessInterface::removeChild(void)
{
    if (m_child) {
        StrongProcessInterfacePtr child = m_child;  // This keeps the child from getting destroyed when we clear it
        m_child.reset(); // Remove self as owner of the original pointer
        return m_child;
    }
    return nullptr;
}

bool ProcessInterface::checkFinished()
{
    bool dead = isDead();
    if (dead)
    {
        // run the appropriate exit function
        switch (getState())
        {
        case ProcessState::kSucceeded:
        {
            onSuccess();
            if (m_processQueue) {
                // Child processes require a process queue be assigned
                StrongProcessInterfacePtr child = removeChild();
                if (child)
                    m_processQueue->attachProcess(child);
                else {
#ifdef DEBUG_MODE
                    std::cout << ("Successful process chain completion");
#endif
                }
            }
            break;
        }

        case ProcessState::kFailed:
        {
            onFail();
#ifdef DEBUG_MODE
            std::cout << ("Failed process");
#endif
            break;
        }

        case ProcessState::kAborted:
        {
            onAbort();
#ifdef DEBUG_MODE
            std::cout << ("Aborted process");
#endif
            break;
        }
        }
    }

    return dead;
}

} // End namespaces