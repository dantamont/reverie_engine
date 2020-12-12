#include "GbEventListener.h"

#include <QVariant>
#include <QVariantList>

#include "../GbCoreEngine.h"
#include "../resource/GbResourceCache.h"
#include "../events/GbEventManager.h"

#include "../components/GbListenerComponent.h"

#include "../scripting/GbPythonAPI.h"
#include "../scripting/GbPythonScript.h"
#include "../scene/GbSceneObject.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// EventListener
/////////////////////////////////////////////////////////////////////////////////////////////
EventListener::EventListener(ListenerComponent* lc):
    m_listenerComponent(lc)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
EventListener::EventListener(ListenerComponent * lc, const QJsonValue & json):
    m_listenerComponent(lc)
{
    loadFromJson(json);
}
/////////////////////////////////////////////////////////////////////////////////////////////
EventListener::~EventListener()
{
    m_listenerComponent->sceneObject()->engine()->eventManager()->removeListener(this);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void EventListener::setEventTypes(const std::vector<int> eventTypes)
{
    // Remove as responder from event manager for all previous event types
    m_listenerComponent->sceneObject()->engine()->eventManager()->removeListener(this);
    m_eventTypes.clear();

    // Add as listener for new types
    for (int type : eventTypes) {
        addEventType(type);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void EventListener::addEventType(int type)
{
    m_eventTypes.insert(type);
    m_listenerComponent->sceneObject()->engine()->eventManager()->addListener(type, this);
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool EventListener::testEvent(CustomEvent* ev)
{
    // Check event type
    int eventType = ev->type();
    bool takeAction = m_eventTypes.find(eventType) != m_eventTypes.end();
    if (takeAction) {
        // Event type matches, so make sure that event test passes
        //QVariantList args = QVariantList() << QVariant::fromValue(ev);
        try {
            bool testOut = m_pythonListener.attr("event_test")(*ev).cast<bool>();
            takeAction &= testOut;
        }
        catch (py::error_already_set& err) {
            const GString& className = m_script->getClassName();
            GString errG(err.what());
            logError("testEvent:: Failed to test " + GString(className) + ": " + errG);
        }
    }

    return takeAction;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void EventListener::perform(CustomEvent* ev)
{
    // Return if event test failed
    if (!testEvent(ev)) return;

    try {
        m_pythonListener.attr("perform")(*ev);
    }
    catch (py::error_already_set& err) {
        const GString& className = m_script->getClassName();
        GString errG(err.what());
        logError("perform:: Failed to perform " + GString(className) + ": " + errG);
    }

}
/////////////////////////////////////////////////////////////////////////////////////////////
void EventListener::initializeScript(const QString & filepath)
{
    // Return if no filepath
    QFile file(filepath);
    if (!file.exists()) {
#ifdef DEBUG_MODE
        throw("Warning, filepath to script not found:" + filepath);
#else
        logWarning("Warning, filepath to script not found:" + filepath);
#endif
        return;
    }

    // Set script for this component
    // Note: Adds script to the scenario (and python) if not present
    m_path = filepath;
    m_script = m_listenerComponent->sceneObject()->engine()->resourceCache()->guaranteeHandleWithPath(m_path,
        Resource::kPythonScript)->resourceAs<PythonClassScript>();

    // Create the instantiation of class from the script
    m_pythonListener = m_script->instantiate(m_listenerComponent->sceneObject());

}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue EventListener::asJson() const
{
    QJsonObject object;
    if (!m_path.isEmpty()) {
        object.insert("path", m_path.c_str());
    }

    QJsonArray eventTypes;
    for (const int& type : m_eventTypes) {
        eventTypes.append(type);
    }
    object.insert("eventTypes", eventTypes);

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void EventListener::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    // Load file path
    const QJsonObject& object = json.toObject();
    if (object.contains("path")) {
        m_path = object.value("path").toString();
    }

    // Set event types
    QJsonArray eventTypes = object.value("eventTypes").toArray();
    for (const QJsonValue& type : eventTypes) {
        addEventType(type.toInt());
    }

    // Initialize behavior
    if (!m_path.isEmpty()) {
        initializeScript(m_path);
    }
}



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
