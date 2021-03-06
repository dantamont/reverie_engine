#include "GAnimMotion.h"
#include "GAnimationState.h"
#include "GAnimStateMachine.h"
#include "GAnimTransition.h"
#include "GAnimationController.h"

#include "../GCoreEngine.h"
#include "../resource/GResourceCache.h"
#include "../resource/GResource.h"

#include "../geometry/GMatrix.h"
#include "../geometry/GTransform.h"
#include "../utils/GInterpolation.h"
#include "../threading/GParallelLoop.h"
#include "../processes/GAnimationProcess.h"
#include "../processes/GProcessManager.h"

#include "../rendering/geometry/GMesh.h"
#include "../rendering/geometry/GSkeleton.h"
#include "../rendering/models/GModel.h"
#include "../rendering/shaders/GShaderProgram.h"
#include "../rendering/renderer/GRenderCommand.h"
#include <atomic>

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Motion
/////////////////////////////////////////////////////////////////////////////////////////////
Motion::Motion(AnimationController* controller) :
    m_controller(controller)//,
    //m_previousTime(-1)
{
    static std::atomic<size_t> s_numMotions = 0;
    setName("motion" + GString::FromNumber((size_t)s_numMotions));
    s_numMotions++;

    //restartTimer();
}
/////////////////////////////////////////////////////////////////////////////////////////////
Motion::~Motion()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
BaseAnimationState * Motion::currentState() const
{
    return m_controller->stateMachine()->getState(m_stateID);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Motion::setState(const GString & name)
{
    m_stateID = m_controller->stateMachine()->getState(name)->getUuid();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Motion::setState(const BaseAnimationState* state)
{
    m_stateID = state->getUuid();
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Motion::queueMove(const GStringView & stateName)
{
    // TODO: Only lock when accessing queue, don't need to lock the whole mutex
    BaseAnimationState* state = m_controller->stateMachine()->getState(stateName);
    if (state) {
        std::unique_lock lock(m_controller->m_motionActionMutex);
        m_controller->m_motionActionQueue.emplace_back(MotionAction{ AnimationMotionAction::kMove,
            this,
            state });
        return true;
    }
    else {
        return false;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Motion::isDone() const
{
    BaseAnimationState* current = currentState();
    if (!current) {
        return true;
    }

    bool isDone;
    switch (current->stateType()) {
    case AnimationStateType::kAnimation:
    {
        AnimationState* state = static_cast<AnimationState*>(current);
        isDone = true;
        for (const AnimationClip& clip : state->clips()) {
            // TODO: Try to avoid reloading here
            if (!clip.animationHandle()) {
                isDone = false;
                break;
            }

            // Return if no animation loaded
            Animation* anim = clip.animationHandle()->resourceAs<Animation>();
            if (!anim) {
                isDone = false;
                break;
            }

            // Get the poses from the animation clip
            bool clipDone;
            anim->getAnimationTime(
                m_timer.getElapsed<float>(),
                clip.settings(),
                state->playbackMode(),
                clipDone);
            isDone &= clipDone;
        }
        break;
    }
    case AnimationStateType::kTransition:
    {
        AnimationTransition& transition = current->as<AnimationTransition>();
        isDone = transition.isDone();
        break;
    }
    default:
        throw("Error, unsupported type");
        break;
    }

    return isDone;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Motion::pause()
{
    m_motionStatusFlags = m_motionStatusFlags | ~(size_t)AnimMotionStatusFlag::kPlaying;
    m_timer.stop();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Motion::play()
{
    m_motionStatusFlags = m_motionStatusFlags | (size_t)AnimMotionStatusFlag::kPlaying;
    m_timer.start();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Motion::move(const BaseAnimationState * state)
{
    BaseAnimationState* current = currentState();
    const BaseAnimationState* nextState;

    AnimationStateMachine* sm = m_controller->stateMachine();

    restartTimer();
    if (current) {

        if (current->getUuid() == state->getUuid()) {
            return;
        }

        // Perform callback for exiting state
        current->onExit(*this);

        switch (current->stateType()) {
        case AnimationStateType::kAnimation:
        {
            // Obtain connection that leads to state (either through transition or directly)
            int outConnectionIndex;
            bool hasConnection = current->connectsTo(state, sm, &outConnectionIndex);

            // If the connection has an associated transition, that is the next state
            if (hasConnection) {
                const StateConnection& connection = sm->getConnection(outConnectionIndex);
                if (connection.hasTransition()) {
                    nextState = connection.transition(sm);
                }
                else {
                    nextState = state;
                }
            }
            else {
#ifdef DEBUG_MODE
                const GString& currentName = current->getName();
                const GString& nextName = state->getName();
                throw("Error, state" + currentName + " does not connect to state" + nextName);
#else
                Logger::LogWarning("Error, state" + current->getName() + " does not connect to state" + state->getName());
#endif
                nextState = state;
            }
        }
            break;
        case AnimationStateType::kTransition:
        {
            // If currently transitioning, can either continue transition or reverse
            AnimationTransition& ts = current->as<AnimationTransition>();
            if (state->getUuid() == ts.start()->getUuid()) {
                // Reverse the transition
                // Check that there is actually a connection, and use transition instead 
                // of reverting back directly to state
                int connectionIndex;
                bool connects = ts.end()->connectsTo(ts.start(), sm, &connectionIndex);
                if (connects) {
                    // If there is a connection back to the original state
                    StateConnection& connection = sm->getConnection(connectionIndex);
                    if (connection.hasTransition()) {
                        nextState = connection.transition(sm);
                    }
                    else {
                        nextState = state;
                    }
                }
                else {
                    // Don't do anything, there is no connection back to the original state
                    nextState = nullptr;
                }
            }
            else {
                // Transition is finished, so set next state
                nextState = state;
            }
        }
            break;
        }
    }
    else {
        // Don't need to check for transition if setting initial state
        nextState = state;
    }

    if (nextState) {
        // Set the current state
        setState(nextState);

        // Enter the new state
        nextState->onEntry(*this);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Motion::autoMove()
{
    BaseAnimationState* current = currentState();
    if (!current) {
        // Cannot move from a non-state
        return;
    }

    AnimationStateMachine* sm = m_controller->stateMachine();
    for (int connectionIndex : current->connections()) {
        const StateConnection& connection = m_controller->stateMachine()->getConnection(connectionIndex);
        if (connection.start(sm)->getUuid() == current->getUuid()) {
            // If this is the start of the connection, then move to the end state, and break
            move(connection.end(sm));
            break;
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
float Motion::elapsedTime() const
{
    return m_timer.getElapsed<float>();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Motion::restartTimer()
{
    //m_previousTime = Timer::ToSeconds<float>(m_timer.restart());
    m_timer.restart();
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Motion::asJson(const SerializationContext& context) const
{
    QJsonObject object;
    object.insert("name", m_name.c_str());
    object.insert("stateName", currentState()->getName().c_str());
    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Motion::loadFromJson(const QJsonValue & json, const SerializationContext & context)
{
    QJsonObject object = json.toObject();
    if (object.contains("name")) {
        setName(object["name"].toString());
    }
    setState(object["stateName"].toString());
    restartTimer();
}



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// MotionAction
/////////////////////////////////////////////////////////////////////////////////////////////
void MotionAction::perform(AnimationController& controller)
{
    // TODO:
    //throw("unimplemented");

    switch (m_actionType) {
    case AnimationMotionAction::kDestroy:
        controller.removeMotion(m_motion);
        break;
    case AnimationMotionAction::kMove:
        if (m_nextState) {
            m_motion->move(m_nextState);
        }
        else {
            m_motion->autoMove();
        }
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
