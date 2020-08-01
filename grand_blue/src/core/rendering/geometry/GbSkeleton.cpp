#include "GbSkeleton.h"

#include "GbMesh.h"
#include "../../GbCoreEngine.h"
#include "../../resource/GbResourceCache.h"
#include "../../processes/GbProcess.h"
#include "../models/GbModel.h"
#include "../materials/GbMaterial.h"
#include "../shaders/GbShaders.h"
#include "../../loop/GbSimLoop.h"

namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BoneData
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Bone::Bone(): 
    Object()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Bone::Bone(const QString & name, int index) :
    Object(name),
    m_index(index)
{
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Mesh Node
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SkeletonJoint::SkeletonJoint(const QString & uniqueName, Skeleton* skeleton):
    Object(uniqueName),
    m_skeleton(skeleton)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SkeletonJoint::SkeletonJoint(const SkeletonJoint & meshNode, Skeleton* skeleton, SkeletonJoint* parent, bool justBones):
    Object(meshNode.m_name),
    m_skeleton(skeleton),
    m_parent(parent),
    m_bone(meshNode.m_bone),
    m_transform(meshNode.m_transform)
{
    for (SkeletonJoint* otherChild : meshNode.m_children) {
        if (otherChild->hasBone() || !justBones) {
            SkeletonJoint* child = new SkeletonJoint(*otherChild, m_skeleton, this, justBones);
            addChild(child);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SkeletonJoint::~SkeletonJoint()
{
    // Unnecessary, handled by skeleton deletion
    //for(MeshNode* child: m_children)
    //    delete child;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void MeshNode::draw(CoreEngine* core, const std::shared_ptr<Gb::ShaderProgram>& shaderProgram, int glMode)
//{
//    // FIXME: Handle local transforms
//
//    // Perform actual draw routine
//    for (const std::pair<QString, VertexArrayData*>& meshPair : m_meshData) {
//        const VertexArrayData& mesh = *meshPair.second;
//
//        if (mesh.hasMaterial()) {
//            std::shared_ptr<Material> mtl =
//                core->resourceCache()->getMaterial(mesh.m_materialName, false);
//
//            // Draw geometry if there is a material
//            if (mtl) {
//                // Bind material
//                if (!mtl->isBound()) mtl->bind(shaderProgram);
//
//                // Draw
//                mesh.drawGeometry(glMode);
//
//                // Unbind material
//                if (mtl->isBound()) mtl->release();
//            }
//        }
//    }
//
//    // Draw child node meshes
//    for (MeshNode* child : m_children) {
//        child->draw(core, shaderProgram, glMode);
//    }
//}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void MeshNode::addMeshData(VertexArrayData* meshData)
//{
//    m_meshData[meshData->getName()] = meshData;
//}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SkeletonJoint::addChild(const QString& uniqueName)
{
    // Add child node to this node's list of direct children
    m_children.push_back(new SkeletonJoint(
        uniqueName,
        m_skeleton));
    SkeletonJoint* node = m_children.back();

    // Set parent of the child node
    node->m_parent = this;

    // Add child node to the skeleton's map of all nodes
    m_skeleton->m_nodes.emplace(uniqueName, node);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SkeletonJoint::addChild(SkeletonJoint * node)
{    // Add child node to this node's list of direct children
    m_children.push_back(node);

    // Set parent of the child node
    node->m_parent = this;

    // Add child node to the skeleton's map of all nodes
    m_skeleton->m_nodes.emplace(node->getName(), node);

    // Add to bone map
    if (node->hasBone()) {
        m_skeleton->m_boneNodes[node->bone().m_index] = node;
    }
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Skeleton
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Skeleton::Skeleton(const QString & uniqueName):
    Resource(uniqueName, kSkeleton)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Skeleton::Skeleton(const Skeleton & other):
    Skeleton(other, false)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Skeleton::Skeleton(const Skeleton & other, bool justBones) :
    Resource(other.m_name, kSkeleton),
    m_globalInverseTransform(other.m_globalInverseTransform),
    m_inverseBindPose(other.m_inverseBindPose)
{
    m_boneNodes.resize(other.m_boneNodes.size());
    if (justBones) {
        if (other.isMalformed()) {
            // Skeleton is malformed, since it is not clear which child with a bone should be the root
            throw("Error, skeleton is malformed.  More than one potential root");
        }

        m_root = new SkeletonJoint(*other.m_boneNodes[0], this, nullptr, justBones);
    }
    else{
        m_root = new SkeletonJoint(*other.m_root, this, nullptr, justBones);
    }

    m_nodes.emplace(m_root->getName(), m_root);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Skeleton::~Skeleton()
{
    // Delete all nodes of the skeleton
    for (const std::pair<QString, SkeletonJoint*> nodePair : m_nodes) {
        delete nodePair.second;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Skeleton::onRemoval(ResourceCache * cache)
{
    Q_UNUSED(cache);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Skeleton::isMalformed() const
{
    return isMalformed(m_root);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Skeleton::isMalformed(const SkeletonJoint * node) const
{
    bool malformed = false;
    bool foundBone = false;
    for (const SkeletonJoint* child : node->children()) {
        if (foundBone && child->hasBone()) {
            // There are multiple child bones on the first level with bones, so malformed
            malformed = true;
        }

        if (child->hasBone()) {
            foundBone = true;
        }
    }

    if (malformed || foundBone) {
        return malformed;
    }
    else {
        // If not malformed and haven't found a bone, continue
        for (const SkeletonJoint* child : node->children()) {
            malformed |= isMalformed(child);
            if (malformed) break;
        }
    }

    return malformed;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Skeleton::addRootNode(const QString& name)
{
    // Set name of skeleton to root node name
    setName(name);

    // Add root node
    m_root = new SkeletonJoint(name, this);
    m_nodes.emplace(name, m_root);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Skeleton Skeleton::prunedBoneSkeleton() const
{
    return Skeleton(*this, true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<Matrix4x4g> Skeleton::defaultPose() const
{
    size_t size = m_boneNodes.size();
    std::vector<Matrix4x4g> transforms(size);
    for (size_t i = 0; i < size; i++) {
        //const MeshNode* node = m_boneNodes[i];
        transforms[i] = Matrix4x4g();
    }

    return transforms;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Skeleton::constructInverseBindPose()
{
    if (!m_inverseBindPose.size()) {
        m_inverseBindPose.resize(m_boneNodes.size());
    }

    constructInverseBindPose(*m_root);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Skeleton::constructInverseBindPose(SkeletonJoint & node)
{
    if (node.hasBone()) {
        const Bone& bone = node.bone();
        m_inverseBindPose[bone.m_index] = bone.m_offsetMatrix;
    }

    for (SkeletonJoint* child : node.children()) {
        constructInverseBindPose(*child);
    }
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespacing