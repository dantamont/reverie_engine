#include "GStateComponent.h"

#include "../GCoreEngine.h"
#include "../events/GEvent.h"
#include "../events/GEventManager.h"
#include "../events/GEventListener.h"

#include "../scene/GScene.h"
#include "../scene/GScenario.h"
#include "../scene/GSceneObject.h"

namespace rev {
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
rev::StateComponent::~StateComponent()
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
QJsonValue StateComponent::asJson(const SerializationContext& context) const
{
    QJsonObject object = Component::asJson(context).toObject();

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