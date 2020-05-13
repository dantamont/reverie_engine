#include "GbInverseKinematics.h"
#include "../geometry/GbMatrix.h"
#include "../rendering/geometry/GbSkeleton.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////////////////////
// IK Node
//////////////////////////////////////////////////////////////////////////////////////////////////
IKNode::IKNode()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
IKNode::IKNode(IKChain* chain):
    m_chain(chain)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
IKNode::IKNode(IKChain* chain, const MeshNode & meshNode):
    m_chain(chain)
{
    // Set name to match mesh node
    m_name = meshNode.getName();

    // Set flags
    if (!meshNode.parent()) {
        // If the node doesn't have a parent, it's the root node
        m_flags.setFlag(kRoot, true);
        m_flags.setFlag(kSubBase, true);
    }
    else if (meshNode.children().size() > 1) {
        // If the node has more than one child, it's a subbase
        m_flags.setFlag(kSubBase, true);
    }
    else if (meshNode.children().size() == 0) {
        m_flags.setFlag(kEndEffector, true);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
IKNode::~IKNode()
{
    for (IKNode* child : m_children) {
        delete child;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
IKNode* IKNode::addChild(const MeshNode& child)
{
    Vec::EmplaceBack(m_children, new IKNode(m_chain, child));
    return m_children.back();
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// IK Chain
//////////////////////////////////////////////////////////////////////////////////////////////////
IKChain::IKChain()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
IKChain::IKChain(const Skeleton & skeleton)
{
    initialize(skeleton);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
IKChain::~IKChain()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void IKChain::initialize(const Skeleton & skeleton)
{
    if (!skeleton.hasRoot()) {
        throw("Error, skeleton has no root node");
    }
    addNode(*skeleton.root());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void IKChain::addNode(const MeshNode & meshNode, IKNode* parent)
{
    IKNode* thisNode = nullptr;
    if(!parent){
        // Create node as root node
        m_root = IKNode(this, meshNode);
        thisNode = &m_root;
    }
    else {
        // Create node
        thisNode = parent->addChild(meshNode);
    }

    // Add to node map
    if (thisNode->getName().isEmpty()) throw("Error, node does not have unique name");
    m_nodes[thisNode->getName()] = thisNode;

    // Add children recursively
    for (const MeshNode* child: meshNode.children()) {
        addNode(*child, thisNode);
    }
}




//////////////////////////////////////////////////////////////////////////////////////////////////
// IK
//////////////////////////////////////////////////////////////////////////////////////////////////
void IK::Solve(const Vector3 & target, 
    std::vector<Vector3>& inOutJointPositions,
    const std::vector<double>& jointDistances,
    double tolerance,
    size_t maxNumIter)
{

    // Get the distance between the root and the target
    Vector3& root = inOutJointPositions[0];
    double dist = (root - target).length();

    // Check whether or not the target is within reach
    double armLength = std::accumulate(jointDistances.begin(), jointDistances.end(), 0.0);
    size_t numJoints = inOutJointPositions.size();
    if (dist > armLength) {
        // The target is unreachable
        for (size_t i = 0; i < numJoints; i++) {
            // Find the distance r between the target and the joint position
            const Vector3& currentPos = inOutJointPositions[i];
            double r = (target - currentPos).length();
            double factor = jointDistances[i] / r;
            inOutJointPositions[i + 1] = (1.0 - factor) * currentPos + factor * target;
        }
    }
    else {
        // The target is reachable
        Vector3 initialRootPosition = root;

        // Check whether the distance between the end effector and target is greater than a tolerance
        const Vector3& effectorPos = inOutJointPositions.back();
        double diffEffector = (effectorPos - target).length();
        size_t count = 0;
        while (diffEffector > tolerance && count < maxNumIter) {
            // STAGE 1: FORWARD REACHING
            // Set the end effector as the target
            inOutJointPositions.back() = target;
            for (size_t i = numJoints - 1; i > -1; i--) {
                double currentBoneLength = jointDistances[i];

                // Find the distance between the new joint position and current joint
                const Vector3& nextPos = inOutJointPositions[i + 1];
                Vector3& pos = inOutJointPositions[i];
                double r = (nextPos - pos).length();
                double factor = currentBoneLength / r;

                // Find the new joint position
                pos = (1 - factor) * nextPos + factor * pos;
            }

            // STAGE 2: BACKWARD REACHING
            // Set the root to its initial position
            root = initialRootPosition;
            for (size_t i = 0; i < numJoints; i++) {
                double currentBoneLength = jointDistances[i];

                // Find the distance between the new joint position and current joint
                Vector3& nextPos = inOutJointPositions[i + 1];
                const Vector3& pos = inOutJointPositions[i];
                double r = (nextPos - pos).length();
                double factor = currentBoneLength / r;

                // Find the new joint position
                nextPos = (1 - factor) * pos + factor * nextPos;
            }

            // Update tolerance check
            diffEffector = (effectorPos - target).length();

            count++;
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void IK::SolveCGA(const Vector3 & target,
    std::vector<Vector3>& inOutJointPositions,
    const std::vector<double>& jointDistances, 
    double tolerance,
    size_t maxNumIter)
{

    // Get the distance between the root and the target
    Vector3& root = inOutJointPositions[0];
    double dist = (root - target).length();

    // Check whether or not the target is within reach
    double armLength = std::accumulate(jointDistances.begin(), jointDistances.end(), 0.0);
    size_t numJoints = inOutJointPositions.size();
    if (dist > armLength) {
        // The target is unreachable
        // Perform one BACKWARD REACHING iteration to construct a straight line to the target
        for (size_t i = 0; i < numJoints; i++) {
            // Find the nearest point on sphere, with centre the joint position p_i and radius
            // the distance d_i , from a point is space, target
            Vector3& p_i = inOutJointPositions[i];
            double d_i = jointDistances[i];
            NearestPointSphere(p_i, d_i, target, inOutJointPositions[i + 1]);
        }
    }
    else {
        // The target is reachable
        Vector3 initialRootPosition = root;

        // Check whether the distance between the end effector and target is greater than a tolerance
        const Vector3& effectorPos = inOutJointPositions.back();
        double diffEffector = (effectorPos - target).length();
        size_t count = 0;
        while (diffEffector > tolerance && count < maxNumIter) {
            // STAGE 1: FORWARD REACHING
            // Set the end effector as the target
            inOutJointPositions.back() = target;
            for (size_t i = numJoints - 1; i > -1; i--) {
                // Find the nearest point on sphere, with centre the joint position pi+1 and
                // radius the distance di, from a point is space, pi
                Vector3& p_i = inOutJointPositions[i];
                const Vector3& p_next = inOutJointPositions[i+1];
                double d_i = jointDistances[i];
                NearestPointSphere(p_next, d_i, p_i, p_i);
            }

            // STAGE 2: BACKWARD REACHING
            // Set the root to its initial position
            root = initialRootPosition;
            for (size_t i = 0; i < numJoints; i++) {
                // Find the nearest point on sphere, with centre the joint position pi and
                // radius the distance di, from a point is space, pi + 1
                Vector3& p_i = inOutJointPositions[i];
                const Vector3& p_next = inOutJointPositions[i + 1];
                double d_i = jointDistances[i];
                NearestPointSphere(p_i, d_i, p_next, p_i);
            }

            // Update tolerance check
            diffEffector = (effectorPos - target).length();

            count++;
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
IK::IK()
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////
IK::~IK()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void IK::NearestPointSphere(const Vector3 & center, double radius, const Vector3 & pointInSpace, Vector3 & outPointOnSphere)
{
    // Get a vector from the sphere center to the point
    outPointOnSphere = pointInSpace - center;

    // Adjust length to radius of sphere
    outPointOnSphere.normalize();
    outPointOnSphere *= radius;

    // Convert to world-space
    outPointOnSphere += center;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void IK::GetCentroid(const std::vector<Vector3>& points, Vector3 & outCentroid) {
    outCentroid = std::accumulate(points.begin(), points.end(), Vector3(0.0, 0.0, 0.0)) / points.size();
}







//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
}