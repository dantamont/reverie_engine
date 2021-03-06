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

#ifndef GB_ANIMATION_STATE_H
#define GB_ANIMATION_STATE_H

// QT

// Internal
#include "GAnimation.h"

namespace rev {
/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class StateConnection;
class AnimationStateMachine;
class BaseAnimationState;
class Motion;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

enum class AnimationStateType {
    kNull = 0, // Null state type, invalid
    kAnimation,
    kTransition
};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class BaseAnimationState
/// @brief Base class for animation and transition states
class BaseAnimationState : public Object, public Nameable, public Identifiable, public Serializable {
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    BaseAnimationState(AnimationStateMachine* stateMachine);
    BaseAnimationState(const GString& name, AnimationStateMachine* stateMachine);
    ~BaseAnimationState();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief The connections that this state has to other states
    const std::vector<int>& connections() { return m_connections; }
    
    int machineIndex() const { return m_machineIndex; }
    void setMachineIndex(int idx) { m_machineIndex = idx; }

    /// @}


    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Return casted as another state
    template <typename T>
    T& as() {
        return dynamic_cast<T&>(*this);
    }

    /// @brief Whether or not the connection starts at the given state
    bool connectsFrom(const BaseAnimationState* state, AnimationStateMachine* sm, int* outConnectionIndex = nullptr) const;

    /// @brief Whether or not the connection ends at the given state
    bool connectsTo(const BaseAnimationState* state, AnimationStateMachine* sm, int* outConnectionIndex = nullptr) const;

    /// @brief Defines the playback behavior of the state
    AnimationPlaybackMode playbackMode() const { return m_playbackMode; }

    /// @brief Detach connections from all other states
    void detachConnections(AnimationStateMachine* sm);

    /// @brief Add a connection between this and another state
    void addConnection(StateConnection* connection);

    /// @brief Remove a connection from this state
    void removeConnection(StateConnection* connection);

    /// @brief Return the animation state type
    virtual AnimationStateType stateType() const = 0;

    /// @brief What happens on the transition into this state
    virtual void onEntry(Motion& motion) const = 0;

    /// @brief What happens on the transition out of this state
    virtual void onExit(Motion& motion) const = 0;
    
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "BaseAnimationState"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "rev::BaseAnimationState"; }
   
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
    /// @name Private Members
    /// @{

    /// @brief Index in either state machine list of states or transitions
    int m_machineIndex = -1;

    /// @brief The state machine that this state belongs to
    AnimationStateMachine* m_stateMachine;

    /// @brief Playback mode for the animation state
    AnimationPlaybackMode m_playbackMode = AnimationPlaybackMode::kLoop;

    /// @brief Indices of connections to other states in state machine list
    std::vector<int> m_connections;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class AnimationState
/// @brief Instantiation of an animation
class AnimationState: public BaseAnimationState {
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    AnimationState(AnimationStateMachine* stateMachine);
    AnimationState(const GString& name, AnimationStateMachine* stateMachine);
    ~AnimationState();

    /// @}


    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{


    const std::vector<AnimationClip>& clips() const { return m_clips; }
    std::vector<AnimationClip>& clips() { return m_clips; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Return the animation state type
    virtual AnimationStateType stateType() const override { return AnimationStateType::kAnimation; }

    /// @brief What happens on the transition into this state
    virtual void onEntry(Motion& motion) const override;

    /// @brief What happens on the transition out of this state
    virtual void onExit(Motion& motion) const override;

    /// @brief Add an animation clip (base animation)
    void addClip(const std::shared_ptr<ResourceHandle>& animation);

    /// @brief Remove the specified animation clip
    void removeClip(const AnimationClip& clip);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "AnimationState"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "rev::AnimationState"; }
    /// @}

private:
    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    // See: https://www.gamasutra.com/view/feature/131863/animation_blending_achieving_.php
    /// @brief Animations for this state, with blend settings
    std::vector<AnimationClip> m_clips;

    /// @brief Child sub-state of the current state
    //AnimationState* m_parent = nullptr;
    //AnimationState* m_child = nullptr;

    /// @brief Pointer to the core engine
    //CoreEngine* m_engine;

    /// @}
};



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif