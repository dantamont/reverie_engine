#include "core/animation/GAnimMotion.h"
#include "core/animation/GAnimationState.h"
#include "core/animation/GAnimStateMachine.h"
#include "core/animation/GAnimTransition.h"
#include "core/animation/GAnimationController.h"

#include "core/GCoreEngine.h"
#include "core/resource/GResourceCache.h"
#include "core/resource/GResource.h"

#include "fortress/containers/math/GMatrix.h"
#include "fortress/containers/math/GTransform.h"
#include "fortress/math/GInterpolation.h"
#include "fortress/thread/GParallelLoop.h"
#include "core/processes/GAnimationProcess.h"
#include "core/processes/GProcessManager.h"

#include "core/rendering/geometry/GMesh.h"
#include "core/rendering/geometry/GSkeleton.h"
#include "core/rendering/models/GModel.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/renderer/GRenderCommand.h"
#include <atomic>

namespace rev {



// Motion

Motion::Motion(AnimationController* controller) :
    m_controller(controller),
    m_stateID(false)
{
    static std::atomic<size_t> s_numMotions = 0;
    setName("motion" + GString::FromNumber((size_t)s_numMotions));
    s_numMotions++;

    //restartTimer();
}

Motion::~Motion()
{
}

BaseAnimationState * Motion::currentState() const
{
    return m_controller->stateMachine()->getState(m_stateID);
}

void Motion::setState(const GString & name)
{
    if (BaseAnimationState* state = m_controller->stateMachine()->getState(name)) {
        m_stateID = state->getUuid();
    }
    else {
        Logger::LogWarning("Warning, state with name " + name + " not found");
    }
}

void Motion::setState(const BaseAnimationState* state)
{
    m_stateID = state->getUuid();
}

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
        Logger::Throw("Error, unsupported type");
        break;
    }

    return isDone;
}

void Motion::pause()
{
    m_motionStatusFlags.setFlag(AnimMotionStatusFlag::kPlaying, false);
    m_timer.stop();
}

void Motion::play()
{
    m_motionStatusFlags.setFlag(AnimMotionStatusFlag::kPlaying, true);
    m_timer.start();
}

void Motion::move(const BaseAnimationState * state)
{
    BaseAnimationState* current = currentState();
    const BaseAnimationState* nextState;

    AnimationStateMachine* sm = m_controller->stateMachine();

    //Logger::LogDebug("Move called to state " + state->getName());

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
            //Logger::LogDebug("Moving from animation state" + current->getName());

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
                Logger::Throw("Error, state" + currentName + " does not connect to state" + nextName);
#else
                Logger::LogWarning("Error, state" + current->getName() + " does not connect to state" + state->getName());
#endif
                nextState = state;
            }
        }
            break;
        case AnimationStateType::kTransition:
        {
            //Logger::LogDebug("Moving from transition" + current->getName());

            // If currently transitioning, can either continue transition or reverse
            AnimationTransition& ts = current->as<AnimationTransition>();
            if (state->getUuid() == ts.start()->getUuid()) {
                // Reverse the transition
                // Check that there is actually a connection, and use transition instead 
                // of reverting back directly to state
                //Logger::LogDebug("Reversing from transition" + current->getName());

                int connectionIndex;
                bool connects = ts.end()->connectsTo(ts.start(), sm, &connectionIndex);
                if (connects) {
                    //Logger::LogDebug("Connects to state " + state->getName());

                    // If there is a connection back to the original state
                    StateConnection& connection = sm->getConnection(connectionIndex);
                    if (connection.hasTransition()) {
                        nextState = connection.transition(sm);

                        //Logger::LogDebug("Connection has transition, moving to" + nextState->getName());
                    }
                    else {
                        //Logger::LogDebug("Connection has no transition, moving to" + state->getName());

                        nextState = state;
                    }
                }
                else {
                    // Don't do anything, there is no connection back to the original state
                    nextState = nullptr;
                }
            }
            else {
                //Logger::LogDebug("Transition finished, moving from" + current->getName() + " to " + state->getName());

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

        //Logger::LogDebug("Moving animation to " + nextState->getName());
        //Logger::LogDebug("--------------------------------------------------------");
    }
    else {
        Logger::LogDebug("================ No next state ===============");
    }
}

void Motion::autoMove()
{
    BaseAnimationState* current = currentState();
    if (!current) {
        // Cannot move from a non-state
        return;
    }

    // FIXME: Doesn't work if current is a transition.
    AnimationStateMachine* sm = m_controller->stateMachine();
    for (int connectionIndex : current->connections()) {
        const StateConnection& connection = m_controller->stateMachine()->getConnection(connectionIndex);
        switch (current->stateType()) {
        case AnimationStateType::kAnimation:
            // If currently on a state, just need to find the other side of the connection
            if (connection.start(sm)->getUuid() == current->getUuid()) {
                // If this is the start of the connection, then move to the end state, and break
                move(connection.end(sm));
                break;
            }
            break;
        case AnimationStateType::kTransition:
            // If currently on a transition, also need to find the other side of the connection, but
            // there is no check required
            move(connection.end(sm));
            break;
        default:
            Logger::Throw("Unrecognized state type");
            break;
        }
    }
}

float Motion::elapsedTime() const
{
    return m_timer.getElapsed<float>();
}

void Motion::restartTimer()
{
    //m_previousTime = Timer::ToSeconds<float>(m_timer.restart());
    m_timer.restart();
}

void to_json(json& orJson, const Motion& korObject)
{
    orJson["name"] = korObject.m_name.c_str();
    orJson["id"] = korObject.m_uuid; ///< @todo Remove, used only for widgets
    orJson["stateName"] = korObject.currentState()->getName().c_str();
    orJson["behaviorFlags"] = (int)korObject.m_motionBehaviorFlags;
}

void from_json(const json& korJson, Motion& orObject)
{
    if (korJson.contains("name")) {
        orObject.setName(korJson["name"].get_ref<const std::string&>().c_str());
    }
    if (korJson.contains("behaviorFlags")) {
        orObject.m_motionBehaviorFlags = MotionBehaviorFlags(korJson["behaviorFlags"].get<Int32_t>());
    }
    GString stateName = korJson["stateName"].get_ref<const std::string&>().c_str();
    orObject.setState(stateName);
    orObject.restartTimer();
}



// MotionAction
void MotionAction::perform(AnimationController& controller)
{
    // TODO:
    //Logger::Throw("unimplemented");

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




} // End namespaces
