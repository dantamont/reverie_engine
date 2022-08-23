#include "core/animation/GAnimationController.h"

#include "core/GCoreEngine.h"
#include "core/events/GEventManager.h"
#include "core/resource/GResourceCache.h"
#include "core/resource/GResource.h"

#include "core/scene/GSceneObject.h"
#include "core/scene/GScene.h"

#include "fortress/containers/math/GMatrix.h"
#include "fortress/containers/math/GTransform.h"
#include "fortress/math/GInterpolation.h"
#include "fortress/thread/GParallelLoop.h"

#include "core/rendering/geometry/GMesh.h"
#include "core/rendering/geometry/GSkeleton.h"
#include "core/rendering/models/GModel.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/shaders/GUniformContainer.h"
#include "core/rendering/renderer/GRenderCommand.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"

#include "core/processes/GAnimationProcess.h"
#include "core/processes/GProcessManager.h"

#include "core/animation/GAnimTransition.h"
#include "core/animation/GAnimationState.h"
#include "core/animation/GAnimationManager.h"

#include "core/components/GTransformComponent.h"

namespace rev {



// AnimationController


AnimationController::AnimationController(SceneObject * so, const nlohmann::json& json):
    m_sceneObject(so),
    m_blendQueue(this),
    m_stateMachine(nullptr)
{
    json.get_to(*this);
}

AnimationController::AnimationController(SceneObject * so, const std::shared_ptr<ResourceHandle>& model):
    m_sceneObject(so),
    m_modelHandle(model),
    m_blendQueue(this),
    m_stateMachine(nullptr)
{
}

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

void AnimationController::updateMotions()
{
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

Motion * AnimationController::getMotion(const Uuid & uuid)
{
    auto iter = std::find_if(m_motions.begin(), m_motions.end(),
        [uuid](const Motion& motion) {
        return motion.getUuid() == uuid;
    });
    if (iter == m_motions.end()) {
        Logger::Throw("Error, motion not found");
        return nullptr;
    }
    else {
        return &(*iter);
    }
}

Motion * AnimationController::getMotion(const GString & name)
{
    auto iter = std::find_if(m_motions.begin(), m_motions.end(),
        [name](const Motion& motion) {
        return motion.getName() == name;
    });
    if (iter == m_motions.end()) {
        Logger::Throw("Error, motion not found");
        return nullptr;
    }
    else {
        return &(*iter);
    }
}

Motion * AnimationController::addMotion(BaseAnimationState * state)
{
    Vec::EmplaceBack(m_motions, this);
    Motion& motion = m_motions.back();
    motion.move(state);

    //// @todo  Emit signal that motion was added
    //static GAddedAnimationMotionMessage message;
    //message.setStateMachineId(m_stateMachine->getUuid());
    //m_addedMotionSignal.emitForAll(&message);

    return &motion;
}

void AnimationController::removeMotion(Motion * motion)
{
    auto iter = std::find_if(m_motions.begin(), m_motions.end(),
        [motion](const Motion& m) {
        return m.getUuid() == motion->getUuid();
    });
    if (iter == m_motions.end()) {
        Logger::Throw("Error, motion not found");
    }
    m_motions.erase(iter);
    
    //// @todo Emit signal that motion was removed
    //static GRemovedAnimationMotionMessage message;
    //message.setStateMachineId(m_stateMachine->getUuid());
    //m_removedMotionSignal.emitForAll(&message);
}

Model* AnimationController::getModel() const
{
    if (!m_modelHandle) {
        Logger::Throw("Error, no model handle found");
    }
    return m_modelHandle->resourceAs<Model>();
}

void AnimationController::addState(AnimationState * state)
{
    m_stateMachine->addState(state);
}

void AnimationController::addTransition(AnimationTransition * state)
{
    m_stateMachine->addTransition(state);
}

bool AnimationController::applyUniforms(DrawCommand& drawCommand)
{
    Model* model = getModel();
    if (!model) {
        return false;
    }

    ShaderProgram* shader = drawCommand.shaderProgram();
    ShaderProgram* prepassShader = drawCommand.prepassShaderProgram();

    /// @todo Set these persistent uniforms where the actual values are set in the skeleton
    RenderContext& context = m_sceneObject->scene()->engine()->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();
    if (m_uniforms.m_true.isEmpty()) {
        m_uniforms.m_true.setValue(true, uc);
        m_uniforms.m_globalInverseTransform.setValue(model->skeleton()->globalInverseTransform(), uc);
        m_uniforms.m_inverseBindPose.setValue(model->skeleton()->inverseBindPose(), uc);
    }
    {
        std::shared_lock lock(m_process->mutex());
        m_uniforms.m_boneTransforms.setValue(m_process->transforms(), uc);
    }

    // Set uniform that this model is animated
    drawCommand.setUniform(
        m_uniforms.m_true,
        shader->uniformMappings().m_isAnimated, 
        prepassShader ? prepassShader->uniformMappings().m_isAnimated: -1);

    // Set global inverse transform
    drawCommand.setUniform(
        m_uniforms.m_globalInverseTransform,
        shader->uniformMappings().m_globalInverseTransform,
        prepassShader ? prepassShader->uniformMappings().m_globalInverseTransform : -1
    );

    // Set inverse bind poses
    drawCommand.setUniform(
        m_uniforms.m_inverseBindPose,
        shader->uniformMappings().m_inverseBindPoseTransforms,
        prepassShader ? prepassShader->uniformMappings().m_inverseBindPoseTransforms : -1
    );
    
    // Bind remaining uniforms if controller is all set up (actual pose)
    drawCommand.setUniform(
        m_uniforms.m_boneTransforms,
        shader->uniformMappings().m_boneTransforms,
        prepassShader ? prepassShader->uniformMappings().m_boneTransforms : -1
    );

    return true;
}

void to_json(json& orJson, const AnimationController& korObject)
{
      // Add the mesh used by this controller to JSON
    if (korObject.m_modelHandle) {
        orJson["model"] = korObject.m_modelHandle->getName().c_str();
    }

    // Add motions to JSON
    json motions = json::array();
    for (const Motion& motion : korObject.m_motions) {
        motions.push_back(motion);
    }
    orJson["motions"] = motions;
    
    // State machine
    orJson["stateMachine"] = korObject.m_stateMachine->getName().c_str();
    orJson["stateMachineId"] = korObject.m_stateMachine->getUuid(); /// @todo Remove, used only for widgets

    orJson["isPlaying"] = korObject.m_isPlaying;
}

void from_json(const json& korJson, AnimationController& orObject)
{
    // Set mesh from resource cache
    if (korJson.contains("model")) {
        GString modelName;
        korJson["model"].get_to(modelName);
        orObject.m_modelHandle = ResourceCache::Instance().getHandleWithName(modelName, EResourceType::eModel);
    }

    // Load animation states from JSON
    if (!korJson.contains("animationStates")) {
        if (!korJson.contains("stateMachine")) {
            Logger::Throw("Error, no state machine JSON Found");
        }

        if (korJson["stateMachine"].is_object()) {
            // TODO: Deprecated, state machine now stored in animation manager
            orObject.m_stateMachine = orObject.m_sceneObject->scene()->engine()->animationManager()->addStateMachine();
            korJson["stateMachine"].get_to(*orObject.m_stateMachine);
        }
        else {
            orObject.m_stateMachine = orObject.m_sceneObject->scene()->engine()->animationManager()->getStateMachine(
                korJson["stateMachine"].get_ref<const std::string&>().c_str());
        }
    }
    else {
        // TODO: Deprecated, load animation states into state machine
        orObject.m_stateMachine = orObject.m_sceneObject->scene()->engine()->animationManager()->addStateMachine();
        korJson.get_to(*orObject.m_stateMachine);
    }

    // Load motions
    if (!korJson.contains("currentState")) {
        const json& motions = korJson["motions"];
        for (const json& motionJson : motions) {
            orObject.m_motions.emplace_back(&orObject);
            motionJson.get_to(orObject.m_motions.back());
        }
    }
    else{
        // Deprecated, convert current state to a motion
        GString stateName;
        korJson["currentState"].get_to(stateName);
        BaseAnimationState* state = orObject.m_stateMachine->getState(stateName);
        orObject.m_motions.push_back(Motion(&orObject));
        orObject.m_motions.back().move(state);
    }

    korJson["isPlaying"].get_to(orObject.m_isPlaying);
}


void AnimationController::initializeProcess(const SceneObject& so)
{
    m_process = std::make_shared<AnimationProcess>(m_sceneObject->scene()->engine(),
        this,
        &so.transform());

    // Add process for this animation to the process manager queue
    m_sceneObject->scene()->engine()->processManager()->animationThread().attachProcess(m_process);
}




} // End namespaces
