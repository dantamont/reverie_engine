// Includes
#include "fortress/containers/math/GTransform.h"
#include "fortress/math/GInterpolation.h"

namespace rev {

TransformInterface::TransformInterface()
{
    setUniqueId();
}

TransformInterface::TransformInterface(const TransformInterface& other):
    m_inheritanceType(other.m_inheritanceType)
{
    if (other.m_parent) {
        m_parent = other.m_parent;
    }

    // Don't support copying with children, since transform no longer owns children
    if (m_children.size()) {
        throw("Error, copy not supported with transform that has children");
    }
}

TransformInterface::~TransformInterface()
{
    // Remove from parent
    if (m_parent) {
        m_parent->removeChild(this);
    }

    /// @note Transform does not own children, since transforms may not live as raw pointers
    for (TransformInterface* child : m_children) {
        child->clearParent(true);
    }
}

TransformInterface& TransformInterface::operator=(const TransformInterface& other)
{
    m_inheritanceType = other.m_inheritanceType;

    if (other.m_parent) {
        m_parent = other.m_parent;
    }

    // Don't support copying with children, since transform no longer owns children
    if (m_children.size()) {
        throw("Error, copy not supported with transform that has children");
    }

    return *this;
}

void TransformInterface::setParent(TransformInterface* p)
{
    // Clear old parent
    if (m_parent) {
        m_parent->removeChild(this);
    }

    // Set parent and refresh world matrix
    m_parent = p;
    p->addChild(this);
    computeWorldMatrix();
}

void TransformInterface::clearParent(bool recomputeWorld)
{
    if (m_parent) {
        m_parent->removeChild(this);
        m_parent = nullptr;
        if (recomputeWorld) {
            computeWorldMatrix();
        }
    }
}

bool TransformInterface::hasChild(const TransformInterface& child, size_t* idx)
{
    Uint64_t uuid = child.m_id;
    auto iter = std::find_if(m_children.begin(), m_children.end(),
        [&](TransformInterface* t) {
            return t->m_id == uuid;
        });

    if (iter != m_children.end()) {
        if (idx) {
            *idx = iter - m_children.begin();
        }
        return true;
    }
    else {
        return false;
    }
}

void TransformInterface::addChild(TransformInterface* c)
{
#ifdef DEBUG_MODE
    if (hasChild(*c)) {
        throw("Error, transform already has the given child");
    }
#endif
    m_children.push_back(c);
}

void TransformInterface::removeChild(TransformInterface* c)
{
    size_t idx;
    if (!hasChild(*c, &idx)) {
        throw("Error, transform does not have the specified child");
    }
    m_children.erase(m_children.begin() + idx);
}

Uint64_t TransformInterface::s_count = 0;

} // end namespacing