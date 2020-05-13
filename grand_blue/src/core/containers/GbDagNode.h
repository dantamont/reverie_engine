#ifndef GB_DAG_NODE_H
#define GB_DAG_NODE_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// External

// Project
#include "../GbObject.h"
#include "../encoding/GbUUID.h"
#include "../containers/GbContainerExtensions.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** @class DagNode
    @brief  A node of a Directed Acyclic Graph (DAG)
    @details A directed acyclic graph is more generalized notion of a tree structure, except that in a DAG, a child may
    have more than one parent, as opposed to in a tree, where each child may only have at most one parent.

    @note To achieve functionality without memory leaks, all DAG nodes are cached in a static map.  
    To completely delete a DAG node, they must be manually erased from this static map using the 
    eraseFromNodeMap method.
*/
class DagNode: 
    public Object,
    public std::enable_shared_from_this<DagNode>
{
public:
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @name Static Methods
    /// @{

    /// @brief Types of DAG nodes
    enum NodeType {
        kBase = -1, // Invalid, should never be used
        kSceneObject
    };

    /// @brief Create a shared pointer to a DAG node (since the constructor is private)
    static std::shared_ptr<DagNode> create();

    /// @brief Erases a node with the given UUID from the static list
    static void eraseFromNodeMap(const Uuid& uuid);

    /// @brief Returns map of DAG nodes 
    static std::unordered_map<Uuid, std::shared_ptr<DagNode>>& getDagNodeMap() { return NODES; }

    /// @}

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @name Destructor
    /// @{
    virtual ~DagNode();
    /// @}

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @name Public Methods
    /// @{

    /// @brief Add a parent to the given node
    void addParent(std::shared_ptr<DagNode> parent);

    /// @brief Add a child to the given node
    void addChild(std::shared_ptr<DagNode> child);

    /// @brief Remove a parent from the given node
    void removeParent(const Uuid& parentUuid);

    /// @brief Remove a child from the given node
    void removeChild(const Uuid& childUuid);
    void removeChild(std::shared_ptr<DagNode> child);

    /// @brief Outputs a string containing the hierarchy description of this node
    QString hierarchyDescription(int level = 0);

    /// @}

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @name Properties
    /// @{

    /// @brief Whether or not the node has children
    inline bool hasChildren() const { return numChildren() != 0; }

    /// @brief Whether or not the node has children
    inline bool hasParents() const { return numParents() != 0; }

    /// @brief Number of parent DAG nodes
    inline unsigned int numParents() const { return m_parents.size(); }

    /// @brief Number of child DAG nodes
    inline unsigned int numChildren() const { return m_children.size(); }

    /// @property Node Type
    /// @brief DAG node type
    static inline NodeType type() { return kBase; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "DagNode"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::DagNode"; }
    /// @}

protected:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    /// @brief Commands that have access to DAG node private methods
    friend class AddSceneObjectCommand;

    /// @}

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @name Constructors/
    /// @{
    DagNode();
    /// @}
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::vector<std::shared_ptr<DagNode>> parents();

    /// @brief returns a shared pointer to this object from the static node map
    std::shared_ptr<DagNode> sharedPtr();

    /// @brief Adds to static list of nodes
    void addToNodeMap();
    
    /// @brief Child DAG nodes of this node
    std::unordered_map<Uuid, std::shared_ptr<DagNode>> m_children;
    
    /// @brief Parent DAG nodes of this node
    std::unordered_map<Uuid, std::weak_ptr<DagNode>> m_parents;

private:

    /// @brief Adds child to the given node
    /// @Detailed Does not add to any other structures or alter other nodes
    void addChildPrivate(std::shared_ptr<DagNode> child);

    /// @brief Adds parent to the given node
    /// @Detailed Does not add to any other structures or alter other nodes
    void addParentPrivate(std::shared_ptr<DagNode> parent);

    /// @brief Remove child from the given node
    /// @Detailed Does not alter other nodes
    void removeChildPrivate(const Uuid& childUuid);

    /// @brief Remove parent from the given node
    /// @Detailed Does not alter other nodes
    void removeParentPrivate(const Uuid& parentUuid);

    /// @brief map of all DAG nodes
    static std::unordered_map<Uuid, std::shared_ptr<DagNode>> NODES;

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 
