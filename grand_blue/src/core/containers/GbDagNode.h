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
template<typename T>
class DagNode: public Object, public std::enable_shared_from_this<DagNode<T>>
{
public:
    //static_assert(
    //    std::is_base_of<DagNode<T>, T>::value,
    //    "T must be a descendant of DagNode<T>"
    //    );

    typedef std::vector<std::shared_ptr<T>> NodeVector;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @name Static Methods
    /// @{

    /// @brief Erases a node with the given UUID from the static list
    static void EraseFromNodeMap(const Uuid& uuid) {
        // Clear from static map
        if (s_nodes.find(uuid) == s_nodes.end()) {
            throw("Error, DAG node is not contained in, and therefore cannot be deleted from, the global node list");
        }

        s_nodes.erase(uuid);
    }

    /// @brief Returns map of DAG nodes 
    static tsl::robin_map<Uuid, std::shared_ptr<T>>& DagNodeMap() { return s_nodes; }

    /// @}

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @name Destructor
    /// @{

    virtual ~DagNode() {
    }

    /// @}

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @name Public Methods
    /// @{

    bool hasChild(const Uuid& uuid, size_t& outIndex) {
        bool found = false;
        size_t numChildren = m_children.size();
        for (size_t i = 0; i < numChildren; i++) {
            if (m_children[i]->getUuid() == uuid) {
                outIndex = i;
                return true;
            }
        }
        return found;
    }

    /// @brief Add a parent to the given node
    void addParent(const std::shared_ptr<T>& parent) {
        // Add parent to this node
        addParentPrivate(parent);

        // Add this node as a child of the parent node
        parent->addChildPrivate(sharedPtr());
    }

    /// @brief Add a child to the given node
    void addChild(const std::shared_ptr<T>& child) {
        // Add child to this node
        addChildPrivate(child);

        // Add this node as a parent of the child node
        child->addParentPrivate(sharedPtr());
    }

    /// @brief Remove a parent from the given node
    void removeParent(const Uuid& parentUuid) {
        if (std::shared_ptr<T> parent = m_parents.at(parentUuid).lock()) {
            parent->removeChildPrivate(m_uuid);
        }
        removeParentPrivate(parentUuid);
    }

    /// @brief Remove a child from the given node
    void removeChild(DagNode* child)
    {
        child->removeParentPrivate(m_uuid);
        removeChildPrivate(child->getUuid());
    }
    void removeChild(const std::shared_ptr<DagNode>& child) {
        child->removeParentPrivate(m_uuid);
        removeChildPrivate(child->getUuid());
    }

    /// @brief Outputs a string containing the hierarchy description of this node
    GString hierarchyDescription(int level = 0) {
        GString margin = QStringLiteral("\t").repeated(level);
        GString tabbed = margin + "\t";
        GString description = margin + m_uuid.asString();

        for (const std::shared_ptr<DagNode>& child : m_children) {
            if (child->hasChildren()) {
                description += "\n" + child->hierarchyDescription(level + 1);
            }
            else {
                description += "\n" + tabbed + child->getUuid().asString();
            }
        }

        return description;
    }

    /// @}

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @name Properties
    /// @{

    std::vector<std::shared_ptr<T>>& children() { return m_children; }
    const std::vector<std::shared_ptr<T>>& children() const { return m_children; }

    /// @brief Whether or not the node has children
    inline bool hasChildren() const { return numChildren() != 0; }

    /// @brief Whether or not the node has children
    inline bool hasParents() const { return numParents() != 0; }

    /// @brief Number of parent DAG nodes
    inline unsigned int numParents() const { return m_parents.size(); }

    /// @brief Number of child DAG nodes
    inline unsigned int numChildren() const { return m_children.size(); }

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

    DagNode(): Object() { 
    }

    /// @}
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    NodeVector parents() {

        // Generate vector of shared pointers to valid parents
        auto parents = std::vector<std::shared_ptr<T>>();
        parents.reserve(m_parents.size());
        for (auto& parentPair : m_parents) {
            if (std::shared_ptr<T> parent = parentPair.second.lock()) {
                parents.push_back(parent);
            }
        }

        // Update map of parents if some are invalid
        if (m_parents.size() != parents.size()) {
            m_parents.clear();
            for (const std::shared_ptr<T>& parent : parents) {
                Map::Emplace(m_parents, parent->getUuid(), parent);
            }
        }

        return parents;
    }

    /// @brief returns a shared pointer to this object from the static node map
    const std::shared_ptr<T>& sharedPtr() {
        return s_nodes.at(m_uuid);
    }

    /// @brief Adds to static list of nodes
    void addToNodeMap() {
        // Clear from static map
        if (s_nodes.find(m_uuid) != s_nodes.end()) {
            logCritical("Error, DAG node is already present in the global map");
            throw("Error, DAG node is already present in the global map");
        }

        Map::Emplace(s_nodes, m_uuid, std::static_pointer_cast<T>(shared_from_this()));
    }
    
    /// @brief Child DAG nodes of this node
    NodeVector m_children;
    
    /// @brief Parent DAG nodes of this node
    tsl::robin_map<Uuid, std::weak_ptr<T>> m_parents;

private:

    virtual void onAddChild(const std::shared_ptr<T>& child) = 0;
    virtual void onAddParent(const std::shared_ptr<T>& parent) = 0;
    virtual void onRemoveChild(const std::shared_ptr<T>& child) = 0;
    virtual void onRemoveParent(const std::shared_ptr<T>& parent) = 0;

    /// @brief Adds child to the given node
    /// @Detailed Does not add to any other structures or alter other nodes
    void addChildPrivate(const std::shared_ptr<T>& child) {
        size_t childIndex;
        if (hasChild(child->getUuid(), childIndex)) {
            logCritical("Error, node is already a child of the current node");
            throw std::range_error("Error, node is already a child of the current node");
        }
        m_children.push_back(child);

        onAddChild(child);
    }

    /// @brief Adds parent to the given node
    /// @Detailed Does not add to any other structures or alter other nodes
    void addParentPrivate(const std::shared_ptr<T>& parent) {
        if (m_parents.find(parent->getUuid()) != m_parents.end()) {
            logCritical("Error, node is already a parent of the current node");
            throw std::range_error("Error, node is already a parent of the current node");
        }
        Map::Emplace(m_parents, parent->getUuid(), parent);

        onAddParent(parent);
    }

    /// @brief Remove child from the given node
    /// @Detailed Does not alter other nodes
    void removeChildPrivate(const Uuid& childUuid) {
        size_t childIndex;
        if (!hasChild(childUuid, childIndex)) {
            throw std::range_error("Error, no child with the given UUID was found for removal");
        }
        onRemoveChild(m_children[childIndex]);
        m_children.erase(m_children.begin() + childIndex);
    }

    /// @brief Remove parent from the given node
    /// @Detailed Does not alter other nodes
    void removeParentPrivate(const Uuid& parentUuid) {
        if (!Map::HasKey(m_parents, parentUuid)) {
            throw std::range_error("Error, no parent with the given UUID was found for removal");
        }
        std::shared_ptr<T> parent = m_parents[parentUuid].lock();
        onRemoveParent(parent);
        m_parents.erase(parentUuid);
    }

    /// @brief map of all DAG nodes
    static tsl::robin_map<Uuid, std::shared_ptr<T>> s_nodes;

};

template<typename T>
tsl::robin_map<Uuid, std::shared_ptr<T>> DagNode<T>::s_nodes = {};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 
