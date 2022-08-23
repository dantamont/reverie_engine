#include "core/components/GListenerComponent.h"

#include "core/GCoreEngine.h"
#include "core/events/GEvent.h"
#include "core/events/GEventManager.h"
#include "core/events/GEventListener.h"

#include "core/scripting/GPythonScript.h"

#include "core/scene/GScene.h"
#include "core/scene/GScenario.h"
#include "core/scene/GSceneObject.h"

namespace rev {

ListenerComponent::ListenerComponent() :
    Component(ComponentType::kListener)
{
}

ListenerComponent::ListenerComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, ComponentType::kListener)
{
    setSceneObject(sceneObject());
    sceneObject()->setComponent(this);
}

rev::ListenerComponent::~ListenerComponent()
{
    // Delete event listener
    delete m_listener;
}

void ListenerComponent::initializeListener(const QString & filepath)
{
    if (m_listener) {
        delete m_listener;
    }

    m_listener = new EventListener(this);
    m_listener->initializeScript(filepath.toStdString());
}

void ListenerComponent::reset()
{
    if (m_listener) {
        // Reload script
        m_listener->script()->reload();

        // Refresh instance of script
        /// @FIXME: This doesn't seem to be reinitializing properly
        m_listener->script()->instantiate(sceneObject());
    }
}

void ListenerComponent::enable()
{
    Component::enable();
}
 
void ListenerComponent::disable()
{
    Component::disable();
}

void ListenerComponent::preDestruction(CoreEngine * core)
{
    core->eventManager()->removeListener(m_listener);
}

void to_json(json& orJson, const ListenerComponent& korObject)
{
    ToJson<Component>(orJson, korObject);
    if (korObject.m_listener) {
        orJson["listener"] = *korObject.m_listener;
    }
}

void from_json(const json& korJson, ListenerComponent& orObject)
{
    FromJson<Component>(korJson, orObject);

    // Delete previous listener
    if (orObject.m_listener) {
        delete orObject.m_listener;
    }

    // Initialize listener
    if (korJson.contains("listener")) {
        orObject.m_listener = new EventListener(&orObject, korJson.at("listener"));
    }
}


} // end namespacing