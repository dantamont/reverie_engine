///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GTool.h"
// Standard Includes

// External

// Internal
#include "../../core/service/GServiceManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Implementations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Tool
Tool::Tool(const QString &name, QWidget* parent) :
    AbstractService(name),
    QWidget(parent)
{
    ServiceManager& sm = ServiceManager::getInstance();
    sm.addService(this);
    QEvent* postConstructionEvent = new QEvent(sm.postConstructionEventType());
    qApp->postEvent(this, postConstructionEvent, Qt::HighEventPriority);
    setObjectName(name);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString Tool::signalSlotInfo() const
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
bool Tool::event(QEvent* event)
{
    ServiceManager& sm = ServiceManager::getInstance();
    if (event->type() == sm.postConstructionEventType()) {
        if (!m_postConstructionDone) {
            postConstruction();
            sm.connectDeferred(this);
            m_postConstructionDone = true;
        }
        event->accept();
    }
    else {
        return QWidget::event(event);
    }
    return true;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // view
} // rev
