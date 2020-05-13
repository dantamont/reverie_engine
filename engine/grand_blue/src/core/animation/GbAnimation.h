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
#include "../resource/GbResource.h"
#include "../mixins/GbLoadable.h"
#include "../geometry/GbTransform.h"
#include "../containers/GbContainerExtensions.h"

namespace Gb {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class Mesh;
class ResourceHandle;
class MeshNode;
class CoreEngine;
class Animation;
class Skeleton;
struct AnimationSettings;
class ThreadPool;
class AnimationProcess;
class SkeletonPose;
template<class D, size_t N> class SquareMatrix;
typedef SquareMatrix<real_g, 3> Matrix3x3g;
typedef SquareMatrix<real_g, 4> Matrix4x4g;
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
enum class AnimationPlaybackMode {
    kSingleShot,
    kLoop,
    kPingPong
};

/// @struct AnimationSettings
/// @brief Represents settings for animation playback
struct AnimationSettings: public Serializable
{
    /// @brief Multiplicative speed factor
    float m_speedFactor = 1.0;
    float m_blendWeight = 1.0;
    int m_tickOffset = 0;
    float m_timeOffsetSec = 0;

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}
};


/// @struct Transition Settings
/// @brief Represents settings for an animation transition
struct TransitionSettings
{
    /// @brief Transition settings
    float m_fadeInTimeSec = 0;
    float m_fadeInBlendWeight = 1.0;

    float m_fadeOutTimeSec = 0;
    float m_fadeOutBlendWeight = 1.0;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @struct BlendSet
/// @brief Structure for blending animations
struct BlendSet {
    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @}

    std::vector<Vector3g> m_translations;
    std::vector<Quaternion> m_rotations;
    std::vector<Vector3g> m_scales;
};
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @struct BlendPose
/// @brief Container for node blend sets
struct BlendPose {
    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Blend the blend sets for this pose to obtain a vector of final local transforms
    void blend();

    inline size_t size() const { return m_blendSets.size(); }

    /// @}

    /// @brief Blend sets, mapped by node name
    std::unordered_map<QString, BlendSet> m_blendSets;

    // TODO: Remove weights from the blend pose, this should be driven at a higher level 
    std::vector<float> m_weights;

    /// @brief Final map of Transforms
    std::unordered_map<QString, Transform> m_transforms;

    size_t m_numNonBones = 0;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class JointAnimation
/// @brief Represents the animation history of a single joint
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

    inline std::vector<Vector3g>& translations() { return m_translations; }
    inline const std::vector<Vector3g>& translations() const { return m_translations; }

    inline std::vector<Quaternion>& rotations() { return m_rotations; }
    inline const std::vector<Quaternion>& rotations() const { return m_rotations; }

    inline std::vector<Vector3g>& scales() { return m_scales; }
    inline const std::vector<Vector3g>& scales() const { return m_scales; }

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
    void interpolate(size_t first, size_t second, float weight, BlendSet& out) const;

    /// @}

private:

    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    std::vector<Vector3g> m_translations;
    std::vector<Quaternion> m_rotations;
    std::vector<Vector3g> m_scales;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class SkeletonPose
class SkeletonPose {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    SkeletonPose();
    SkeletonPose(Mesh* mesh, BlendPose&& frames);
    ~SkeletonPose();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Iterate over node hierarchy to generate a skeletal pose for an animation frame
    void getBoneTransforms(std::vector<Matrix4x4g>& outBoneTransforms, bool forceBlend = false);

    /// @}

private:
    friend class AnimationState;

    void processNodeHierarchy(MeshNode& node, Transform& parentTransform,
        std::vector<Matrix4x4g>& outBoneTransforms);


    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    BlendPose m_blendPose;

    Mesh* m_mesh;

    static QMutex POSE_MUTEX;

    static Transform IDENTITY;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class Animation
class Animation : public Resource
{
public:

    static ThreadPool ANIMATION_THREADPOOL;


    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructorrs

    /// @{
    //Animation();
    Animation(const QString& uniqueName);
    ~Animation();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Return duration in seconds of the animation
    float getTimeDuration() const;

    /// @brief Return FPS of the animation
    float getRate() const;

    /// @brief Return map of bone transforms at the given animation time (in seconds)
    /// @details Transform vector is ordered by bone ID
    /// Speed factor is multiplicative
    /// Returns false if done playing
    void getAnimationFrame(Mesh* mesh, 
        float timeInSec, 
        const AnimationSettings& settings,
        AnimationPlaybackMode mode,
        int numPlays,
        bool& isDonePlaying,
        BlendPose& outFrames);

    /// @brief What to do on removal from resource cache
    void onRemoval(ResourceCache* cache = nullptr) override;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Members
    /// @{

    /// @brief Map of all node animations, indexed by node name
    /// @note Animations may exist for nodes without bones, so a vector ordered by bones is not sufficient
    //std::vector<SkeletonPose> m_poses;
    std::unordered_map<QString, NodeAnimation> m_nodeAnimations;

    /// @brief Map of node names to their respective bone indices in each pose
    std::unordered_map<QString, int> m_boneIndices;

    /// @brief Tick-rate of the animation
    double m_ticksPerSecond = 0;

    /// @brief Animation duration (in number of frames)
    double m_durationInTicks;

    /// @brief Times for each frame of the animation (assumed to be the same for T, S, R
    std::vector<float> m_times;

    /// @brief The number of animations that correspond to nodes without bones
    size_t m_numNonBones = 0;

    /// @}

private:
    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @brief Get the animation time from the given inputs
    float getAnimationTime(float timeInSec,
        const AnimationSettings& settings,
        AnimationPlaybackMode mode,
        int numPlays,
        bool& isDonePlaying) const;

    /// @brief Get neighboring frames
    void interpolatePose(float animationTime, BlendPose& outFrames) const;

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
    AnimationClip(CoreEngine* engine, const QJsonValue& json);
    AnimationClip(const QString& name, const std::shared_ptr<ResourceHandle>& animation,
        CoreEngine* engine);
    ~AnimationClip();

    //---------------------------------------------------------------------------------------
    /// @name Properties

    /// @property Settings
    AnimationSettings& settings() { return m_settings; }

    /// @brief Number of node animations that do not have corresponding bones
    size_t numNonBones() const { return animation()->m_numNonBones; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Set the duration of the clip in seconds
    void setDuration(float secs);

    /// @brief Return skeletal pose at the given animation time (in seconds)
    void getAnimationFrame(Mesh* mesh, float timeInSec, BlendPose& outFrames);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

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
    std::shared_ptr<ResourceHandle> m_animationHandle;

    AnimationSettings m_settings;

    /// @brief Playback mode for the animation state
    AnimationPlaybackMode m_playbackMode = AnimationPlaybackMode::kLoop;

    /// @brief Number of plays, used for Loop and PingPong modes
    int m_numPlays = -1;

    /// @brief Whether or not the clip is done playing
    bool m_isDone;

    /// @brief Pointer to the core engine
    CoreEngine* m_engine;


    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class AnimationState
/// @brief Instantiation of an animation
class AnimationState: public Object, public Serializable {
public:
    AnimationState(const QString& name, CoreEngine* engine);
    ~AnimationState();

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Add an animation clip (base animation)
    void addClip(std::shared_ptr<ResourceHandle> animation);
    void addClip(AnimationClip* clip);

    /// @brief Add a layer (parallel-playing state)
    void addLayer(AnimationClip* state);

    /// @brief Return skeletal pose at the given animation time (in seconds)
    void getAnimationFrame(Mesh* mesh, float timeInSec, bool& isDone, 
        SkeletonPose& outSkeletalPose);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "AnimationState"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::AnimationState"; }
    /// @}

private:
    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    // TODO: Maybe 
    // See: https://www.gamasutra.com/view/feature/131863/animation_blending_achieving_.php
    /// @brief Animation layers to blend with this animation
    /// @note Useful for things like injuries, carrying different weapons, etc.
    std::unordered_map<QString, AnimationClip*> m_layers;

    /// @brief Pointers to animations for this state may or may not be getting blended)
    std::unordered_map<QString, AnimationClip*> m_clips;

    /// @brief Child sub-state of the current state
    //AnimationState* m_parent = nullptr;
    AnimationState* m_child = nullptr;

    //TransitionSettings m_transitionSettings;

    /// @brief Pointer to the core engine
    CoreEngine* m_engine;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class AnimationController
/// @brief Class representing a set of controllable animations
class AnimationController : public Object, public Serializable {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    AnimationController(CoreEngine* engine, const QJsonValue& json);
    AnimationController(CoreEngine* engine, const std::shared_ptr<ResourceHandle>& mesh);
    ~AnimationController();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    const std::shared_ptr<AnimationProcess>& process() { return m_process; }

    /// @brief Get mesh for the animation controller
    std::shared_ptr<Mesh> getMesh() const;

    /// @brief Add state to the controller
    void addState(AnimationState* state);

    /// @property isPlaying
    bool isPlaying() const { return m_isPlaying; }
    void setPlaying(bool play) { m_isPlaying = play; }

    /// @brief Set shader uniforms
    void bindUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "AnimationController"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::AnimationController"; }
    /// @}

protected:

    friend class AnimationProcess;


    //--------------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @

    void initializeProcess();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @

    /// @brief Whether or not the controller is playing
    bool m_isPlaying = true;

    /// @brief The mesh being used by the controller
    std::shared_ptr<ResourceHandle> m_meshHandle;

    /// @brief The current animation state
    AnimationState* m_currentState = nullptr;

    /// @brief The process for running the animation controller
    std::shared_ptr<AnimationProcess> m_process;

    /// @brief Pointer to the core engine
    CoreEngine* m_engine;

    /// @brief Map of all animation states in the controller
    std::unordered_map<QString, AnimationState*> m_stateMap;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif