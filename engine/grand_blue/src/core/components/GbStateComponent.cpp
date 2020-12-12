#include "GbStateComponent.h"

#include "../GbCoreEngine.h"
#include "../events/GbEvent.h"
#include "../events/GbEventManager.h"
#include "../events/GbEventListener.h"

#include "../scene/GbScene.h"
#include "../scene/GbScenario.h"
#include "../scene/GbSceneObject.h"

namespace Gb {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
StateComponent::StateComponent() :
    Component(ComponentType::kStateMachine)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
StateComponent::StateComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, ComponentType::kStateMachine)
{
    setSceneObject(sceneObject());
    sceneObject()->addComponent(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Gb::StateComponent::~StateComponent()
{
    //// Delete event listeners
    //for (const std::pair<QEvent::Type, std::map<QString, EventListener*>>& listenerMapPair : m_eventListeners) {
    //    for (const std::pair<QString, EventListener*>& listenerPair : listenerMapPair.second) {
    //        m_engine->eventManager()->removeListener(listenerPair.second);
    //        delete listenerPair.second;
    //    }
    //}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void StateComponent::enable()
{
    Component::enable();
}
//////////////// ///////////////////////////////////////////////////////////////////////////////////////////////////////
void StateComponent::disable()
{
    Component::disable();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue StateComponent::asJson() const
{
    QJsonObject object = Component::asJson().toObject();

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void StateComponent::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)
    Component::loadFromJson(json);
    //const QJsonObject& object = json.toObject();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing