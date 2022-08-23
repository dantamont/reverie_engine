#pragma once

// QT
#include <QElapsedTimer>

// Internal
#include "GAnimationState.h"
#include "GAnimMotion.h"

namespace rev {

class AnimationStateMachine;
class AnimationTransition;

/// @class StateConnection
/// @brief A connection between two states in the state machine
class StateConnection: public IdentifiableInterface {
public:
    /// @name Constructors/Destructor
    /// @{

    StateConnection();
    StateConnection(BaseAnimationState* start, BaseAnimationState* end);
    ~StateConnection();

    /// @}

    /// @name Properties
    /// @{

    const AnimationTransition* transition(AnimationStateMachine* sm) const;

    int transitionIndex() const;
    bool hasTransition() const;
    void setTransitionIndex(int transitionIndex);

    int machineIndex() const { return m_machineIndex; }
    void setMachineIndex(int idx) { m_machineIndex = idx; }

    BaseAnimationState* start(AnimationStateMachine* sm);
    const BaseAnimationState* start(const AnimationStateMachine* sm) const;

    BaseAnimationState* end(AnimationStateMachine* sm);
    const BaseAnimationState* end(const AnimationStateMachine* sm) const;

    int getStartState() const {
        return m_startState;
    }
    void setStartState(int startState) {
        m_startState = startState;
    }

    int getEndState() const {
        return m_endState;
    }
    void setEndState(int endState) {
        m_endState = endState;
    }

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const StateConnection& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, StateConnection& orObject);


    /// @}

private:

    /// @name Private members
    /// @{

    /// @brief Indices of start and end states in state machine states list
    Int32_t m_startState = -1;
    Int32_t m_endState = -1;

    /// @brief Index of transition in transitions list
    /// @details if negative, connection has no transition
    Int32_t m_transitionIndex = -1;

    /// @brief Index of the connection in the state machine
    Int32_t m_machineIndex = -1;

    /// @}
};

/// @class AnimationStateMachine
/// @brief The state machine that outlines all possible logical flows for a set of animations
/// @details Is used by the animation controller
class AnimationStateMachine: public IdentifiableInterface, public NameableInterface {
public:

    /// @name Constructors/Destructor
    /// @{

    AnimationStateMachine();
    ~AnimationStateMachine();

    /// @}

    /// @name Properties
    /// @{

    const std::vector<StateConnection>& connections() { return m_connections; }

    /// @brief All non-transitional states associated with the state machine, may not actually be active in the state machine
    const std::vector<AnimationState*>& states() const { return m_animationStates; }

    /// @brief All transitional states associated with the state machine
    const std::vector<AnimationTransition*>& transitions() const { return m_transitions; }

    /// @}

    /// @name Public methods
    /// @{

    /// @brief Add state to the state machine
    void addState(BaseAnimationState* state);

    /// @brief Add a transition to the state machine
    void addTransition(AnimationTransition* transition);

    /// @brief Add a connection between two states
    StateConnection* addConnection(BaseAnimationState* start, BaseAnimationState* end);

    /// @brief Remove a connection
    void removeConnection(StateConnection* connection);

    /// @brief Remove a transition
    void removeTransition(StateConnection& connection);

    /// @brief Remove a state from the controller
    void removeState(BaseAnimationState* state);

    /// @brief Get the state with the given name
    BaseAnimationState* getState(int idx) const;
    BaseAnimationState* getState(const GStringView& name);
    BaseAnimationState* getState(const GString& name);
    BaseAnimationState* getState(const Uuid& uuid);

    /// @brief Get the connection with the UUID
    StateConnection* getConnection(const Uuid& uuid);

    /// @brief Get the connection at the specified index in the connections list
    StateConnection& getConnection(int idx);

    /// @breif Get the transition at the given index
    AnimationTransition * getTransition(int idx);
    AnimationTransition* getTransition(const GString& name);
    AnimationTransition* getTransition(const Uuid& uuid);

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const AnimationStateMachine& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, AnimationStateMachine& orObject);


    /// @}

protected:

    /// @name Protected Members
    /// @{

    ///// @brief Pointer to top animation state in hierarchy
    ///// @details There is not really anything special about the "root", since the graph cycles over itself
    //BaseAnimationState* m_root = nullptr;

    /// @brief The connections between states in the state machine
    std::vector<StateConnection> m_connections;

    /// @brief Indices of erased connections that can be overwritten
    std::vector<uint32_t> m_erasedConnections;

    /// @brief Vector of all states, independent of order or presence in state machine
    std::vector<AnimationState*> m_animationStates;

    /// @brief Vector of erased states to allow storage as indices
    std::vector<int> m_erasedStates;

    /// @brief The transitions between states (associated with connections)
    std::vector<AnimationTransition*> m_transitions;

    /// @brief Vector of indices of all erased transitions, to allow for indexing in connections
    std::vector<int> m_erasedTransitions;

    /// @}
};


} // End namespaces
