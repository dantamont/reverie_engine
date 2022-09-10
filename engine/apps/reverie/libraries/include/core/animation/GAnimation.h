#pragma once

// See:
// https://www.studica.com/blog/game-design-tutorial-blend-trees-unity
// https://gamedev.stackexchange.com/questions/112143/when-to-use-a-blend-tree-vs-state-machine-for-animation
// https://www.gamasutra.com/view/feature/3456/animation_blending_achieving_.php

// Inverse Kinematics:
// http://soliduscode.com/using-jacobian-for-inverse-kinematics/
// http://soliduscode.com/iksolver-class/

// QT
#include <string>
#include <vector>
#include <set> 
#include <QMutex>

// Internal
#include "fortress/time/GStopwatchTimer.h"
#include "core/resource/GResourceHandle.h"
#include "fortress/types/GLoadable.h"
#include "heave/kinematics/GTransform.h"
#include "fortress/containers/GContainerExtensions.h"

namespace rev {


class Model;
class ResourceHandle;
class ResourceCache;
class SkeletonJoint;
class CoreEngine;
class Animation;
class Skeleton;
struct AnimationSettings;
class ThreadPool;
class AnimationProcess;
class DrawCommand;
struct BlendSet;

constexpr int NUM_BONES_PER_VERTEX = 4;
struct VertexBoneData
{
    uint IDs[NUM_BONES_PER_VERTEX];
    float Weights[NUM_BONES_PER_VERTEX];
};


/// @brief Describes playback mode of an animation state
enum class AnimationPlaybackMode {
    kSingleShot = 0,
    kLoop,
    kPingPong
};

/// @struct AnimationSettings
/// @brief Represents settings for animation playback
struct AnimationSettings
{
    /// @brief Multiplicative speed factor
    float m_speedFactor = 1.0;

    /// @brief For blending the clip with others
    // TODO: Make this a mask
    float m_blendWeight = 1.0;

    /// @brief For offsetting playback by some number of ticks
    int m_tickOffset = 0;

    /// @brief For offsetting playback by some number of seconds
    float m_timeOffsetSec = 0;

    /// @brief The number of times to play the animation (for loop and ping-pong)
    int m_numPlays = -1;

    /// @brief Cached animation time, for use if animation is to be frozen (note, NOT in seconds)
    int m_animationTime = -1;

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const AnimationSettings& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, AnimationSettings& orObject);


    /// @}
};


/// @struct AnimationFrameData
/// @brief Structure for blending animations
/// @details Represents a single frame of animation data for a skeleton
struct AnimationFrameData {
    /// @name Public Members
    /// @{

    /// @brief Indexed by index of joint in skeleton
    std::vector<Vector3> m_translations;
    std::vector<Quaternion> m_rotations;
    std::vector<Vector3> m_scales;

    /// @}
};


enum class AnimPlayStatusFlag {
    kFadingOut = 1 << 0,
    kFadingIn = 1 << 1
};

struct TransitionPlayData {
    float m_totalTime;
    float m_fadeInTime;
    float m_fadeOutTime;
};

/// @brief Play data representing an animation that is to be blended
struct AnimationPlayData {
    AnimationPlayData(const std::shared_ptr<ResourceHandle>& animHandle, const AnimationSettings& settings,
        const AnimationPlaybackMode& mode, const StopwatchTimer& timer, uint32_t statusFlags);
    ~AnimationPlayData();

    bool isFadingIn() const {
        return m_statusFlags & (uint32_t)AnimPlayStatusFlag::kFadingIn;
    }

    bool isFadingOut() const {
        return m_statusFlags & (uint32_t)AnimPlayStatusFlag::kFadingOut;
    }

    std::shared_ptr<ResourceHandle> m_animationHandle;
    AnimationSettings m_settings;
    AnimationPlaybackMode m_playbackMode;

    ///// @brief The motion associated with this animation
    //Motion* m_motion = nullptr;

    TransitionPlayData m_transitionData;

    /// @brief Matches the timer of the motion that generated this play data
    StopwatchTimer m_timer;

    /// @brief Transition timer, used when animation is fading in or out
    StopwatchTimer m_transitionTimer;

    /// @brief Status flag to determine whether or not a clip is transitioning and needs to be weighted accordingly
    uint32_t m_statusFlags = 0;
};



/// @class NodeAnimation
/// @brief Essentially an animation track, represents the animation history of a single joint
class NodeAnimation {
public:
    /// @name Constructors/Destructor
    /// @{

    NodeAnimation();
    ~NodeAnimation();

    /// @}

    /// @name Properties
    /// @{

    inline std::vector<Vector3>& translations() { return m_translations; }
    inline const std::vector<Vector3>& translations() const { return m_translations; }

    inline std::vector<Quaternion>& rotations() { return m_rotations; }
    inline const std::vector<Quaternion>& rotations() const { return m_rotations; }

    inline std::vector<Vector3>& scales() { return m_scales; }
    inline const std::vector<Vector3>& scales() const { return m_scales; }

    /// @}

    /// @name Public Methods
    /// @{

    inline size_t size() const { return m_translations.size(); }

    /// @brief Interpolate between the given two frame indices with the given weight
    void interpolate(uint32_t first, uint32_t second, float weight, AnimationFrameData& out) const;

    /// @}

private:

    /// @name Private Members
    /// @{

    std::vector<Vector3> m_translations;
    std::vector<Quaternion> m_rotations;
    std::vector<Vector3> m_scales;

    /// @}
};


/// @class Animation
class Animation : public Resource {
public:

    static ThreadPool ANIMATION_THREADPOOL;

    /// @name Constructors/Destructorrs
    /// @{

    Animation();
    ~Animation();

    /// @}


    /// @name Public Methods
    /// @{

    /// @brief The times for each animation frame, in number of ticks
    const std::vector<float>& frameTimes() const { return m_times; }

    /// @brief Get the type of resource stored by this handle
    virtual GResourceType getResourceType() const override {
        return EResourceType::eAnimation;
    }

    /// @brief Return duration in seconds of the animation
    float getTimeDuration() const;

    /// @brief Return FPS of the animation
    float getRate() const;

    /// @brief Return map of bone transforms at the given animation time (in seconds)
    /// @details Transform vector is ordered by bone ID
    /// @params[in] clipIndex the index of the clip in the blend queue for which the frame is being retrieved
    /// Speed factor is multiplicative
    /// Returns false if done playing
    void getAnimationFrame(float timeInSec, 
        const AnimationSettings& settings,
        AnimationPlaybackMode mode,
        bool& isDonePlaying,
        BlendSet& outFrames,
        uint32_t clipIndex = 0);

    /// @brief Get the animation time from the given inputs
    float getAnimationTime(float timeInSec,
        const AnimationSettings& settings,
        AnimationPlaybackMode mode,
        bool& isDonePlaying) const;


    /// @brief What to do on removal from resource cache
    void onRemoval(ResourceCache* cache = nullptr) override;

    /// @}


    /// @name Public Members
    /// @{

    /// @brief Map of all node animations
    /// @details Animations are in the order of which their nodes are reached in a breadth-first-search (BFS) of the joint hierarchy
    /// @note Animations may exist for nodes without bones, so a vector ordered by bones is not sufficient
    std::vector<NodeAnimation> m_nodeAnimations;

    /// @brief Tick-rate of the animation
    double m_ticksPerSecond = 0;

    /// @brief Animation duration (in number of frames)
    double m_durationInTicks;

    /// @brief Time in ticks for each frame of the animation (assumed to be the same for T, S, R)
    std::vector<float> m_times;

    /// @brief The number of animations that correspond to nodes without bones
    uint32_t m_numNonBones = 0;

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const Animation& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, Animation& orObject);


    /// @}

private:
    /// @name Private Methods
    /// @{

    /// @brief Get neighboring frames
    /// @param[in] offset The offset in outFrames at which to insert node animation attributes
    void getInterpolatedFrame(float animationTime, BlendSet& outFrames, uint32_t offset = 0) const;

    /// @brief Return the index of the TRS prior to the given time
    uint32_t getFrameIndex(float time) const;

    /// @}
};

/// @class AnimationClip
/// @brief Instantiation of a single animation
class AnimationClip :  public NameableInterface, public IdentifiableInterface {
public:

    /// @name Constructors/Destructor
    AnimationClip(const nlohmann::json& json);
    AnimationClip(const GString& name, const std::shared_ptr<ResourceHandle>& animation);
    ~AnimationClip();

    /// @}

    /// @name Properties

    const std::shared_ptr<ResourceHandle>& animationHandle() const {
        return m_animationHandle;
    }
    void setAnimationHandle(const std::shared_ptr<ResourceHandle>& animationHandle) const {
        m_animationHandle = animationHandle;
    }

    const GString& animationHandleName() const { return m_animationHandleName; }


    /// @property Settings
    AnimationSettings& settings() { return m_settings; }
    const AnimationSettings& settings() const { return m_settings; }


    /// @brief Number of node animations that do not have corresponding bones
    uint32_t numNonBones() const { return animation()->m_numNonBones; }

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Set the duration of the clip in seconds
    void setDuration(float secs);

    /// @brief Verify that the clip has a handle
    bool verifyLoaded(ResourceCache* cache) const;

    ///// @brief Return skeletal pose at the given animation time (in seconds)
    //void getAnimationFrame(float timeInSec, bool& isDonePlaying, 
    //    BlendSet& outFrames, 
    //    AnimationPlaybackMode mode,
    //    uint32_t clipIndex) const;

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const AnimationClip& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, AnimationClip& orObject);


    /// @}

private:
    /// @name Private Members
    /// @{

    /// @brief Get animation
    Animation* animation() const;

    /// @}

    /// @name Private Members
    /// @{

    /// @brief Resource handle for the animation
    /// @details Made mutable so that animation can be loaded in during getAnimationFrame if not yet assigned
    mutable std::shared_ptr<ResourceHandle> m_animationHandle = nullptr;

    /// @brief Cache animation handle name for deferred assignment
    GString m_animationHandleName;

    AnimationSettings m_settings;

    /// @brief Number of plays, used for Loop and PingPong modes
    //int m_numPlays = -1;

    /// @}
};


} // End namespaces
