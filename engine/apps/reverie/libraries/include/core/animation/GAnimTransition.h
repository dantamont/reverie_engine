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
#include "fortress/time/GStopwatchTimer.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class Motion;
class StateConnection;
class ResourceCache;

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
class TransitionSettings {
public:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const TransitionSettings& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, TransitionSettings& orObject);


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

    const StopwatchTimer& timer() const { return m_timer; }

    /// @brief Return the transition time based on the transition settings
    float transitionTime() const {
        return std::max(m_settings.m_fadeInTimeSec, m_settings.m_fadeOutTimeSec);
    }

    bool isDone() const {
        return m_timer.getElapsed<float>() > transitionTime();
    }

    /// @brief Return clip data and weights
    void getClips(const Motion& motion, std::vector<AnimationPlayData>& outData, std::vector<float>& outWeights, ResourceCache* cache) const;

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
    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const AnimationTransition& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, AnimationTransition& orObject);


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
    mutable StopwatchTimer m_timer;

    // TODO: Maybe a different set of settings if reversed
    TransitionSettings m_settings;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif