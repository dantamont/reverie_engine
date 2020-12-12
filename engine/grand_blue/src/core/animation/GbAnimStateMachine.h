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
#include "GbAnimationState.h"
#include "GbAnimMotion.h"

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class AnimationStateMachine;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class StateConnection
/// @brief A connection between two states in the state machine
class StateConnection: public Object, public Serializable {
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

    size_t machineIndex() const { return m_machineIndex; }
    void setMachineIndex(size_t idx) { m_machineIndex = idx; }

    BaseAnimationState* start() { return m_startState; }
    const BaseAnimationState* start() const { return m_startState; }

    BaseAnimationState* end() { return m_endState; }
    const BaseAnimationState* end() const { return m_endState; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}


private:

    //---------------------------------------------------------------------------------------
    /// @name Private members
    /// @{

    BaseAnimationState* m_startState;
    BaseAnimationState* m_endState;

    /// @brief Index of the connection in the state machine
    int m_machineIndex = -1;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class AnimationStateMachine
/// @brief The state machine that outlines all possible logical flows for a set of animations
/// @details Is used by the animation controller
class AnimationStateMachine: public Object, public Serializable {
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

    /// @brief All states associated with the state machine, may not actually be active in the state machine
    const std::vector<BaseAnimationState*>& states() const { return m_animationStates; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief Add state to the controller
    void addState(BaseAnimationState* state);

    /// @brief Add a connection between two states
    StateConnection* addConnection(BaseAnimationState* start, BaseAnimationState* end);

    /// @brief Remove a connection
    void removeConnection(StateConnection* connection);

    /// @brief Remove a state from the controller
    void removeState(BaseAnimationState* state);

    /// @brief Get the state with the given name
    BaseAnimationState* getState(const GString& name);
    BaseAnimationState* getState(const Uuid& name);

    /// @brief Get the connection with the UUID
    StateConnection* getConnection(const Uuid& uuid);

    /// @brief Get the connection at the specified index in the connections list
    StateConnection& getConnection(int idx);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

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

    /// @brief Vector of all states, independent of order or presence in state machine
    std::vector<BaseAnimationState*> m_animationStates;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif