#include "GbEventManager.h"

#include "../GbCoreEngine.h"
#include "../resource/GbResource.h"
#include "../resource/GbResourceCache.h"
#include "GbEvent.h"
#include "GbEventListener.h"

#include "../../view/GbWidgetManager.h"
#include "../../view/GL/GbGLWidget.h"
#include "../input/GbInputHandler.h"
#include "../rendering/renderer/GbMainRenderer.h"
#include "../debugging/GbDebugManager.h"
#include "../components/GbCamera.h"

namespace Gb {

EventManager::EventManager(CoreEngine* engine):
    Manager(engine, "EventManager")
{
    // Manage connections

    // Connect post-construction routine to resource cache and emit signal
    // Queued connection will ensure slot is executed in receiver's thread
    QObject::connect(this,
        &EventManager::doneLoadingResource,
        m_engine->resourceCache(),
        &ResourceCache::runPostConstruction,
        Qt::QueuedConnection);

    // Connect signal to resize a resource to resource cache load routine
    QObject::connect(this,
        &EventManager::doneLoadingResource,
        m_engine->resourceCache(),
        (void(ResourceCache::*)(std::shared_ptr<ResourceHandle>))&ResourceCache::reloadResource,
        Qt::QueuedConnection);
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
    m_queueMutex.lock();
    std::vector<CustomEvent> events(m_eventQueue);
    m_queueMutex.unlock();

    for (CustomEvent& event : events) {
        processEvent(&event);
    }
    m_eventQueue.clear();
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

    // Add listener
    const Uuid& listenerHash = listener->getUuid();
    if (Map::HasKey(m_eventListeners[type], listenerHash)) {
#ifdef DEBUG_MODE
        throw("Error, listener is already added for this event type");
#endif
    }

    m_eventListeners[type][listenerHash] = listener;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void EventManager::removeListener(EventListener * listener)
{
    const Uuid& uuidStr = listener->getUuid();
    for (std::pair<const QEvent::Type, std::unordered_map<Uuid, EventListener*>>& listenerMapPair : m_eventListeners) {
        if (Map::HasKey(listenerMapPair.second, uuidStr)) {
            listenerMapPair.second.erase(uuidStr);
        }
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
    for (const std::pair<Uuid, EventListener*>& listenerPair : m_eventListeners.at(eventType)) {
        listenerPair.second->perform(event);
    }

    // Delete the custom event
    //delete event;
}

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces