#include "GbAnimation.h"

#include "../GbCoreEngine.h"
#include "../resource/GbResourceCache.h"
#include "../resource/GbResource.h"

#include "../geometry/GbMatrix.h"
#include "../geometry/GbTransform.h"
#include "../utils/GbInterpolation.h"
#include "../utils/GbParallelization.h"
#include "../processes/GbAnimationProcess.h"
#include "../processes/GbProcessManager.h"

#include "../rendering/geometry/GbMesh.h"
#include "../rendering/geometry/GbSkeleton.h"
#include "../rendering/models/GbModel.h"
#include "../rendering/shaders/GbShaders.h"
#include "../rendering/renderer/GbRenderCommand.h"

#include "../animation/GbBlendQueue.h"

//#define NUMBER_OF_ANIMATION_THREADS 3
#define NUMBER_OF_ANIMATION_THREADS std::thread::hardware_concurrency() - 2

#define USE_THREADING false

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// AnimationSettings
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue AnimationSettings::asJson() const
{
    QJsonObject object;
    object["speedFactor"] = m_speedFactor;
    object["blendWeight"] = m_blendWeight;
    object["tickOffset"] = m_tickOffset;
    object["timeOffsetSec"] = m_timeOffsetSec;
    object["numPlays"] = m_numPlays;

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationSettings::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    QJsonObject object = json.toObject();
    m_speedFactor = object["speedFactor"].toDouble();
    m_blendWeight = object["blendWeight"].toDouble();
    m_tickOffset = object["tickOffset"].toInt();
    m_timeOffsetSec = object["timeOffsetSec"].toDouble();
    m_numPlays = json["numPlays"].toInt(-1);
}


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// AnimationPlayData
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationPlayData::AnimationPlayData(const std::shared_ptr<ResourceHandle>& animHandle,
    const AnimationSettings & settings, const AnimationPlaybackMode & mode, const Timer& timer,
    size_t statusFlags) :
    m_animationHandle(animHandle),
    m_settings(settings),
    m_playbackMode(mode),
    m_timer(timer),
    m_statusFlags(statusFlags)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationPlayData::~AnimationPlayData()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// NodeAnimation 
/////////////////////////////////////////////////////////////////////////////////////////////
NodeAnimation::NodeAnimation()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
NodeAnimation::~NodeAnimation()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void NodeAnimation::interpolate(size_t first, 
    size_t second,
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



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Animation
/////////////////////////////////////////////////////////////////////////////////////////////
ThreadPool Animation::ANIMATION_THREADPOOL(NUMBER_OF_ANIMATION_THREADS);
/////////////////////////////////////////////////////////////////////////////////////////////
//Animation::Animation()
//{
//}
/////////////////////////////////////////////////////////////////////////////////////////////
Animation::Animation(const GString & uniqueName):
    Resource(uniqueName)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Animation::~Animation()
{
    //// Delete node animations
    //for (const std::pair<QString, NodeAnimation*> animPair : m_nodeAnimations) {
    //    delete animPair.second;
    //}
}
/////////////////////////////////////////////////////////////////////////////////////////////
float Animation::getTimeDuration() const
{
    return m_durationInTicks * (1.0/m_ticksPerSecond);
}
/////////////////////////////////////////////////////////////////////////////////////////////
float Animation::getRate() const
{
    return 1.0f / getTimeDuration();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Animation::getAnimationFrame(float timeInSec,
    const AnimationSettings& settings,
    AnimationPlaybackMode mode,
    bool& isDonePlaying,
    BlendSet& outFrames,
    size_t clipIndex)
{
    // Get timing info
    float animationTime = getAnimationTime(timeInSec, settings, mode, isDonePlaying);

    // Interpolate to get skeletal pose
    getInterpolatedFrame(animationTime, outFrames, clipIndex);

    isDonePlaying = false;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Animation::onRemoval(ResourceCache * cache)
{
    Q_UNUSED(cache)
}
/////////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////
void Animation::getInterpolatedFrame(float animationTime, BlendSet& outPose, size_t offset) const
{
    // No keyFrames, so raise error
    if (m_nodeAnimations.size() == 0) {
#ifdef DEBUG_MODE
        throw("Error, no keyframes found");
#endif
    }
    
    size_t frameIndex = getFrameIndex(animationTime);
    size_t nextIndex = (frameIndex + 1);
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
    size_t stride = outPose.stride();
    for (size_t i = 0; i < nodeAnimations.size(); i++) {
        // Index in blendset to add node animation
        size_t idx = i * stride + offset;

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
/////////////////////////////////////////////////////////////////////////////////////////////
size_t Animation::getFrameIndex(float time) const
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
/////////////////////////////////////////////////////////////////////////////////////////////
//void Animation::addNodeAnimation(NodeAnimation * anim)
//{
//    anim->m_animation = this;
//    m_nodeAnimations.emplace(anim->m_nodeName, anim);
//}



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Single Animation
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationClip::AnimationClip(CoreEngine* engine, const QJsonValue & json):
    Object(json.toObject()["name"].toString())
{
    loadFromJson(json, { engine });
}
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationClip::AnimationClip(const GString& name, const std::shared_ptr<ResourceHandle>& animation) :
    Object(name),
    m_animationHandle(animation)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationClip::~AnimationClip()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationClip::setDuration(float secs)
{
    float dur = animation()->getTimeDuration();
    m_settings.m_speedFactor = dur / secs;
}
///////////////////////////////////////////////////////////////////////////////////////////////
//void AnimationClip::getAnimationFrame(float timeInSec, bool& isDone, BlendSet& outFrames,
//    AnimationPlaybackMode playbackMode,
//    size_t clipIndex) const
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
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue AnimationClip::asJson() const
{
    QJsonObject object;
    
    const GString* animName;
    if (!m_animationHandle) {
        animName = &m_animationHandleName;
    }
    else {
        animName = &m_animationHandle->getName();
    }

    if (!m_name.isEmpty()) {
        object["name"] = m_name.c_str();
    }
    else {
        object["name"] = animName->c_str();
    }
    object["animation"] = animName->c_str();
    object["settings"] = m_settings.asJson();

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationClip::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    if (context.m_engine) {
        // FIXME: Causing a race condition, need thread-safe maps for retrieving handles
        QString animationHandleName = json["animation"].toString();
        m_animationHandle = context.m_engine->resourceCache()->getHandleWithName(animationHandleName,
            Resource::kAnimation);
        if (!m_animationHandle) {
            m_animationHandleName = animationHandleName;
        }
    }
    else {
        logWarning("Warning, no engine pointer found for animation clip");
    }
    m_name = json["name"].toString();
    m_settings.loadFromJson(json["settings"]);
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Animation> AnimationClip::animation() const
{
    if (!m_animationHandle) {
        return nullptr;
    }
    else if (m_animationHandle->resource(false)) {
        return std::static_pointer_cast<Animation>(m_animationHandle->resource(false));
    }
    else {
        return nullptr;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
