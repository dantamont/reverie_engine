#include "GAnimationController.h"

#include "../GCoreEngine.h"
#include "../events/GEventManager.h"
#include "../resource/GResourceCache.h"
#include "../resource/GResource.h"

#include "../scene/GSceneObject.h"
#include "../scene/GScene.h"

#include "../geometry/GMatrix.h"
#include "../geometry/GTransform.h"
#include "../utils/GInterpolation.h"
#include "../threading/GParallelLoop.h"

#include "../rendering/geometry/GMesh.h"
#include "../rendering/geometry/GSkeleton.h"
#include "../rendering/models/GModel.h"
#include "../rendering/shaders/GShaderProgram.h"
#include "../rendering/renderer/GRenderCommand.h"

#include "../processes/GAnimationProcess.h"
#include "../processes/GProcessManager.h"

#include "../animation/GAnimTransition.h"
#include "../animation/GAnimationState.h"
#include "GAnimationManager.h"

#include "../components/GTransformComponent.h"


namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// AnimationController
/////////////////////////////////////////////////////////////////////////////////////////////

AnimationController::AnimationController(SceneObject * so, const QJsonValue & json):
    m_sceneObject(so),
    m_blendQueue(this),
    m_stateMachine(nullptr)
{
    loadFromJson(json);
}
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationController::AnimationController(SceneObject * so, const std::shared_ptr<ResourceHandle>& model):
    m_sceneObject(so),
    m_modelHandle(model),
    m_blendQueue(this),
    m_stateMachine(nullptr)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationController::~AnimationController()
{
    // Abort the process for this animation
    if (m_process) {
        std::unique_lock lock(m_process->mutex());
        if (!m_process->isAborted()) {
            m_process->abort();
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationController::updateMotions()
{
    // TODO: Think up a mechanism for transitioning when interrupting a transition that accounts for
    // The current pose of the interrupted transition. I was thinking of adding a "reverse" flag
    // to back out transitions, but maybe I should have something more generic.
    // Need to think about it

    // TODO: Maybe drive everything from the queue, so when motion is done, will queue a move
    // This would hopefully prevent undefined behavior if a transition is interrupted, since 
    // The most recent commands would always supercede previous ones
    // However, this might be fine as it is, time will tell

    // Check which motions are autoplaying or done with a transition, and perform actions
    for (Motion& motion : m_motions) {
        if (motion.isDone()) {
            // If the motion is done playing it's current state
            if (motion.isAutoPlaying()) {
                // If motion is flagged to autoplay, switch state if done
                motion.autoMove();
            }
            else if (motion.currentState()->stateType() == AnimationStateType::kTransition) {
                // If the motion's current state is a transition, automatically move to next state
                motion.autoMove();
            }
        }
    }

    // Perform all user-generated motion actions, swapping queues.
    m_motionActionMutex.lock();
    m_motionActionQueue.swap(m_motionActions);
    m_motionActionQueue.clear();
    m_motionActionMutex.unlock();
    for (MotionAction& action: m_motionActions) {
        action.perform(*this);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
Motion * AnimationController::getMotion(const Uuid & uuid)
{
    auto iter = std::find_if(m_motions.begin(), m_motions.end(),
        [uuid](const Motion& motion) {
        return motion.getUuid() == uuid;
    });
    if (iter == m_motions.end()) {
        throw("Error, motion not found");
        return nullptr;
    }
    else {
        return &(*iter);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
Motion * AnimationController::getMotion(const GString & name)
{
    auto iter = std::find_if(m_motions.begin(), m_motions.end(),
        [name](const Motion& motion) {
        return motion.getName() == name;
    });
    if (iter == m_motions.end()) {
        throw("Error, motion not found");
        return nullptr;
    }
    else {
        return &(*iter);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
Motion * AnimationController::addMotion(BaseAnimationState * state)
{
    Vec::EmplaceBack(m_motions, this);
    Motion& motion = m_motions.back();
    motion.move(state);
    emit m_sceneObject->scene()->engine()->animationManager()->addedMotion(motion.getUuid());
    return &motion;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationController::removeMotion(Motion * motion)
{
    auto iter = std::find_if(m_motions.begin(), m_motions.end(),
        [motion](const Motion& m) {
        return m.getUuid() == motion->getUuid();
    });
    if (iter == m_motions.end()) {
        throw("Error, motion not found");
    }
    m_motions.erase(iter);
    emit m_sceneObject->scene()->engine()->animationManager()->removedMotion(motion->getUuid());
}
/////////////////////////////////////////////////////////////////////////////////////////////
Model* AnimationController::getModel() const
{
    if (!m_modelHandle) {
        throw("Error, no model handle found");
    }
    return m_modelHandle->resourceAs<Model>();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationController::addState(AnimationState * state)
{
    m_stateMachine->addState(state);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationController::addTransition(AnimationTransition * state)
{
    m_stateMachine->addTransition(state);
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool AnimationController::bindUniforms(DrawCommand& drawCommand)
{
    Model* model = getModel();
    if (!model) return false;

    // Set uniform that this model is animated
    drawCommand.addUniform("isAnimated", true);

    // Set global inverse transform
    drawCommand.addUniform("globalInverseTransform", model->skeleton()->globalInverseTransform());

    // Set inverse bind poses
    drawCommand.addUniform("inverseBindPoseTransforms", model->skeleton()->inverseBindPose());
    
    // Bind remaining uniforms if controller is all set up (actual pose)
    std::shared_lock lock(m_process->mutex());
    drawCommand.addUniform("boneTransforms", m_process->transforms());

    return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool AnimationController::bindUniforms(ShaderProgram& shaderProgram)
{
    Model* model = getModel();
    if (!model) return false;

    // Set uniform that this model is animated
    shaderProgram.setUniformValue("isAnimated", true);

    // Set global inverse transform
    shaderProgram.setUniformValue("globalInverseTransform", model->skeleton()->globalInverseTransform());

    // Set inverse bind poses
    shaderProgram.setUniformValue("inverseBindPoseTransforms", model->skeleton()->inverseBindPose());

    // Bind remaining uniforms if controller is all set up (actual pose)
    std::shared_lock lock(m_process->mutex());
    shaderProgram.setUniformValue("boneTransforms", m_process->transforms(), false);

    return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue AnimationController::asJson(const SerializationContext& context) const
{
    QJsonObject object;

    // Add the mesh used by this controller to JSON
    if (m_modelHandle) {
        object.insert("model", m_modelHandle->getName().c_str());
    }

    // Add motions to JSON
    QJsonArray motions;
    for (const Motion& motion : m_motions) {
        motions.append(motion.asJson());
    }
    object.insert("motions", motions);
    
    // State machine
    object.insert("stateMachine", m_stateMachine->getName().c_str());

    object.insert("isPlaying", m_isPlaying);

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationController::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    QJsonObject object = json.toObject();

    // Set mesh from resource cache
    if (object.contains("model")) {
        QString modelName = object["model"].toString();
        m_modelHandle = m_sceneObject->scene()->engine()->resourceCache()->getHandleWithName(modelName, ResourceType::kModel);
    }

    // Load animation states from JSON
    if (!object.contains("animationStates")) {
        if (!object.contains("stateMachine")) {
            throw("Error, no state machine JSON Found");
        }

        if (object["stateMachine"].isObject()) {
            // TODO: Deprecated, state machine now stored in animation manager
            m_stateMachine = m_sceneObject->scene()->engine()->animationManager()->addStateMachine();
            m_stateMachine->loadFromJson(object["stateMachine"], context);
        }
        else {
            m_stateMachine = m_sceneObject->scene()->engine()->animationManager()->getStateMachine(object["stateMachine"].toString());
        }
    }
    else {
        // TODO: Deprecated, load animation states into state machine
        m_stateMachine = m_sceneObject->scene()->engine()->animationManager()->addStateMachine();
        m_stateMachine->loadFromJson(object, context);
    }

    // Load motions
    if (!object.contains("currentState")) {
        QJsonArray motions = object["motions"].toArray();
        for (const QJsonValue& motionJson : motions) {
            m_motions.emplace_back(this);
            m_motions.back().loadFromJson(motionJson);
        }
    }
    else{
        // Deprecated, convert current state to a motion
        GString stateName(object["currentState"].toString());
        BaseAnimationState* state = m_stateMachine->getState(stateName);
        m_motions.push_back(Motion(this));
        m_motions.back().move(state);
    }

    m_isPlaying = object["isPlaying"].toBool();
}

/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationController::initializeProcess(const SceneObject& so)
{
    m_process = std::make_shared<AnimationProcess>(m_sceneObject->scene()->engine(),
        this,
        &so.transform());

    // Add process for this animation to the process manager queue
    m_sceneObject->scene()->engine()->processManager()->animationThread().attachProcess(m_process);
}



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
