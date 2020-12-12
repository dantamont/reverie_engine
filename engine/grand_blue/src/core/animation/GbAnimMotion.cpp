#include "GbAnimMotion.h"
#include "GbAnimationState.h"
#include "GbAnimStateMachine.h"
#include "GbAnimTransition.h"

#include "../GbCoreEngine.h"
#include "../resource/GbResourceCache.h"
#include "../resource/GbResource.h"

#include "../geometry/GbMatrix.h"
#include "../geometry/GbTransform.h"
#include "../utils/GbInterpolation.h"
#include "../utils/GbParallelization.h"
#include "../processes/GbAnimationProcess.h"
#include "../processes/GbProcessManager.h"

#include "../rendering/geometry/GbMesh.h"
#include "../rendering/geometry/GbSkeleton.h"
#include "../rendering/models/GbModel.h"
#include "../rendering/shaders/GbShaders.h"
#include "../rendering/renderer/GbRenderCommand.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Motion
/////////////////////////////////////////////////////////////////////////////////////////////
Motion::Motion(AnimationStateMachine* stateMachine) :
    m_stateMachine(stateMachine)//,
    //m_previousTime(-1)
{
    static size_t s_numMotions = 0;
    setName("motion" + GString::FromNumber(s_numMotions));
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
    return m_stateMachine->getState(m_stateID);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Motion::setState(const GString & name)
{
    m_stateID = m_stateMachine->getState(name)->getUuid();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Motion::setState(const BaseAnimationState* state)
{
    m_stateID = state->getUuid();
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
            std::shared_ptr<Animation> anim = clip.animationHandle()->resourceAs<Animation>(false);
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
        AnimationTransition* transition = static_cast<AnimationTransition*>(current);
        isDone = transition->isDone();
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
    restartTimer();
    if (current) {
        // Perform callback for exiting state
        current->onExit(*this);

        // Obtain connection that leads to state (either through transition or directly)
#ifdef DEBUG_MODE
        int outConnectionIndex;
        if (!current->connectsTo(state, m_stateMachine, &outConnectionIndex)) {
            throw("Error, does not connect to state");
        }
#endif
        //const BaseAnimationState* nextState = m_stateMachine->connections()[outConnectionIndex].end();
        //setState(nextState);
    //}
    //else {
        // Perform callback for entering state

        // If no current state, set without a transition
        //setState(state);
    //}
    }

    // Set the current state
    setState(state);

    // Enter the new state
    state->onEntry(*this);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Motion::autoMove()
{
    BaseAnimationState* current = currentState();
    if (!current) {
        // Cannot move from a non-state
        return;
    }

    for (int connectionIndex : current->connections()) {
        const StateConnection& connection = m_stateMachine->getConnection(connectionIndex);
        if (connection.start()->getUuid() == current->getUuid()) {
            // If this is the start of the connection, then move to the end state, and break
            move(connection.end());
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
//void Motion::stopTimer()
//{
//    m_timer.stop();
//}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Motion::asJson() const
{
    QJsonObject object;
    object.insert("stateName", currentState()->getName().c_str());
    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Motion::loadFromJson(const QJsonValue & json, const SerializationContext & context)
{
    QJsonObject object = json.toObject();
    setState(object["stateName"].toString());
    restartTimer();
    //if (!m_state) {
    //    throw("Error, state not found for the motion");
    //}
}



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// MotionAction
/////////////////////////////////////////////////////////////////////////////////////////////
void MotionAction::perform()
{
    // TODO:
    throw("unimplemented");
}

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
