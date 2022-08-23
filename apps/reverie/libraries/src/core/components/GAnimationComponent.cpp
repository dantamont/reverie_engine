#include "core/components/GAnimationComponent.h"

#include "core/components/GModelComponent.h"
#include "core/GCoreEngine.h"
#include "core/resource/GResourceCache.h"

#include "core/scene/GScene.h"
#include "core/scene/GScenario.h"
#include "core/scene/GSceneObject.h"

#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/shaders/GUniform.h"
#include "core/rendering/models/GModel.h"
#include "core/rendering/geometry/GSkeleton.h"
#include "core/rendering/renderer/GRenderCommand.h"

#include "core/processes/GAnimationProcess.h"
#include "core/processes/GProcessManager.h"


namespace rev {

BoneAnimationComponent::BoneAnimationComponent() :
    Component(ComponentType::kBoneAnimation),
    m_animationController(sceneObject().get(), static_cast<const std::shared_ptr<ResourceHandle>& >(nullptr))
{
    // Just for Qt, should never be called
}

BoneAnimationComponent::BoneAnimationComponent(const BoneAnimationComponent & other) :
    Component(other.sceneObject(), ComponentType::kBoneAnimation),
    m_animationController(sceneObject().get(), modelHandle())
{
    setSceneObject(sceneObject());
    sceneObject()->setComponent(this);

    // Start animation process
    m_animationController.initializeProcess(*sceneObject());

    // The simplest way to copy animation controller is to just load the json serialization
    json{ other.m_animationController }.get_to(m_animationController);

    sceneObject()->updateBounds(sceneObject()->transform());
}

BoneAnimationComponent::BoneAnimationComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, ComponentType::kBoneAnimation),
    m_animationController(sceneObject().get(), modelHandle())
{
    setSceneObject(sceneObject());
    sceneObject()->setComponent(this);

    // Start animation process
    m_animationController.initializeProcess(*sceneObject());

    sceneObject()->updateBounds(sceneObject()->transform());
}

rev::BoneAnimationComponent::~BoneAnimationComponent()
{
}

//void BoneAnimationComponent::initializeAnimationController()
//{
//    // TODO: Just iterate through child of model
//    ResourceCache::Instance().resources().forEach(
//    [this](const std::pair<Uuid, std::shared_ptr<ResourceHandle>>& resourcePair)
//    {
//        if (resourcePair.second->getResourceType() != EResourceType::eAnimation) {
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

void BoneAnimationComponent::enable()
{
    Component::enable();
    m_animationController.setPlaying(true);
}
 
void BoneAnimationComponent::disable()
{
    Component::disable();
    m_animationController.setPlaying(false);
}

void BoneAnimationComponent::updateBounds(const IndexedTransform& transform)
{
    // Make sure that there is only one boundiing box
    SceneObject& so = *sceneObject();

    if (so.m_worldBounds.geometry().size() != 1) {
        so.m_worldBounds.geometry().clear();
        so.m_worldBounds.geometry().emplace_back();
    }

    // Set bounding box
    Model* model = m_animationController.getModel();
    if (model) {
        // Model may not yet be loaded
        const AABB& skeletonBounds = model->skeleton()->boundingBox();
        skeletonBounds.recalculateBounds(transform.worldMatrix(), so.m_worldBounds.geometry().back());
    }
}

void to_json(json& orJson, const BoneAnimationComponent& korObject)
{
    ToJson<Component>(orJson, korObject);
    orJson["animationController"] = korObject.m_animationController;
}

void from_json(const json& korJson, BoneAnimationComponent& orObject)
{
    FromJson<Component>(korJson, orObject);

    // Load animations
    korJson.at("animationController").get_to(orObject.m_animationController);
}

const std::shared_ptr<ResourceHandle>& BoneAnimationComponent::modelHandle() const
{
    return sceneObject()->getComponent<ModelComponent>(ComponentType::kModel)->modelHandle();
}


} // end namespacing