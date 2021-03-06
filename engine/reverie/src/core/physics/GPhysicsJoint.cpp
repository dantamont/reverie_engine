#include "GPhysicsJoint.h"
#include "GPhysicsActor.h"

#include <core/geometry/GTransform.h>

namespace rev {

//////////////////////////////////////////////////////////////////////////////////////////////////
// PhysicsJoint
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsJoint::PhysicsJoint()
{
}

PhysicsJoint::~PhysicsJoint()
{
    PX_RELEASE(m_pJoint);
}

void PhysicsJoint::setActors(const PhysicsActor & a1, const PhysicsActor & a2)
{
#ifdef DEBUG_MODE
    if (!m_pJoint) {
        throw("Error, no joint defined");
    }
#endif

    auto* ra1 = a1.as<physx::PxRigidActor>();
    auto* ra2 = a2.as<physx::PxRigidActor>();

#ifdef DEBUG_MODE
    if (!ra2 || !ra2) {
        // Check that both actors are valid rigid actors
        throw("Error, actors not rigid actors");
    }
#endif

    m_pJoint->setActors(ra1, ra2);
}

QJsonValue PhysicsJoint::asJson(const SerializationContext & context) const
{
    QJsonObject object;
    return object;
}

void PhysicsJoint::loadFromJson(const QJsonValue & json, const SerializationContext & context)
{
}



//////////////////////////////////////////////////////////////////////////////////////////////////
// FixedPhysicsJoint
//////////////////////////////////////////////////////////////////////////////////////////////////
FixedPhysicsJoint::FixedPhysicsJoint()
{
}

FixedPhysicsJoint::FixedPhysicsJoint(const PhysicsActor & a0, const Transform & localFrame0, const PhysicsActor & a1, const Transform & localFrame1)
{
    initialize(a0, localFrame0, a1, localFrame1);
}

FixedPhysicsJoint::~FixedPhysicsJoint()
{
}

QJsonValue FixedPhysicsJoint::asJson(const SerializationContext & context) const
{
    QJsonObject object = PhysicsJoint::asJson(context).toObject();
    object["jointType"] = (int)jointType();
    return object;
}

void FixedPhysicsJoint::loadFromJson(const QJsonValue & json, const SerializationContext & context)
{
    PhysicsJoint::loadFromJson(json, context);
}

void FixedPhysicsJoint::initialize(const PhysicsActor & a0, const Transform & localFrame0, const PhysicsActor & a1, const Transform & localFrame1)
{
#ifdef DEBUG_MODE
    if (!m_pJoint) {
        throw("Error, no joint defined");
    }
#endif

    auto* ra0 = a0.as<physx::PxRigidActor>();
    auto* ra1 = a1.as<physx::PxRigidActor>();

#ifdef DEBUG_MODE
    if (!ra0 || !ra1) {
        // Check that both actors are valid rigid actors
        throw("Error, actors not rigid actors");
    }
#endif

    physx::PxTransform t0 = localFrame0.asPhysX();
    physx::PxTransform t1 = localFrame1.asPhysX();
    createJoint<PhysicsJointType::kFixed>(ra0, t0, ra1, t1);
}



//////////////////////////////////////////////////////////////////////////////////////////////////
}