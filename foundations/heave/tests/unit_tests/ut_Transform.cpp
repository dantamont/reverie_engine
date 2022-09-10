#include <gtest/gtest.h>
#include "fortress/constants/GConstants.h"
#include "heave/kinematics/GTransform.h"


// Tests
using namespace rev;

constexpr double dr = Constants::DegToRad;

TEST(TransformTest, TestRotate)
{
    // Expected ZYX rotation matrix
    EulerAngles angles(20*dr, 45*dr, 90*dr);
    Quaternion quat = Quaternion::fromEulerAngles(angles);
    Matrix3x3 mat{ std::array<Vector3, 3>{
        Vector3{0.6644630, 0.6644630,  0.3420202},
        Vector3{0.2418448, 0.2418448, -0.9396926},
        Vector3{ -0.7071068, 0.7071068, 0} }
    };
    const Matrix4x4 expectedMat(mat.transposed());

    Transform transform;
    transform.addRotation(angles);
    EXPECT_EQ(expectedMat, transform.worldMatrix());
}

TEST(TransformTest, TestScale)
{
    // Expected ZYX rotation matrix
    EulerAngles angles(20 * dr, 45 * dr, 90 * dr);
    Quaternion quat = Quaternion::fromEulerAngles(angles);
    double scale = 7;
    Matrix3x3 mat{ std::array<Vector3, 3>{
        Vector3{0.6644630, 0.6644630,  0.3420202},
        Vector3{0.2418448, 0.2418448, -0.9396926},
        Vector3{ -0.7071068, 0.7071068, 0} }
    };
    Matrix4x4 expectedMat(mat.transposed());
    expectedMat.column(0) *= scale;
    expectedMat.column(1) *= scale;
    expectedMat.column(2) *= scale;

    Transform transform;
    transform.setScale(scale);
    transform.addRotation(angles);
    EXPECT_EQ(expectedMat, transform.worldMatrix());
}

TEST(TransformTest, TestTranslate)
{
    // Expected ZYX rotation matrix
    EulerAngles angles(20 * dr, 45 * dr, 90 * dr);
    Quaternion quat = Quaternion::fromEulerAngles(angles);
    double scale = 7;
    Matrix3x3 mat{ std::array<Vector3, 3>{
        Vector3{0.6644630, 0.6644630,  0.3420202},
        Vector3{0.2418448, 0.2418448, -0.9396926},
        Vector3{ -0.7071068, 0.7071068, 0} }
    };
    const Vector3 translation(1, 2, 3);
    Matrix4x4 expectedMat(mat.transposed());
    expectedMat.column(0) *= scale;
    expectedMat.column(1) *= scale;
    expectedMat.column(2) *= scale;
    expectedMat(0, 3) = translation[0];
    expectedMat(1, 3) = translation[1];
    expectedMat(2, 3) = translation[2];

    Transform transform;
    transform.setScale(scale);
    transform.addRotation(angles);
    transform.setTranslation(translation);
    EXPECT_EQ(expectedMat, transform.worldMatrix());
}

TEST(TransformTest, TestParent)
{
    // Expected ZYX rotation matrix
    EulerAngles angles(20 * dr, 45 * dr, 90 * dr);
    Quaternion quat = Quaternion::fromEulerAngles(angles);
    double scale = 7;
    Matrix3x3 mat{ std::array<Vector3, 3>{
        Vector3{0.6644630, 0.6644630,  0.3420202},
        Vector3{0.2418448, 0.2418448, -0.9396926},
        Vector3{ -0.7071068, 0.7071068, 0} }
    };
    const Vector3 translation(1, 2, 3);
    Matrix4x4 expectedMat(mat.transposed());
    expectedMat.column(0) *= scale;
    expectedMat.column(1) *= scale;
    expectedMat.column(2) *= scale;
    expectedMat(0, 3) = translation[0];
    expectedMat(1, 3) = translation[1];
    expectedMat(2, 3) = translation[2];

    Transform parentTransform;
    Transform transform;
    transform.setParent(&parentTransform);
    transform.setScale(scale);
    transform.addRotation(angles);
    parentTransform.setTranslation(translation);
    EXPECT_EQ(expectedMat, transform.worldMatrix());

    EXPECT_EQ(parentTransform.hasChild(transform), true);
}


TEST(TransformTest, TestJson)
{
    // Expected ZYX rotation matrix
    EulerAngles angles(20 * dr, 45 * dr, 90 * dr);
    Quaternion quat = Quaternion::fromEulerAngles(angles);
    double scale = 7;
    Matrix3x3 mat{ std::array<Vector3, 3>{
        Vector3{0.6644630, 0.6644630,  0.3420202},
        Vector3{0.2418448, 0.2418448, -0.9396926},
        Vector3{ -0.7071068, 0.7071068, 0} }
    };
    const Vector3 translation(1, 2, 3);
    Matrix4x4 expectedMat(mat.transposed());
    expectedMat.column(0) *= scale;
    expectedMat.column(1) *= scale;
    expectedMat.column(2) *= scale;
    expectedMat(0, 3) = translation[0];
    expectedMat(1, 3) = translation[1];
    expectedMat(2, 3) = translation[2];

    Transform transform;
    transform.setScale(scale);
    transform.addRotation(angles);
    transform.setTranslation(translation);

    json tj = transform;
    EXPECT_EQ(tj.contains("translation"), true);
    EXPECT_EQ(tj.contains("rotation"), true);
    EXPECT_EQ(tj.contains("scale"), true);
}

TEST(TransformTest, TestPointerTransform)
{
    // Expected ZYX rotation matrix
    EulerAngles angles(20 * dr, 45 * dr, 90 * dr);
    Quaternion quat = Quaternion::fromEulerAngles(angles);
    double scale = 7;
    Matrix3x3 mat{ std::array<Vector3, 3>{
        Vector3{0.6644630, 0.6644630,  0.3420202},
        Vector3{0.2418448, 0.2418448, -0.9396926},
        Vector3{ -0.7071068, 0.7071068, 0} }
    };
    const Vector3 translation(1, 2, 3);
    Matrix4x4 expectedMat(mat.transposed());
    expectedMat.column(0) *= scale;
    expectedMat.column(1) *= scale;
    expectedMat.column(2) *= scale;
    expectedMat(0, 3) = translation[0];
    expectedMat(1, 3) = translation[1];
    expectedMat(2, 3) = translation[2];

    Matrix4x4 matrix;
    Matrix4x4 parentMatrix;
    PointerTransform transform(&matrix);
    PointerTransform parentTransform(&parentMatrix);
    transform.setParent(&parentTransform);
    transform.setScale(scale);
    transform.addRotation(angles);
    parentTransform.setTranslation(translation);

    EXPECT_EQ(expectedMat, transform.worldMatrix());
    EXPECT_EQ(parentTransform.hasChild(transform), true);
}

TEST(TransformTest, TestIndexedTransform)
{
    // Expected ZYX rotation matrix
    EulerAngles angles(20 * dr, 45 * dr, 90 * dr);
    Quaternion quat = Quaternion::fromEulerAngles(angles);
    double scale = 7;
    Matrix3x3 mat{ std::array<Vector3, 3>{
        Vector3{0.6644630, 0.6644630,  0.3420202},
        Vector3{0.2418448, 0.2418448, -0.9396926},
        Vector3{ -0.7071068, 0.7071068, 0} }
    };
    const Vector3 translation(1, 2, 3);
    Matrix4x4 expectedMat(mat.transposed());
    expectedMat.column(0) *= scale;
    expectedMat.column(1) *= scale;
    expectedMat.column(2) *= scale;
    expectedMat(0, 3) = translation[0];
    expectedMat(1, 3) = translation[1];
    expectedMat(2, 3) = translation[2];

    std::vector<Matrix4x4> matrices{Matrix4x4(), Matrix4x4()};
    IndexedTransform transform(matrices, 0);
    IndexedTransform parentTransform(matrices, 1);
    transform.setParent(&parentTransform);
    transform.setScale(scale);
    transform.addRotation(angles);
    parentTransform.setTranslation(translation);

    EXPECT_EQ(expectedMat, transform.worldMatrix());
    EXPECT_EQ(parentTransform.hasChild(transform), true);
}