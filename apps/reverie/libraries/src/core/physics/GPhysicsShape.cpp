#include "core/physics/GPhysicsShape.h"

#include "core/physics/GPhysicsShapePrefab.h"
#include "core/physics/GPhysicsActor.h"
#include "core/physics/GPhysicsManager.h"
#include "core/physics/GPhysicsMaterial.h"
#include "core/physics/GPhysicsGeometry.h"
#include "core/scene/GScene.h"
#include "fortress/system/memory/GPointerTypes.h"

namespace rev {



// PhysicsShape

PhysicsShape::PhysicsShape() :
    m_pxShape(nullptr),
    m_prefab(nullptr) {
}

PhysicsShape::PhysicsShape(PhysicsShapePrefab & prefab, RigidBody * body) :
    m_prefab(&prefab),
    m_pxShape(prefab.createExclusive(*body)),
    m_body(body)
{
    m_prefab->addInstance(this);
}

PhysicsShape::~PhysicsShape()
{
    // Shapes should be released when detached from actors
    //PX_RELEASE(m_pxShape);
    if (m_prefab) {
        m_prefab->removeInstance(this);
    }
}

void PhysicsShape::detach() const
{
    if (!m_body) {
        Logger::Throw("Error, no body associated with shape");
    }

    if (m_body->actor()) {
        m_body->as<physx::PxRigidActor>()->detachShape(*m_pxShape, true);
    }
#ifdef DEBUG_MODE
    else {
        Logger::LogWarning("Body should always have actor");
    }
#endif
}

void PhysicsShape::reinitialize()
{
    // Delete old PxShape and replace on rigid actor with new one
    m_body->as<physx::PxRigidActor>()->detachShape(*m_pxShape, true);
    if (m_prefab) {
        // TODO: See if this check is only necessary for blueprint-originated objects
        m_pxShape = m_prefab->createExclusive(*m_body);
    }
}

void PhysicsShape::setPrefab(PhysicsShapePrefab & prefab, bool removeFromOldPrefab)
{
    if (m_prefab->getUuid() == prefab.getUuid()) {
        return;
    }

    // Remove as an instance from previous prefab
    if (removeFromOldPrefab && m_prefab) {
        m_prefab->removeInstance(this);
    }

    // Set new prefab and reinitialize
    m_prefab = &prefab;
    m_prefab->addInstance(this);
    reinitialize();
}

void PhysicsShape::prepareForDelete()
{
    m_prefab = nullptr;
}




}