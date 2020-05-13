#include "GbLogEvent.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
LogEvent::LogEvent(
    const QString& namespaceName,
    const QString& logMessage,
    const QString& threadId,
    LogLevel messageLevel) :
    Event()
{ 
    if (!threadId.isEmpty()) {
        m_logMessage += "Thread " + threadId + ": ";
    }
    m_logMessage += logMessage;
    m_namespaceName = namespaceName;
    m_logLevel = messageLevel;
}
/////////////////////////////////////////////////////////////////////////////////////////////
LogEvent::~LogEvent()
{
}


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
