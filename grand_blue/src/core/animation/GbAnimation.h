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

#ifndef GB_ANIMATION_H
#define GB_ANIMATION_H

// QT
#include <string>
#include <vector>
#include <set> 
#include <QMutex>

// Internal
#include "../GbObject.h"
#include "../time/GbTimer.h"
#include "../resource/GbResource.h"
#include "../mixins/GbLoadable.h"
#include "../geometry/GbTransform.h"
#include "../containers/GbContainerExtensions.h"

namespace Gb {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
constexpr int NUM_BONES_PER_VERTEX = 4;
struct VertexBoneData
{
    uint IDs[NUM_BONES_PER_VERTEX];
    float Weights[NUM_BONES_PER_VERTEX];
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Describes playback mode of an animation state
enum class AnimationPlaybackMode {
    kSingleShot = 0,
    kLoop,
    kPingPong
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @struct AnimationSettings
/// @brief Represents settings for animation playback
struct AnimationSettings: public Serializable
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

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @struct AnimationFrameData
/// @brief Structure for blending animations
/// @details Represents a single frame of animation data for a skeleton
struct AnimationFrameData {
    //---------------------------------------------------------------------------------------
    /// @name Public Members
    /// @{

    /// @brief Indexed by index of joint in skeleton
    std::vector<Vector3> m_translations;
    std::vector<Quaternion> m_rotations;
    std::vector<Vector3> m_scales;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
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
        const AnimationPlaybackMode& mode, const Timer& timer, size_t statusFlags);
    ~AnimationPlayData();

    bool isFadingIn() const {
        return m_statusFlags & (size_t)AnimPlayStatusFlag::kFadingIn;
    }

    bool isFadingOut() const {
        return m_statusFlags & (size_t)AnimPlayStatusFlag::kFadingOut;
    }

    std::shared_ptr<ResourceHandle> m_animationHandle;
    AnimationSettings m_settings;
    AnimationPlaybackMode m_playbackMode;

    ///// @brief The motion associated with this animation
    //Motion* m_motion = nullptr;

    TransitionPlayData m_transitionData;

    /// @brief Matches the timer of the motion that generated this play data
    Timer m_timer;

    /// @brief Transition timer, used when animation is fading in or out
    Timer m_transitionTimer;

    /// @brief Status flag to determine whether or not a clip is transitioning and needs to be weighted accordingly
    size_t m_statusFlags = 0;
};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class NodeAnimation
/// @brief Essentially an animation track, represents the animation history of a single joint
class NodeAnimation {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    NodeAnimation();
    ~NodeAnimation();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    inline std::vector<Vector3>& translations() { return m_translations; }
    inline const std::vector<Vector3>& translations() const { return m_translations; }

    inline std::vector<Quaternion>& rotations() { return m_rotations; }
    inline const std::vector<Quaternion>& rotations() const { return m_rotations; }

    inline std::vector<Vector3>& scales() { return m_scales; }
    inline const std::vector<Vector3>& scales() const { return m_scales; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Operators
    /// @{
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    inline size_t size() const { return m_translations.size(); }

    /// @brief Interpolate between the given two frame indices with the given weight
    void interpolate(size_t first, size_t second, float weight, AnimationFrameData& out) const;

    /// @}

private:

    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    std::vector<Vector3> m_translations;
    std::vector<Quaternion> m_rotations;
    std::vector<Vector3> m_scales;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class Animation
class Animation : public Resource {
public:

    static ThreadPool ANIMATION_THREADPOOL;

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructorrs

    /// @{
    //Animation();
    Animation(const GString& uniqueName);
    ~Animation();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief The times for each animation frame, in number of ticks
    const std::vector<float>& frameTimes() const { return m_times; }

    /// @brief Get the type of resource stored by this handle
    virtual Resource::ResourceType getResourceType() const override {
        return Resource::kAnimation;
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
        size_t clipIndex = 0);

    /// @brief Get the animation time from the given inputs
    float getAnimationTime(float timeInSec,
        const AnimationSettings& settings,
        AnimationPlaybackMode mode,
        bool& isDonePlaying) const;


    /// @brief What to do on removal from resource cache
    void onRemoval(ResourceCache* cache = nullptr) override;

    /// @}

    //---------------------------------------------------------------------------------------
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
    size_t m_numNonBones = 0;

    /// @}

private:
    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @brief Get neighboring frames
    /// @param[in] offset The offset in outFrames at which to insert node animation attributes
    void getInterpolatedFrame(float animationTime, BlendSet& outFrames, size_t offset = 0) const;

    /// @brief Return the index of the TRS prior to the given time
    size_t getFrameIndex(float time) const;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class AnimationClip
/// @brief Instantiation of a single animation
class AnimationClip : public Object, public Serializable {
public:
    //---------------------------------------------------------------------------------------
    /// @name Static

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    AnimationClip(CoreEngine* engine, const QJsonValue& json);
    AnimationClip(const GString& name, const std::shared_ptr<ResourceHandle>& animation);
    ~AnimationClip();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties

    const std::shared_ptr<ResourceHandle>& animationHandle() const {
        return m_animationHandle;
    }
    void setAnimationHandle(const std::shared_ptr<ResourceHandle>& animationHandle) {
        m_animationHandle = animationHandle;
    }

    /// @property Settings
    AnimationSettings& settings() { return m_settings; }
    const AnimationSettings& settings() const { return m_settings; }


    /// @brief Number of node animations that do not have corresponding bones
    size_t numNonBones() const { return animation()->m_numNonBones; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Set the duration of the clip in seconds
    void setDuration(float secs);

    ///// @brief Return skeletal pose at the given animation time (in seconds)
    //void getAnimationFrame(float timeInSec, bool& isDonePlaying, 
    //    BlendSet& outFrames, 
    //    AnimationPlaybackMode mode,
    //    size_t clipIndex) const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "AnimationClip"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::AnimationClip"; }
    /// @}

private:
    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    /// @brief Get animation
    std::shared_ptr<Animation> animation() const;

    /// @}

    //---------------------------------------------------------------------------------------
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

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif