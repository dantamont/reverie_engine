#include "core/events/GEventListener.h"

// #include <QVariant>
// #include <QVariantList>

#include "core/GCoreEngine.h"
#include "core/resource/GResourceCache.h"
#include "core/events/GEventManager.h"

#include "core/components/GListenerComponent.h"

#include "core/scripting/GPythonAPI.h"
#include "core/scripting/GPythonScript.h"
#include "core/scene/GSceneObject.h"
#include "core/scene/GScene.h"

namespace rev {



// EventListener

EventListener::EventListener(ListenerComponent* lc):
    m_listenerComponent(lc)
{
}

EventListener::EventListener(ListenerComponent * lc, const nlohmann::json& json):
    m_listenerComponent(lc)
{
    json.get_to(*this);
}

EventListener::~EventListener()
{
}

void EventListener::setEventTypes(const std::vector<int> eventTypes)
{
    // Remove as responder from event manager for all previous event types
    m_listenerComponent->sceneObject()->scene()->engine()->eventManager()->removeListener(this);
    m_eventTypes.clear();

    // Add as listener for new types
    for (int type : eventTypes) {
        addEventType(type);
    }
}

void EventListener::addEventType(int type)
{
    m_eventTypes.insert(type);
    m_listenerComponent->sceneObject()->scene()->engine()->eventManager()->addListener(type, this);
}

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
            GString className = m_script->getClassName().toStdString();
            GString errG(err.what());
            Logger::LogError("testEvent:: Failed to test " + GString(className) + ": " + errG);
        }
    }

    return takeAction;
}

void EventListener::perform(CustomEvent* ev)
{
    // Return if event test failed
    if (!testEvent(ev)) return;

    try {
        m_pythonListener.attr("perform")(*ev);
    }
    catch (py::error_already_set& err) {
        GString className = m_script->getClassName().toStdString();
        GString errG(err.what());
        Logger::LogError("perform:: Failed to perform " + GString(className) + ": " + errG);
    }

}

void EventListener::initializeScript(const GString & filepath)
{
    // Return if no filepath
    QFile file(filepath.c_str());
    if (!file.exists()) {
#ifdef DEBUG_MODE
        Logger::Throw("Warning, filepath to script not found:" + filepath);
#else
        Logger::LogWarning("Warning, filepath to script not found:" + filepath);
#endif
        return;
    }

    // Set script for this component
    // Note: Adds script to the scenario (and python) if not present
    m_path = filepath;
    m_script = ResourceCache::Instance().guaranteeHandleWithPath(m_path,
        EResourceType::ePythonScript)->resourceAs<PythonClassScript>();

    // Create the instantiation of class from the script
    m_pythonListener = m_script->instantiate(m_listenerComponent->sceneObject());

}

void to_json(json& orJson, const EventListener& korObject)
{
    if (!korObject.m_path.isEmpty()) {
        orJson["path"] = korObject.m_path.c_str();
    }

    json eventTypes = json::array();
    for (const int& type : korObject.m_eventTypes) {
        eventTypes.push_back(type);
    }
    orJson["eventTypes"] = eventTypes;
}

void from_json(const json& korJson, EventListener& orObject)
{
    // Load file path
    if (korJson.contains("path")) {
        orObject.m_path = korJson.at("path");
    }

    // Set event types
    const json& eventTypes = korJson.at("eventTypes");
    for (const json& type : eventTypes) {
        orObject.addEventType(type.get<Int32_t>());
    }

    // Initialize behavior
    if (!orObject.m_path.isEmpty()) {
        orObject.initializeScript(orObject.m_path);
    }
}




} // End namespaces
