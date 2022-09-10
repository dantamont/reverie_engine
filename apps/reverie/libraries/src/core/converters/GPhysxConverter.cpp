#include "core/converters/GPhysXConverter.h"
#include "fortress/containers/math/GVector.h"
#include "fortress/containers/math/GQuaternion.h"
#include "heave/kinematics/GTransform.h"

using namespace physx;

namespace rev {

physx::PxTransform PhysxConverter::ToPhysX(const TransformInterface& transform)
{
    const Vector3& position = transform.getPosition();
    const Quaternion& q = transform.getRotationQuaternion();
    physx::PxQuat quat(q.x(), q.y(), q.z(), q.w());
    return physx::PxTransform(position.x(), position.y(), position.z(), quat);
}

physx::PxVec3 PhysxConverter::ToPhysX(const Vector3d& vec3)
{
    return physx::PxVec3(vec3.x(), vec3.y(), vec3.z());
}

physx::PxVec3 PhysxConverter::ToPhysX(const Vector3& vec3)
{
    return physx::PxVec3(vec3.x(), vec3.y(), vec3.z());
}

Vector3 PhysxConverter::FromPhysX(const physx::PxVec3& vec)
{
    return Vector3(vec.x, vec.y, vec.z);
}

Quaternion PhysxConverter::FromPhysX(const physx::PxQuat& quat)
{
    return Quaternion(quat.x, quat.y, quat.z, quat.w);
}

} // End namespaces
