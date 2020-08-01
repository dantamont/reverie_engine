#include "GbEventListener.h"

#include <QVariant>
#include <QVariantList>

#include "../GbCoreEngine.h"
#include "../resource/GbResourceCache.h"
#include "../events/GbEventManager.h"

#include "../scripting/GbPythonAPI.h"
#include "../scripting/GbPythonScript.h"
#include "../scene/GbSceneObject.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// EventListener
/////////////////////////////////////////////////////////////////////////////////////////////
EventListener::EventListener(CoreEngine* engine):
    m_engine(engine)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
EventListener::EventListener(CoreEngine * engine, const QJsonValue & json):
    m_engine(engine)
{
    loadFromJson(json);
}
/////////////////////////////////////////////////////////////////////////////////////////////
EventListener::~EventListener()
{
    m_engine->eventManager()->removeListener(this);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void EventListener::addEventType(int type)
{
    m_eventTypes.insert(type);
    m_engine->eventManager()->addListener(type, this);
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool EventListener::testEvent(CustomEvent* ev)
{
    // Check event type
    int eventType = ev->type();
    bool takeAction = m_eventTypes.find(eventType) != m_eventTypes.end();
    if (takeAction) {
        // Event type matches, so make sure that event test passes
        QVariantList args = QVariantList() << QVariant::fromValue(ev);
        QVariant testOut = m_pythonListener.call("event_test", args);

        takeAction &= testOut.toBool();
    }

    return takeAction;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void EventListener::perform(CustomEvent* ev)
{
    // Return if event test failed
    if (!testEvent(ev)) return;

    QVariantList args = QVariantList() << QVariant::fromValue(ev);
    QVariant actionOut = m_pythonListener.call("perform", args);
    int succeeded = actionOut.toInt();
    if (!succeeded) {
        QString stdErr = PythonAPI::get()->getStdErr();
        logError("perform:: Failed to perform action: " + stdErr);
        PythonAPI::get()->clearStdErr();
    }

#ifdef DEBUG_MODE
    PythonAPI::get()->logStdOut();
    PythonAPI::get()->printAndClearErrors();
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void EventListener::initializeScript(const QString & filepath, const std::shared_ptr<SceneObject>& object)
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
    m_script = m_engine->resourceCache()->guaranteeHandleWithPath(m_path,
        Resource::kPythonScript)->resourceAs<PythonClassScript>();

    // Create the instantiation of class from the script
    m_pythonListener = m_script->instantiate(object);

}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue EventListener::asJson() const
{
    QJsonObject object;
    if (!m_path.isEmpty()) {
        object.insert("path", m_path);
    }

    if (sceneObject()) {
        object.insert("sceneObject", sceneObject()->getName());
    }

    QJsonArray eventTypes;
    for (const int& type : m_eventTypes) {
        eventTypes.append(type);
    }
    object.insert("eventTypes", eventTypes);

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void EventListener::loadFromJson(const QJsonValue & json)
{
    // Load file path
    const QJsonObject& object = json.toObject();
    if (object.contains("path")) {
        m_path = object.value("path").toString();
    }

    // Set scene object
    if (object.contains("sceneObject")) {
        QString name = object.value("sceneObject").toString();
        m_sceneObject = SceneObject::getByName(name);
    }

    // Set event types
    QJsonArray eventTypes = object.value("eventTypes").toArray();
    for (const QJsonValue& type : eventTypes) {
        addEventType(type.toInt());
    }

    // Initialize behavior
    if (!m_path.isEmpty() && sceneObject()) {
        initializeScript(m_path, sceneObject());
    }
}



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
