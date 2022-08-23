#pragma once

// See:
// https://www.studica.com/blog/game-design-tutorial-blend-trees-unity
// https://gamedev.stackexchange.com/questions/112143/when-to-use-a-blend-tree-vs-state-machine-for-animation
// https://www.gamasutra.com/view/feature/3456/animation_blending_achieving_.php

// Inverse Kinematics:
// http://soliduscode.com/using-jacobian-for-inverse-kinematics/
// http://soliduscode.com/iksolver-class/

// Internal
#include "fortress/types/GIdentifiable.h"
#include "fortress/types/GNameable.h"
#include "fortress/types/GLoadable.h"
#include "fortress/types/GString.h"
#include "fortress/time/GStopwatchTimer.h"
#include "fortress/layer/framework/GFlags.h"

#include <fortress/json/GJson.h>

namespace rev {

class BaseAnimationState;
class AnimationStateMachine;
class AnimationController;

enum class AnimMotionBehaviorFlag {
    kDestroyOnDone = 1 << 0, // Whether or not to destroy motion when done playing
    kAutoPlay = 1 << 1 // Whether or not to move motion to the next available state when done playing
};
MAKE_FLAGS(AnimMotionBehaviorFlag, MotionBehaviorFlags);
MAKE_BITWISE(AnimMotionBehaviorFlag);

enum class AnimMotionStatusFlag {
    kPlaying = 1 << 0 // Whether or not the motion is playing. If false, is paused
};
MAKE_FLAGS(AnimMotionStatusFlag, MotionStatusFlags);
MAKE_BITWISE(AnimMotionStatusFlag);


/// @class Motion
/// @brief Represents an entity traversing the animation state machine
/// @details Use composition, not inheritance
class Motion : public NameableInterface, public IdentifiableInterface {
public:

    /// @name Constructors/Destructor
    /// @{

    Motion(AnimationController* controller);
    ~Motion();

    /// @}

    /// @name Properties
    /// @{

    BaseAnimationState* currentState() const;

    /// @brief The time elapsed since timer started (in sec)
    float elapsedTime() const;

    const StopwatchTimer& timer() const { return m_timer; }
    void setTimer(const StopwatchTimer& timer) { m_timer = timer; }

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Queue motion to move to the state with the given name
    /// @details Queued moves must specify a destination state that is a neighber of the current state, with 
    /// allowing for one transition in between
    bool queueMove(const GStringView& stateName);

    /// @brief Whether or not the motion is autoplaying
    bool isAutoPlaying() const {
        return m_motionBehaviorFlags.testFlag(AnimMotionBehaviorFlag::kAutoPlay);
    }

    /// @brief Whether or not the motion's state is still playing any clips
    bool isDone() const;

    /// @brief Whether or not the motion is paused
    bool isPlaying() const {
        return m_motionStatusFlags.testFlag(AnimMotionStatusFlag::kPlaying);
    }

    /// @brief Pause the motion, or play it
    void pause();
    void play();

    /// @brief Move to the specified state states
    void move(const BaseAnimationState* state);

    /// @brief Move to the next available state, based on first valid connection found
    void autoMove();

    /// @brief Start and stop timer
    void restartTimer();

    /// @}


    /// @name Friend methods
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const Motion& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, Motion& orObject);

    /// @}

private:
    /// @name Private Methods
    /// @{

    /// @brief Set state given the state's unique name
    void setState(const GString& name);
    void setState(const BaseAnimationState* state);

    /// @}

    /// @name Private Members
    /// @{

    MotionBehaviorFlags m_motionBehaviorFlags;
    mutable MotionStatusFlags m_motionStatusFlags;

    /// @brief The controller that governs this motion, and thereby the state machine that this motion traverses
    AnimationController* m_controller;

    /// @brief The uuid of the current state
    Uuid m_stateID;

    /// @brief The timer for the motion
    StopwatchTimer m_timer;

    /// @brief The most recent time at which the timer was restarted, in seconds
    /// @details Used for determining playback of animations during transitions
    //float m_previousTime;

    /// @}
};


enum class AnimationMotionAction {
    kDestroy,
    kMove
};

/// @struct MotionAction
struct MotionAction {
    AnimationMotionAction m_actionType;
    Motion* m_motion;
    BaseAnimationState* m_nextState = nullptr;

    /// @brief Perform the motion action once queued
    /// @params[in] controller The controller governing this motion
    void perform(AnimationController& controller);
};


} // End namespaces
