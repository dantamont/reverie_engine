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
class Bone : public Object {
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    Bone();
    Bone(const QString& name, int index);
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Methods
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Members
    /// @{
    friend class Mesh;

    int m_index = -1;

    /// @brief the bone offset matrix (inverse bind pose transform), 
    /// @details defines conversion from mesh space to local bone space from mesh space
    // Local space (or bone space) is in relation to parent joint
    // Model space is in relation to model's origin
    // The bind (pose) transform is the position and orientation of the joint in model space
    // The inverse bind pose transform is the inverse of this, so it brings the bone position and
    // orientation to the origin
    Matrix4x4g m_offsetMatrix; 

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

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    SkeletonJoint(const QString& uniqueName, Skeleton* skeleton);
    SkeletonJoint(const SkeletonJoint& meshNode, Skeleton* skeleton, SkeletonJoint* parent, bool justBones = false);
    ~SkeletonJoint();
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Methods
    /// @{

    /// @brief Whether or not the node has a bone or not
    bool hasBone() const { return m_bone.m_index >= 0; }

    /// @brief Whether or not the node is animated
    bool isAnimated() const { return m_jointFlags.testFlag(kIsAnimated); }
    void setAnimated(bool animated) { m_jointFlags.setFlag(kIsAnimated, animated); }

    /// @brief Add a child node to this node
    void addChild(const QString& uniqueName);

    /// @brief Siblings of this node, including this node
    std::vector<SkeletonJoint*>& siblings() const { return m_parent->children(); }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties 
    /// @{

    int skeletonTransformIndex() const {
        return m_skeletonTransformIndex;
    }
    void setSkeletonTransformIndex(int idx) {
        m_skeletonTransformIndex = idx;
    }

    SkeletonJoint* parent() { return m_parent; }
    const SkeletonJoint* parent() const { return m_parent; }

    const Skeleton& skeleton() const { return *m_skeleton; }
    Skeleton& skeleton() { return *m_skeleton; }

    const Transform& getTransform() const { return m_transform; }
    Transform& transform() { return m_transform; }

    /// @brief The bone corresponding to this node
    const Bone& bone() const { return m_bone; }
    Bone& bone() { return m_bone; }

    /// @brief Children
    std::vector<SkeletonJoint*>& children() { return m_children; }
    const std::vector<SkeletonJoint*>& children() const { return m_children; }

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

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private methods 
    /// @{

    void addChild(SkeletonJoint* node);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private members 
    /// @{

    /// @brief Pointer to the base skeleton
    Skeleton* m_skeleton;

    /// @brief Bone corresponding to this mesh node
    Bone m_bone;

    /// @brief The index associated with this joint in vector of skeletal animation transforms
    int m_skeletonTransformIndex;

    /// @brief Whether or not the joint has animation data associated with it
    QFlags<JointFlag> m_jointFlags;

    /// @brief Transform of the node
    /// FIXME: Actually implement this in rendering
    Transform m_transform;

    /// @brief Direct children of this mesh node
    std::vector<SkeletonJoint*> m_children;

    ///// @brief Map of mesh data
    //std::unordered_map<QString, VertexArrayData*> m_meshData;

    /// @brief Pointer to the parent mesh node
    SkeletonJoint* m_parent = nullptr;

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
    Skeleton(const QString& uniqueName);
    Skeleton(const Skeleton& other);
    Skeleton(const Skeleton& other, bool justBones);
    ~Skeleton();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methhods
    /// @{

    void onRemoval(ResourceCache* cache = nullptr) override;

    /// @brief Whether or not the skeleton is malformed for inverse kinematics
    bool isMalformed() const;

    const SkeletonJoint* root() const { return m_root; }
    SkeletonJoint* root() { return m_root; }

    const std::vector<Matrix4x4g>& inverseBindPose() const { return m_inverseBindPose; }

    const Matrix4x4g& globalInverseTransform() const { return m_globalInverseTransform; }
    void setGlobalInverseTransform(const Matrix4x4g& mat) { m_globalInverseTransform = mat; }

    /// @brief whether or not the skeleton has a root node
    bool hasRoot() const { return m_root != nullptr; }

    /// @brief Add root node if doesn't exist
    void addRootNode(const QString& name);

    /// @brief Get the vector of nodes that have bones
    const std::vector<SkeletonJoint*>& boneNodes() const { return m_boneNodes; }

    const std::unordered_map<QString, SkeletonJoint*>& nodes() const { return m_nodes; }

    /// @brief Return hierarchy with just bones (root may not have a bone)
    Skeleton prunedBoneSkeleton() const;

    /// @brief Get the node with the given name
    const SkeletonJoint* getNode(const QString& name) const { return m_nodes.at(name); }
    SkeletonJoint* getNode(const QString& name) { return m_nodes.at(name); }

    /// @brief Return default pose of the skeleton
    std::vector<Matrix4x4g> defaultPose() const;

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
    void constructInverseBindPose(SkeletonJoint& node);

    /// @brief Recursive function for determining if skeleton is malformed
    bool isMalformed(const SkeletonJoint* node) const;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private members 
    /// @{

    /// @brief global inverse transform of the skeleton
    Matrix4x4g m_globalInverseTransform;

    /// @brief Pointer to the root joint
    SkeletonJoint* m_root = nullptr;

    /// @brief Map of all joints
    std::unordered_map<QString, SkeletonJoint*> m_nodes;

    /// @brief Vector of joints with bones
    std::vector<SkeletonJoint*> m_boneNodes;

    /// @brief Inverse bind pose of the skeleton
    std::vector<Matrix4x4g> m_inverseBindPose;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif