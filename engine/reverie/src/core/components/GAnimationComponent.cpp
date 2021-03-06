#include "GAnimationComponent.h"

#include "GModelComponent.h"
#include "../GCoreEngine.h"
#include "../resource/GResourceCache.h"

#include "../scene/GScene.h"
#include "../scene/GScenario.h"
#include "../scene/GSceneObject.h"

#include "../rendering/shaders/GShaderProgram.h"
#include "../rendering/shaders/GUniform.h"
#include "../rendering/models/GModel.h"
#include "../rendering/geometry/GSkeleton.h"
#include "../rendering/renderer/GRenderCommand.h"

#include "../processes/GAnimationProcess.h"
#include "../processes/GProcessManager.h"


namespace rev {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BoneAnimationComponent::BoneAnimationComponent() :
    Component(ComponentType::kBoneAnimation),
    m_animationController(sceneObject().get(), nullptr)
{
    // Just for Qt, should never be called
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BoneAnimationComponent::BoneAnimationComponent(const BoneAnimationComponent & other) :
    Component(other.sceneObject(), ComponentType::kBoneAnimation),
    m_animationController(sceneObject().get(), modelHandle())
{
    setSceneObject(sceneObject());
    sceneObject()->addComponent(this);

    // Start animation process
    m_animationController.initializeProcess(*sceneObject());

    // The simplest way to copy animation controller is to just load the json serialization
    m_animationController.loadFromJson(other.m_animationController.asJson(), { sceneObject()->scene()->engine() });
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BoneAnimationComponent::BoneAnimationComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, ComponentType::kBoneAnimation),
    m_animationController(sceneObject().get(), modelHandle())
{
    setSceneObject(sceneObject());
    sceneObject()->addComponent(this);

    // Start animation process
    m_animationController.initializeProcess(*sceneObject());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
rev::BoneAnimationComponent::~BoneAnimationComponent()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void BoneAnimationComponent::initializeAnimationController()
//{
//    // TODO: Just iterate through child of model
//    m_engine->resourceCache()->resources().forEach(
//    [this](const std::pair<Uuid, std::shared_ptr<ResourceHandle>>& resourcePair)
//    {
//        if (resourcePair.second->getResourceType() != ResourceType::kAnimation) {
//            return;
//        }
//
//        if (resourcePair.second->getPath() == modelHandle()->getPath()) {
//            // If an animation corresponding to the current model is found, add to animation controller
//            AnimationState* state = new AnimationState(resourcePair.second->getName());
//            state->addClip(resourcePair.second);
//            m_animationController.addState(state);
//        }
//    }
//    );
//}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BoneAnimationComponent::enable()
{
    Component::enable();
    m_animationController.setPlaying(true);
}
//////////////// ///////////////////////////////////////////////////////////////////////////////////////////////////////
void BoneAnimationComponent::disable()
{
    Component::disable();
    m_animationController.setPlaying(false);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BoneAnimationComponent::updateBounds(const Transform & transform)
{
    // Make sure that there is only one boundiing box
    SceneObject& so = *sceneObject();

    if (so.m_worldBounds.geometry().size() != 1) {
        so.m_worldBounds.geometry().clear();
        so.m_worldBounds.geometry().emplace_back();
    }

    // Set bounding box
    const AABB& skeletonBounds = m_animationController.getModel()->skeleton()->boundingBox();
    skeletonBounds.recalculateBounds(transform.worldMatrix(), so.m_worldBounds.geometry().back());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue BoneAnimationComponent::asJson(const SerializationContext& context) const
{
    QJsonObject object = Component::asJson(context).toObject();
    object.insert("animationController", m_animationController.asJson());

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BoneAnimationComponent::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    Component::loadFromJson(json);
    const QJsonObject& object = json.toObject();

    // Load animations
    m_animationController.loadFromJson(object["animationController"], { sceneObject()->scene()->engine() });
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const std::shared_ptr<ResourceHandle>& BoneAnimationComponent::modelHandle() const
{
    return sceneObject()->hasComponent<ModelComponent>(ComponentType::kModel)->modelHandle();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing