#include "GbEulerAngles.h"
#include "GbMatrix.h"

namespace Gb {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EulerAngles::EulerAngles():
    m_angles(Vector3f(0, 0, 0)),
    m_rotationOrder({ Axis::kAxisZ, Axis::kAxisY, Axis::kAxisX }),
    m_rotationType(RotationType::kBody)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EulerAngles::EulerAngles(float ax, float ay, float az, const Axes & rotationOrder, RotationType rotationType) :
    m_angles(Vector3f(ax, ay, az)),
    m_rotationOrder(rotationOrder),
    m_rotationType(rotationType)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EulerAngles::EulerAngles(const Vector3f & angles, const Axes & rotationOrder, RotationType rotationType):
    m_angles(angles),
    m_rotationOrder(rotationOrder),
    m_rotationType(rotationType)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EulerAngles::~EulerAngles()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Matrix4x4f EulerAngles::toRotationMatrix() const
{
    Matrix4x4f transformMatrix;
    switch (m_rotationType) {
    case RotationType::kSpace:
        transformMatrix = std::move(computeExtrinsicRotationMatrix(m_angles, m_rotationOrder));
        break;
    case RotationType::kBody:
        transformMatrix = std::move(computeIntrinsicRotationMatrix(m_angles, m_rotationOrder));
        break;
    }

    return transformMatrix;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Matrix4x4f EulerAngles::computeExtrinsicRotationMatrix(const Vector3f & angles, const Axes & axisOrder)
{
    Matrix4x4 rotationMtx;

    Matrix4x4 X;
    X.addRotateX(angles.x());

    Matrix4x4 Y;
    Y.addRotateY(angles.y());

    Matrix4x4 Z;
    Z.addRotateZ(angles.z());

    // Apply body rotations to the coordinate transformation matrix
    for (size_t i = 0; i < axisOrder.size(); i++) {
        Axis axis = axisOrder.at(i);
        switch (axis) {
        case Axis::kAxisX:
            rotationMtx = X * rotationMtx;
            break;

        case Axis::kAxisY:
            rotationMtx = Y * rotationMtx;
            break;

        case Axis::kAxisZ:
            rotationMtx = Z * rotationMtx;
            break;
        }
    }

    return rotationMtx.toFloatMatrix();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Matrix4x4f EulerAngles::computeIntrinsicRotationMatrix(const Vector3f & angles, const Axes & axisOrder)
{
    Matrix4x4 rotationMtx;
    Matrix4x4 axisRotationMtx;

    Vector3 axisVector;
    Vector3 rotatedAxisVector;
    for (size_t i = 0; i < axisOrder.size(); i++) {
        Axis axis = axisOrder.at(i);
        axisRotationMtx.setToIdentity();
        switch (axis) {
        case Axis::kAxisX:
            axisVector = Vector3(1, 0, 0);
            rotatedAxisVector = rotationMtx.multPoint(axisVector);
            axisRotationMtx.addRotate(rotatedAxisVector, angles.x());
            rotationMtx = axisRotationMtx * rotationMtx;
            break;
        case Axis::kAxisY:
            axisVector = Vector3(0, 1, 0);
            rotatedAxisVector = rotationMtx.multPoint(axisVector);
            axisRotationMtx.addRotate(rotatedAxisVector, angles.y());
            rotationMtx = axisRotationMtx * rotationMtx;
            break;

        case Axis::kAxisZ:
            axisVector = Vector3(0, 0, 1);
            rotatedAxisVector = rotationMtx.multPoint(axisVector);
            axisRotationMtx.addRotate(rotatedAxisVector, angles.z());
            rotationMtx = axisRotationMtx * rotationMtx;
            break;
        }
    }

    return rotationMtx.toFloatMatrix();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}