#include "GbAnimation.h"

#include "../GbCoreEngine.h"
#include "../resource/GbResourceCache.h"
#include "../resource/GbResource.h"

#include "../geometry/GbMatrix.h"
#include "../rendering/geometry/GbMesh.h"
#include "../rendering/geometry/GbSkeleton.h"
#include "../geometry/GbTransform.h"
#include "../utils/GbInterpolation.h"
#include "../utils/GbParallelization.h"
#include "../processes/GbAnimationProcess.h"
#include "../processes/GbProcessManager.h"
#include "../rendering/shaders/GbShaders.h"


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

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationSettings::loadFromJson(const QJsonValue & json)
{
    QJsonObject object = json.toObject();
    m_speedFactor = object["speedFactor"].toDouble();
    m_blendWeight = object["blendWeight"].toDouble();
    m_tickOffset = object["tickOffset"].toInt();
    m_timeOffsetSec = object["timeOffsetSec"].toDouble();
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// BlendPose 
/////////////////////////////////////////////////////////////////////////////////////////////
void BlendPose::blend()
{
    if (m_transforms.size()) {
        m_transforms.clear();
    }

    ParallelLoopGenerator loop(&Animation::ANIMATION_THREADPOOL, USE_THREADING);

    //size_t mapSize = m_blendSets.size();
    QReadWriteLock mutex;
    //loop.parallelFor(mapSize, [&](int start, int end) {
    //std::unordered_map<QString, BlendSet>::const_iterator it = std::next(m_blendSets.begin(), start);
    //std::unordered_map<QString, BlendSet>::const_iterator endIt = std::next(m_blendSets.begin(), end);
    //
    std::unordered_map<QString, BlendSet>::const_iterator it = m_blendSets.begin();
    std::unordered_map<QString, BlendSet>::const_iterator endIt = m_blendSets.end();
    for (it; it != endIt; it++) {
        const BlendSet& blendSet = it->second;
        const QString& nodeName = it->first;
        const std::vector<Vector3g>& translations = blendSet.m_translations;
        const std::vector<Quaternion>& rotations = blendSet.m_rotations;
        const std::vector<Vector3g>& scales = blendSet.m_scales;

        mutex.lockForWrite();
        // Necessary to avoid move and destruction
        // C++17 - Try emplace
        Map::Emplace(m_transforms, nodeName,
            Interpolation::lerp(translations, m_weights).asDouble(),
            Quaternion::average(rotations, m_weights),
            Interpolation::lerp(scales, m_weights).asDouble(),
            false);
        mutex.unlock();
    }
    //});
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
    BlendSet & out) const
{
    Vec::EmplaceBack(out.m_translations,
            Interpolation::lerp(m_translations.at(first),
                m_translations.at(second), 
                weight)
    );
    Vec::EmplaceBack(out.m_rotations, 
        Quaternion::slerp(m_rotations.at(first), m_rotations.at(second), weight)
    );
    Vec::EmplaceBack(out.m_scales, 
        Interpolation::lerp(m_scales.at(first), m_scales.at(second), weight)
    );
}




/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Skeleton Pose
/////////////////////////////////////////////////////////////////////////////////////////////
SkeletonPose::SkeletonPose()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
SkeletonPose::SkeletonPose(Mesh * mesh, BlendPose&& blendPose):
    m_blendPose(blendPose),
    m_mesh(mesh)
{
    m_blendPose.blend();
}

/////////////////////////////////////////////////////////////////////////////////////////////
SkeletonPose::~SkeletonPose()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void SkeletonPose::getBoneTransforms(std::vector<Matrix4x4g>& outBoneTransforms, 
    bool forceBlend)
{
    // Blend if not blended
    if (!m_blendPose.m_transforms.size() || forceBlend) {
        m_blendPose.blend();
    }

    // TODO: use a reserve and push back
    if (!outBoneTransforms.size()) {
        size_t numBones = m_blendPose.size() - m_blendPose.m_numNonBones;
        outBoneTransforms.resize(numBones);
    }
    MeshNode* root = m_mesh->skeleton().root();
    processNodeHierarchy(*root, IDENTITY, outBoneTransforms);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void SkeletonPose::processNodeHierarchy(MeshNode& node, 
    Transform & parentTransform,
    std::vector<Matrix4x4g>& outBoneTransforms)
{
    // By default, use local node transform
    Transform* localTransform = &node.transform();
    std::unordered_map<QString, Transform>& blendedTransforms = m_blendPose.m_transforms;

    // If node has an animation use that transform
    const QString& nodeName = node.getName();
    if (Map::HasKey(blendedTransforms, nodeName)) {
        localTransform = &blendedTransforms[nodeName];
        localTransform->computeLocalMatrix();
    }

    // Get global transform of the node
    localTransform->m_worldMatrix = 
        std::move(parentTransform.worldMatrix() * 
        localTransform->localMatrix());

    // Only add to transforms list if this node has a bone
    // Transforms of nodes without bones only affect the hierarchy of transforms, 
    // they are not send to the animation shader
    // See: https://www.youtube.com/watch?v=F-kcaonjHf8&list=PLRIWtICgwaX2tKWCxdeB7Wv_rTET9JtWW&index=2 (starting at ~3:00)
    if (node.hasBone()) {
        uint idx = node.bone().m_index;
        Vec::Replace<Matrix4x4g>(outBoneTransforms.begin() + idx, 
            localTransform->m_worldMatrix 
            //node.getBone().m_offsetMatrix
            );
    }

    for (MeshNode* child : node.children()) {
        processNodeHierarchy(*child, *localTransform, outBoneTransforms);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
QMutex SkeletonPose::POSE_MUTEX;
/////////////////////////////////////////////////////////////////////////////////////////////
Transform SkeletonPose::IDENTITY = Transform();



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
Animation::Animation(const QString & uniqueName):
    Resource(uniqueName, kAnimation)
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
void Animation::getAnimationFrame(Mesh * mesh,
    float timeInSec,
    const AnimationSettings& settings,
    AnimationPlaybackMode mode,
    int numPlays,
    bool& isDonePlaying,
    BlendPose& outFrames)
{
    Q_UNUSED(mesh);

    // Get timing info
    float animationTime = getAnimationTime(timeInSec, settings,
        mode, numPlays, isDonePlaying);

    // Interpolate to get skeletal pose
    interpolatePose(animationTime, outFrames);

    //// Process skeletal hierarchy to get world transforms
    //Transform identity;
    //pose.processNodeHierarchy(mesh->m_skeleton.m_root, identity);

    isDonePlaying = false;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Animation::onRemoval(ResourceCache * cache)
{
    Q_UNUSED(cache)
}
/////////////////////////////////////////////////////////////////////////////////////////////
float Animation::getAnimationTime(float timeInSec,
    const AnimationSettings & settings, AnimationPlaybackMode mode,
    int numPlays, bool & isDonePlaying) const
{
    // Get timing info
    float ticksPerSec = m_ticksPerSecond != 0 ? m_ticksPerSecond : 25.0f;
    ticksPerSec *= settings.m_speedFactor;
    float timeInTicks = (timeInSec + settings.m_timeOffsetSec) * ticksPerSec + settings.m_tickOffset;
    float animationTime = fmod(timeInTicks, m_durationInTicks);
    int playCount = int(timeInTicks / m_durationInTicks);

    // Determine whether or not play count has been reached
    if (numPlays > 0) {
        if (playCount >= numPlays) {
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
void Animation::interpolatePose(float animationTime, BlendPose& outPose) const
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
    std::unordered_map<QString, BlendSet>& outFrames = outPose.m_blendSets;
    const std::unordered_map<QString, NodeAnimation>& nodeAnimations = m_nodeAnimations;
    if (!outFrames.size()) {
        for (const std::pair<QString, NodeAnimation>& nodeAnimationPair: nodeAnimations) {
            outFrames.emplace(nodeAnimationPair.first, BlendSet());
        }
    }

    //ParallelLoopGenerator loop(&Animation::ANIMATION_THREADPOOL, USE_THREADING);

    //size_t mapSize = nodeAnimations.size();
    //loop.parallelFor(mapSize, [&](int start, int end) {
        //std::unordered_map<QString, NodeAnimation>::const_iterator it = std::next(nodeAnimations.begin(), start);
        //std::unordered_map<QString, NodeAnimation>::const_iterator endIt = std::next(nodeAnimations.begin(), end);
    std::unordered_map<QString, NodeAnimation>::const_iterator it = nodeAnimations.begin();
    std::unordered_map<QString, NodeAnimation>::const_iterator endIt = nodeAnimations.end();
    for (it; it != endIt; it++) {

        const NodeAnimation& nodeAnimation = it->second;
        const QString& nodeName = it->first;

        float numFrames = nodeAnimation.size();
        BlendSet& currentBlendSet = outFrames[nodeName];
        const std::vector<Vector3g>& nodeTranslations = nodeAnimation.translations();
        const std::vector<Quaternion>& nodeRotations = nodeAnimation.rotations();
        const std::vector<Vector3g>& nodeScales = nodeAnimation.scales();

        // Only one value, so cannot interpolate
        if (numFrames == 1) {
            Vec::EmplaceBack(currentBlendSet.m_translations, nodeTranslations[0]);
            Vec::EmplaceBack(currentBlendSet.m_rotations, nodeRotations[0]);
            Vec::EmplaceBack(currentBlendSet.m_scales, nodeScales[0]);
            continue;
        }

        if (nextIndex >= numFrames) {
            // Don't interpolate if at last frame
            Vec::EmplaceBack(currentBlendSet.m_translations, nodeTranslations.back());
            Vec::EmplaceBack(currentBlendSet.m_rotations, nodeRotations.back());
            Vec::EmplaceBack(currentBlendSet.m_scales, nodeScales.back());
            continue;
        }

        // Get weight factor
        if (animationTime > finalTime) {
            Vec::EmplaceBack(currentBlendSet.m_translations, nodeTranslations.back());
            Vec::EmplaceBack(currentBlendSet.m_rotations, nodeRotations.back());
            Vec::EmplaceBack(currentBlendSet.m_scales, nodeScales.back());
        }
        else {
            // Interpolate the frame
            // TODO: Reserve space in blend set
            BlendSet interpFrame;
            nodeAnimation.interpolate(
                frameIndex, nextIndex, weight, interpFrame);
            Vec::EmplaceBack(currentBlendSet.m_translations, std::move(interpFrame.m_translations.back()));
            Vec::EmplaceBack(currentBlendSet.m_rotations, std::move(interpFrame.m_rotations.back()));
            Vec::EmplaceBack(currentBlendSet.m_scales, std::move(interpFrame.m_scales.back()));

        }
    }

    //});


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
    Object(json.toObject()["name"].toString()),
    m_engine(engine)
{
    loadFromJson(json);
}
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationClip::AnimationClip(const QString& name, const std::shared_ptr<ResourceHandle>& animation,
    CoreEngine* engine) :
    Object(name),
    m_engine(engine),
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
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationClip::getAnimationFrame(Mesh * mesh, 
    float timeInSec, 
    BlendPose& outFrames)
{
    if (!animation()) return;

    if (outFrames.m_numNonBones == 0) {
        outFrames.m_numNonBones = numNonBones();
    }

    return animation()->getAnimationFrame(mesh,
        timeInSec, 
        m_settings,
        m_playbackMode, 
        m_numPlays, 
        m_isDone,
        outFrames);
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue AnimationClip::asJson() const
{
    QJsonObject object;
    
    if (!m_name.isEmpty()) {
        object["name"] = m_name;
    }
    else {
        object["name"] = m_animationHandle->getName();
    }
    object["animation"] = m_animationHandle->asJson();
    object["settings"] = m_settings.asJson();
    object["playbackMode"] = int(m_playbackMode);
    object["numPlays"] = m_numPlays;

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationClip::loadFromJson(const QJsonValue & json)
{
    if (m_engine) {
        QJsonObject animationHandle = json["animation"].toObject();
        m_animationHandle = m_engine->resourceCache()->getResourceHandle(animationHandle);
    }
    else {
        logWarning("Warning, no engine pointer found for animation clip");
    }
    m_name = json["name"].toString();
    m_settings.loadFromJson(json["settings"]);
    m_playbackMode = AnimationPlaybackMode(json["playbackMode"].toInt());
    m_numPlays = json["numPlays"].toInt();
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Animation> AnimationClip::animation() const
{
    if (m_animationHandle->resource(false)) {
        return std::static_pointer_cast<Animation>(m_animationHandle->resource(false));
    }
    else {
        return nullptr;
    }
}




/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Animation State
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationState::AnimationState(const QString& name, CoreEngine* engine):
    Object(name),
    m_engine(engine)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationState::~AnimationState()
{
    // Delete all animation clips
    for (const std::pair<QString, AnimationClip*>& animationPair : m_clips) {
        delete animationPair.second;
    }

    // Delete all animation layers
    for (const std::pair<QString, AnimationClip*>& layerPair : m_layers) {
        delete layerPair.second;
    }
    
    // Delete child
    if (m_child) {
        delete m_child;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationState::addClip(std::shared_ptr<ResourceHandle> animationHandle)
{
    m_clips.emplace(animationHandle->getName(), 
        new AnimationClip(animationHandle->getName(), animationHandle, m_engine));
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationState::addClip(AnimationClip * clip)
{
    m_clips.emplace(clip->getName(), clip);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationState::addLayer(AnimationClip * state)
{
    m_layers.emplace(state->getName(), state);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationState::getAnimationFrame(Mesh * mesh, 
    float timeInSec, bool& isDone,
    SkeletonPose& outSkeletalPose)
{
    // Set mesh
    outSkeletalPose.m_mesh = mesh;

    // Return if no clips
    if (!m_clips.size()) return;

    // Iterate through animations and blend if necessary
    // TODO: Convert flat list into a blend tree hierarchy, to more easily generate weights
    // See: https://www.gamasutra.com/view/feature/131863/animation_blending_achieving_.php
    BlendPose& outPose = outSkeletalPose.m_blendPose;
    std::vector<float>& normalizedBlendWeights = outPose.m_weights;
    outPose.m_blendSets.clear();
    normalizedBlendWeights.clear();
    normalizedBlendWeights.reserve(m_clips.size());
    for (const std::pair<QString, AnimationClip*>& clipPair : m_clips) {
        // Get normalized blend weight for the clip
        AnimationClip* clip = clipPair.second;
        float clipWeight = clip->settings().m_blendWeight;

        // Get the poses from the animation clip
        clip->getAnimationFrame(mesh,
            timeInSec,
            outPose);

        // Add weights to weight vector
        Vec::EmplaceBack(normalizedBlendWeights, clipWeight);
    }

    // Normalize blend weights
    float blendMag = std::accumulate(normalizedBlendWeights.begin(),
        normalizedBlendWeights.end(), 0.0f);
    float blendMagInv = 1.0 / blendMag;
    for (size_t i = 0; i < normalizedBlendWeights.size(); i++) {
        normalizedBlendWeights[i] = normalizedBlendWeights[i] * blendMagInv;
    }

    isDone = false;
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue AnimationState::asJson() const
{
    QJsonObject object;

    // Save layers to json
    QJsonObject layers;
    for (const std::pair<QString, AnimationClip*>& layerPair : m_layers) {
        layers.insert(layerPair.first, layerPair.second->asJson());
    }
    object.insert("layers", layers);

    // Save clips to json
    QJsonObject clips;
    for (const std::pair<QString, AnimationClip*>& clipPair : m_clips) {
        clips.insert(clipPair.first, clipPair.second->asJson());
    }
    object.insert("clips", clips);

    // Save child to json
    if (m_child) {
        object.insert("child", m_child->asJson());
    }


    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationState::loadFromJson(const QJsonValue & json)
{
    QJsonObject object = json.toObject();

    // Load layers
    QJsonObject layers = object["layers"].toObject();
	const QStringList& layerKeys = layers.keys();
    for (const QString& layerName: layerKeys) {
        QJsonObject layer = layers[layerName].toObject();
        addClip(new AnimationClip(m_engine, layer));
    }

    // Load clips
    QJsonObject clips = object["clips"].toObject();
    const QStringList& clipKeys = clips.keys();
    for (const QString& clipName : clipKeys) {
        QJsonObject clip = clips[clipName].toObject();
        addClip(new AnimationClip(m_engine, clip));
    }

    // Load child
    if (object.contains("child")) {
        QJsonObject child = object["child"].toObject();
        QString childName = child["name"].toString();
        m_child = new AnimationState(childName, m_engine);
        m_child->loadFromJson(child);
    }
}




/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// AnimationController
/////////////////////////////////////////////////////////////////////////////////////////////

AnimationController::AnimationController(CoreEngine * engine, const QJsonValue & json):
    m_engine(engine)
{
    loadFromJson(json);
}
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationController::AnimationController(CoreEngine * engine, const std::shared_ptr<ResourceHandle>& mesh):
    m_engine(engine),
    m_meshHandle(mesh)
{
    initializeProcess();
}
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationController::~AnimationController()
{
    for (const std::pair<QString, AnimationState*>& statePair : m_stateMap) {
        delete statePair.second;
    }

    // Abort the process for this animation
    if(!m_process->isAborted())
        m_process->abort();
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> AnimationController::getMesh() const
{
    if (m_meshHandle->resource(false)) {
        return std::static_pointer_cast<Mesh>(m_meshHandle->resource(false));
    }
    else {
        return nullptr;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationController::addState(AnimationState * state)
{
    if (Map::HasKey(m_stateMap, state->getName())) {
        logWarning("State map already had a state with name " + state->getName());
        delete m_stateMap[state->getName()];
    }
    m_stateMap[state->getName()] = state;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationController::bindUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram)
{
    Uniform transformUniform = Uniform("boneTransforms", m_process->m_transforms);

    std::shared_ptr<Mesh> mesh = getMesh();
    if (!mesh) return;

    // Don't save this value to the shader's cached values 
    // (this flag set may be unnecessary)
    transformUniform.setPersistence(false);

    // Set uniform that this model is animated
    shaderProgram->setUniformValue("isAnimated", true);

    // Set global inverse transform
    shaderProgram->setUniformValue("globalInverseTransform", mesh->skeleton().globalInverseTransform());

    // Set inverse bind poses
    shaderProgram->setUniformValue("inverseBindPoseTransforms", mesh->skeleton().inverseBindPose());

    // Set pose uniform value in the shader
    shaderProgram->setUniformValue(transformUniform, false);
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue AnimationController::asJson() const
{
    QJsonObject object;

    // Add the mesh used by this controller to JSON
    if (m_meshHandle) {
        object.insert("mesh", m_meshHandle->getName());
    }

    // Add the current state of this controller to JSON
    if (m_currentState) {
        object.insert("currentState", m_currentState->getName());
    }

    object.insert("isPlaying", m_isPlaying);

    // Add all states to json
    QJsonObject animationStates;
    for (const std::pair<QString, AnimationState*>& animPair : m_stateMap) {
        animationStates.insert(animPair.first, animPair.second->asJson());
    }
    object.insert("animationStates", animationStates);


    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationController::loadFromJson(const QJsonValue & json)
{
    QJsonObject object = json.toObject();

    // Set mesh from resource cache
    if (object.contains("mesh")) {
        QString meshName = object["mesh"].toString();
        m_meshHandle = m_engine->resourceCache()->getMesh(meshName);
    }

    // Load animation states from JSON
    if (object.contains("animationStates")) {
        QJsonObject animStates = object["animationStates"].toObject();
        for (const QString& animName : animStates.keys()) {
            AnimationState* state = new AnimationState(animName, m_engine);
            state->loadFromJson(animStates[animName]);
            m_stateMap[animName.toLower()] = state;
        }
    }

    // Set the current state
    if (object.contains("currentState")) {
        QString stateName = object["currentState"].toString();
        m_currentState = m_stateMap[stateName];
    }

    m_isPlaying = object["isPlaying"].toBool();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationController::initializeProcess()
{
    m_process = std::make_shared<AnimationProcess>(m_engine, this, m_engine->processManager());

    // Add process for this animation to the process manager queue
    m_engine->processManager()->attachProcess(m_process);
}



















/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
