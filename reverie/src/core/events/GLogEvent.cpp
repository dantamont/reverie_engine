#include "GLogEvent.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
LogEvent::LogEvent(
    const GString& namespaceName,
    const GString& logMessage,
    const GString& threadId,
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
