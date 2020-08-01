///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "test_GbEulerAngle.h"

#include "../../grand_blue/src/core/geometry/GbMatrix.h"
#include "../../grand_blue/src/core/geometry/GbQuaternion.h"
#include "../../grand_blue/src/core/GbConstants.h"
#include "../../grand_blue/src/core/geometry/GbEulerAngles.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace Gb;

void TestEulerAngles::toAngle()
{
    EulerAngles defaultAngles = EulerAngles(1, 2, 3, { Axis::kAxisZ, Axis::kAxisY, Axis::kAxisX }, RotationSpace::kBody);
    EulerAngles testTypeConversion = EulerAngles(1, 2, 3, EulerAngles::kZYX, RotationSpace::kBody);
    bool isGood = defaultAngles == testTypeConversion;

    // Test intrinsic rotation vs reverse extrinsic
    EulerAngles intrinsic = EulerAngles(1.0f, 2.0f, 3.0f, {Axis::kAxisZ, Axis::kAxisY, Axis::kAxisX}, RotationSpace::kInertial);
    EulerAngles extrinsic = EulerAngles(3.0f, 2.0f, 1.0f, { Axis::kAxisX, Axis::kAxisY, Axis::kAxisZ }, RotationSpace::kBody);
    logInfo("Intrinsic: " + QString(intrinsic.toRotationMatrixF()));
    logInfo("Extrinsic: " + QString(extrinsic.toRotationMatrixF()));
    bool sameReversed = intrinsic.toRotationMatrixF() == extrinsic.toRotationMatrixF();

    // Test conversion between types
    EulerAngles xyz = EulerAngles(1.0f, 0.7f, 0.5f, EulerAngles::kXYZ, RotationSpace::kBody);
    EulerAngles xzy = xyz.toAngles(EulerAngles::kXZY, RotationSpace::kBody);
    EulerAngles yxz = xyz.toAngles(EulerAngles::kYXZ, RotationSpace::kBody);
    EulerAngles yzx = xyz.toAngles(EulerAngles::kYZX, RotationSpace::kBody);
    EulerAngles zxy = xyz.toAngles(EulerAngles::kZXY, RotationSpace::kBody);
    EulerAngles zyx = xyz.toAngles(EulerAngles::kZYX, RotationSpace::kBody);

    QVERIFY(isGood);
    QVERIFY(xzy.angles() == Vector3f(1.3384, 0.3754432, 0.7648796));
    QVERIFY(yxz.angles() == Vector3f(1.0004413, 0.6991829, 1.2870502));
    QVERIFY(yzx.angles() == Vector3f(-0.1449225, 0.8253185, 1.2494136));
    QVERIFY(zxy.angles() == Vector3f(1.0419727, 1.1321679, -0.232755));
    QVERIFY(zyx.angles() == Vector3f(0.8305686, -0.0981179, 1.1425862));
    QVERIFY(sameReversed);
}