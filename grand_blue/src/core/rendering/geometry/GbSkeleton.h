#ifndef GB_SKELETON_H
#define GB_SKELETON_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Standard

// QT

// Internal
#include "GbBuffers.h"
#include "../../resource/GbResource.h"
#include "../../geometry/GbMatrix.h"
#include "../../animation/GbAnimation.h"
#include "../../containers/GbContainerExtensions.h"
#include "../../containers/GbString.h"
#include "../../geometry/GbCollisions.h"

namespace Gb {
class ObjReader;

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class Model;
class Skeleton;
class CoreEngine;
class Mesh;
class VertexArrayData;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class Bone
/// See: https://gamedev.stackexchange.com/questions/26382/i-cant-figure-out-how-to-animate-my-loaded-model-with-assimp
class Bone {
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    Bone();
    Bone(int index);
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    Bone& operator=(const Bone& other);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Members
    /// @{
    friend class Mesh;

    /// @brief Index of the bone in skeleton's vector of bone nodes
    int m_index = -1;

    /// @brief the bone offset matrix (inverse bind pose transform), 
    /// @details defines conversion from mesh space to local bone space from mesh space
    // Local space (or bone space) is in relation to parent joint
    // Model space is in relation to model's origin
    // The bind (pose) transform is the position and orientation of the joint in model space, i.e.,
    // the bind pose is the T-pose of the skeleton, and the bind pose matrix of a joint describes the joint
    // transform in T-pose
    // The inverse bind pose transform is the inverse of this, so it brings the bone position and
    // orientation to the origin
    Matrix4x4g m_invBindPose; 

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class SkeletonJoint:
class SkeletonJoint : public Object {
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum JointFlag {
        kIsAnimated = 1 << 0
    };
    typedef QFlags<JointFlag> JointFlags;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    SkeletonJoint(const GString& uniqueName);
    SkeletonJoint();
    ~SkeletonJoint();
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Methods
    /// @{

    /// @brief Get index in parent skeleton
    size_t getIndex(const Skeleton& skeleton) const;

    /// @brief Whether the joint has the specified child index
    bool hasChild(const size_t& child) const {
        for (const size_t& c : m_children) {
            if (c == child) {
                return true;
            }
        }

        return false;
    }

    /// @brief Whether or not the node has a bone or not
    bool hasBone() const { return m_bone.m_index >= 0; }

    /// @brief Whether or not the node is animated
    bool isAnimated() const;
    void setAnimated(bool animated);

    /// @brief Siblings of this node, including this node
    const std::vector<size_t>& siblings(const Skeleton& skeleton) const { return parent(skeleton)->children(); }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties 
    /// @{

    /// @brief The index associated with this joint in vector of skeletal animation transforms
    int skeletonTransformIndex() const {
        return m_skeletonTransformIndex;
    }
    void setSkeletonTransformIndex(int idx) {
        m_skeletonTransformIndex = idx;
    }

    /// @brief Get parent joint, given the skeleton
    SkeletonJoint* parent(Skeleton& skeleton);
    const SkeletonJoint* parent(const Skeleton& skeleton) const;

    const TransformMatrices& getTransform() const { return m_transform; }
    TransformMatrices& transform() { return m_transform; }

    /// @brief The bone corresponding to this node
    const Bone& bone() const { return m_bone; }
    Bone& bone() { return m_bone; }

    /// @brief Children
    std::vector<size_t>& children() { return m_children; }
    const std::vector<size_t>& children() const { return m_children; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "SkeletonJoint"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::SkeletonJoint"; }
    /// @}

private:
    //---------------------------------------------------------------------------------------
    /// @name Friends

    friend class CubeMap;
    friend class ObjReader;
    friend class ModelReader;
    friend class Animation;
    friend class NodeAnimation;
    friend class Skeleton;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private methods 
    /// @{

    /// @brief Initialize the joint with the specified parameters
    /// @note Included to avoid difficulties with recursive construction
    /// @param[in] copyNode the node to copy attributes from onto this one
    /// @param[in] otherSkeleton the skeleton that copyNode belongs to
    /// @param[in] skeleton the skeleton that this node belongs to
    /// @param[in] parentIndx the index of this node in its parent skeleton's m_nodes list
    /// @param[in] uninitializedNodes a vector of nodes of the parent skeleton that have yet to be assigned
    /// @param[in] justBones whether or not to limit child copy to those with bones
    void initializeFromJoint(const SkeletonJoint& copyNode,
        const Skeleton& otherSkeleton,
        Skeleton& skeleton, 
        size_t parentIndex, 
        std::vector<size_t>& uninitializedNodes,
        bool justBones = false);


    //void addChild(SkeletonJoint* node);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private members 
    /// @{

    /// @brief Bone corresponding to this mesh node
    Bone m_bone;

    /// @brief The index associated with this joint in vector of skeletal animation transforms
    int m_skeletonTransformIndex;

    /// @brief Whether or not the joint has animation data associated with it
    size_t m_jointFlags = 0;

    /// @brief Transform of the node
    /// FIXME: Actually implement this in rendering
    TransformMatrices m_transform;

    /// @brief index of  the parent node in skeleton's node list
    int m_parent = -1;

    /// @brief Direct children of this mesh node, stored as indices in skeleton's node array
    std::vector<size_t> m_children;


    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class Skeleton
class Skeleton : public Resource {
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{

    /// @brief Skeleton name is name of root node
    Skeleton(const GString& uniqueName);
    Skeleton(const Skeleton& other);
    Skeleton(const Skeleton& other, bool justBones);
    ~Skeleton();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methhods
    /// @{

    const AABB& boundingBox() { return m_boundingBox; }

    /// @brief Get the type of resource stored by this handle
    virtual Resource::ResourceType getResourceType() const override {
        return Resource::kSkeleton;
    }

    void onRemoval(ResourceCache* cache = nullptr) override;

    /// @brief Whether or not the skeleton is malformed for inverse kinematics
    bool isMalformed() const;

    const SkeletonJoint* root() const { 
        return m_nodes.size() ? &m_nodes[0]: nullptr;
    }
    SkeletonJoint* root() { 
        return m_nodes.size() ? &m_nodes[0] : nullptr;
    }

    const std::vector<Matrix4x4g>& inverseBindPose() const { return m_inverseBindPose; }

    const Matrix4x4g& globalInverseTransform() const { return m_globalInverseTransform; }
    void setGlobalInverseTransform(const Matrix4x4g& mat) { m_globalInverseTransform = mat; }

    /// @brief whether or not the skeleton has a root node
    bool hasRoot() const { return m_nodes.size() > 0; }

    /// @brief Add root node if doesn't exist
    void addRootNode(const GString& name);

    /// @brief Get the vector of nodes that have bones
    const std::vector<size_t>& boneNodes() const { return m_boneNodes; }

    /// @brief Get the number of bones in the skeleton
    size_t numBones() const { return m_boneNodes.size(); }

    /// @brief Get the vector of nodes in the skeleton
    const std::vector<SkeletonJoint>& nodes() const { return m_nodes; }

    /// @brief Return hierarchy with just bones (root may not have a bone)
    Skeleton prunedBoneSkeleton() const;

    /// @brief Get the node with the given name
    const SkeletonJoint& getNode(const GString& name) const;
    SkeletonJoint& getNode(const GString& name);
    SkeletonJoint& getNode(const size_t& idx);
    const SkeletonJoint& getNode(const size_t& idx) const;

    /// @brief Add a child node to the node at the specified index
    /// @return Returns the index of the child node in m_nodes
    size_t addChild(size_t parentIndex, const GString& uniqueName);

    /// @brief Return default pose of the skeleton
    /// @details This is just a list of identity matrices of the proper length.
    /// @note To get the bind pose of the skeleton, inverse the inverseBindTransform for each bone in the vector
    void identityPose(std::vector<Matrix4x4g>& outPose) const;

    /// @brief Generate the bounding box for the skeleton
    void generateBoundingBox();

    /// @}

private:
    //---------------------------------------------------------------------------------------
    /// @name Friends
    friend class SkeletonJoint;
    friend class ModelReader;
    friend class Animation;
    friend class NodeAnimation;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private methods
    
    /// @brief Recursive function to construct inverse bind pose
    void constructInverseBindPose();
    void constructInverseBindPose(const size_t& nodeIndex);

    /// @brief Recursive function for determining if skeleton is malformed
    bool isMalformed(const SkeletonJoint& node) const;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private members 
    /// @{

    /// @brief global inverse transform of the skeleton
    Matrix4x4g m_globalInverseTransform;

    /// @brief Map of all joints
    std::vector<SkeletonJoint> m_nodes;

    /// @brief Vector of joints with bones, stored as index in m_nodes vector
    std::vector<size_t> m_boneNodes;

    /// @brief Inverse bind pose of the skeleton
    std::vector<Matrix4x4g> m_inverseBindPose;

    /// @brief The AABB corresponding to the bind pose (t-pose) of the skeleton
    AABB m_boundingBox;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif