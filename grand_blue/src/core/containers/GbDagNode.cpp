///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GbDagNode.h"

// Standard Includes

// External

// Project

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Implementations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::unordered_map<Uuid, std::shared_ptr<DagNode>> DagNode::NODES = {};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DagNode::DagNode() : Object()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DagNode::~DagNode()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<DagNode> DagNode::create()
{
    DagNode dagNode = DagNode();
    auto dagNodePtr = std::make_shared<DagNode>(std::move(dagNode));
    dagNodePtr->addToNodeMap();

    return dagNodePtr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DagNode::eraseFromNodeMap(const Uuid& uuid)
{
    // Clear from static map
    if (NODES.find(uuid) == NODES.end()) {
        throw("Error, DAG node is not contained in, and therefore cannot be deleted from, the global node list");
    }

    NODES.erase(uuid);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DagNode::addParent(std::shared_ptr<DagNode> parent)
{
    // Add parent to this node
    addParentPrivate(parent);

    // Add this node as a child of the parent node
    parent->addChildPrivate(sharedPtr());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DagNode::addChild(std::shared_ptr<DagNode> child)
{
    // Add child to this node
    addChildPrivate(child);

    // Add this node as a parent of the child node
    child->addParentPrivate(sharedPtr());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DagNode::removeParent(const Uuid& parentUuid)
{
    if (std::shared_ptr<DagNode> parent = m_parents.at(parentUuid).lock()) {
        parent->removeChildPrivate(m_uuid);
    }

    removeParentPrivate(parentUuid);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DagNode::removeChild(const Uuid& childUuid)
{
    std::shared_ptr<DagNode>& child = m_children.at(childUuid);
    child->removeParentPrivate(m_uuid);

    removeChildPrivate(childUuid);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DagNode::removeChild(std::shared_ptr<DagNode> child)
{
    child->removeParentPrivate(m_uuid);

    removeChildPrivate(child->getUuid());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString DagNode::hierarchyDescription(int level)
{
    QString margin = QStringLiteral("\t").repeated(level);
    QString tabbed = margin + "\t";
    QString description = margin + m_uuid.asString();

    for (auto& childPair : m_children) {
        if (childPair.second->hasChildren()) {
            description += "\n" + childPair.second->hierarchyDescription(level + 1);
        }
        else {
            description += "\n" + tabbed + childPair.second->getUuid().asString();
        }
    }

    return description;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<std::shared_ptr<DagNode>> DagNode::parents()
{

    // Generate vector of shared pointers to valid parents
    auto parents = std::vector<std::shared_ptr<DagNode>>();
    parents.reserve(m_parents.size());
    for (auto& parentPair : m_parents) {
        if (std::shared_ptr<DagNode> parent = parentPair.second.lock()) {
            parents.push_back(parent);
        }
    }

    // Update map of parents if some are invalid
    if (m_parents.size() != parents.size()) { 
        m_parents.clear();
        for (const std::shared_ptr<DagNode>& parent : parents) {
            Map::Emplace(m_parents, parent->getUuid(), parent);
        }
    }

    return parents;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<DagNode> DagNode::sharedPtr()
{
    return NODES.at(m_uuid);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DagNode::addToNodeMap()
{
    // Clear from static map
    if (NODES.find(m_uuid) != NODES.end()) {
        logCritical("Error, DAG node is already present in the global map");
        throw("Error, DAG node is already present in the global map");
    }

    Map::Emplace(NODES, m_uuid, shared_from_this());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DagNode::addChildPrivate(std::shared_ptr<DagNode> child)
{
    if (m_children.find(child->getUuid()) != m_children.end()) {
        logCritical("Error, node is already a child of the current node");
        throw std::range_error("Error, node is already a child of the current node");
    }
    Map::Emplace(m_children, child->getUuid(), child);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DagNode::addParentPrivate(std::shared_ptr<DagNode> parent)
{
    if (m_parents.find(parent->getUuid()) != m_parents.end()) {
        logCritical("Error, node is already a parent of the current node");
        throw std::range_error("Error, node is already a parent of the current node");
    }
    Map::Emplace(m_parents, parent->getUuid(), parent);

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DagNode::removeChildPrivate(const Uuid& childUuid)
{
    if (!Map::HasKey(m_children, childUuid)) {
        throw std::range_error("Error, no child with the given UUID was found for removal");
    }
    m_children.erase(childUuid);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DagNode::removeParentPrivate(const Uuid& parentUuid)
{
    if (!Map::HasKey(m_parents, parentUuid)) {
        throw std::range_error("Error, no parent with the given UUID was found for removal");
    }
    m_parents.erase(parentUuid);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing
