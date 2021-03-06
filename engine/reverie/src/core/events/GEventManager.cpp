#include "GEventManager.h"

#include "../GCoreEngine.h"
#include "../resource/GResource.h"
#include "../resource/GResourceCache.h"
#include "GEvent.h"
#include "GEventListener.h"

#include "../../view/GWidgetManager.h"
#include "../../view/GL/GGLWidget.h"
#include "../input/GInputHandler.h"
#include "../rendering/renderer/GMainRenderer.h"
#include "../debugging/GDebugManager.h"
#include "../components/GCameraComponent.h"

namespace rev {

EventManager::EventManager(CoreEngine* engine):
    Manager(engine, "EventManager")
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
EventManager::~EventManager()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool EventManager::event(QEvent* ev)
{
    addEvent(ev);
    return QObject::event(ev);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void EventManager::addEvent(QEvent* ev)
{
    QMutexLocker lock(&m_queueMutex);
    Vec::EmplaceBack(m_eventQueue, CustomEvent(ev));
}
/////////////////////////////////////////////////////////////////////////////////////////////
void EventManager::processEvents()
{
    static std::vector<CustomEvent> events;
    m_queueMutex.lock();
    m_eventQueue.swap(events);
    m_eventQueue.clear();
    m_queueMutex.unlock();

    for (CustomEvent& event : events) {
        processEvent(&event);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void EventManager::addListener(int eventType, EventListener * listener)
{
    // Convert type
    QEvent::Type type = QEvent::Type(eventType);

    // Add a vector of listeners for the given event type if none exist
    if (!Map::HasKey(m_eventListeners, type)) {
        m_eventListeners[type] = {};
    }

    // Check if listener is already added
#ifdef DEBUG_MODE
    size_t idx;
    if(hasListener(type, listener, idx)){
        throw("Error, listener is already added for this event type");
    }
#endif

    // Add listener
    m_eventListeners[type].push_back(listener);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void EventManager::removeListener(EventListener * listener)
{
    //const Uuid& uuid = listener->getUuid();
    for (auto it = m_eventListeners.begin(); it != m_eventListeners.end(); ++it) {
        size_t idx;
        if (hasListener(it->first, listener, idx)) {
            std::vector<EventListener*>& listeners = it.value();
            listeners.erase(listeners.begin() + idx);
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool EventManager::hasListener(QEvent::Type type, EventListener * listener, size_t& iterPos)
{
    std::vector<EventListener*>& listeners = m_eventListeners[type];
    const Uuid& uuid = listener->getUuid();
    auto iter = std::find_if(listeners.begin(), listeners.end(),
        [&](EventListener* l) {
        return l->getUuid() == uuid;
    });

    if (iter != listeners.end()) {
        iterPos = iter - listeners.begin();
        return true;
    }
    else {
        return false;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void EventManager::processEvent(CustomEvent* event)
{
    QEvent::Type eventType = event->type();
//#ifdef DEBUG_MODE
//    logInfo(QStringLiteral("Processing event of type ") + QString::number(int(eventType)));
//#endif

    //if (eventType == QEvent::MouseButtonRelease) {
    //    Vector2g pos = 
    //        std::move(m_engine->widgetManager()->mainGLWidget()->inputHandler().mouseHandler().widgetMousePosition());
    //    logInfo("widget pos: " + QString(pos));

    //    auto& camera = *m_engine->debugManager()->camera();
    //    Vector3g worldPos;
    //    Vector3g translation = camera.camera().getViewMatrix().inversed().getTranslationVector();
    //    camera.camera().widgetToRayDirection(pos, worldPos, *m_engine->widgetManager()->mainGLWidget()->renderer());
    //    logInfo("world pos: " + QString(worldPos));
    //    logInfo("---");
    //}

    // Return if no listeners correspond to the event type
    if (!Map::HasKey(m_eventListeners, eventType)) return;

    // Perform actions for all listeners corresponding to the event type
    for (const auto& listener : m_eventListeners.at(eventType)) {
        listener->perform(event);
    }

    // Delete the custom event
    //delete event;
}

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces