// See:
// https://www.studica.com/blog/game-design-tutorial-blend-trees-unity
// https://gamedev.stackexchange.com/questions/112143/when-to-use-a-blend-tree-vs-state-machine-for-animation
// https://www.gamasutra.com/view/feature/3456/animation_blending_achieving_.php

// Inverse Kinematics:
// http://soliduscode.com/using-jacobian-for-inverse-kinematics/
// http://soliduscode.com/iksolver-class/

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_ANIM_STATE_MACHINE_H
#define GB_ANIM_STATE_MACHINE_H

// QT
#include <QElapsedTimer>

// Internal
#include "GAnimationState.h"
#include "GAnimMotion.h"

namespace rev {
/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class AnimationStateMachine;
class AnimationTransition;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class StateConnection
/// @brief A connection between two states in the state machine
class StateConnection: public Object, public Identifiable, public Serializable {
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    StateConnection();
    StateConnection(BaseAnimationState* start, BaseAnimationState* end);
    ~StateConnection();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    const AnimationTransition* transition(AnimationStateMachine* sm) const;
    //AnimationTransition* transition() { return m_transition; }

    //void setTransition(AnimationTransition* transition) {
    //    m_transition = transition;
    //}

    int transitionIndex() const;
    bool hasTransition() const;
    void setTransitionIndex(int transitionIndex) {
        if (m_transitionIndex > -1) {
            throw("Error, already has transition");
        }
        m_transitionIndex = transitionIndex;
    }

    int machineIndex() const { return m_machineIndex; }
    void setMachineIndex(int idx) { m_machineIndex = idx; }

    BaseAnimationState* start(AnimationStateMachine* sm);
    const BaseAnimationState* start(AnimationStateMachine* sm) const;

    BaseAnimationState* end(AnimationStateMachine* sm);
    const BaseAnimationState* end(AnimationStateMachine* sm) const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}


private:

    //---------------------------------------------------------------------------------------
    /// @name Private members
    /// @{

    /// @brief Indices of start and end states in state machine states list
    int m_startState = -1;
    int m_endState = -1;

    /// @brief Index of transition in transitions list
    /// @details if negative, connection has no transition
    int m_transitionIndex = -1;

    /// @brief Index of the connection in the state machine
    int m_machineIndex = -1;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class AnimationStateMachine
/// @brief The state machine that outlines all possible logical flows for a set of animations
/// @details Is used by the animation controller
class AnimationStateMachine: public Object, public Identifiable, public Nameable, public Serializable {
public:

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    AnimationStateMachine();
    ~AnimationStateMachine();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    const std::vector<StateConnection>& connections() { return m_connections; }

    /// @brief All non-transitional states associated with the state machine, may not actually be active in the state machine
    const std::vector<BaseAnimationState*>& states() const { return m_animationStates; }

    /// @brief All transitional states associated with the state machine
    const std::vector<AnimationTransition*>& transitions() const { return m_transitions; }

    /// @}

    //---------------------------------------------------------------------------------------
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
    BaseAnimationState* getState(int idx);
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

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:

    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    ///// @brief Pointer to top animation state in hierarchy
    ///// @details There is not really anything special about the "root", since the graph cycles over itself
    //BaseAnimationState* m_root = nullptr;

    /// @brief The connections between states in the state machine
    std::vector<StateConnection> m_connections;

    /// @brief Indices of erased connections that can be overwritten
    std::vector<size_t> m_erasedConnections;

    /// @brief Vector of all states, independent of order or presence in state machine
    std::vector<BaseAnimationState*> m_animationStates;

    /// @brief Vector of erased states to allow storage as indices
    std::vector<int> m_erasedStates;

    /// @brief The transitions between states (associated with connections)
    std::vector<AnimationTransition*> m_transitions;

    /// @brief Vector of indices of all erased transitions, to allow for indexing in connections
    std::vector<int> m_erasedTransitions;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif