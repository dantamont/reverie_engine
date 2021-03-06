///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GTestQuaternion.h"

#include <core/geometry/GMatrix.h>
#include <core/geometry/GQuaternion.h>
#include <core/GConstants.h>
#include <core/GLogger.h>

namespace rev {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//rev::Vector3f QuaternionTest::toEulerAngles(const rev::Quaternion & q)
//{
//
//    // roll (x-axis rotation)
//    double sinr_cosp = +2.0 * (q.w * q.x + q.y * q.z);
//    double cosr_cosp = +1.0 - 2.0 * (q.x * q.x + q.y * q.y);
//    double aX = atan2(sinr_cosp, cosr_cosp);
//
//    // pitch (y-axis rotation)
//    double aY;
//    double sinp = +2.0 * (q.w * q.y - q.z * q.x);
//    if (fabs(sinp) >= 1)
//        aY = copysign(M_PI / 2, sinp); // use 90 degrees if out of range
//    else
//        aY = asin(sinp);
//
//    // yaw (z-axis rotation)
//    double siny_cosp = +2.0 * (q.w * q.z + q.x * q.y);
//    double cosy_cosp = +1.0 - 2.0 * (q.y * q.y + q.z * q.z);
//    double aZ = atan2(siny_cosp, cosy_cosp);
//
//    auto angles = rev::Vector3f(aX, aY, aZ);
//    Logger::LogInfo(angles);
//    return angles;
//}

void rev::QuaternionTest::perform()
{
}

void QuaternionTest::construction()
{
    auto quaternion = rev::Quaternion();

    auto quat2 = rev::Quaternion(1, 2, 3, 4);
    auto quat3 = rev::Quaternion(quat2.asJson());

    assert_(quat2.x() == quat3.x() && quat3.x() == 1);
    assert_(quat2.y() == quat3.y() && quat3.y() == 2);
    assert_(quat2.z() == quat3.z() && quat3.z() == 3);
    assert_(quat2.w() == quat3.w() && quat3.w() == 4);
}

void QuaternionTest::axisAngle()
{
    auto quaternion1 = rev::Quaternion::fromAxisAngle({1, 0, 0}, 45 * rev::Constants::DEG_TO_RAD);
    auto mtx1 = quaternion1.toRotationMatrix4x4();

    auto mtx2 = rev::Matrix4x4();
    mtx2.addRotateX(45 * rev::Constants::DEG_TO_RAD);

    if (mtx1 != mtx2) {
        Logger::LogInfo("Axis Angle Matrix");
        Logger::LogInfo((QString)mtx1);
        Logger::LogInfo("Set rotation matrix");
        Logger::LogInfo((QString)mtx2);
    }
    else {
        Logger::LogInfo("Match");
        Logger::LogInfo((QString)mtx1);
    }

    auto quaternion2 = rev::Quaternion::fromAxisAngle({ 0, 1, 0 }, 45 * rev::Constants::DEG_TO_RAD);
    auto mtx3 = quaternion2.toRotationMatrix4x4();

    auto mtx4 = rev::Matrix4x4();
    mtx4.addRotateY(45 * rev::Constants::DEG_TO_RAD);

    auto quaternion3 = rev::Quaternion::fromAxisAngle({ 0, 0, 1 }, 45 * rev::Constants::DEG_TO_RAD);
    auto mtx5 = quaternion3.toRotationMatrix4x4();

    auto mtx6 = rev::Matrix4x4();
    mtx6.addRotateZ(45 * rev::Constants::DEG_TO_RAD);

    assert_(mtx1 == mtx2);
    assert_(mtx3 == mtx4);
    assert_(mtx5 == mtx6);
}



void QuaternionTest::slerp()
{
    auto quat1 = rev::Quaternion(0.0, 0.0, 0.1, 1);
    auto quat2 = rev::Quaternion(0.1, 0.1, 0.1, 1);

    std::vector<rev::Quaternion> quats = { quat1, quat2 };
    std::vector<float> weights = { 1, 1 };

    rev::Quaternion::Average(quats, weights);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}

