#include "GbListenerComponent.h"

#include "../GbCoreEngine.h"
#include "../events/GbEvent.h"
#include "../events/GbEventManager.h"
#include "../events/GbEventListener.h"

#include "../scripting/GbPythonScript.h"

#include "../scene/GbScene.h"
#include "../scene/GbScenario.h"
#include "../scene/GbSceneObject.h"

namespace Gb {
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
Gb::ListenerComponent::~ListenerComponent()
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
QJsonValue ListenerComponent::asJson() const
{
    QJsonObject object = Component::asJson().toObject();
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