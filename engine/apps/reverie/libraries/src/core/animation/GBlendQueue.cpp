#include "core/animation/GBlendQueue.h"

#include "core/animation/GAnimStateMachine.h"
#include "core/animation/GAnimationController.h"
#include "core/animation/GAnimTransition.h"

#include "core/GCoreEngine.h"
#include <core/scene/GSceneObject.h>
#include <core/scene/GScene.h>
#include "core/resource/GResourceCache.h"
#include "core/resource/GResourceHandle.h"

#include "fortress/containers/math/GMatrix.h"
#include "heave/kinematics/GTransform.h"
#include "fortress/math/GInterpolation.h"
#include "fortress/thread/GParallelLoop.h"
#include "core/processes/GAnimationProcess.h"
#include "core/processes/GProcessManager.h"

#include "core/rendering/geometry/GMesh.h"
#include "core/rendering/geometry/GSkeleton.h"
#include "core/rendering/models/GModel.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/renderer/GRenderCommand.h"

namespace rev {




// BlendSet

void BlendSet::resize(uint32_t stride, uint32_t numJoints)
{
    m_stride = stride;
    m_numJoints = numJoints;
    m_translations.resize(stride * numJoints);
    m_rotations.resize(stride * numJoints);
    m_scales.resize(stride * numJoints);
}



// BlendQueue

BlendQueue::BlendQueue(AnimationController* controller):
    m_controller(controller)
{
}

BlendQueue::~BlendQueue()
{
}

void BlendQueue::updateCurrentClips(double processTimeSec, Skeleton* skeleton)
{
    m_currentPlayData.clear();
    m_untimedClipWeights.clear();
    m_slerpWeights.clear();

    // TODO: Use motion times to create a vector of "local" times to play each animation
    for (const Motion& motion : m_controller->motions()) {
        BaseAnimationState& baseState = *motion.currentState();

        switch (baseState.stateType()) {
        case AnimationStateType::kTransition: {
            AnimationTransition& transition = baseState.as<AnimationTransition>();
            transition.getClips(motion, m_currentPlayData, m_untimedClipWeights, &ResourceCache::Instance());
            break;
        }
        case AnimationStateType::kAnimation:
        {
            AnimationState& state = baseState.as<AnimationState>();
            for (const AnimationClip& clip : state.clips()) {
                // Check if clip has animation handle loaded yet
                // If no animation handle, was not created at time of AnimationClip::loadFromJson, so try to load again
                bool hasHandle = clip.verifyLoaded(&ResourceCache::Instance());

                if (!hasHandle) {
                    // Skip if handle not yet loaded
                    continue;
                }

                //Logger::LogInfo(clip.animationHandle()->getName() + std::to_string(motion.elapsedTime()).c_str());

                Vec::EmplaceBack(m_currentPlayData,
                    clip.animationHandle(),
                    clip.settings(),
                    state.playbackMode(),
                    motion.timer(),
                    0);
                m_untimedClipWeights.push_back(clip.settings().m_blendWeight); // TODO: Scale by state or transition's blend weighhts
            }
            break;
        }
        default:
            Logger::Throw("unimplemented");
        }
    }

    // Update the size of the current blend set
    uint32_t numClips = (uint32_t)m_currentPlayData.size();
    if (numClips) {
        uint32_t numJoints = (uint32_t)skeleton->nodes().size();
        m_blendSet.resize(numClips, numJoints);
    }
    
}

void BlendQueue::updateAnimationFrame(float timeInSec, Skeleton* skeleton, std::vector<Matrix4x4g>& outTransforms)
{
    // Generate new animation frame data
    //AnimationFrameData data;
    blendAnimations(timeInSec);

    // Add transforms to vector to be used as a uniform
    // Resizing instead of reserving, because (I think) a clear every step would be a bit more expensive
    if (!outTransforms.size()) {
        uint32_t numBones = skeleton->numBones();
#ifdef DEBUG_MODE
        if (!numBones) {
            Logger::Throw("Error, no bones in skeleton");
        }
#endif
        outTransforms.resize(numBones);
    }


    // Use skeleton to process node hierarchy after animation blend
    processBoneHierarchy(skeleton, outTransforms);
}

void BlendQueue::updateClipWeights()
{
    uint32_t size = (uint32_t)m_currentPlayData.size();
    m_clipWeights = m_untimedClipWeights;
    for (uint32_t i = 0; i < size; i++) {
        const AnimationPlayData& playData = m_currentPlayData[i];
        if (playData.isFadingIn()) {
            // Get fade-in blend weight, and scale
            float transitionTime = playData.m_transitionTimer.getElapsed<float>();
            const TransitionPlayData& t = playData.m_transitionData;
            float fadeInWeight = (transitionTime - t.m_totalTime + t.m_fadeInTime) / t.m_fadeInTime;
            m_clipWeights[i] *= std::max(0.0f, fadeInWeight);
        }
        else if (playData.isFadingOut()) {
            // Get fade-out blend weight, and scale
            float transitionTime = playData.m_transitionTimer.getElapsed<float>();
            const TransitionPlayData& t = playData.m_transitionData;
            float fadeOutWeight = 1 - (transitionTime / t.m_fadeOutTime);
            m_clipWeights[i] *= std::max(0.0f, fadeOutWeight);
        }
    }

    normalizeClipWeights();
    calculateSlerpWeights();
}

void BlendQueue::processBoneHierarchy(Skeleton * skeleton, std::vector<Matrix4x4g>& outBoneTransforms)
{
    SkeletonJoint* root = skeleton->root();
    processBoneHierarchy(skeleton, *root, Transform::Identity().m_matrices, outBoneTransforms);
}

void BlendQueue::processBoneHierarchy(Skeleton * skeleton, SkeletonJoint & node, const TransformMatrices<Matrix4x4>& parentTransform, std::vector<Matrix4x4g>& outBoneTransforms)
{
    // By default, use local node transform
    TransformMatrices<Matrix4x4>* localTransform = &node.transform();
    std::vector<Transform>& blendedTransforms = m_localTransforms;

    if (!blendedTransforms.size()) {
        Logger::LogWarning("No transforms found for blend pose");
        return;
    }

    // If node has an animation use that transform
    //const QString& nodeName = node.getName();
    if (node.isAnimated()) {
        Transform& localTrans = blendedTransforms[node.skeletonTransformIndex()];
        localTrans.computeLocalMatrix();
        localTransform = &localTrans.m_matrices;
    }

    // Get global transform of the node
    localTransform->worldMatrix() =
        std::move(parentTransform.worldMatrix() *
            localTransform->m_localMatrix);

    // Only add to transforms list if this node has a bone
    // Transforms of nodes without bones only affect the hierarchy of transforms, 
    // they are not send to the animation shader
    /// \see https://www.youtube.com/watch?v=F-kcaonjHf8&list=PLRIWtICgwaX2tKWCxdeB7Wv_rTET9JtWW&index=2 (starting at ~3:00)
    if (node.hasBone()) {
        uint idx = node.bone().m_index;
        //Vec::Replace<Matrix4x4g>(outBoneTransforms.begin() + idx, 
        //    localTransform->worldMatrix());
        outBoneTransforms[idx] = localTransform->worldMatrix();
    }

    // Call recursively for all children
    for (uint32_t child : node.children()) {
        processBoneHierarchy(skeleton, skeleton->getNode(child), *localTransform, outBoneTransforms);
    }
}

void BlendQueue::blendAnimations(float timeInSec)
{
    m_localTransforms.clear();

    // Return if no clips
    uint32_t numClips = (uint32_t)m_currentPlayData.size();
    if (!numClips) { 
        return; 
    }

    // Iterate through animation clips to obtain frames for each clip
    bool isDone = true;
    BlendSet& blendSet = m_blendSet;
    for (uint32_t i = 0; i < numClips; i++) {
        const AnimationPlayData& playData = m_currentPlayData[i];

        // Skip if no animation loaded
        Animation* anim = playData.m_animationHandle->resourceAs<Animation>();
        if (!anim) {
            continue;
        }

        // Get the poses from the animation clip
        bool clipDone;
        //clip.getAnimationFrame(timeInSec, clipDone, blendSet, i);
        float elapsedSec = playData.m_timer.getElapsed<float>();
        anim->getAnimationFrame(
            elapsedSec,
            playData.m_settings,
            playData.m_playbackMode,
            clipDone,
            blendSet,
            i);
        isDone &= clipDone;
    }

    // Blend animations if there are more than one, otherwise return the first clip's animation
    uint32_t numTransforms = blendSet.numJoints();
    uint32_t stride = blendSet.stride();
    if (stride == 1) {
        // TODO: Don't use transform at all, use static methods to process hierarchy directly from translation, rotation quaternion, and scale
        for (uint32_t i = 0; i < numTransforms; i++) {
            Vec::EmplaceBack(m_localTransforms, blendSet.m_translations[i], blendSet.m_rotations[i], blendSet.m_scales[i], false);
        }
    }
    else {
        const std::vector<float>& clipWeights = m_clipWeights;
        const std::vector<float>& slerpWeights = m_slerpWeights;
        const std::vector<Vector3>& translations = blendSet.m_translations;
        const std::vector<Quaternion>& rotations = blendSet.m_rotations;
        const std::vector<Vector3>& scales = blendSet.m_scales;
        uint32_t numEntries = numTransforms * stride;
        for (uint32_t i = 0; i < numEntries; i+=stride) {
            // Necessary to avoid move and destruction
            Vec::EmplaceBack(m_localTransforms,
                Interpolation::lerp(translations.data() + i, clipWeights).asReal(),
                Quaternion::Slerp(rotations.data() + i, slerpWeights), // Best combo of looks and  performance
                //Quaternion::EigenAverage(rotations.data() + i, clipWeights), // Looks great, but is way too slow
                //Quaternion::Average(rotations.data() + i, clipWeights), // Fastest, but has artifacts
                Interpolation::lerp(scales.data() + i, clipWeights).asReal(),
                false);

        }
    }

#ifdef DEBUG_MODE
    if (m_localTransforms.size() != m_blendSet.numJoints()) {
        Logger::Throw("Error, incorrect number of local transforms found");
    }
#endif
}

void BlendQueue::normalizeClipWeights()
{
    // Normalize blend weights
    uint32_t numClips = (uint32_t)m_currentPlayData.size();
    float blendMag = std::accumulate(m_clipWeights.begin(), m_clipWeights.end(), 0.0f);
    float blendMagInv = 1.0 / blendMag;
    for (uint32_t i = 0; i < numClips; i++) {
        m_clipWeights[i] = m_clipWeights[i] * blendMagInv;
    }
}

void BlendQueue::calculateSlerpWeights()
{
    // Calculate slerp weights
    // Slerp weights scale each weight to account for successive slerping of multiple quaternions
    uint32_t numClips = (uint32_t)m_currentPlayData.size();
    float denom = m_clipWeights[0];
    for (uint32_t i = 1; i < numClips; i++) {
        float currentClipWeight = m_clipWeights[i];
        denom += currentClipWeight;
        m_slerpWeights.push_back(currentClipWeight / denom); // TODO: Don't have to do this for last iteration, since denom == 1
    }
}






} // End namespaces
