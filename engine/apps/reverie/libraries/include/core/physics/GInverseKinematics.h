#pragma once

// Standard
#include <vector>
#include <unordered_map>

// QT
#include <QFlags>
#include <QString>

// Internal
#include "fortress/types/GIdentifiable.h"
#include "fortress/containers/math/GVector.h"

namespace rev {

class CoreEngine;
class Skeleton;
class SkeletonJoint;
class IKChain;


/// @class IKNode
/// @brief A kinematic node
class IKNode: public IdentifiableInterface {
public:
    
    /// @name Static
    /// @{

    enum NodeType {
        kEndEffector = (1 << 0), // End of a chain
        kSubBase =     (1 << 1), // Intermediate meeting of two or more chains
        kRoot =        (1 << 2) // The root of the chain
    };
    typedef QFlags<NodeType> NodeTypeFlags;

    /// @}

    /// @name Constructors/Destructors
    /// @{

    IKNode();
    IKNode(IKChain* chain);
    IKNode(IKChain* chain, const SkeletonJoint& meshNode);
    virtual ~IKNode();

    /// @}

    /// @name Public methods
    /// @{

    bool isRoot() const {
        return m_flags.testFlag(kRoot);
    }
    bool isSubBase() const {
        return m_flags.testFlag(kSubBase);
    }
    bool isEndEffector() const {
        return m_flags.testFlag(kEndEffector);
    }

    /// @brief Add a child to this node, returning the child
    IKNode* addChild(const SkeletonJoint& child);

    /// @}

protected:
    friend class IKChain;

    /// @name Protected Members
    /// @{

    size_t m_nodeID;

    /// @brief Direct children of this node
    std::vector<IKNode*> m_children;

    IKChain* m_chain = nullptr;

    Vector3d m_position;

    /// @brief Flags denoting the type of node
    NodeTypeFlags m_flags;

    /// @}

};
typedef QFlags<IKNode::NodeType> NodeTypeFlags;


/// @class IKChain
/// @brief A chain of kinematic nodes
class IKChain {
public:

    /// @name Constructors/Destructors
    /// @{

    IKChain(const Skeleton& skeleton);
    virtual ~IKChain();

    /// @}

    /// @name Public methods
    /// @{

    IKNode* getNode(int id) const;
    IKNode* getNode(const SkeletonJoint& joint) const;

    /// @}

protected:
    friend class IKNode;

    /// @name Protected Methods
    /// @{

    void initialize(const Skeleton& skeleton);
    void addNode(const SkeletonJoint& child, IKNode* parent = nullptr);

    /// @}

    /// @name Protected Members
    /// @{

    /// @brief The skeleton associated with this IK chain
    const Skeleton& m_skeleton;

    /// @brief The root of the IK chain
    IKNode m_root;

    /// @brief Flat vector of nodes for quick access by unique ID
    // TODO: Store by value, and store indices of children in IK node itself
    std::vector<IKNode*> m_nodes;

    /// @}
};



/// @class IK
/// @brief Class for inverse kinematics
/// @notes See: https://discourse.urho3d.io/t/solved-ik-foot-placement/1010
/// 
class IK {
public:
    /// @name Static
    /// @{
    
    /// @brief The actual algorithm (FABRIK)
    /// @details See:http://www.andreasaristidou.com/publications/papers/FABRIK.pdf
    static void Solve(const Vector3d& target, 
        std::vector<Vector3d>& inOutJointPositions,
        const std::vector<double>& jointDistances,
        double tolerance = 1e-6,
        size_t maxNumIter = 100);

    /// @brief The FABRIK algorithm, with CGA (Conformal Geometric Algebra)
    /// @details See: grand-blue-engine\docs\Physics\Inverse_Kinematics_CGA.pdf
    static void SolveCGA(const Vector3d& target,
        std::vector<Vector3d>& inOutJointPositions,
        const std::vector<double>& jointDistances,
        double tolerance = 1e-6,
        size_t maxNumIter = 100);

    /// @}

    /// @name Constructors/Destructor
    /// @{

    IK();
    ~IK();

    /// @}


protected:
    /// @name Protected Methods
    /// @{

    /// @brief Obtain the nearest point on a sphere from a point in space. 
    /// @details center, radius, pointInSpace, pointOnSphere
    static void NearestPointSphere(const Vector3d& center,
        double radius,
        const Vector3d& pointInSpace,
        Vector3d& outPointOnSphere);
    
    /// @brief Obtain the centroid of the given set of points
    static void GetCentroid(const std::vector<Vector3d>& points, Vector3d& outCentroid);

    /// @}

};



} // End namespaces
