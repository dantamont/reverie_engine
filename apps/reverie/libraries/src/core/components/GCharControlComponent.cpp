
// Includes

#include "core/components/GCharControlComponent.h"

// Standard Includes

// External

// Project
#include "core/components/GTransformComponent.h"
#include "core/scene/GSceneObject.h"
#include "core/scene/GScene.h"

#include "fortress/containers/math/GTransform.h"
#include "core/GCoreEngine.h"
#include "core/physics/GPhysicsActor.h"
#include "core/physics/GPhysicsShape.h"
#include "core/physics/GPhysicsMaterial.h"
#include "core/physics/GPhysicsGeometry.h"
#include "core/physics/GPhysicsManager.h"
#include "core/physics/GPhysicsScene.h"
#include "core/physics/GCharacterController.h"

#include "fortress/json/GJson.h"


// Namespace Definitions

namespace rev {


// CharControlComponent

CharControlComponent::CharControlComponent() :
    Component(ComponentType::kCharacterController)
{
}

CharControlComponent::CharControlComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, ComponentType::kCharacterController)
{
    if (!object->scene()->physics()) {
        object->scene()->addPhysics();
    }

    object->setComponent(this);

    /// Create a basic capsule controller
    auto desc = std::make_shared<CapsuleControllerDescription>();
    desc->m_material = PhysicsManager::DefaultMaterial();
    desc->m_radius = 1.0;
    desc->m_height = 1.0;
    m_controller = cctManager()->CreateController(desc, sceneObject());
}

CharControlComponent::~CharControlComponent()
{
}

void CharControlComponent::move(const Vector3 & disp)
{
    m_controller->move(disp);
}

void CharControlComponent::setGravity(const Vector3& gravity)
{
    m_controller->setGravity(gravity);
}

void CharControlComponent::enable()
{
    Component::enable();
}

void CharControlComponent::disable()
{
    Component::disable();
}

void to_json(json& orJson, const CharControlComponent& korObject)
{
    ToJson<Component>(orJson, korObject);
    if (korObject.m_controller) {
        orJson["controller"] = *korObject.m_controller;
    }
}

void from_json(const json& korJson, CharControlComponent& orObject)
{
    if (orObject.m_controller) {
        // Remove old controller from controller manager
        orObject.cctManager()->removeController(*orObject.m_controller);
    }

    // Load character controller from JSON
    if (korJson.contains("controller")) {
        orObject.m_controller = orObject.cctManager()->CreateController(korJson.at("controller"));
    }

    FromJson<Component>(korJson, orObject);
}

const std::shared_ptr<CCTManager>& CharControlComponent::cctManager() const
{
    return sceneObject()->scene()->physics()->cctManager();
}






} // end namespacing