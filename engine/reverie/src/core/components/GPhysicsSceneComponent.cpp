#include "GPhysicsSceneComponent.h"

#include "../GCoreEngine.h"

#include "../scene/GScene.h"
#include "../scene/GScenario.h"
#include "../scene/GSceneObject.h"

#include "../physics/GPhysicsManager.h"
#include "../physics/GPhysicsScene.h"

namespace rev {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsSceneComponent::PhysicsSceneComponent() :
    Component(ComponentType::kPhysicsScene, true),
    m_physicsScene(nullptr)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsSceneComponent::PhysicsSceneComponent(const PhysicsSceneComponent & component) :
    Component(component.scene(), ComponentType::kPhysicsScene),
    m_physicsScene(nullptr)
{
    initialize();
    scene()->addComponent(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsSceneComponent::PhysicsSceneComponent(Scene* object, const QJsonValue & json) :
    Component(object, ComponentType::kPhysicsScene),
    m_physicsScene(nullptr)
{
    loadFromJson(json);
    scene()->addComponent(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsSceneComponent::PhysicsSceneComponent(Scene* object) :
    Component(object, ComponentType::kPhysicsScene),
    m_physicsScene(nullptr)
{
    initialize();
    scene()->addComponent(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
rev::PhysicsSceneComponent::~PhysicsSceneComponent()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsSceneComponent::enable()
{
    Component::enable();
}
//////////////// ///////////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsSceneComponent::disable()
{
    Component::disable();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue PhysicsSceneComponent::asJson(const SerializationContext& context) const
{
    QJsonObject object = Component::asJson(context).toObject();
    object.insert("physicsScene", m_physicsScene->asJson());
    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsSceneComponent::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Component::loadFromJson(json, context);
    const QJsonObject& object = json.toObject();

    // Load physics scene from json
    if (!m_physicsScene) {
        initialize();
    }
    m_physicsScene->loadFromJson(object.value("physicsScene"));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsSceneComponent::initialize()
{
    // Create physics scene
    m_physicsScene = PhysicsScene::create(scene());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing