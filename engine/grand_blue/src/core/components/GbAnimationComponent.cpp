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

#include "../animation/GbAnimation.h"

namespace Gb {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BoneAnimationComponent::BoneAnimationComponent() :
    Component(kBoneAnimation)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BoneAnimationComponent::BoneAnimationComponent(const BoneAnimationComponent & other) :
    Component(other.sceneObject(), kBoneAnimation)
{
    setSceneObject(sceneObject());
    sceneObject()->addComponent(this);

    // The simplest way to copy animation controller is to just load the json serialization
    m_animationController = std::make_unique<AnimationController>(m_engine, model()->meshHandle());
    m_animationController->loadFromJson(other.m_animationController->asJson());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BoneAnimationComponent::BoneAnimationComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, kBoneAnimation)
{
    setSceneObject(sceneObject());
    sceneObject()->addComponent(this);

    m_animationController = std::make_unique<AnimationController>(m_engine, model()->meshHandle());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Gb::BoneAnimationComponent::~BoneAnimationComponent()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BoneAnimationComponent::initializeAnimationController()
{
    for (const std::pair<QString, std::shared_ptr<ResourceHandle>>& resourcePair : 
        m_engine->resourceCache()->resources()) {

        if (resourcePair.second->getType() != Resource::kAnimation) continue;

        if (resourcePair.second->getPath() == model()->getFilePath()) {
            // If an animation corresponding to the current model is found, add to animation controller
            AnimationState* state = new AnimationState(resourcePair.first, m_engine);
            state->addClip(resourcePair.second);
            m_animationController->addState(state);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BoneAnimationComponent::bindUniforms(const std::shared_ptr<ShaderProgram>&  shaderProgram)
{
    m_animationController->bindUniforms(shaderProgram);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BoneAnimationComponent::enable()
{
    Component::enable();
    m_animationController->setPlaying(true);
}
//////////////// ///////////////////////////////////////////////////////////////////////////////////////////////////////
void BoneAnimationComponent::disable()
{
    Component::disable();
    m_animationController->setPlaying(false);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue BoneAnimationComponent::asJson() const
{
    QJsonObject object = Component::asJson().toObject();
    if (m_animationController) {
        object.insert("animationController", m_animationController->asJson());
    }

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BoneAnimationComponent::loadFromJson(const QJsonValue & json)
{
    Component::loadFromJson(json);
    const QJsonObject& object = json.toObject();

    // Load animations
    if (object.contains("animationController")) {
        if (object["animationController"].isString()) {
            // Option for initializing an animation controller via a JSON string flag
            QString controllerString = object["animationController"].toString();
            if (controllerString == "initialize") {
                initializeAnimationController();
            }
        }
        else {
            m_animationController->loadFromJson(object["animationController"]);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Model> BoneAnimationComponent::model() const
{
    return sceneObject()->modelComponent()->model();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing