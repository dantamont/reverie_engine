#ifndef GB_INVERSE_KINEMATICS_H
#define GB_INVERSE_KINEMATICS_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Standard
#include <vector>
#include <unordered_map>

// QT
#include <QFlags>
#include <QString>

// Internal
#include "../GbObject.h"
#include "../geometry/GbVector.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class Skeleton;
class MeshNode;
class IKChain;

//////////////////////////////////////////////////////////////////////////////////
// Macro Definitions
//////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Type Definitions
/////////////////////////////////////////////////////////////////////////////////////////////
//template<typename D, size_t size> class Vector;
//typedef Vector<double, 3> Vector3;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class IKNode
/// @brief A kinematic node
class IKNode: public Object {
public:
    
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum NodeType {
        kEndEffector = (1 << 0), // End of a chain
        kSubBase =     (1 << 1), // Intermediate meeting of two or more chains
        kRoot =        (1 << 2) // The root of the chain
    };
    typedef QFlags<NodeType> NodeTypeFlags;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructors
    /// @{

    IKNode();
    IKNode(IKChain* chain);
    IKNode(IKChain* chain, const MeshNode& meshNode);
    virtual ~IKNode();

    /// @}

    //---------------------------------------------------------------------------------------
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
    IKNode* addChild(const MeshNode& child);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "IKNode"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::IKNode"; }
    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{


    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Direct children of this node
    std::vector<IKNode*> m_children;

    IKChain* m_chain = nullptr;

    Vector3 m_position;

    /// @brief Flags denoting the type of node
    NodeTypeFlags m_flags;

    /// @}

};
typedef QFlags<IKNode::NodeType> NodeTypeFlags;


/////////////////////////////////////////////////////////////////////////////////////////////
/// @class IKChain
/// @brief A chain of kinematic nodes
class IKChain {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructors
    /// @{

    IKChain();
    IKChain(const Skeleton& skeleton);
    virtual ~IKChain();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    IKNode* getNode(const QString& name) {
        return m_nodes[name];
    }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "IKChain"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::IKChain"; }
    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    void initialize(const Skeleton& skeleton);
    void addNode(const MeshNode& child, IKNode* parent = nullptr);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief The root of the IK chain
    IKNode m_root;

    /// @brief Flat map of nodes for quick access by unique name
    std::unordered_map<QString, IKNode*> m_nodes;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
/// @class IK
/// @brief Class for inverse kinematics
/// @notes See: https://discourse.urho3d.io/t/solved-ik-foot-placement/1010
/// 
class IK {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    
    /// @brief The actual algorithm (FABRIK)
    /// @details See:http://www.andreasaristidou.com/publications/papers/FABRIK.pdf
    static void Solve(const Vector3& target, 
        std::vector<Vector3>& inOutJointPositions,
        const std::vector<double>& jointDistances,
        double tolerance = 1e-6,
        size_t maxNumIter = 100);

    /// @brief The FABRIK algorithm, with CGA (Conformal Geometric Algebra)
    /// @details See: grand-blue-engine\docs\Physics\Inverse_Kinematics_CGA.pdf
    static void SolveCGA(const Vector3& target,
        std::vector<Vector3>& inOutJointPositions,
        const std::vector<double>& jointDistances,
        double tolerance = 1e-6,
        size_t maxNumIter = 100);

    /// @}


    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    IK();
    ~IK();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{


    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{


    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Obtain the nearest point on a sphere from a point in space. 
    /// @details center, radius, pointInSpace, pointOnSphere
    static void NearestPointSphere(const Vector3& center,
        double radius,
        const Vector3& pointInSpace,
        Vector3& outPointOnSphere);
    
    /// @brief Obtain the centroid of the given set of points
    static void GetCentroid(const std::vector<Vector3>& points, Vector3& outCentroid);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @}

};




//////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif