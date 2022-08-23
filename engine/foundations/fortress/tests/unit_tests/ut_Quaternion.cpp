///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <gtest/gtest.h>

#include "fortress/containers/math/GMatrix.h"
#include "fortress/containers/math/GQuaternion.h"
#include <fortress/constants/GConstants.h>

namespace rev {

TEST(QuaternionTest, construction)
{
    auto quaternion = rev::Quaternion();

    auto quat2 = rev::Quaternion(1, 2, 3, 4);
    json quatJson(quat2);
    auto quat3 = rev::Quaternion(quatJson);

    EXPECT_EQ(quat2.x(), 1);
    EXPECT_EQ(quat2.y(), 2);
    EXPECT_EQ(quat2.z(), 3);
    EXPECT_EQ(quat2.w(), 4);
    EXPECT_EQ(quat2.x(), quat3.x());
    EXPECT_EQ(quat2.y(), quat3.y());
    EXPECT_EQ(quat2.z(), quat3.z());
    EXPECT_EQ(quat2.w(), quat3.w());
}

TEST(QuaternionTest, axisAngle)
{
    auto quaternion1 = rev::Quaternion::fromAxisAngle({ 1, 0, 0 }, 45 * rev::Constants::DegToRad);
    auto mtx1 = quaternion1.toRotationMatrix4x4();

    auto mtx2 = rev::Matrix4x4();
    mtx2.addRotateX(45 * rev::Constants::DegToRad);

    if (mtx1 != mtx2) {
        std::cout << ("Axis Angle Matrix");
        std::cout << ((std::string)mtx1);
        std::cout << ("Set rotation matrix");
        std::cout << ((std::string)mtx2);
    }
    else {
        std::cout << ("Match");
        std::cout << ((std::string)mtx1);
    }

    auto quaternion2 = rev::Quaternion::fromAxisAngle({ 0, 1, 0 }, 45 * rev::Constants::DegToRad);
    auto mtx3 = quaternion2.toRotationMatrix4x4();

    auto mtx4 = rev::Matrix4x4();
    mtx4.addRotateY(45 * rev::Constants::DegToRad);

    auto quaternion3 = rev::Quaternion::fromAxisAngle({ 0, 0, 1 }, 45 * rev::Constants::DegToRad);
    auto mtx5 = quaternion3.toRotationMatrix4x4();

    auto mtx6 = rev::Matrix4x4();
    mtx6.addRotateZ(45 * rev::Constants::DegToRad);

    EXPECT_EQ(mtx1 ,  mtx2);
    EXPECT_EQ(mtx3 ,  mtx4);
    EXPECT_EQ(mtx5 ,  mtx6);
}



TEST(QuaternionTest, slerp)
{
    auto quat1 = rev::Quaternion(0.0, 0.0, 0.1, 1);
    auto quat2 = rev::Quaternion(0.1, 0.1, 0.1, 1);

    std::vector<rev::Quaternion> quats = { quat1, quat2 };
    std::vector<float> weights = { 1, 1 };

    rev::Quaternion::Average(quats, weights);

}


} // End namespaces

