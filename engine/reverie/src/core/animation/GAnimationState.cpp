#include "GAnimationState.h"
#include "GAnimStateMachine.h"
#include "GAnimTransition.h"

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

namespace rev {
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Base Animation State
/////////////////////////////////////////////////////////////////////////////////////////////
BaseAnimationState::BaseAnimationState(AnimationStateMachine* stateMachine):
    m_stateMachine(stateMachine)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
BaseAnimationState::BaseAnimationState(const GString & name, AnimationStateMachine* stateMachine):
    Nameable(name),
    m_stateMachine(stateMachine)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
BaseAnimationState::~BaseAnimationState()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////
void BaseAnimationState::addConnection(StateConnection * connection)
{
#ifdef DEBUG_MODE
    auto iter = std::find_if(m_connections.begin(), m_connections.end(),
        [connection](int c) {
        return c == connection->machineIndex();
    });
    if (iter != m_connections.end()) {
        throw("Error, connection already found");
    }
#endif


    m_connections.push_back(connection->machineIndex());
}
/////////////////////////////////////////////////////////////////////////////////////////////
void BaseAnimationState::removeConnection(StateConnection * connection)
{
    auto iter = std::find_if(m_connections.begin(), m_connections.end(),
        [connection](int c) {
        return c == connection->machineIndex();
    });

#ifdef DEBUG_MODE
    if (iter == m_connections.end()) {
        throw("Error, connection not found");
    }
#endif

    m_connections.erase(iter);
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue BaseAnimationState::asJson(const SerializationContext& context) const
{
    QJsonObject object;

    // Save type to JSON
    object.insert("stateType", (int)stateType());

    if (m_name.isEmpty()) {
        throw("Error, every state must have a unique name");
    }

    // Save name to Json
    object.insert("name", m_name.c_str());

    object["playbackMode"] = int(m_playbackMode);

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void BaseAnimationState::loadFromJson(const QJsonValue & json, const SerializationContext & context)
{
    Q_UNUSED(context);

    QJsonObject object = json.toObject();

    // Load name
    if (!object.contains("name")) {
        throw("Error, every state needs a unique name");
    }
    m_name = object["name"].toString();

    m_playbackMode = AnimationPlaybackMode(json["playbackMode"].toInt(
        (int)AnimationPlaybackMode::kLoop));
}


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Animation State
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationState::AnimationState(AnimationStateMachine* stateMachine) :
    BaseAnimationState(stateMachine)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationState::AnimationState(const GString& name, AnimationStateMachine* stateMachine):
    BaseAnimationState(name, stateMachine)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationState::onEntry(Motion& motion) const
{

}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationState::onExit(Motion& motion) const
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
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
///////////////////////////n////////////////////////////////////////////////////////////////////
void AnimationState::removeClip(const AnimationClip & clip)
{
    // TODO: Make sure that values in vector don't cause problems on reallocation
    auto iter = std::find_if(m_clips.begin(), m_clips.end(),
        [clip](const AnimationClip& c) {
        return c.getUuid() == clip.getUuid();
    }
    );

    m_clips.erase(iter);

    // delete *iter;
}
///////////////////////////////////////////////////////////////////////////////////////////////
//void AnimationState::addLayer(AnimationClip * state)
//{
//    m_layers.push_back(state);
//}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue AnimationState::asJson(const SerializationContext& context) const
{
    QJsonObject object = BaseAnimationState::asJson(context).toObject();

    // Save clips to json
    QJsonObject clips;
    for (const AnimationClip& clip : m_clips) {
        clips.insert(clip.getName().c_str(), clip.asJson());
    }
    object.insert("clips", clips);


    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationState::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    BaseAnimationState::loadFromJson(json, context);
    QJsonObject object = json.toObject();

    // Load clips
    QJsonObject clips = object["clips"].toObject();
    const QStringList& clipKeys = clips.keys();
    for (const QString& clipName : clipKeys) {
        QJsonObject clipJson = clips[clipName].toObject();
        //addClip(clip);
        Vec::EmplaceBack(m_clips, context.m_engine, clipJson);

    }
}


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
