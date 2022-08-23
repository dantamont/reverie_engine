#include "core/physics/GPhysicsJoint.h"
#include "core/physics/GPhysicsActor.h"

#include "fortress/containers/math/GTransform.h"
#include "core/converters/GPhysxConverter.h"
#include "logging/GLogger.h"

namespace rev {


// PhysicsJoint

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
        Logger::Throw("Error, no joint defined");
    }
#endif

    auto* ra1 = a1.as<physx::PxRigidActor>();
    auto* ra2 = a2.as<physx::PxRigidActor>();

#ifdef DEBUG_MODE
    if (!ra2 || !ra2) {
        // Check that both actors are valid rigid actors
        Logger::Throw("Error, actors not rigid actors");
    }
#endif

    m_pJoint->setActors(ra1, ra2);
}

void to_json(json& orJson, const PhysicsJoint& korObject)
{
}

void from_json(const json& korJson, PhysicsJoint& orObject)
{
}




// FixedPhysicsJoint

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

void to_json(json& orJson, const FixedPhysicsJoint& korObject)
{
    ToJson<PhysicsJoint>(orJson, korObject);
    orJson["jointType"] = (int)korObject.jointType();
}

void from_json(const json& korJson, FixedPhysicsJoint& orObject)
{
    FromJson<PhysicsJoint>(korJson, orObject);
}

void FixedPhysicsJoint::initialize(const PhysicsActor & a0, const Transform & localFrame0, const PhysicsActor & a1, const Transform & localFrame1)
{
#ifdef DEBUG_MODE
    if (!m_pJoint) {
        Logger::Throw("Error, no joint defined");
    }
#endif

    auto* ra0 = a0.as<physx::PxRigidActor>();
    auto* ra1 = a1.as<physx::PxRigidActor>();

#ifdef DEBUG_MODE
    if (!ra0 || !ra1) {
        // Check that both actors are valid rigid actors
        Logger::Throw("Error, actors not rigid actors");
    }
#endif

    physx::PxTransform t0 = PhysxConverter::ToPhysX(localFrame0);
    physx::PxTransform t1 = PhysxConverter::ToPhysX(localFrame1);
    createJoint<PhysicsJointType::kFixed>(ra0, t0, ra1, t1);
}




}