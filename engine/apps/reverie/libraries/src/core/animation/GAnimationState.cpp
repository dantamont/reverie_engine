#include "core/animation/GAnimationState.h"
#include "core/animation/GAnimStateMachine.h"
#include "core/animation/GAnimTransition.h"

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

namespace rev {


// Base Animation State

BaseAnimationState::BaseAnimationState(AnimationStateMachine* stateMachine):
    m_stateMachine(stateMachine)
{
}

BaseAnimationState::BaseAnimationState(const GString & name, AnimationStateMachine* stateMachine):
    NameableInterface(name),
    m_stateMachine(stateMachine)
{
}

BaseAnimationState::~BaseAnimationState()
{
}

bool BaseAnimationState::connectsFrom(const BaseAnimationState * state, AnimationStateMachine* sm, int * outConnectionIndex) const
{
    // Iterate over connections to find connected state
    const std::vector<StateConnection>& connections = sm->connections();
    std::vector<BaseAnimationState*> connectedStates;
    for (int connectionIndex : m_connections) {
        const StateConnection& connection = connections[connectionIndex];
        //if (connection.end()->stateType() == AnimationStateType::kTransition) {
        //    // If connection is a transition, see if ends with this state, and starts from given
        //    const AnimationTransition* transition = static_cast<const AnimationTransition*>(connection.end());
        //    if (transition->start()->getUuid() == state->getUuid() &&
        //        transition->end()->getUuid() == m_uuid) {
        //        *outConnectionIndex = connectionIndex;
        //        return true;
        //    }
        //}
        //else {
            if (connection.end(sm)->getUuid() != m_uuid) {
                // Skip if this state isn't the end of connection
                continue;
            }

            const BaseAnimationState* previous = connection.start(sm);
            if (previous->getUuid() == state->getUuid()) {
                *outConnectionIndex = connectionIndex;
                return true;
            }
        //}
    }
    return false;
}

bool BaseAnimationState::connectsTo(const BaseAnimationState * state, AnimationStateMachine* sm, int * outConnectionIndex) const
{
    // Iterate over connections to find connected state
    const std::vector<StateConnection>& connections = sm->connections();
    std::vector<BaseAnimationState*> connectedStates;
    for (int connectionIndex : m_connections) {
        const StateConnection& connection = connections[connectionIndex];
        //if (connection.end()->stateType() == AnimationStateType::kTransition) {
        //    // If connection is a transition, see if starts with this state, and ends from given
        //    const AnimationTransition* transition = static_cast<const AnimationTransition*>(connection.end());
        //    if (transition->end()->getUuid() == state->getUuid() &&
        //        transition->start()->getUuid() == m_uuid) {
        //        *outConnectionIndex = connectionIndex;
        //        return true;
        //    }
        //}
        //else {
            if (connection.start(sm)->getUuid() != m_uuid) {
                // Skip if this state isn't the start of connection
                continue;
            }

            const BaseAnimationState* next = connection.end(sm);
            if (next->getUuid() == state->getUuid()) {
                *outConnectionIndex = connectionIndex;
                return true;
            }
        //}
    }
    return false;
}

void BaseAnimationState::detachConnections(AnimationStateMachine* sm)
{
    // Clear connections from other states
    for (int connectionIndex : m_connections) {
        StateConnection& connection = sm->getConnection(connectionIndex);
        BaseAnimationState* startState = connection.start(sm);
        BaseAnimationState* endState = connection.end(sm);
        if (startState->getUuid() != getUuid()) {
            // If start state is not this one, remove connection from end state
            startState->removeConnection(&connection);
        }
        else {
            // If end state is not this one, remove connection from state
            endState->removeConnection(&connection);
        }
    }
}

void BaseAnimationState::addConnection(StateConnection * connection)
{
#ifdef DEBUG_MODE
    auto iter = std::find_if(m_connections.begin(), m_connections.end(),
        [connection](int c) {
        return c == connection->machineIndex();
    });
    if (iter != m_connections.end()) {
        Logger::Throw("Error, connection already found");
    }
#endif


    m_connections.push_back(connection->machineIndex());
}

void BaseAnimationState::removeConnection(StateConnection * connection)
{
    auto iter = std::find_if(m_connections.begin(), m_connections.end(),
        [connection](int c) {
        return c == connection->machineIndex();
    });

#ifdef DEBUG_MODE
    if (iter == m_connections.end()) {
        Logger::Throw("Error, connection not found");
    }
#endif

    m_connections.erase(iter);
}

void to_json(json& orJson, const BaseAnimationState& korObject)
{
    // Save type to JSON
    orJson["stateType"] = (int)korObject.stateType();
    orJson["id"] = korObject.getUuid(); /// @todo Remove, used for widgets only

    if (korObject.m_name.isEmpty()) {
        Logger::Throw("Error, every state must have a unique name");
    }

    // Save name to Json
    orJson[JsonKeys::s_name] = korObject.m_name.c_str();
    orJson["playbackMode"] = int(korObject.m_playbackMode);
}

void from_json(const json& korJson, BaseAnimationState& orObject)
{
    // Load name
    if (!korJson.contains("name")) {
        Logger::Throw("Error, every state needs a unique name");
    }
    korJson["name"].get_to(orObject.m_name);

    orObject.m_playbackMode = AnimationPlaybackMode(korJson.value("playbackMode", (int)AnimationPlaybackMode::kLoop));
}




// Animation State

AnimationState::AnimationState(AnimationStateMachine* stateMachine) :
    BaseAnimationState(stateMachine)
{
}

AnimationState::AnimationState(const GString& name, AnimationStateMachine* stateMachine):
    BaseAnimationState(name, stateMachine)
{
}

AnimationState::~AnimationState()
{
    // Delete all animation clips
    //for (const AnimationClip* animationClip : m_clips) {
    //    delete animationClip;
    //}

    //// Delete all animation layers
    //for (AnimationClip* layer : m_layers) {
    //    delete layer;
    //}
    
    // Delete child
    //if (m_child) {
    //    delete m_child;
    //}
}

AnimationClip& AnimationState::getClip(const Uuid& id)
{
    auto iter = std::find_if(m_clips.begin(), m_clips.end(),
        [id](const AnimationClip& clip)
        {
            return clip.getUuid() == id;
        }
    );
#ifdef DEBUG_MODE
    assert(iter != m_clips.end() && "Error, clip not found");
#endif
    return *iter;
}

void AnimationState::onEntry(Motion& motion) const
{

}

void AnimationState::onExit(Motion& motion) const
{
}

void AnimationState::addClip(const std::shared_ptr<ResourceHandle>& animationHandle)
{
    static std::atomic<size_t> clipCount = 0;
    if (animationHandle) {
        Vec::EmplaceBack(m_clips, animationHandle->getName(), animationHandle);
    }
    else {
        Vec::EmplaceBack(m_clips, GString("clip_") + GString::FromNumber((size_t)clipCount), nullptr);
        clipCount++;
    }
}

void AnimationState::removeClip(const AnimationClip & clip)
{
    removeClip(clip.getUuid());
}

void AnimationState::removeClip(const Uuid& clipId)
{
    // TODO: Make sure that values in vector don't cause problems on reallocation
    auto iter = std::find_if(m_clips.begin(), m_clips.end(),
        [clipId](const AnimationClip& c) {
            return c.getUuid() == clipId;
        }
    );

    m_clips.erase(iter);
}

//void AnimationState::addLayer(AnimationClip * state)
//{
//    m_layers.push_back(state);
//}

void to_json(json& orJson, const AnimationState& korObject)
{
    ToJson<BaseAnimationState>(orJson, korObject);

    // Save clips to json
    orJson["clips"] = json::object();
    for (const AnimationClip& clip : korObject.m_clips) {
        orJson["clips"][clip.getName().c_str()] = clip;
    }
}

void from_json(const json& korJson, AnimationState& orObject)
{
    FromJson<BaseAnimationState>(korJson, orObject);

    // Load clips
    const json& clips = korJson["clips"];
    for (const auto& clipPair : clips.items()) {
        const json& clipJson = clipPair.value();
        Vec::EmplaceBack(orObject.m_clips, clipJson);
    }
}



} // End namespaces
