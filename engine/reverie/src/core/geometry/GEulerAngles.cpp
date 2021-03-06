#include "GEulerAngles.h"
#include "GMatrix.h"
#include "GQuaternion.h"

namespace rev {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EulerAngles::EulerAngles():
    m_angles(Vector3f(0, 0, 0)),
    m_rotationOrder({ Axis::kAxisZ, Axis::kAxisY, Axis::kAxisX }),
    m_rotationSpace(RotationSpace::kBody)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EulerAngles::EulerAngles(const Quaternion & quaternion, EulerType rotationType, RotationSpace rotationSpace)
{
    *this = toAngles(quaternion.toRotationMatrix().toDoubleMatrix(), rotationType, rotationSpace);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EulerAngles::EulerAngles(float ax, float ay, float az, EulerType rotationType, RotationSpace rotationSpace) :
    m_angles(Vector3f(ax, ay, az)),
    m_rotationOrder(typeToAxes(rotationType, rotationSpace)),
    m_rotationSpace(rotationSpace)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EulerAngles::EulerAngles(float ax, float ay, float az, const Axes & rotationOrder, RotationSpace rotationSpace) :
    m_angles(Vector3f(ax, ay, az)),
    m_rotationOrder(rotationOrder),
    m_rotationSpace(rotationSpace)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EulerAngles::EulerAngles(const Vector3f & angles, const Axes & rotationOrder, RotationSpace rotationSpace):
    m_angles(angles),
    m_rotationOrder(rotationOrder),
    m_rotationSpace(rotationSpace)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EulerAngles::~EulerAngles()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Matrix4x4 EulerAngles::toRotationMatrixF() const
{
    return toRotationMatrix().toFloatMatrix();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Matrix4x4d EulerAngles::toRotationMatrix() const
{
    Matrix4x4d transformMatrix = Matrix4x4d::EmptyMatrix();
    switch (m_rotationSpace) {
    case RotationSpace::kInertial:
        transformMatrix = std::move(computeExtrinsicRotationMatrix(m_angles, m_rotationOrder));
        break;
    case RotationSpace::kBody:
        transformMatrix = std::move(computeIntrinsicRotationMatrix(m_angles, m_rotationOrder));
        break;
    }

    return transformMatrix;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EulerAngles::EulerType EulerAngles::getType() const
{
    return axesToType(m_rotationOrder, m_rotationSpace);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion EulerAngles::toQuaternion() const
{
    return Quaternion::fromEulerAngles(*this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Matrix4x4d EulerAngles::computeExtrinsicRotationMatrix(const Vector3f & angles, const Axes & axisOrder)
{
    Matrix4x4d rotationMtx;
    Matrix4x4d axisRotationMtx;

    Vector3d axisVector;
    for (size_t i = 0; i < axisOrder.size(); i++) {
        Axis axis = axisOrder.at(i);
        axisRotationMtx.setToIdentity();
        switch (axis) {
        case Axis::kAxisX:
            axisVector = Vector3d(1, 0, 0);
            axisRotationMtx.addRotate(axisVector, angles[i]);
            rotationMtx = axisRotationMtx * rotationMtx;
            break;
        case Axis::kAxisY:
            axisVector = Vector3d(0, 1, 0);
            axisRotationMtx.addRotate(axisVector, angles[i]);
            rotationMtx = axisRotationMtx * rotationMtx;
            break;

        case Axis::kAxisZ:
            axisVector = Vector3d(0, 0, 1);
            axisRotationMtx.addRotate(axisVector, angles[i]);
            rotationMtx = axisRotationMtx * rotationMtx;
            break;
        }
    }

    return rotationMtx;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Matrix4x4d EulerAngles::computeIntrinsicRotationMatrix(const Vector3f & angles, const Axes & axisOrder)
{
    Matrix4x4d rotationMtx;
    Matrix4x4d axisRotationMtx;

    // See https://en.wikipedia.org/wiki/Davenport_chained_rotations,
    // Intrinsic rotations are (amazingly) somehow just the reverse application of extrinsic ones
    Vector3d axisVector = Vector3d::EmptyVector();
    for (int i = axisOrder.size() - 1; i >= 0; i--) {
        Axis axis = axisOrder.at(i);
        axisRotationMtx.setToIdentity();
        switch (axis) {
        case Axis::kAxisX:
            axisVector = Vector3d(1, 0, 0);
            axisRotationMtx.addRotate(axisVector, angles[i]);
            rotationMtx = axisRotationMtx * rotationMtx;
            break;
        case Axis::kAxisY:
            axisVector = Vector3d(0, 1, 0);
            axisRotationMtx.addRotate(axisVector, angles[i]);
            rotationMtx = axisRotationMtx * rotationMtx;
            break;

        case Axis::kAxisZ:
            axisVector = Vector3d(0, 0, 1);
            axisRotationMtx.addRotate(axisVector, angles[i]);
            rotationMtx = axisRotationMtx * rotationMtx;
            break;
        }
    }

    //Vector3 rotatedAxisVector;
    //for (size_t i = 0; i < axisOrder.size(); i++) {
    //    Axis axis = axisOrder.at(i);
    //    axisRotationMtx.setToIdentity();
    //    switch (axis) {
    //    case Axis::kAxisX:
    //        axisVector = Vector3(1, 0, 0);
    //        rotatedAxisVector = rotationMtx.multPoint(axisVector);
    //        axisRotationMtx.addRotate(rotatedAxisVector, angles[i]);
    //        rotationMtx = axisRotationMtx * rotationMtx;
    //        break;
    //    case Axis::kAxisY:
    //        axisVector = Vector3(0, 1, 0);
    //        rotatedAxisVector = rotationMtx.multPoint(axisVector);
    //        axisRotationMtx.addRotate(rotatedAxisVector, angles[i]);
    //        rotationMtx = axisRotationMtx * rotationMtx;
    //        break;

    //    case Axis::kAxisZ:
    //        axisVector = Vector3(0, 0, 1);
    //        rotatedAxisVector = rotationMtx.multPoint(axisVector);
    //        axisRotationMtx.addRotate(rotatedAxisVector, angles[i]);
    //        rotationMtx = axisRotationMtx * rotationMtx;
    //        break;
    //    }
    //}

    return rotationMtx;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EulerAngles EulerAngles::toAngles(EulerType type, RotationSpace space) const
{
    EulerType thisType = getType();

    if (thisType == type && m_rotationSpace == space) {
        // Return this set of angles if it matches the input type
        return *this;
    }
    Matrix3x3d matrix = toRotationMatrix();
    return EulerAngles::toAngles(std::move(matrix), type, space);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EulerAngles::EulerType EulerAngles::axesToType(const Axes & axes, RotationSpace type)
{
    if (type == RotationSpace::kBody) {
        // Intrinsic
        if (axes[2] == Axis::kAxisX) {
            if (axes[1] == Axis::kAxisY) {
                if (axes[0] == Axis::kAxisZ) {
                    return kZYX;
                }
                else if (axes[0] == Axis::kAxisX) {
                    return kXYX;
                }
            }
            else if (axes[1] == Axis::kAxisZ) {
                if (axes[0] == Axis::kAxisY) {
                    return kYZX;
                }
                else if (axes[0] == Axis::kAxisX) {
                    return kXZX;
                }
            }
        }
        else if (axes[2] == Axis::kAxisY) {
            if (axes[1] == Axis::kAxisX) {
                if (axes[0] == Axis::kAxisZ) {
                    return kZXY;
                }
                else if (axes[0] == Axis::kAxisY) {
                    return kYXY;
                }
            }
            else if (axes[1] == Axis::kAxisZ) {
                if (axes[0] == Axis::kAxisY) {
                    return kYZY;
                }
                else if (axes[0] == Axis::kAxisX) {
                    return kXZY;
                }
            }
        }
        else if (axes[2] == Axis::kAxisZ) {
            if (axes[1] == Axis::kAxisY) {
                if (axes[0] == Axis::kAxisZ) {
                    return kZYZ;
                }
                else if (axes[0] == Axis::kAxisX) {
                    return kXYZ;
                }
            }
            else if (axes[1] == Axis::kAxisX) {
                if (axes[0] == Axis::kAxisY) {
                    return kYXZ;
                }
                else if (axes[0] == Axis::kAxisZ) {
                    return kZXZ;
                }
            }
        }
    }
    else{
        // Extrinsic
        if (axes[2] == Axis::kAxisX) {
            if (axes[1] == Axis::kAxisY) {
                if (axes[0] == Axis::kAxisZ) {
                    return kXYZ;
                }
                else if (axes[0] == Axis::kAxisX) {
                    return kXYX;
                }
            }
            else if (axes[1] == Axis::kAxisZ) {
                if (axes[0] == Axis::kAxisY) {
                    return kXZY;
                }
                else if (axes[0] == Axis::kAxisX) {
                    return kXZX;
                }
            }
        }
        else if (axes[2] == Axis::kAxisY) {
            if (axes[1] == Axis::kAxisX) {
                if (axes[0] == Axis::kAxisZ) {
                    return kYXZ;
                }
                else if (axes[0] == Axis::kAxisY) {
                    return kYXY;
                }
            }
            else if (axes[1] == Axis::kAxisZ) {
                if (axes[0] == Axis::kAxisY) {
                    return kYZY;
                }
                else if (axes[0] == Axis::kAxisX) {
                    return kYZX;
                }
            }
        }
        else if (axes[2] == Axis::kAxisZ) {
            if (axes[1] == Axis::kAxisY) {
                if (axes[0] == Axis::kAxisZ) {
                    return kZYZ;
                }
                else if (axes[0] == Axis::kAxisX) {
                    return kZYX;
                }
            }
            else if (axes[1] == Axis::kAxisX) {
                if (axes[0] == Axis::kAxisY) {
                    return kZXY;
                }
                else if (axes[0] == Axis::kAxisZ) {
                    return kZXZ;
                }
            }
        }
    }

    return EulerType::kInvalid;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EulerAngles::Axes EulerAngles::typeToAxes(const EulerType & type, RotationSpace space)
{
    switch (type) {
    // Proper Euler Angles
    case kZXZ:
        return { Axis::kAxisZ, Axis::kAxisX, Axis::kAxisZ };
    case kXYX:
        return { Axis::kAxisX, Axis::kAxisY, Axis::kAxisX };
    case kYZY:
        return { Axis::kAxisY, Axis::kAxisZ, Axis::kAxisY };
    case kZYZ:
        return { Axis::kAxisZ, Axis::kAxisY, Axis::kAxisZ };
    case kXZX:
        return { Axis::kAxisX, Axis::kAxisZ, Axis::kAxisX };
    case kYXY:
        return { Axis::kAxisY, Axis::kAxisX, Axis::kAxisY };
    // Tait-Bryan Angles
    case kXYZ:
        if(space == RotationSpace::kBody)
            return { Axis::kAxisX, Axis::kAxisY, Axis::kAxisZ };
        else
            return { Axis::kAxisZ, Axis::kAxisY, Axis::kAxisX };
    case kXZY:
        if (space == RotationSpace::kBody)
            return { Axis::kAxisX, Axis::kAxisZ, Axis::kAxisY };
        else
            return { Axis::kAxisY, Axis::kAxisZ, Axis::kAxisX };
    case kYXZ:
        if (space == RotationSpace::kBody)
            return { Axis::kAxisY, Axis::kAxisX, Axis::kAxisZ };
        else
            return { Axis::kAxisZ, Axis::kAxisX, Axis::kAxisY };
    case kYZX:
        if (space == RotationSpace::kBody)
            return { Axis::kAxisY, Axis::kAxisZ, Axis::kAxisX };
        else
            return { Axis::kAxisX, Axis::kAxisZ, Axis::kAxisY };
    case kZXY:
        if (space == RotationSpace::kBody)
            return { Axis::kAxisZ, Axis::kAxisX, Axis::kAxisY };
        else
            return { Axis::kAxisY, Axis::kAxisX, Axis::kAxisZ };
    case kZYX:
        if (space == RotationSpace::kBody)
            return { Axis::kAxisZ, Axis::kAxisY, Axis::kAxisX };
        else
            return { Axis::kAxisX, Axis::kAxisY, Axis::kAxisZ };
    default:
        throw("Type not recognized");
        break;
    }
    
    return Axes();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EulerAngles EulerAngles::toAngles(const Matrix3x3d & m, EulerType type, RotationSpace space)
{
    EulerAngles angles;
    angles.m_rotationOrder = typeToAxes(type, space);
    angles.m_rotationSpace = space;

    // See: https://en.wikipedia.org/wiki/Euler_angles
    float angle1, angle2, angle3;
    //float s1, s2, s3, c2;

    // FIXME: May not be working properly, gimbal lock with 180deg
    // See: https://ntrs.nasa.gov/api/citations/19770024290/downloads/19770024290.pdf
    switch (type) {
    // Proper Euler Angles
    case kZXZ:
    {
        double m22 = m(2, 2);
        angle1 = atan2(m(0, 2), -m(1, 2));
        angle2 = atan2(sqrt(1 - (m22 * m22)), m22);
        angle3 = atan2(m(2, 0), m(2, 1));
        break;
    }
    case kXYX:
    {
        double m00 = m(0, 0);
        angle1 = atan2(m(1, 0), -m(2, 0));
        angle2 = atan2(sqrt(1 - (m00 * m00)), m00);
        angle3 = atan2(m(0, 1), m(0, 2));
        break;
    }
    case kYZY:
    {
        double m11 = m(1, 1);
        angle1 = atan2(m(2, 1), -m(0, 1));
        angle2 = atan2(sqrt(1 - (m11 * m11)), m11);
        angle3 = atan2(m(1, 2), m(1, 0));
        break;
    }
    case kZYZ:
    {
        double m22 = m(1, 1);
        angle1 = atan2(m(1, 2), m(0, 2));
        angle2 = atan2(sqrt(1 - (m22 * m22)), m22);
        angle3 = atan2(m(2, 1), -m(2, 0));
        break;
    }
    case kXZX:
    {
        double m00 = m(0, 0);
        angle1 = atan2(m(2, 0), m(1, 0));
        angle2 = atan2(sqrt(1 - (m00 * m00)), m00);
        angle3 = atan2(m(0, 2), -m(0, 1));
        break;
    }
    case kYXY:
    {
        double m11 = m(1, 1);
        angle1 = atan2(m(0, 1), m(2, 1));
        angle2 = atan2(sqrt(1 - (m11 * m11)), m11);
        angle3 = atan2(m(1, 0), -m(1, 2));
        break;
    }
    // Tait-Bryan Angles
    case kXZY:
    {
        double m01 = m(0, 1);
        angle1 = atan2(m(2, 1), m(1, 1));
        angle2 = atan2(-m01, sqrt(1 - (m01 * m01)));
        angle3 = atan2(m(0, 2), m(0, 0));
        break;
    }
    case kXYZ:
    {
        double m02 = m(0, 2);
        angle1 = atan2(-m(1, 2), m(2, 2));
        angle2 = atan2(m02, sqrt(1 - (m02 * m02)));
        angle3 = atan2(-m(0, 1), m(0, 0));
        break;
    }
    case kYXZ:
    {
        double m12 = m(1, 2);
        angle1 = atan2(m(0, 2), m(2, 2));
        angle2 = atan2(-m12, sqrt(1 - (m12 * m12)));
        angle3 = atan2(m(1, 0), m(1, 1));
        break;
    }
    case kYZX:
    {
        double m10 = m(1, 0);
        angle1 = atan2(-m(2, 0), m(0, 0));
        angle2 = atan2(m10, sqrt(1 - (m10 * m10)));
        angle3 = atan2(-m(1, 2), m(1, 1));
        break;
    }
    case kZYX:
    {
        double m20 = m(2, 0);
        angle1 = atan2(m(1, 0), m(0, 0));
        angle2 = atan2(-m20, sqrt(1 - (m20 * m20)));
        angle3 = atan2(m(2, 1), m(2, 2));
        break;
    }
    case kZXY:
    {
        double m21 = m(2, 1);
        angle1 = atan2(-m(0, 1), m(1, 1));
        angle2 = atan2(m21, sqrt(1 - (m21 * m21)));
        angle3 = atan2(-m(2, 0), m(2, 2));
        break;
    }
    default:
        throw("Type not recognized");
        break;
    }

    //if (isnan(angle1)) {
    //    s1 = clamp(s1);
    //    angle1 = asin(s1);
    //}
    //if (isnan(angle2)) {
    //    s2 = clamp(s2);
    //    angle2 = asin(s2);
    //}
    //if (isnan(angle3)) {
    //    s3 = clamp(s3);
    //    angle3 = asin(s3);
    //}

    angles.m_angles = Vector3f(angle1, angle2, angle3);
    return angles;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double EulerAngles::clamp(double val, double min, double max)
{
    if (val < min)
        return min;
    else if (val > max)
        return max;
    else
        return val;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool operator==(const EulerAngles & e1, const EulerAngles & e2)
{
    return e1.toRotationMatrixF() == e2.toRotationMatrixF();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool operator!=(const EulerAngles & e1, const EulerAngles & e2)
{
    return !(e1 == e2);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}