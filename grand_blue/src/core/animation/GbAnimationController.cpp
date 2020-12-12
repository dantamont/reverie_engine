#include "GbAnimationController.h"

#include "../GbCoreEngine.h"
#include "../events/GbEventManager.h"
#include "../resource/GbResourceCache.h"
#include "../resource/GbResource.h"

#include "../geometry/GbMatrix.h"
#include "../geometry/GbTransform.h"
#include "../utils/GbInterpolation.h"
#include "../utils/GbParallelization.h"

#include "../rendering/geometry/GbMesh.h"
#include "../rendering/geometry/GbSkeleton.h"
#include "../rendering/models/GbModel.h"
#include "../rendering/shaders/GbShaders.h"
#include "../rendering/renderer/GbRenderCommand.h"

#include "../animation/GbAnimTransition.h"
#include "../animation/GbAnimationState.h"
#include "GbAnimationManager.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// AnimationController
/////////////////////////////////////////////////////////////////////////////////////////////

AnimationController::AnimationController(CoreEngine * engine, const QJsonValue & json):
    m_engine(engine),
    m_blendQueue(this),
    m_stateMachine(nullptr)
{
    loadFromJson(json);
}
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationController::AnimationController(CoreEngine * engine, const std::shared_ptr<ResourceHandle>& model):
    m_engine(engine),
    m_modelHandle(model),
    m_blendQueue(this),
    m_stateMachine(nullptr)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationController::~AnimationController()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationController::updateMotions()
{
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
    m_motionActionQueue.swap(m_motionActions);
    m_motionActionQueue.clear();
    for (MotionAction& action: m_motionActions) {
        action.perform();
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
Motion * AnimationController::addMotion(BaseAnimationState * state)
{
    Vec::EmplaceBack(m_motions, m_stateMachine);
    Motion& motion = m_motions.back();
    motion.move(state);
    emit m_engine->eventManager()->addedMotion(motion.getUuid());
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
    emit m_engine->eventManager()->removedMotion(motion->getUuid());
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Model> AnimationController::getModel() const
{
    if (!m_modelHandle) {
        throw("Error, no model handle found");
    }
    if (m_modelHandle->resource(false)) {
        return m_modelHandle->resourceAs<Model>(false);
    }
    else {
        return nullptr;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationController::addState(AnimationState * state)
{
    m_stateMachine->addState(state);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationController::addTransition(AnimationTransition * state)
{
    m_stateMachine->addState(state);
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool AnimationController::bindUniforms(DrawCommand& drawCommand)
{
    std::shared_ptr<Model> model = getModel();
    if (!model) return false;

    // Set uniform that this model is animated
    drawCommand.setUniform(Uniform("isAnimated", true));

    // Set global inverse transform
    drawCommand.setUniform(Uniform("globalInverseTransform", model->skeleton()->globalInverseTransform()));

    // Set inverse bind poses
    drawCommand.setUniform(Uniform("inverseBindPoseTransforms", model->skeleton()->inverseBindPose()));
    
    return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool AnimationController::bindUniforms(ShaderProgram& shaderProgram)
{
    std::shared_ptr<Model> model = getModel();
    if (!model) return false;

    // Set uniform that this model is animated
    shaderProgram.setUniformValue("isAnimated", true);

    // Set global inverse transform
    shaderProgram.setUniformValue("globalInverseTransform", model->skeleton()->globalInverseTransform());

    // Set inverse bind poses
    shaderProgram.setUniformValue("inverseBindPoseTransforms", model->skeleton()->inverseBindPose());

    return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue AnimationController::asJson() const
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
        m_modelHandle = m_engine->resourceCache()->getHandleWithName(modelName, Resource::kModel);
    }

    // Load animation states from JSON
    if (!object.contains("animationStates")) {
        if (!object.contains("stateMachine")) {
            throw("Error, no state machine JSON Found");
        }

        if (object["stateMachine"].isObject()) {
            // Deprecated, state machine now stored in animation manager
            m_stateMachine = m_engine->animationManager()->addStateMachine();
            m_stateMachine->loadFromJson(object["stateMachine"], context);
        }
        else {
            m_stateMachine = m_engine->animationManager()->getStateMachine(object["stateMachine"].toString());
        }
    }
    else {
        // Deprecated, load animation states into state machine
        m_stateMachine = m_engine->animationManager()->addStateMachine();
        m_stateMachine->loadFromJson(object, context);
    }

    // Load motions
    if (!object.contains("currentState")) {
        QJsonArray motions = object["motions"].toArray();
        for (const QJsonValue& motionJson : motions) {
            m_motions.emplace_back(m_stateMachine);
            m_motions.back().loadFromJson(motionJson);
        }
    }
    else{
        // Deprecated, convert current state to a motion
        GString stateName(object["currentState"].toString());
        BaseAnimationState* state = m_stateMachine->getState(stateName);
        m_motions.push_back(Motion(m_stateMachine));
        m_motions.back().move(state);
    }

    m_isPlaying = object["isPlaying"].toBool();
}




/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
