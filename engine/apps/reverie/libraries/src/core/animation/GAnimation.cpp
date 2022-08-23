#include "core/animation/GAnimation.h"

#include "core/GCoreEngine.h"
#include "core/resource/GResourceCache.h"
#include "core/resource/GResource.h"

#include "fortress/containers/math/GMatrix.h"
#include "fortress/containers/math/GTransform.h"
#include "fortress/math/GInterpolation.h"
#include "fortress/thread/GParallelLoop.h"
#include "core/processes/GAnimationProcess.h"
#include "core/processes/GProcessManager.h"

#include "core/rendering/geometry/GMesh.h"
#include "core/rendering/geometry/GSkeleton.h"
#include "core/rendering/models/GModel.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/renderer/GRenderCommand.h"

#include "core/animation/GBlendQueue.h"

//#define NUMBER_OF_ANIMATION_THREADS 3
#define NUMBER_OF_ANIMATION_THREADS std::thread::hardware_concurrency() - 2

#define USE_THREADING false

namespace rev {


// AnimationSettings

void to_json(json& orJson, const AnimationSettings& korObject)
{
    orJson["speedFactor"] = korObject.m_speedFactor;
    orJson["blendWeight"] = korObject.m_blendWeight;
    orJson["tickOffset"] = korObject.m_tickOffset;
    orJson["timeOffsetSec"] = korObject.m_timeOffsetSec;
    orJson["numPlays"] = korObject.m_numPlays;
}

void from_json(const json& korJson, AnimationSettings& orObject)
{
    korJson["speedFactor"].get_to(orObject.m_speedFactor);
    korJson["blendWeight"].get_to(orObject.m_blendWeight);
    korJson["tickOffset"].get_to(orObject.m_tickOffset);
    korJson["timeOffsetSec"].get_to(orObject.m_timeOffsetSec);
    korJson["numPlays"].get_to(orObject.m_numPlays);
}




// AnimationPlayData

AnimationPlayData::AnimationPlayData(const std::shared_ptr<ResourceHandle>& animHandle,
    const AnimationSettings & settings, const AnimationPlaybackMode & mode, const StopwatchTimer& timer,
    uint32_t statusFlags) :
    m_animationHandle(animHandle),
    m_settings(settings),
    m_playbackMode(mode),
    m_timer(timer),
    m_statusFlags(statusFlags)
{
}

AnimationPlayData::~AnimationPlayData()
{
}



// NodeAnimation 

NodeAnimation::NodeAnimation()
{
}

NodeAnimation::~NodeAnimation()
{
}

void NodeAnimation::interpolate(uint32_t first, 
    uint32_t second,
    float weight, 
    AnimationFrameData & out) const
{
    Vec::EmplaceBack(out.m_translations,
            Interpolation::lerp(m_translations.at(first),
                m_translations.at(second), 
                weight)
    );
    Vec::EmplaceBack(out.m_rotations, 
        Quaternion::Slerp(m_rotations.at(first), m_rotations.at(second), weight)
    );
    Vec::EmplaceBack(out.m_scales, 
        Interpolation::lerp(m_scales.at(first), m_scales.at(second), weight)
    );
}





// Animation

ThreadPool Animation::ANIMATION_THREADPOOL(NUMBER_OF_ANIMATION_THREADS);

//Animation::Animation()
//{
//}

Animation::Animation()
{
}
//Animation::Animation(const GString & uniqueName):
//    Resource(uniqueName)
//{
//}

Animation::~Animation()
{
    //// Delete node animations
    //for (const std::pair<QString, NodeAnimation*> animPair : m_nodeAnimations) {
    //    delete animPair.second;
    //}
}

float Animation::getTimeDuration() const
{
    return m_durationInTicks * (1.0/m_ticksPerSecond);
}

float Animation::getRate() const
{
    return 1.0f / getTimeDuration();
}

void Animation::getAnimationFrame(float timeInSec,
    const AnimationSettings& settings,
    AnimationPlaybackMode mode,
    bool& isDonePlaying,
    BlendSet& outFrames,
    uint32_t clipIndex)
{
    // Get timing info
    float animationTime = getAnimationTime(timeInSec, settings, mode, isDonePlaying);

    // Interpolate to get skeletal pose
    getInterpolatedFrame(animationTime, outFrames, clipIndex);

    isDonePlaying = false;
}

void Animation::onRemoval(ResourceCache *)
{
}

float Animation::getAnimationTime(float timeInSec, const AnimationSettings & settings, AnimationPlaybackMode mode, bool & isDonePlaying) const
{
    // Get timing info
    float ticksPerSec = m_ticksPerSecond != 0 ? m_ticksPerSecond : 25.0f;
    ticksPerSec *= settings.m_speedFactor;
    float timeInTicks = (timeInSec + settings.m_timeOffsetSec) * ticksPerSec + settings.m_tickOffset;
    float animationTime = fmod(timeInTicks, m_durationInTicks);
    int playCount = int(timeInTicks / m_durationInTicks);

    // Determine whether or not play count has been reached
    if (settings.m_numPlays > 0) {
        if (playCount >= settings.m_numPlays) {
            // If played more than max num allowable times, stop animating
            isDonePlaying = true;
            return animationTime;
        }
    }

    // Perform playback mode-specific functionality
    switch (mode) {
    case AnimationPlaybackMode::kSingleShot: {
        if (playCount > 1) {
            isDonePlaying = true;
            return animationTime;
        }
        break;
    }
    case AnimationPlaybackMode::kPingPong:
        if (playCount % 2 == 1) {
            // If is an odd play count, ping-pong the animation
            animationTime = m_durationInTicks - animationTime;
        }
        break;
    case AnimationPlaybackMode::kLoop:
    default:
        break;
    }

    isDonePlaying = false;
    return animationTime;
}

void Animation::getInterpolatedFrame(float animationTime, BlendSet& outPose, uint32_t offset) const
{
    // No keyFrames, so raise error
    if (m_nodeAnimations.size() == 0) {
#ifdef DEBUG_MODE
        Logger::Throw("Error, no keyframes found");
#endif
    }
    
    uint32_t frameIndex = getFrameIndex(animationTime);
    uint32_t nextIndex = (frameIndex + 1);
    float finalTime = m_times.back();

    float deltaTime;
    float weight;
    if (nextIndex < m_times.size()) {
        deltaTime = m_times[nextIndex] - m_times[frameIndex];
        weight = (animationTime - m_times[frameIndex]) / deltaTime;
    }

    // If there are no output frames generated
    //std::vector<AnimationFrameData>& outFrames = outPose.m_blendSets;
    const std::vector<NodeAnimation>& nodeAnimations = m_nodeAnimations;

    // Perform interpolation
    // Insert into blend set with appropriate stride
    uint32_t stride = outPose.stride();
    for (uint32_t i = 0; i < nodeAnimations.size(); i++) {
        // Index in blendset to add node animation
        uint32_t idx = i * stride + offset;

        const NodeAnimation& nodeAnimation = nodeAnimations[i];

        float numFrames = nodeAnimation.size();
        const std::vector<Vector3>& nodeTranslations = nodeAnimation.translations();
        const std::vector<Quaternion>& nodeRotations = nodeAnimation.rotations();
        const std::vector<Vector3>& nodeScales = nodeAnimation.scales();

        // Only one value, so cannot interpolate
        if (numFrames == 1) {
            outPose.m_translations[idx] = nodeTranslations[0];
            outPose.m_rotations[idx] = nodeRotations[0];
            outPose.m_scales[idx] = nodeScales[0];
            continue;
        }

        if (nextIndex >= numFrames) {
            // Don't interpolate if at last frame
            outPose.m_translations[idx] = nodeTranslations.back();
            outPose.m_rotations[idx] = nodeRotations.back();
            outPose.m_scales[idx] = nodeScales.back();
            continue;
        }

        // Get weight factor
        if (animationTime > finalTime) {
            outPose.m_translations[idx] = nodeTranslations.back();
            outPose.m_rotations[idx] = nodeRotations.back();
            outPose.m_scales[idx] = nodeScales.back();
        }
        else {
            // Interpolate the frame
            AnimationFrameData interpFrame;
            nodeAnimation.interpolate(
                frameIndex, nextIndex, weight, interpFrame);
            outPose.m_translations[idx] = std::move(interpFrame.m_translations.back());
            outPose.m_rotations[idx] = std::move(interpFrame.m_rotations.back());
            outPose.m_scales[idx] = std::move(interpFrame.m_scales.back());

        }
    }

}

uint32_t Animation::getFrameIndex(float time) const
{
    // Get the first iterator greater than this one
    auto it = std::upper_bound(m_times.begin(), m_times.end(), 
        time);

    // Don't decrement iterator if this is the first time
    if (it != m_times.begin()) {
        it -= 1;
    }
    return it - m_times.begin();
}

//void Animation::addNodeAnimation(NodeAnimation * anim)
//{
//    anim->m_animation = this;
//    m_nodeAnimations.emplace(anim->m_nodeName, anim);
//}





// Single Animation

AnimationClip::AnimationClip(const nlohmann::json& json):
    NameableInterface(json.at("name").get_ref<const std::string&>().c_str())
{
    json.get_to(*this);
}

AnimationClip::AnimationClip(const GString& name, const std::shared_ptr<ResourceHandle>& animation) :
    NameableInterface(name),
    m_animationHandle(animation)
{
}

AnimationClip::~AnimationClip()
{
}

void AnimationClip::setDuration(float secs)
{
    float dur = animation()->getTimeDuration();
    m_settings.m_speedFactor = dur / secs;
}

bool AnimationClip::verifyLoaded(ResourceCache* cache) const
{
    bool hasHandle = m_animationHandle != nullptr;
    if (!hasHandle) {
        if (m_animationHandleName.isEmpty()) {
            return false;
        }
        std::shared_ptr<ResourceHandle> animHandle = cache->getHandleWithName(m_animationHandleName, EResourceType::eAnimation);
        setAnimationHandle(animHandle);
        hasHandle = animHandle != nullptr;
    }

    return hasHandle;
}

//void AnimationClip::getAnimationFrame(float timeInSec, bool& isDone, BlendSet& outFrames,
//    AnimationPlaybackMode playbackMode,
//    uint32_t clipIndex) const
//{
//    if (!animation()) { 
//        return; 
//    }
//
//    // Get clip's animation frame, returning true if done playing
//    animation()->getAnimationFrame(
//        timeInSec, 
//        m_settings,
//        playbackMode,
//        isDone,
//        outFrames,
//        clipIndex);
//}

void to_json(nlohmann::json& orJson, const Animation& korObject)
{
    // Used only for widgets
    orJson["frameTimes"] = korObject.frameTimes();
    orJson["ticksPerSecond"] = korObject.m_ticksPerSecond;
}

void from_json(const nlohmann::json& korJson, Animation& orObject)
{
    // UNUSED
}

void to_json(json& orJson, const AnimationClip& korObject)
{
    
    const GString* animName;
    if (!korObject.m_animationHandle) {
        animName = &korObject.m_animationHandleName;
    }
    else {
        animName = &korObject.m_animationHandle->getName();
    }

    if (!korObject.m_name.isEmpty()) {
        orJson["name"] = korObject.m_name.c_str();
    }
    else {
        orJson["name"] = animName->c_str();
    }
    orJson["animation"] = animName->c_str();
    orJson["settings"] = korObject.m_settings;
    orJson["id"] = korObject.m_uuid; ///< @todo Remove, used for widgets only
}

void from_json(const json& korJson, AnimationClip& orObject)
{
    // FIXME: Causing a race condition, need thread-safe maps for retrieving handles
    GString animationHandleName;
    korJson["animation"].get_to(animationHandleName);
    orObject.m_animationHandle = ResourceCache::Instance().getHandleWithName(animationHandleName,
        EResourceType::eAnimation);
    if (!orObject.m_animationHandle) {
        orObject.m_animationHandleName = animationHandleName;
    }

    korJson["name"].get_to(orObject.m_name);
    korJson["settings"].get_to(orObject.m_settings);
}

Animation* AnimationClip::animation() const
{
    if (!m_animationHandle) {
        return nullptr;
    }
    else {
        return m_animationHandle->resourceAs<Animation>();
    }
}


} // End namespaces
