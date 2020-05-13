#include "GbProcess.h"
#include "GbProcessManager.h"
#include "../GbCoreEngine.h"

// Qt
#include <QApplication>

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
QString Process::getThreadID()
{
    if (isMainThread()) {
        return "Main";
    }
    else {
        return "Auxillary";
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Process::isMainThread()
{
    return QApplication::instance()->thread() == QThread::currentThread();
}
/////////////////////////////////////////////////////////////////////////////////////////////
Process::Process(CoreEngine* engine, ProcessManager* manager) :
    m_child(nullptr),
    m_state(kUninitialized),
    m_engine(engine),
    m_processManager(manager)
{
    m_sortingLayer = m_processManager->sortingLayers().at("default");
}
/////////////////////////////////////////////////////////////////////////////////////////////
Process::Process(CoreEngine * engine,
    SortingLayer * sortLayer,
    ProcessManager* manager) :
    m_child(nullptr),
    m_state(kUninitialized),
    m_engine(engine),
    m_processManager(manager),
    m_sortingLayer(sortLayer)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Process::~Process()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
const SortingLayer & Process::getSortingLayer() const
{
    return *m_sortingLayer;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Process::runProcess(unsigned long deltaMs)
{
    // Process is uninitialized, so checkValidity it
    if (m_state == Process::kUninitialized) onInit();

    // Give the process an update tick if it's running
    if (m_state == Process::kRunning) {
        onUpdate(deltaMs);
    }

    // check to see if the process is dead
    bool dead = checkFinished();

//#ifdef DEBUG_MODE
//    logMessage("Process:: Ran process");
//#endif

    return dead;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Process::runFixed(unsigned long deltaMs)
{
    // Process is uninitialized, so checkValidity it
    if (m_state == Process::kUninitialized) onInit();

    // Give the process  fixedupdate tick if it's running
    if (m_state == Process::kRunning) {
        onFixedUpdate(deltaMs);
    }

    // check to see if the process is dead
    bool dead = checkFinished();

//#ifdef DEBUG_MODE
//    logMessage("Process:: Ran process fixed update");
//#endif

    return dead;
}
/////////////////////////////////////////////////////////////////////////////////////////////
StrongProcessPtr Process::removeChild(void)
{
    if (m_child) {
        StrongProcessPtr child = m_child;  // This keeps the child from getting destroyed when we clear it
        m_child.reset(); // Remove self as owner of the original pointer
        return m_child;
    }
    return nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Process::checkFinished()
{
    bool dead = isDead();
    if (dead)
    {
        // run the appropriate exit function
        switch (getState())
        {
        case Process::kSucceeded:
        {
            onSuccess();
            StrongProcessPtr child = removeChild();
            if (child)
                m_processManager->attachProcess(child);
            else
#ifdef DEBUG_MODE
                logMessage("Successful process chain completion");
#endif
            break;
        }

        case Process::kFailed:
        {
            onFail();
#ifdef DEBUG_MODE
            logMessage("Failed process");
#endif
            break;
        }

        case Process::kAborted:
        {
            onAbort();
#ifdef DEBUG_MODE
            logMessage("Aborted process");
#endif
            break;
        }
        }
    }

    return dead;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Process::logMessage(const QString & message, LogLevel logLevel)
{
    QApplication::postEvent(m_engine, 
        new LogEvent(namespaceName(), message, getThreadID(), logLevel));
}

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces