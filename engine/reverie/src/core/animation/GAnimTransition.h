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

#ifndef GB_ANIMATION_TRANSITION_H
#define GB_ANIMATION_TRANSITION_H

// std
#include <vector>

// Internal
#include "GAnimationState.h"
#include "../time/GTimer.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class Motion;
class StateConnection;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @enum AnimationTransitionType
enum class AnimationTransitionType {
    kSmooth = 0, // Clip A and B both play as A fades into B
    kFirstFrozen // Clip A is paused while fading into B
};

//enum class TransitionPlaybackFlag {
//    kReversed = 1 << 0, // Reverse the transition
//};

/// @struct Transition Settings
/// @brief Represents settings for an animation transition
class TransitionSettings: public Serializable {
public:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

    AnimationTransitionType m_transitionType = AnimationTransitionType::kSmooth;

    /// @brief Transition settings
    float m_fadeInTimeSec = 0;
    float m_fadeInBlendWeight = 1.0;

    float m_fadeOutTimeSec = 0;
    float m_fadeOutBlendWeight = 1.0;
};
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class AnimationTransition
class AnimationTransition: public BaseAnimationState{
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    AnimationTransition(AnimationStateMachine* stateMachine);
    AnimationTransition(AnimationStateMachine* stateMachine, int connectionIndex);
    ~AnimationTransition();
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    TransitionSettings& settings() { return m_settings; }

    const Timer& timer() const { return m_timer; }

    /// @brief Return the transition time based on the transition settings
    float transitionTime() const {
        return std::max(m_settings.m_fadeInTimeSec, m_settings.m_fadeOutTimeSec);
    }

    bool isDone() const {
        return m_timer.getElapsed<float>() > transitionTime();
    }

    /// @brief Return clip data and weights
    void getClips(const Motion& motion, std::vector<AnimationPlayData>& outData, std::vector<float>& outWeights) const;

    const AnimationState* start() const;
    const AnimationState* end() const;

    StateConnection& connection() const;

    /// @brief Return the animation state type
    virtual AnimationStateType stateType() const override { return AnimationStateType::kTransition; }

    /// @brief What happens on the transition into this state
    virtual void onEntry(Motion& motion) const override;

    /// @brief What happens on the transition out of this state
    virtual void onExit(Motion& motion) const override;

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
    /// @name Private Methods
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    /// @brief timer for the transition
    mutable Timer m_timer;

    // TODO: Maybe a different set of settings if reversed
    TransitionSettings m_settings;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif