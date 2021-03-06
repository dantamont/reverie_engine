#include "GListenerComponent.h"

#include "../GCoreEngine.h"
#include "../events/GEvent.h"
#include "../events/GEventManager.h"
#include "../events/GEventListener.h"

#include "../scripting/GPythonScript.h"

#include "../scene/GScene.h"
#include "../scene/GScenario.h"
#include "../scene/GSceneObject.h"

namespace rev {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ListenerComponent::ListenerComponent() :
    Component(ComponentType::kListener)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ListenerComponent::ListenerComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, ComponentType::kListener)
{
    setSceneObject(sceneObject());
    sceneObject()->addComponent(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
rev::ListenerComponent::~ListenerComponent()
{
    // Delete event listener
    delete m_listener;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ListenerComponent::initializeListener(const QString & filepath)
{
    if (m_listener) {
        delete m_listener;
    }

    m_listener = new EventListener(this);
    m_listener->initializeScript(filepath);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ListenerComponent::reset()
{
    if (m_listener) {
        // Reload script
        m_listener->script()->reload();

        // Refresh instance of script
        // FIXME: This doesn't seem to be reinitializing properly
        m_listener->script()->instantiate(sceneObject());
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ListenerComponent::enable()
{
    Component::enable();
}
//////////////// ///////////////////////////////////////////////////////////////////////////////////////////////////////
void ListenerComponent::disable()
{
    Component::disable();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ListenerComponent::preDestruction(CoreEngine * core)
{
    core->eventManager()->removeListener(m_listener);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue ListenerComponent::asJson(const SerializationContext& context) const
{
    QJsonObject object = Component::asJson(context).toObject();
    if (m_listener) {
        object.insert("listener", m_listener->asJson());
    }

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ListenerComponent::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Component::loadFromJson(json, context);
    const QJsonObject& object = json.toObject();

    // Delete previous listener
    if (m_listener) {
        delete m_listener;
    }

    // Initialize listener
    if (object.contains("listener")) {
        m_listener = new EventListener(this, object.value("listener"));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing