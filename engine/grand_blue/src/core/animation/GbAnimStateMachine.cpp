#include "GbAnimStateMachine.h"

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
// State Connection
/////////////////////////////////////////////////////////////////////////////////////////////
StateConnection::StateConnection():
    m_startState(nullptr),
    m_endState(nullptr)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
StateConnection::StateConnection(BaseAnimationState * start, BaseAnimationState* end):
    m_startState(start),
    m_endState(end)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
StateConnection::~StateConnection()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue StateConnection::asJson() const
{
    QJsonObject object;
    object.insert("start", m_startState->getName().c_str());
    object.insert("end", m_endState->getName().c_str());
    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void StateConnection::loadFromJson(const QJsonValue & json, const SerializationContext & context)
{
    QJsonObject object = json.toObject();
    AnimationStateMachine* sm = reinterpret_cast<AnimationStateMachine*>(context.m_data);
    m_startState = sm->getState(object["start"].toString());
    m_endState = sm->getState(object["end"].toString());

    if (!m_startState || !m_endState) {
        throw("Error, states not found");
    }
}



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Animation State Machine
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationStateMachine::AnimationStateMachine() 
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationStateMachine::~AnimationStateMachine()
{
    for (BaseAnimationState* animState : m_animationStates) {
        delete animState;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationStateMachine::addState(BaseAnimationState * state)
{
    auto iter  = std::find_if(m_animationStates.begin(), m_animationStates.end(),
        [state](BaseAnimationState* s) {
        return s->getUuid() == state->getUuid();
    });
    if (iter != m_animationStates.end()) {
        throw("Error, state already present in state machine");
    }
    m_animationStates.push_back(state);
}
/////////////////////////////////////////////////////////////////////////////////////////////
StateConnection* AnimationStateMachine::addConnection(BaseAnimationState * start, BaseAnimationState * end)
{
    Vec::EmplaceBack(m_connections, start, end);
    StateConnection& connection = m_connections.back();

    connection.setMachineIndex(m_connections.size() - 1);
    start->addConnection(&connection);
    end->addConnection(&connection);

    return &connection;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationStateMachine::removeConnection(StateConnection * connection)
{
    // Remove connection from states
    connection->start()->removeConnection(connection);
    connection->end()->removeConnection(connection);

    // Remove connection from connection vector
    auto iter = m_connections.begin() + connection->machineIndex();
    m_connections.erase(iter);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationStateMachine::removeState(BaseAnimationState * state)
{
    // Remove state from the vector of states for the state machine
    auto stateIter = std::find_if(m_animationStates.begin(), m_animationStates.end(),
        [state](BaseAnimationState* s) {
        return s->getUuid() == state->getUuid();
    });

    if (stateIter == m_animationStates.end()) {
        throw("Error, animation state not found");
    }
    m_animationStates.erase(stateIter);

    // Remove connections to and from other states
    state->detachConnections(this);

    // Remove connections to and from the state from list of connections
    for (int connectionIndex : state->connections()) {
#ifdef DEBUG_MODE
        if (connectionIndex < 0) {
            throw("Error, index was never assigned for connection");
        }
#endif
        auto iter = m_connections.begin() + connectionIndex;
        m_connections.erase(iter);
    }

    // Delete state
    delete state;
}
/////////////////////////////////////////////////////////////////////////////////////////////
BaseAnimationState * AnimationStateMachine::getState(const GString & name)
{
    auto iter = std::find_if(m_animationStates.begin(), m_animationStates.end(),
        [name](BaseAnimationState* state) {
        return state->getName() == name;
    });
    if (iter == m_animationStates.end()) {
        return nullptr;
    }
    else {
        return *iter;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
BaseAnimationState * AnimationStateMachine::getState(const Uuid & uuid)
{
    auto iter = std::find_if(m_animationStates.begin(), m_animationStates.end(),
        [uuid](BaseAnimationState* state) {
        return state->getUuid() == uuid;
    });
    if (iter == m_animationStates.end()) {
        return nullptr;
    }
    else {
        return *iter;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
StateConnection * AnimationStateMachine::getConnection(const Uuid & uuid)
{
    auto iter = std::find_if(m_connections.begin(), m_connections.end(),
        [uuid](const StateConnection& c) {
        return c.getUuid() == uuid;
    });
    if (iter == m_connections.end()) {
        return nullptr;
    }
    else {
        return &(*iter);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
StateConnection & AnimationStateMachine::getConnection(int idx)
{
    return m_connections[idx];
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue AnimationStateMachine::asJson() const
{
    QJsonObject object;

    // Add all states to json
    QJsonArray animationStates;
    for (BaseAnimationState* anim : m_animationStates) {
        animationStates.append(anim->asJson());
    }
    object.insert("animationStates", animationStates);
    object.insert("name", m_name.c_str());

    // Add connections to JSON
    QJsonArray connections;
    for (const StateConnection& connection : m_connections) {
        connections.append(connection.asJson());
    }
    object.insert("connections", connections);

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationStateMachine::loadFromJson(const QJsonValue & json, const SerializationContext & context)
{
    QJsonObject object = json.toObject();

    // Load animation states from JSON
    if (object.contains("animationStates")) {
        QJsonArray animStates;
        if (!object["animationStates"].isArray()) {
            // Deprecated 11/1/2020, only kept for backwards compatibility
            QJsonObject statesObject = object["animationStates"].toObject();
            QStringList keys = statesObject.keys();
            for (const QString& key : keys) {
                QJsonObject stateObject = statesObject[key].toObject();
                stateObject.insert("name", key);
                animStates.append(stateObject);
            }
        }
        else {
            animStates = object["animationStates"].toArray();
        }
        for (const QJsonValue& anim : animStates) {
            AnimationStateType stateType = (AnimationStateType)anim["stateType"].toInt(1);
            BaseAnimationState* state;
            switch (stateType) {
            case AnimationStateType::kNull:
                throw("unimplemented");
                break;
            case AnimationStateType::kAnimation:
                state = new AnimationState(this);
                break;
            case AnimationStateType::kTransition:
                throw("unimplemented");
                break;
            default:
                throw("Error, state type unrecognized");
            }
            state->loadFromJson(anim, context);
            addState(state);
        }
    }

    // Load connections from JSON
    if (object.contains("connections")) {
        QJsonArray connections = object["connections"].toArray();
        for (const auto& connection : connections) {
            int idx = m_connections.size();
            m_connections.emplace_back();
            m_connections.back().loadFromJson(connection, {nullptr, reinterpret_cast<char*>(this)});
            m_connections.back().setMachineIndex(idx);
        }
    }

    if (object.contains("name")) {
        setName(object["name"].toString());
    }
    else {
        setName(GString("sm_") + Uuid(true).asString());
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
