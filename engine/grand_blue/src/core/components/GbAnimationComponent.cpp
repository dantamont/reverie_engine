#include "GbAnimationComponent.h"

#include "GbModelComponent.h"
#include "../GbCoreEngine.h"
#include "../resource/GbResourceCache.h"

#include "../scene/GbScene.h"
#include "../scene/GbScenario.h"
#include "../scene/GbSceneObject.h"

#include "../rendering/shaders/GbShaders.h"
#include "../rendering/shaders/GbUniform.h"
#include "../rendering/models/GbModel.h"
#include "../rendering/geometry/GbSkeleton.h"
#include "../rendering/renderer/GbRenderCommand.h"

#include "../processes/GbAnimationProcess.h"
#include "../processes/GbProcessManager.h"

#include "../components/GbTransformComponent.h"

namespace Gb {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BoneAnimationComponent::BoneAnimationComponent() :
    Component(ComponentType::kBoneAnimation),
    m_animationController(m_engine, nullptr)
{
    // Just for Qt, should never be called
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BoneAnimationComponent::BoneAnimationComponent(const BoneAnimationComponent & other) :
    Component(other.sceneObject(), ComponentType::kBoneAnimation),
    m_animationController(m_engine, modelHandle())
{
    setSceneObject(sceneObject());
    sceneObject()->addComponent(this);

    // Start animation process
    initializeProcess();

    // The simplest way to copy animation controller is to just load the json serialization
    m_animationController.loadFromJson(other.m_animationController.asJson(), { m_engine });
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BoneAnimationComponent::BoneAnimationComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, ComponentType::kBoneAnimation),
    m_animationController(m_engine, modelHandle())
{
    setSceneObject(sceneObject());
    sceneObject()->addComponent(this);

    // Start animation process
    initializeProcess();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Gb::BoneAnimationComponent::~BoneAnimationComponent()
{
    // Abort the process for this animation
    std::unique_lock lock(m_process->mutex());
    if (!m_process->isAborted()) {
        m_process->abort();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void BoneAnimationComponent::initializeAnimationController()
//{
//    // TODO: Just iterate through child of model
//    m_engine->resourceCache()->resources().forEach(
//    [this](const std::pair<Uuid, std::shared_ptr<ResourceHandle>>& resourcePair)
//    {
//        if (resourcePair.second->getResourceType() != Resource::kAnimation) {
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
void BoneAnimationComponent::bindUniforms(DrawCommand& drawCommand)
{
    bool bound = m_animationController.bindUniforms(drawCommand);
    if (bound) {
        // Bind remaining uniforms if controller is all set up (actual pose)
        std::shared_lock lock(m_process->mutex());
        drawCommand.setUniform("boneTransforms", m_process->transforms());
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BoneAnimationComponent::bindUniforms(ShaderProgram& shaderProgram)
{
    // TODO: Only used for debug manager right now, deprecate
    bool bound = m_animationController.bindUniforms(shaderProgram);

    if (bound) {
        // Bind remaining uniforms if controller is all set up (actual pose)
        std::shared_lock lock(m_process->mutex());
        shaderProgram.setUniformValue("boneTransforms", m_process->transforms(), false);
    }
}
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
    const AABB& skeletonBounds = m_animationController.getModel()->skeleton()->boundingBox();
    skeletonBounds.recalculateBounds(transform, m_transformedBoundingBox);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue BoneAnimationComponent::asJson() const
{
    QJsonObject object = Component::asJson().toObject();
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
    m_animationController.loadFromJson(object["animationController"], { m_engine });
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const std::shared_ptr<ResourceHandle>& BoneAnimationComponent::modelHandle() const
{
    return sceneObject()->modelComponent()->modelHandle();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void BoneAnimationComponent::initializeProcess()
{
    m_process = std::make_shared<AnimationProcess>(m_engine,
        &m_animationController,
        sceneObject()->transform().get());

    // Add process for this animation to the process manager queue
    m_engine->processManager()->animationThread().attachProcess(m_process);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing