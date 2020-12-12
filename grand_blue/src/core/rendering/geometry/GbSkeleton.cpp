#include "GbSkeleton.h"

#include "GbMesh.h"
#include "../../GbCoreEngine.h"
#include "../../resource/GbResourceCache.h"
#include "../../processes/GbProcess.h"
#include "../models/GbModel.h"
#include "../materials/GbMaterial.h"
#include "../shaders/GbShaders.h"
#include "../../loop/GbSimLoop.h"
#include "../../containers/GbFlags.h"

namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BoneData
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Bone::Bone()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Bone::Bone(int index) :
    m_index(index)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Bone & Bone::operator=(const Bone & other)
{
    m_index = other.m_index;
    m_invBindPose = other.m_invBindPose;
    return *this;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Mesh Node
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SkeletonJoint::SkeletonJoint(const GString & uniqueName):
    Object(uniqueName),
    m_jointFlags(0)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SkeletonJoint::SkeletonJoint() :
    Object("") // Give an empty name, to avoid nullptr when serializing
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SkeletonJoint::~SkeletonJoint()
{
    // Unnecessary, handled by skeleton deletion
    //for(MeshNode* child: m_children)
    //    delete child;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SkeletonJoint::initializeFromJoint(const SkeletonJoint & copyNode,
    const Skeleton& otherSkeleton,
    Skeleton& skeleton,
    size_t parentIndex,
    std::vector<size_t>& uninitializedNodes,
    bool justBones)
{
    m_name = copyNode.m_name;
    m_parent = parentIndex;
    m_bone = copyNode.m_bone;
    m_transform = copyNode.m_transform;
    m_jointFlags = 0;

    std::vector<SkeletonJoint>& joints = skeleton.m_nodes;
    size_t idx = getIndex(skeleton);
    m_children.reserve(copyNode.m_children.size());
    for (const size_t& otherChildIndex : copyNode.m_children) {
        const SkeletonJoint& otherChild = otherSkeleton.getNode(otherChildIndex);
        if (otherChild.hasBone() || !justBones) {

            // Add child node index to this node's list of direct children
            size_t childIndex = uninitializedNodes.back();
            m_children.push_back(childIndex);
            uninitializedNodes.pop_back();

            // Add child node to the skeleton's vector of all nodes
            joints[childIndex].initializeFromJoint(otherChild, otherSkeleton, skeleton, idx, uninitializedNodes, justBones);

            // Add to bone map
            SkeletonJoint& child = joints[childIndex];
            if (child.hasBone()) {
                skeleton.m_boneNodes[child.bone().m_index] = childIndex;
            }
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
size_t SkeletonJoint::getIndex(const Skeleton& skeleton) const
{
    const std::vector<SkeletonJoint>& joints = skeleton.m_nodes;
    for (size_t i = 0; i < joints.size(); i++) {
        if (joints[i].getUuid() == m_uuid) {
            return i;
        }
    }

    throw("Error, joint not found in skeleton");
    return -1;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool SkeletonJoint::isAnimated() const
{
    return Flags::toFlags<JointFlag>(m_jointFlags).testFlag(kIsAnimated);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SkeletonJoint::setAnimated(bool animated)
{
    JointFlags flags = Flags::toFlags<JointFlag>(m_jointFlags);
    flags.setFlag(kIsAnimated, animated);
    m_jointFlags = flags;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SkeletonJoint * SkeletonJoint::parent(Skeleton& skeleton)
{
    return m_parent >= 0 ? &skeleton.m_nodes[m_parent] : nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const SkeletonJoint * SkeletonJoint::parent(const Skeleton& skeleton) const
{
    return m_parent >= 0 ? &skeleton.m_nodes[m_parent] : nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void SkeletonJoint::addChild(SkeletonJoint * node)
//{    // Add child node to this node's list of direct children
//    m_children.push_back(node);
//
//    // Set parent of the child node
//    node->m_parent = this;
//
//    // Add child node to the skeleton's map of all nodes
//    m_skeleton->m_nodes.emplace(node->getName(), node);
//
//    // Add to bone map
//    if (node->hasBone()) {
//        m_skeleton->m_boneNodes[node->bone().m_index] = node;
//    }
//}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Skeleton
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Skeleton::Skeleton(const GString & uniqueName):
    Resource(uniqueName)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Skeleton::Skeleton(const Skeleton & other):
    Skeleton(other, false)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Skeleton::Skeleton(const Skeleton & other, bool justBones) :
    Resource(other.m_name),
    m_globalInverseTransform(other.m_globalInverseTransform),
    m_inverseBindPose(other.m_inverseBindPose)
{
    m_boneNodes.resize(other.m_boneNodes.size());

    if (other.isMalformed()) {
        // Skeleton is malformed, since it is not clear which child with a bone should be the root
        throw("Error, skeleton is malformed.  More than one potential root");
    }

    // Create all skeleton joints before initializing, to avoid resizing vector
    for (const SkeletonJoint& joint : other.m_nodes) {
        if (!justBones || joint.hasBone()) {
            // Append new node to this skeleton
            Vec::EmplaceBack(m_nodes);
        }
    }

    // Create vector of uninitialized node indices
    std::vector<size_t> uninitializedNodes;
    size_t nodeSize = justBones ? m_boneNodes.size() : m_nodes.size();
    uninitializedNodes.reserve(nodeSize - 1); // minus 1 for root node
    for (size_t i = 1; i < nodeSize; i++) {
        uninitializedNodes.push_back(i);
    }
    std::reverse(uninitializedNodes.begin(), uninitializedNodes.end());

    // Initialize nodes
    if (justBones) {
        m_nodes[0].initializeFromJoint(other.getNode(other.m_boneNodes[0]), other, *this, -1, uninitializedNodes, justBones);
    }
    else {
        m_nodes[0].initializeFromJoint(*other.root(), other, *this, -1, uninitializedNodes, justBones);
    }

    if (uninitializedNodes.size() != 0) {
        throw("Error, failed to initialize all nodes");
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Skeleton::~Skeleton()
{
    // Delete all nodes of the skeleton
    //for (const std::pair<GString, SkeletonJoint*> nodePair : m_nodes) {
    //    delete nodePair.second;
    //}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Skeleton::onRemoval(ResourceCache * cache)
{
    Q_UNUSED(cache);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Skeleton::isMalformed() const
{
    return isMalformed(m_nodes[0]);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Skeleton::isMalformed(const SkeletonJoint& node) const
{
    bool malformed = false;
    bool foundBone = false;
    for (const size_t& childIndex : node.children()) {
        const SkeletonJoint& child = m_nodes.at(childIndex);
        if (foundBone && child.hasBone()) {
            // There are multiple child bones on the first level with bones, so malformed
            malformed = true;
        }

        if (child.hasBone()) {
            foundBone = true;
        }
    }

    if (malformed || foundBone) {
        return malformed;
    }
    else {
        // If not malformed and haven't found a bone, continue
        for (const size_t& childIndex : node.children()) {
            const SkeletonJoint& child = m_nodes.at(childIndex);
            malformed |= isMalformed(child);
            if (malformed) break;
        }
    }

    return malformed;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Skeleton::addRootNode(const GString& name)
{
    if (m_nodes.size()) {
        throw("Error, root already exists");
    }

    // Set name of skeleton to root node name
    setName(name);

    // Add root node
    //m_rootIndex = m_nodes.size();
    Vec::EmplaceBack(m_nodes, name);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Skeleton Skeleton::prunedBoneSkeleton() const
{
    return Skeleton(*this, true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const SkeletonJoint& Skeleton::getNode(const GString & name) const
{
    for (size_t i = 0; i < m_nodes.size(); i++) {
        const SkeletonJoint& node = m_nodes[i];
        if (node.getName() == name) {
            return node;
        }
    }

#ifdef DEBUG_MODE
    throw("Node not found");
#endif
    return m_nodes[0];
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SkeletonJoint& Skeleton::getNode(const GString & name)
{
    for (size_t i = 0; i < m_nodes.size(); i++) {
        SkeletonJoint& node = m_nodes[i];
        if (node.getName() == name) {
            return node;
        }
    }

#ifdef DEBUG_MODE
    throw("Node not found");
#endif
    return m_nodes[0];
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SkeletonJoint & Skeleton::getNode(const size_t & idx)
{
#ifdef DEBUG_MODE
    if (m_nodes.size() <= idx) {
        throw("Error, node not found in skeleton");
    }
#endif
    return m_nodes[idx];
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const SkeletonJoint & Skeleton::getNode(const size_t & idx) const
{
#ifdef DEBUG_MODE
    if (m_nodes.size() <= idx) {
        throw("Error, node not found in skeleton");
    }
#endif
    return m_nodes[idx];
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
size_t Skeleton::addChild(size_t parentIndex, const GString& uniqueName)
{
    // Add child node index to this node's list of direct children
    size_t childIndex = m_nodes.size();
    Vec::EmplaceBack(m_nodes[parentIndex].children(), childIndex); // Index is old size of joint vector

    // Add child node to the skeleton's map of all nodes
    Vec::EmplaceBack(m_nodes, uniqueName);

    SkeletonJoint& node = m_nodes.back();

    // Set parent of the child node
    node.m_parent = parentIndex;

    return childIndex;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 void Skeleton::identityPose(std::vector<Matrix4x4g>& outPose) const
{
    size_t size = m_boneNodes.size();
    if (outPose.size() == size) {
        for (size_t i = 0; i < size; i++) {
            //const MeshNode* node = m_boneNodes[i];
            outPose[i] = Matrix4x4g();
        }
    }
    else {
        outPose.clear();
        outPose.reserve(size);
        for (size_t i = 0; i < size; i++) {
            //const MeshNode* node = m_boneNodes[i];
            outPose.emplace_back();
        }
    }

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Skeleton::generateBoundingBox()
{
    // Construct bind pose translations
    std::vector<Vector4> bindPoseTranslations;
    bindPoseTranslations.reserve(m_inverseBindPose.size());
    for (size_t i = 0; i < m_inverseBindPose.size(); i++) {
        bindPoseTranslations.emplace_back(m_inverseBindPose[i].inversed() * Vector4(0, 0, 0, 1));
    }

    // Construct bounds from bind pose translations
    m_boundingBox = AABB();
    m_boundingBox.resize(bindPoseTranslations);
    m_boundingBox.boxData().toContainingCube();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Skeleton::constructInverseBindPose()
{
    if (!m_inverseBindPose.size()) {
        m_inverseBindPose.resize(m_boneNodes.size());
    }

    constructInverseBindPose(0);

    // Generate bounding box for the skeleton
    generateBoundingBox();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Skeleton::constructInverseBindPose(const size_t& nodeIndex)
{
    SkeletonJoint& node = m_nodes[nodeIndex];
    if (node.hasBone()) {
        const Bone& bone = node.bone();
        m_inverseBindPose[bone.m_index] = bone.m_invBindPose;
    }

    for (const size_t& childIndex : node.children()) {
        constructInverseBindPose(childIndex);
    }
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespacing