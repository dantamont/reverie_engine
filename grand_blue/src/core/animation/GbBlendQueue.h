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

#ifndef GB_BLEND_QUEUE_H
#define GB_BLEND_QUEUE_H

// std
#include <vector>

// Internal
#include "../geometry/GbMatrix.h"
#include "../geometry/GbTransform.h"
#include "GbAnimation.h"

namespace Gb {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class Skeleton;
class SkeletonJoint;
class AnimationController;
class AnimationClip;
struct AnimationFrameData;
class ResourceHandle;
class Motion;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class BlendSet
struct BlendSet {

    size_t stride() const {
        return m_stride;
    }
    size_t numJoints() const {
        return m_numJoints;
    }

    /// @brief Resize with a stride and number of joints
    void resize(size_t stride, size_t numJoints);

    std::vector<Vector3> m_translations;
    std::vector<Quaternion> m_rotations;
    std::vector<Vector3> m_scales;

private:
    /// @brief Stride of each vector, aka, number of animations to blend
    /// @details Translations, rotations, and scales are stored in clusters to be averaged together
    size_t m_stride = 1;
    size_t m_numJoints = 0;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class BlendQueue
class BlendQueue {
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    BlendQueue(AnimationController* controller);
    ~BlendQueue();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief The transforms reepresenting the current animation frame, after blending and processing
    //const std::vector<Matrix4x4g>& transforms() { return m_transforms; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Update list of currently playing animations from active motions
    void updateCurrentClips(double processTimeSec, Skeleton* skeleton);

    /// @brief Obtain the transforms for the current frame of the animation
    void updateAnimationFrame(float timeInSec, Skeleton* skeleton, std::vector<Matrix4x4g>& outTransforms);

    /// @brief Update weights for currently playing clips based on transitions
    void updateClipWeights();

    /// @}

private:
    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @brief Iterate over node hierarchy to generate a skeletal pose for an animation frame
    void processBoneHierarchy(Skeleton* skeleton, std::vector<Matrix4x4g>& outBoneTransforms);


    void processBoneHierarchy(Skeleton* skeleton, SkeletonJoint& node, const TransformMatrices& parentTransform,
        std::vector<Matrix4x4g>& outBoneTransforms);

    /// @brief Blend all sets of animation frame data to obtain a vector of final local transforms
    void blendAnimations(float timeInSec);

    /// @brief Normalize the clip weights
    void normalizeClipWeights();

    /// @brief Calculate SLERP weights from clip weights
    void calculateSlerpWeights();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    AnimationController* m_controller;

    /// @brief Current clips being played
    std::vector<AnimationPlayData> m_currentPlayData;

    /// @brief The normalized blend weights of each of the current clips
    std::vector<float> m_clipWeights;

    /// @brief Weights for all clips, assuming no transitions are taking place
    std::vector<float> m_untimedClipWeights;

    /// @brief the weights used to perform slerp over multiple quaternions
    std::vector<float> m_slerpWeights;

    /// @brief Container for blending animations
    BlendSet m_blendSet;

    /// @brief Blend sets, indexed by order found in skeleton via breadth-first-search
    /// @details Each set of skeleton pose data represents a frame of an animation to be blended together
    //std::vector<AnimationFrameData> m_blendFrames;

    /// @brief The weights used to scale each set of animation frame data
    //std::vector<float> m_weights;

    /// @brief Vector of all local transforms (both bones and non-bones), used for processing node hierarchy
    std::vector<Transform> m_localTransforms;

    /// @brief Final vector of matrix transformations, indexed by order found in skeleton via breadth-first-search
    //std::vector<Matrix4x4g> m_transforms;

    /// @brief The number of node animations that don't correspond to bones
    //size_t m_numNonBones = 0;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif