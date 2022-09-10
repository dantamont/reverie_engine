#pragma once

#include "fortress/numeric/GSizedTypes.h"
#include "core/physics/GPhysicsManager.h"

namespace physx {
class PxTransform;
class PxVec3;
class PxQuat;
}

namespace rev {

class TransformInterface;

/// @class PhyPhysxConvertersX 
/// @brief Class for converting a variety of containers to their PhysX variants
class PhysxConverter
{
public:

    /// @brief  Convert from transform to PhysX transform
    static physx::PxTransform ToPhysX(const TransformInterface& transform);
    static physx::PxVec3 ToPhysX(const Vector3d& vec3);
    static physx::PxVec3 ToPhysX(const Vector3& vec3);

    /// @brief Convert PhysX transform to transform
    template<typename TransformType>
    static TransformType FromPhysX(const physx::PxTransform& pt) {
        TransformType t;
        const PxVec3& pos = pxTransform.p;
        const PxQuat& quat = pxTransform.q;
        t.setPosition(PhysxConverter::FromPhysX(pos), false);
        t.setRotation(PhysxConverter::FromPhysX(quat), false);
        t.computeWorldMatrix();

        return t;
    }


    static Vector3 FromPhysX(const physx::PxVec3& vec3);
    static Quaternion FromPhysX(const physx::PxQuat& quat);

};



} // End namespaces
