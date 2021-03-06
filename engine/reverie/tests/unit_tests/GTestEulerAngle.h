#ifndef TEST_GB_EULER_ANGLE_H
#define TEST_GB_EULER_ANGLE_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "../GTest.h"

#include <core/GLogger.h>
#include <core/geometry/GMatrix.h>
#include <core/geometry/GQuaternion.h>
#include <core/GConstants.h>
#include <core/geometry/GEulerAngles.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace rev {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class EulerAnglesTest : public Test
{

public:

    EulerAnglesTest() : Test() {}
    ~EulerAnglesTest() {}

    virtual void perform() {
        {
            EulerAngles defaultAngles = EulerAngles(1, 2, 3, { Axis::kAxisZ, Axis::kAxisY, Axis::kAxisX }, RotationSpace::kBody);
            EulerAngles testTypeConversion = EulerAngles(1, 2, 3, EulerAngles::kZYX, RotationSpace::kBody);
            bool isGood = defaultAngles == testTypeConversion;

            // Test intrinsic rotation vs reverse extrinsic
            EulerAngles intrinsic = EulerAngles(1.0f, 2.0f, 3.0f, { Axis::kAxisZ, Axis::kAxisY, Axis::kAxisX }, RotationSpace::kInertial);
            EulerAngles extrinsic = EulerAngles(3.0f, 2.0f, 1.0f, { Axis::kAxisX, Axis::kAxisY, Axis::kAxisZ }, RotationSpace::kBody);
            Logger::LogInfo("Intrinsic: " + QString(intrinsic.toRotationMatrixF()));
            Logger::LogInfo("Extrinsic: " + QString(extrinsic.toRotationMatrixF()));
            bool sameReversed = intrinsic.toRotationMatrixF() == extrinsic.toRotationMatrixF();

            // Test conversion between types
            EulerAngles xyz = EulerAngles(1.0f, 0.7f, 0.5f, EulerAngles::kXYZ, RotationSpace::kBody);
            EulerAngles xzy = xyz.toAngles(EulerAngles::kXZY, RotationSpace::kBody);
            EulerAngles yxz = xyz.toAngles(EulerAngles::kYXZ, RotationSpace::kBody);
            EulerAngles yzx = xyz.toAngles(EulerAngles::kYZX, RotationSpace::kBody);
            EulerAngles zxy = xyz.toAngles(EulerAngles::kZXY, RotationSpace::kBody);
            EulerAngles zyx = xyz.toAngles(EulerAngles::kZYX, RotationSpace::kBody);

            assert_(isGood);
            assert_(xzy.angles() == Vector3f(1.3384, 0.3754432, 0.7648796));
            assert_(yxz.angles() == Vector3f(1.0004413, 0.6991829, 1.2870502));
            assert_(yzx.angles() == Vector3f(-0.1449225, 0.8253185, 1.2494136));
            assert_(zxy.angles() == Vector3f(1.0419727, 1.1321679, -0.232755));
            assert_(zyx.angles() == Vector3f(0.8305686, -0.0981179, 1.1425862));
            assert_(sameReversed);
        }
    }

private:
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// end namespace
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

#endif