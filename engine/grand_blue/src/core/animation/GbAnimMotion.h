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

#ifndef GB_ANIM_MOTION_H
#define GB_ANIM_MOTION_H

// Internal
#include "../GbObject.h"
#include "../mixins/GbLoadable.h"
#include "../containers/GbString.h"
#include "../time/GbTimer.h"

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class BaseAnimationState;
class AnimationStateMachine;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
enum class AnimMotionBehaviorFlag {
    kDestroyOnDone = 1 << 0, // Whether or not to destroy motion when done playing
    kAutoPlay = 1 << 1 // Whether or not to move motion to the next available state when done playing
};

enum class AnimMotionStatusFlag {
    kPlaying = 1 << 0 // Whether or not the motion is playing. If false, is paused
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class Motion
/// @brief Represents an entity traversing the animation state machine
class Motion : public Object, public Serializable {
public:

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    Motion(AnimationStateMachine* stateMachine);
    ~Motion();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    BaseAnimationState* currentState() const;

    /// @brief The time elapsed since timer started (in sec)
    float elapsedTime() const;

    const Timer& timer() const { return m_timer; }
    void setTimer(const Timer& timer) { m_timer = timer; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Whether or not the motion is autoplaying
    bool isAutoPlaying() const {
        return m_motionBehaviorFlags & (size_t)AnimMotionBehaviorFlag::kAutoPlay;
    }

    /// @brief Whether or not the motion's state is still playing any clips
    bool isDone() const;

    /// @brief Whether or not the motion is paused
    bool isPlaying() const {
        return m_motionStatusFlags | (size_t)AnimMotionStatusFlag::kPlaying;
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
    //void stopTimer();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

private:
    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @brief Set state given the state's unique name
    void setState(const GString& name);
    void setState(const BaseAnimationState* state);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    size_t m_motionBehaviorFlags;
    mutable size_t m_motionStatusFlags;

    /// @brief The state machine that this motion traverses
    AnimationStateMachine* m_stateMachine;

    /// @brief The uuid of the current state
    Uuid m_stateID;

    /// @brief The timer for the motion
    Timer m_timer;

    /// @brief The most recent time at which the timer was restarted, in seconds
    /// @details Used for determining playback of animations during transitions
    //float m_previousTime;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
enum class AnimationMotionAction {
    kDestroy,
    kMove
};

/// @struct MotionAction
struct MotionAction {
    AnimationMotionAction m_actionType;
    Motion* m_motion;
    BaseAnimationState* m_nextState;

    void perform();
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif