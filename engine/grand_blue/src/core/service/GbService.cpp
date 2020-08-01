///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GbService.h"

// Standard Includes
#include <memory.h>

// External
#include <QtWidgets>

// Internal
#include "GbServiceManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Implementations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AbstractService
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AbstractService::AbstractService(const QString& name, bool defer) :
    m_postConstructionDone(false),
    m_deferPostConstruction(defer)
{
    m_name = name;
    ServiceManager& sm = ServiceManager::getInstance();
    sm.addService(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AbstractService::~AbstractService()
{
    ServiceManager& sm = ServiceManager::getInstance();
    sm.removeService(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AbstractService::deferredSenderConnect(const char* sender_name,
    const char* sender_signal,
    const char* receiver_slot)
{
    ServiceManager& sm = ServiceManager::getInstance();
    return sm.addDeferredSenderConnection(QString(sender_name),
                                          QString(sender_signal),
                                          getName(), 
										  QString(receiver_slot));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AbstractService::deferredSenderConnect(const QString& sender_name,
    const char* sender_signal,
    const char* receiver_slot)
{
    ServiceManager& sm = ServiceManager::getInstance();
    return sm.addDeferredSenderConnection(sender_name,
                                          QString(sender_signal),
                                          getName(), 
										  QString(receiver_slot));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const AbstractService& AbstractService::getService(const QString& service_name)
{
    ServiceManager& sm = ServiceManager::getInstance();
    return sm.getService(service_name);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Service
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Service::Service(const QString& name, QObject* parent, bool defer) :
    AbstractService(name, defer),
    QObject(parent)
{
    ServiceManager& sm = ServiceManager::getInstance();
    QEvent* postConstructionEvent = new QEvent(sm.postConstructionEventType());
    qApp->postEvent(this, postConstructionEvent, Qt::HighEventPriority);
    setObjectName(name);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Service::Service(const char * name, QObject * parent, bool deferPostConstruction) : 
    Service(QString(name), parent, deferPostConstruction) {
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString Service::signalSlotInfo() const
{
    QString result("Signal Slot Info: ");
    result.append(namespaceName());
    const QMetaObject* metaObj = metaObject();
    QMetaMethod mm;

    // Gather signal info
    result.append("\n  Signals:\n");
    for (int i = 0; i < metaObj->methodCount(); ++i) {
        mm = metaObj->method(i);
        if (mm.methodType() == QMetaMethod::Signal) {
            result.append("    ");
            result.append(mm.methodSignature());
            result.append("\n");
        }
    }
    result.append("  Slots:\n");
    for (int i = 0; i < metaObj->methodCount(); ++i) {
        mm = metaObj->method(i);
        if (mm.methodType() == QMetaMethod::Slot) {
            result.append("    ");
            result.append(mm.methodSignature());
            result.append("\n");
        }
    }
    return result;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Service::event(QEvent* event)
{
    ServiceManager& sm = ServiceManager::getInstance();
    if (event->type() == sm.postConstructionEventType()) {
        if (!m_postConstructionDone && !m_deferPostConstruction) {
            postConstruction();
            sm.connectDeferred(this);
            m_postConstructionDone = true;
        }
        event->accept();
    } else {
        return QObject::event(event);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // Gb
