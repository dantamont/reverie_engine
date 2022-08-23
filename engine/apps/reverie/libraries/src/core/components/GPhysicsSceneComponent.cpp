#include "core/components/GPhysicsSceneComponent.h"

#include "core/GCoreEngine.h"

#include "core/scene/GScene.h"
#include "core/scene/GScenario.h"
#include "core/scene/GSceneObject.h"

#include "core/physics/GPhysicsManager.h"
#include "core/physics/GPhysicsScene.h"

namespace rev {

PhysicsSceneComponent::PhysicsSceneComponent() :
    Component(ComponentType::kPhysicsScene, true),
    m_physicsScene(nullptr)
{
}

PhysicsSceneComponent::PhysicsSceneComponent(const PhysicsSceneComponent & component) :
    Component(component.scene(), ComponentType::kPhysicsScene),
    m_physicsScene(nullptr)
{
    initialize();
    scene()->setComponent(this);
}

PhysicsSceneComponent::PhysicsSceneComponent(Scene* object, const nlohmann::json& json) :
    Component(object, ComponentType::kPhysicsScene),
    m_physicsScene(nullptr)
{
    json.get_to(*this);
    scene()->setComponent(this);
}

PhysicsSceneComponent::PhysicsSceneComponent(Scene* object) :
    Component(object, ComponentType::kPhysicsScene),
    m_physicsScene(nullptr)
{
    initialize();
    scene()->setComponent(this);
}

rev::PhysicsSceneComponent::~PhysicsSceneComponent()
{
}

void PhysicsSceneComponent::enable()
{
    Component::enable();
}
 
void PhysicsSceneComponent::disable()
{
    Component::disable();
}

void to_json(json& orJson, const PhysicsSceneComponent& korObject)
{
    ToJson<Component>(orJson, korObject);
    orJson["physicsScene"] = *korObject.m_physicsScene;
}

void from_json(const json& korJson, PhysicsSceneComponent& orObject)
{
    // Return if invalid json
    if (!korJson.contains("physicsScene")) {
        return;
    }

    FromJson<Component>(korJson, orObject);

    // Load physics scene from json
    if (!orObject.m_physicsScene) {
        orObject.initialize();
    }
    korJson.at("physicsScene").get_to(*orObject.m_physicsScene);
}

void PhysicsSceneComponent::initialize()
{
    // Create physics scene
    m_physicsScene = PhysicsScene::create(scene());
}


} // end namespacing