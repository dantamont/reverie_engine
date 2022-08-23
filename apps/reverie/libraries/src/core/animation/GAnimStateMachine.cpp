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



// State Connection

StateConnection::StateConnection()
{
}

StateConnection::StateConnection(BaseAnimationState * start, BaseAnimationState* end):
    m_startState(start->machineIndex()),
    m_endState(end->machineIndex())
{
}

StateConnection::~StateConnection()
{
}

const AnimationTransition * StateConnection::transition(AnimationStateMachine * sm) const
{
    return sm->getTransition(m_transitionIndex);
}

int StateConnection::transitionIndex() const
{
    return m_transitionIndex;
}

bool StateConnection::hasTransition() const
{
    return m_transitionIndex > -1;
}

void StateConnection::setTransitionIndex(int transitionIndex)
{
    if (m_transitionIndex > -1 && transitionIndex != -1) {
        Logger::Throw("Error, already has transition");
    }
    m_transitionIndex = transitionIndex;
}

void to_json(nlohmann::json& orJson, const StateConnection& korObject)
{
    orJson["start"] = korObject.m_startState;
    orJson["end"] = korObject.m_endState;
    orJson["transitionIndex"] = korObject.m_transitionIndex;
    orJson["machineIndex"] = korObject.m_machineIndex;

}

void from_json(const nlohmann::json& korJson, StateConnection& orObject)
{
    orObject.m_startState = korJson["start"].get<Int32_t>();
    orObject.m_endState = korJson["end"].get<Int32_t>();
    orObject.m_transitionIndex = korJson["transitionIndex"].get<Int32_t>();
    orObject.m_machineIndex = korJson["machineIndex"].get<Int32_t>();
}

BaseAnimationState * StateConnection::start(AnimationStateMachine * sm)
{
    return sm->getState(m_startState);
}

const BaseAnimationState * StateConnection::start(const AnimationStateMachine * sm) const
{
    return sm->getState(m_startState);
}

BaseAnimationState * StateConnection::end(AnimationStateMachine * sm)
{
    return sm->getState(m_endState);
}

const BaseAnimationState * StateConnection::end(const AnimationStateMachine * sm) const
{
    return sm->getState(m_endState);
}



// Animation State Machine

AnimationStateMachine::AnimationStateMachine() 
{
}

AnimationStateMachine::~AnimationStateMachine()
{
    for (BaseAnimationState* animState : m_animationStates) {
        delete animState;
    }

    for (AnimationTransition* transition : m_transitions) {
        delete transition;
    }
}

void AnimationStateMachine::addState(BaseAnimationState * state)
{
    switch (state->stateType()) {
    case AnimationStateType::kAnimation:
    {
#ifdef DEBUG_MODE
        auto iter = std::find_if(m_animationStates.begin(), m_animationStates.end(),
            [state](AnimationState* s) {
            return s->getUuid() == state->getUuid();
        });
        if (iter != m_animationStates.end()) {
            Logger::Throw("Error, state already present in state machine");
        }
#endif
        int stateIndex;
        if (m_erasedStates.size()) {
            stateIndex = m_erasedStates.back();
            m_erasedStates.pop_back();
#ifdef DEBUG_MODE
            if (m_animationStates[stateIndex]) {
                Logger::Throw("Error, pointer is not null at erased state location");
            }
#endif
            m_animationStates[stateIndex] = static_cast<AnimationState*>(state);
        }
        else {
            stateIndex = (uint32_t)m_animationStates.size();
            m_animationStates.push_back(static_cast<AnimationState*>(state));
        }
        m_animationStates[stateIndex]->setMachineIndex(stateIndex);
    }
        break;
    case AnimationStateType::kTransition:
        addTransition(dynamic_cast<AnimationTransition*>(state));
        break;
    }
}

void AnimationStateMachine::addTransition(AnimationTransition * transition)
{
#ifdef DEBUG_MODE
    auto iter = std::find_if(m_transitions.begin(), m_transitions.end(),
        [transition](AnimationTransition* t) {
        return t->getUuid() == transition->getUuid();
    });
    if (iter != m_transitions.end()) {
        Logger::Throw("Error, transition already present in state machine");
    }
#endif

    // Add transition to internal vector of transitions
    int transitionIndex;
    if (m_erasedTransitions.size()) {
        transitionIndex = m_erasedTransitions.back();
        m_erasedTransitions.pop_back();
#ifdef DEBUG_MODE
        if (m_transitions[transitionIndex]) {
            Logger::Throw("Error, pointer is not null at erased transition location");
        }
#endif
        m_transitions[transitionIndex] = transition;
    }
    else {
        transitionIndex = (uint32_t)m_transitions.size();
        m_transitions.push_back(transition);
    }

    // Retrieve connection corresponding to transition
    StateConnection& connection = transition->connection();

    // Associate the transition with its corresponding connection
    connection.setTransitionIndex(transitionIndex);
}

StateConnection* AnimationStateMachine::addConnection(BaseAnimationState * start, BaseAnimationState * end)
{
    StateConnection* connectionPtr;
    if (m_erasedConnections.size()) {
        // If any connections were erased, then use their spot in vector
        uint32_t idx = m_erasedConnections.back();
        m_erasedConnections.pop_back();
        connectionPtr = &m_connections[idx];
        connectionPtr->setMachineIndex(idx);
    }
    else {
        Vec::EmplaceBack(m_connections, start, end);
        connectionPtr = &m_connections.back();
        connectionPtr->setMachineIndex((uint32_t)m_connections.size() - 1);
    }

    start->addConnection(connectionPtr);
    end->addConnection(connectionPtr);

    return connectionPtr;
}

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

void AnimationStateMachine::removeTransition(StateConnection& connection)
{
    // Remove from connection
    int transitionIndex = connection.transitionIndex();
    connection.setTransitionIndex(-1);

    // Remove from state machine and delete
    m_erasedTransitions.push_back(transitionIndex);
    delete m_transitions[transitionIndex];

    // Remove from transitions vector
    //m_transitions.erase(transitionIndex + m_transitions.begin());
    m_transitions[transitionIndex] = nullptr;
}

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
                Logger::Throw("Error, index was never assigned for connection");
            }
#endif

            // Was erasing from connection vector, but this would break indexing of other connections
            StateConnection& connection = getConnection(connectionIndex);
            removeConnection(&connection);
        }

        // Delete state
        delete state;
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
        Logger::Throw("Error, unimplemented");
        break;
    }
}

BaseAnimationState * AnimationStateMachine::getState(int idx) const
{
    return m_animationStates[idx];
}

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

StateConnection & AnimationStateMachine::getConnection(int idx)
{
    return m_connections[idx];
}

AnimationTransition * AnimationStateMachine::getTransition(int idx)
{
    return m_transitions[idx];
}

AnimationTransition * AnimationStateMachine::getTransition(const GString & name)
{
    auto iter = std::find_if(m_transitions.begin(), m_transitions.end(),
        [name](AnimationTransition* t) {
            if (!t) {
                return false;
            }
            return t->getName() == name;
    });
    if (iter == m_transitions.end()) {
        return nullptr;
    }
    else {
        return *iter;
    }
}

AnimationTransition * AnimationStateMachine::getTransition(const Uuid & uuid)
{
    auto iter = std::find_if(m_transitions.begin(), m_transitions.end(),
        [uuid](AnimationTransition* t) {
            if (!t) {
                return false;
            }
            return t->getUuid() == uuid;
    });
    if (iter == m_transitions.end()) {
        return nullptr;
    }
    else {
        return *iter;
    }
}

void to_json(json& orJson, const AnimationStateMachine& korObject)
{
      // Add all non-transitional states to JSON
    orJson["animationStates"] = json::array();
    for (AnimationState* anim : korObject.m_animationStates) {
        orJson["animationStates"].push_back(*anim);
    }

    // Add transitions to JSON
    orJson["transitions"] = json::array();
    for (AnimationTransition* t : korObject.m_transitions) {
        orJson["transitions"].push_back(*t);
    }

    // Add name to JSON
    orJson["name"] = korObject.m_name.c_str();

    /// @todo Removed, used only by widgets
    orJson["uuid"] = korObject.m_uuid;

    // Add connections to JSON
    orJson["connections"] = json::array();
    for (const StateConnection& connection : korObject.m_connections) {
        if (connection.machineIndex() > -1) {
            // Save connection if it wasn't erased
            json connectionJson = json::object();
            connectionJson["start"] = connection.start(&korObject)->getName().c_str();
            connectionJson["end"] = connection.end(&korObject)->getName().c_str();
            orJson["connections"].push_back(connectionJson);
        }
    }
}

void from_json(const json& korJson, AnimationStateMachine& orObject)
{
    // Load animation states from JSON
    if (korJson.contains("animationStates")) {
        json animStates = json::array();
        if (korJson["animationStates"].is_object()) {
            // Deprecated 11/1/2020, only kept for backwards compatibility
            const json& statesObject = korJson["animationStates"];
            for (const auto& el: statesObject.items()) {
                const std::string& key = el.key();
                json stateObject = el.value();
                stateObject["name"] = key;
                animStates.push_back(stateObject);
            }
        }
        else {
            animStates = korJson["animationStates"];
        }
        for (const json& anim : animStates) {
            AnimationStateType stateType = (AnimationStateType)anim.value("stateType", 1);
            BaseAnimationState* state;
            switch (stateType) {
            case AnimationStateType::kNull:
                Logger::Throw("unimplemented");
                break;
            case AnimationStateType::kAnimation:
                state = new AnimationState(&orObject);
                FromJson<AnimationState>(anim, *state);
                break;
            case AnimationStateType::kTransition:
                // TODO: Remove
                // Legacy, transitions are now stored in their own vector
                continue;
            default:
                Logger::Throw("Error, state type unrecognized");
            }
            orObject.addState(state);
        }
    }

    // Load connections from JSON
    if (korJson.contains("connections")) {
        const json& connections = korJson["connections"];
        for (const json& connectionJson : connections) {
            uint32_t idx = (uint32_t)orObject.m_connections.size();
            orObject.m_connections.emplace_back();
            StateConnection& connection = orObject.m_connections.back();

            GString startStateName = connectionJson["start"].get_ref<const std::string&>().c_str();
            GString endStateName = connectionJson["end"].get_ref<const std::string&>().c_str();
            connection.setStartState(orObject.getState(startStateName)->machineIndex());
            connection.setEndState(orObject.getState(endStateName)->machineIndex());

            if (connection.getEndState() < 0 || connection.getStartState() < 0) {
                Logger::Throw("Error, states not found");
            }

            connection.setMachineIndex(idx);

            // Add connection to state
            BaseAnimationState* startState = connection.start(&orObject);
            startState->addConnection(&connection);
            BaseAnimationState* endState = connection.end(&orObject);
            endState->addConnection(&connection);
        }
    }

    // Load transitions from JSON
    if (korJson.contains("transitions")) {
        const json& transitions = korJson["transitions"];
        for (const json& trans : transitions) {
            AnimationTransition* t = new AnimationTransition(&orObject);
            trans.get_to(*t);
            orObject.addTransition(t);
        }
    }


    if (korJson.contains(JsonKeys::s_name)) {
        orObject.setName(korJson[JsonKeys::s_name].get_ref<const std::string&>().c_str());
    }
    else {
        orObject.setName(GString("sm_") + Uuid(true).asString());
    }
}



} // End namespaces
