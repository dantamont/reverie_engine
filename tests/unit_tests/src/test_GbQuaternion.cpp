///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "test_GbQuaternion.h"

#include "../../grand_blue/src/core/geometry/GbMatrix.h"
#include "../../grand_blue/src/core/geometry/GbQuaternion.h"
#include "../../grand_blue/src/core/GbConstants.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Gb::Vector3f TestQuaternion::toEulerAngles(const Gb::Quaternion & q)
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
//    auto angles = Gb::Vector3f(aX, aY, aZ);
//    logInfo(angles);
//    return angles;
//}

void TestQuaternion::construction()
{
    auto quaternion = Gb::Quaternion();

    auto quat2 = Gb::Quaternion(1, 2, 3, 4);
    auto quat3 = Gb::Quaternion(quat2.asJson());

    assert(quat2.x() == quat3.x() && quat3.x() == 1);
    assert(quat2.y() == quat3.y() && quat3.y() == 2);
    assert(quat2.z() == quat3.z() && quat3.z() == 3);
    assert(quat2.w() == quat3.w() && quat3.w() == 4);
}

void TestQuaternion::axisAngle()
{
    auto quaternion1 = Gb::Quaternion::fromAxisAngle({1, 0, 0}, 45 * Gb::Constants::DEG_TO_RAD);
    auto mtx1 = quaternion1.toRotationMatrix4x4();

    auto mtx2 = Gb::Matrix4x4f();
    mtx2.addRotateX(45 * Gb::Constants::DEG_TO_RAD);

    if (mtx1 != mtx2) {
        logInfo("Axis Angle Matrix");
        logInfo(mtx1);
        logInfo("Set rotation matrix");
        logInfo(mtx2);
    }
    else {
        logInfo("Match");
        logInfo(mtx1);
    }

    auto quaternion2 = Gb::Quaternion::fromAxisAngle({ 0, 1, 0 }, 45 * Gb::Constants::DEG_TO_RAD);
    auto mtx3 = quaternion2.toRotationMatrix4x4();

    auto mtx4 = Gb::Matrix4x4f();
    mtx4.addRotateY(45 * Gb::Constants::DEG_TO_RAD);

    auto quaternion3 = Gb::Quaternion::fromAxisAngle({ 0, 0, 1 }, 45 * Gb::Constants::DEG_TO_RAD);
    auto mtx5 = quaternion3.toRotationMatrix4x4();

    auto mtx6 = Gb::Matrix4x4f();
    mtx6.addRotateZ(45 * Gb::Constants::DEG_TO_RAD);

    QVERIFY(mtx1 == mtx2);
    QVERIFY(mtx3 == mtx4);
    QVERIFY(mtx5 == mtx6);
}



void TestQuaternion::slerp()
{
    auto quat1 = Gb::Quaternion(0.0, 0.0, 0.1, 1);
    auto quat2 = Gb::Quaternion(0.1, 0.1, 0.1, 1);

    std::vector<Gb::Quaternion> quats = { quat1, quat2 };
    std::vector<float> weights = { 1, 1 };

    Gb::Quaternion::slerp(quats, weights);

}
