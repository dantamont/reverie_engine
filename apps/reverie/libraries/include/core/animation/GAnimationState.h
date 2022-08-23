#pragma once

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

/// @class BaseAnimationState
/// @brief Base class for animation and transition states
class BaseAnimationState : public NameableInterface, public IdentifiableInterface {
public:
    /// @name Constructors/Destructor
    /// @{

    BaseAnimationState(AnimationStateMachine* stateMachine);
    BaseAnimationState(const GString& name, AnimationStateMachine* stateMachine);
    ~BaseAnimationState();

    /// @}

    /// @name Properties
    /// @{

    /// @brief The connections that this state has to other states
    const std::vector<int>& connections() { return m_connections; }
    
    int machineIndex() const { return m_machineIndex; }
    void setMachineIndex(int idx) { m_machineIndex = idx; }

    /// @}


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

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const BaseAnimationState& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, BaseAnimationState& orObject);


    /// @}

protected:

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


/// @class AnimationState
/// @brief Instantiation of an animation
class AnimationState: public BaseAnimationState {
public:

    /// @name Constructors/Destructor
    /// @{

    AnimationState(AnimationStateMachine* stateMachine);
    AnimationState(const GString& name, AnimationStateMachine* stateMachine);
    ~AnimationState();

    /// @}


    /// @name Properties
    /// @{

    const std::vector<AnimationClip>& clips() const { return m_clips; }
    std::vector<AnimationClip>& clips() { return m_clips; }

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Retrieve a clip from its UUID
    AnimationClip& getClip(const Uuid& id);

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
    void removeClip(const Uuid& clipId);

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const AnimationState& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, AnimationState& orObject);


    /// @}

private:

    /// @name Private Members
    /// @{

    // See: https://www.gamasutra.com/view/feature/131863/animation_blending_achieving_.php
    /// @brief Animations for this state, with blend settings
    std::vector<AnimationClip> m_clips;

    /// @}
};



} // End namespaces
