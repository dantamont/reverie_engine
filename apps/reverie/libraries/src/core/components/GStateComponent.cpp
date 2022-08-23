#include "core/components/GStateComponent.h"

#include "core/GCoreEngine.h"
#include "core/events/GEvent.h"
#include "core/events/GEventManager.h"
#include "core/events/GEventListener.h"

#include "core/scene/GScene.h"
#include "core/scene/GScenario.h"
#include "core/scene/GSceneObject.h"

namespace rev {

StateComponent::StateComponent() :
    Component(ComponentType::kStateMachine)
{
}

StateComponent::StateComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, ComponentType::kStateMachine)
{
    setSceneObject(sceneObject());
    sceneObject()->setComponent(this);
}

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

void StateComponent::enable()
{
    Component::enable();
}
 
void StateComponent::disable()
{
    Component::disable();
}

void to_json(json& orJson, const StateComponent& korObject)
{
    ToJson<Component>(orJson, korObject);
}

void from_json(const json& korJson, StateComponent& orObject)
{
    FromJson<Component>(korJson, orObject);
}


} // end namespacing