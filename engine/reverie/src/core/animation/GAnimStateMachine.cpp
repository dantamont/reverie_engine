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
// State Connection
/////////////////////////////////////////////////////////////////////////////////////////////
StateConnection::StateConnection()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
StateConnection::StateConnection(BaseAnimationState * start, BaseAnimationState* end):
    m_startState(start->machineIndex()),
    m_endState(end->machineIndex())
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
StateConnection::~StateConnection()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
const AnimationTransition * StateConnection::transition(AnimationStateMachine * sm) const
{
    return sm->getTransition(m_transitionIndex);
}
/////////////////////////////////////////////////////////////////////////////////////////////
int StateConnection::transitionIndex() const
{
    return m_transitionIndex;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool StateConnection::hasTransition() const
{
    return m_transitionIndex > -1;
}
/////////////////////////////////////////////////////////////////////////////////////////////
BaseAnimationState * StateConnection::start(AnimationStateMachine * sm)
{
    return sm->getState(m_startState);
}
/////////////////////////////////////////////////////////////////////////////////////////////
const BaseAnimationState * StateConnection::start(AnimationStateMachine * sm) const
{
    return sm->getState(m_startState);
}
/////////////////////////////////////////////////////////////////////////////////////////////
BaseAnimationState * StateConnection::end(AnimationStateMachine * sm)
{
    return sm->getState(m_endState);
}
/////////////////////////////////////////////////////////////////////////////////////////////
const BaseAnimationState * StateConnection::end(AnimationStateMachine * sm) const
{
    return sm->getState(m_endState);
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue StateConnection::asJson(const SerializationContext& context) const
{
    QJsonObject object;
    AnimationStateMachine* sm = reinterpret_cast<AnimationStateMachine*>(context.m_data);
    object.insert("start", start(sm)->getName().c_str());
    object.insert("end", end(sm)->getName().c_str());
    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void StateConnection::loadFromJson(const QJsonValue & json, const SerializationContext & context)
{
    QJsonObject object = json.toObject();
    AnimationStateMachine* sm = reinterpret_cast<AnimationStateMachine*>(context.m_data);
    m_startState = sm->getState(object["start"].toString())->machineIndex();
    m_endState = sm->getState(object["end"].toString())->machineIndex();

    if (m_startState < 0 || m_endState < 0) {
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

    for (AnimationTransition* transition : m_transitions) {
        delete transition;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationStateMachine::addState(BaseAnimationState * state)
{
    switch (state->stateType()) {
    case AnimationStateType::kAnimation:
    {
#ifdef DEBUG_MODE
        auto iter = std::find_if(m_animationStates.begin(), m_animationStates.end(),
            [state](BaseAnimationState* s) {
            return s->getUuid() == state->getUuid();
        });
        if (iter != m_animationStates.end()) {
            throw("Error, state already present in state machine");
        }
#endif
        int stateIndex;
        if (m_erasedStates.size()) {
            stateIndex = m_erasedStates.back();
            m_erasedStates.pop_back();
#ifdef DEBUG_MODE
            if (m_animationStates[stateIndex]) {
                throw("Error, pointer is not null at erased state location");
            }
#endif
            m_animationStates[stateIndex] = state;
        }
        else {
            stateIndex = m_animationStates.size();
            m_animationStates.push_back(state);
        }
        m_animationStates[stateIndex]->setMachineIndex(stateIndex);
    }
        break;
    case AnimationStateType::kTransition:
        addTransition(dynamic_cast<AnimationTransition*>(state));
        break;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationStateMachine::addTransition(AnimationTransition * transition)
{
#ifdef DEBUG_MODE
    auto iter = std::find_if(m_transitions.begin(), m_transitions.end(),
        [transition](AnimationTransition* t) {
        return t->getUuid() == transition->getUuid();
    });
    if (iter != m_transitions.end()) {
        throw("Error, transition already present in state machine");
    }
#endif

    // Add transition to internal vector of transitions
    int transitionIndex;
    if (m_erasedTransitions.size()) {
        transitionIndex = m_erasedTransitions.back();
        m_erasedTransitions.pop_back();
#ifdef DEBUG_MODE
        if (m_transitions[transitionIndex]) {
            throw("Error, pointer is not null at erased transition location");
        }
#endif
        m_transitions[transitionIndex] = transition;
    }
    else {
        transitionIndex = m_transitions.size();
        m_transitions.push_back(transition);
    }

    // Retrieve connection corresponding to transition
    StateConnection& connection = transition->connection();

    // Associate the transition with its corresponding connection
    connection.setTransitionIndex(transitionIndex);
}
/////////////////////////////////////////////////////////////////////////////////////////////
StateConnection* AnimationStateMachine::addConnection(BaseAnimationState * start, BaseAnimationState * end)
{
    StateConnection* connectionPtr;
    if (m_erasedConnections.size()) {
        // If any connections were erased, then use their spot in vector
        size_t idx = m_erasedConnections.back();
        m_erasedConnections.pop_back();
        connectionPtr = &m_connections[idx];
        connectionPtr->setMachineIndex(idx);
    }
    else {
        Vec::EmplaceBack(m_connections, start, end);
        connectionPtr = &m_connections.back();
        connectionPtr->setMachineIndex(m_connections.size() - 1);
    }

    start->addConnection(connectionPtr);
    end->addConnection(connectionPtr);

    return connectionPtr;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationStateMachine::removeConnection(StateConnection * connection)
{
    // TODO: Signal all states when connection removed, so inheritance logic can be implemented
    // TODO: This is for a more generic state machine implementation

    // Remove connection from states
    connection->start(this)->removeConnection(connection);
    connection->end(this)->removeConnection(connection);

    // FIXED, old way would break indexing of connections for other states, just flag as invalid with -1 index
    m_erasedConnections.push_back(connection->machineIndex());
    connection->setMachineIndex(-1);
    // Remove connection from connection vector
    //auto iter = m_connections.begin() + connection->machineIndex();
    //m_connections.erase(iter);

    // Remove transition corresponding to this connection
    if (connection->hasTransition()) {
        removeTransition(*connection);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationStateMachine::removeTransition(StateConnection& connection)
{
    // Remove from connection
    int transitionIndex = connection.transitionIndex();
    connection.setTransitionIndex(-1);

    // Remove from state machine and delete
    m_erasedTransitions.push_back(transitionIndex);
    delete m_transitions[transitionIndex];
    m_transitions[transitionIndex] = nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationStateMachine::removeState(BaseAnimationState * state)
{
    // TODO: Implement an onRemoval routine for states
    switch (state->stateType()) {
    case AnimationStateType::kAnimation:
    {
        // Remove state from the vector of states for the state machine
        m_erasedStates.push_back(state->machineIndex());
        m_animationStates[state->machineIndex()] = nullptr;

        // Remove connections to and from the state from list of connections
        for (int connectionIndex : state->connections()) {
#ifdef DEBUG_MODE
            if (connectionIndex < 0) {
                throw("Error, index was never assigned for connection");
            }
#endif

            // Was erasing from connection vector, but this would break indexing of other connections
            StateConnection& connection = getConnection(connectionIndex);
            removeConnection(&connection);
        }
    }
        break;
    case AnimationStateType::kTransition:
    {
        AnimationTransition& ts = state->as<AnimationTransition>();
        StateConnection& connection = ts.connection();
        removeTransition(connection);
    }
        break;
    default:
        throw("Error, unimplemented");
        break;
    }

    // Delete state
    delete state;
}
/////////////////////////////////////////////////////////////////////////////////////////////
BaseAnimationState * AnimationStateMachine::getState(int idx)
{
    return m_animationStates[idx];
}
/////////////////////////////////////////////////////////////////////////////////////////////
BaseAnimationState * AnimationStateMachine::getState(const GStringView & name)
{
    auto iter = std::find_if(m_animationStates.begin(), m_animationStates.end(),
        [name](BaseAnimationState* state) {
        return GStringView(state->getName()) == name;
    });
    if (iter == m_animationStates.end()) {
        return nullptr;
    }
    else {
        return *iter;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
BaseAnimationState * AnimationStateMachine::getState(const GString & name)
{
    auto iter = std::find_if(m_animationStates.begin(), m_animationStates.end(),
        [name](BaseAnimationState* state) {
        return state->getName() == name;
    });
    if (iter == m_animationStates.end()) {
        // If not found, may be a transition
        return getTransition(name);
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
        // If not found, may be a transition
        return getTransition(uuid);
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
AnimationTransition * AnimationStateMachine::getTransition(int idx)
{
    return m_transitions[idx];
}
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationTransition * AnimationStateMachine::getTransition(const GString & name)
{
    auto iter = std::find_if(m_transitions.begin(), m_transitions.end(),
        [name](AnimationTransition* t) {
        return t->getName() == name;
    });
    if (iter == m_transitions.end()) {
        return nullptr;
    }
    else {
        return *iter;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationTransition * AnimationStateMachine::getTransition(const Uuid & uuid)
{
    auto iter = std::find_if(m_transitions.begin(), m_transitions.end(),
        [uuid](AnimationTransition* t) {
        return t->getUuid() == uuid;
    });
    if (iter == m_transitions.end()) {
        return nullptr;
    }
    else {
        return *iter;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue AnimationStateMachine::asJson(const SerializationContext& context) const
{
    QJsonObject object;

    // Add all non-transitional states to JSON
    QJsonArray animationStates;
    for (BaseAnimationState* anim : m_animationStates) {
        animationStates.append(anim->asJson());
    }
    object.insert("animationStates", animationStates);

    // Add transitions to JSON
    QJsonArray transitions;
    for (AnimationTransition* t : m_transitions) {
        transitions.append(t->asJson());
    }
    object.insert("transitions", transitions);

    // Add name to JSON
    object.insert("name", m_name.c_str());

    // Add connections to JSON
    QJsonArray connections;
    for (const StateConnection& connection : m_connections) {
        if (connection.machineIndex() > -1) {
            // Save connection if it wasn't erased
            AnimationStateMachine* thisNotConst = const_cast<AnimationStateMachine*>(this);
            connections.append(connection.asJson({ nullptr, reinterpret_cast<char*>(thisNotConst) }));
        }
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
                // TODO: Remove
                // Legacy, transitions are now stored in their own vector
                continue;
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
        for (const auto& connectionJson : connections) {
            int idx = m_connections.size();
            m_connections.emplace_back();
            StateConnection& connection = m_connections.back();
            connection.loadFromJson(connectionJson, {nullptr, reinterpret_cast<char*>(this)});
            connection.setMachineIndex(idx);

            // Add connection to state
            BaseAnimationState* startState = connection.start(this);
            startState->addConnection(&connection);
            BaseAnimationState* endState = connection.end(this);
            endState->addConnection(&connection);
        }
    }

    // Load transitions from JSON
    if (object.contains("transitions")) {
        QJsonArray transitions = object["transitions"].toArray();
        for (const QJsonValue& trans : transitions) {
            AnimationTransition* t = new AnimationTransition(this);
            t->loadFromJson(trans, context);
            addTransition(t);
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
