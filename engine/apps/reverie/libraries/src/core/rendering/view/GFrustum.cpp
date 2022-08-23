#include "core/rendering/view/GFrustum.h"
#include "core/rendering/view/GFrustum.h"
#include "core/rendering/view/GFrustum.h"
#include "core/rendering/view/GFrustum.h"

#include "fortress/layer/framework/GFlags.h"

namespace rev {



// Frustum

void Frustum::ExtractFrustumPoints(const Matrix4x4g & viewMatrix, const Matrix4x4g & projectionMatrix, std::array<Vector4, 8>& outPoints)
{
    Matrix4x4d worldToClip = projectionMatrix.toDoubleMatrix() * viewMatrix.toDoubleMatrix();
    Matrix4x4g clipToWorld = worldToClip.inversed().toFloatMatrix();

    static std::array<Vector4, 8> clipSpaceCorners =
    {{
        {-1, -1, 0, 1},
        {-1,  1, 0, 1},
        { 1,  1, 0, 1},
        { 1, -1, 0, 1},
        {-1, -1, 1, 1},
        {-1,  1, 1, 1},
        { 1,  1, 1, 1},
        { 1, -1, 1, 1}
    }};

    for (size_t i = 0; i < 8; i++) {
        // Clip to world transformation
        outPoints[i] = clipToWorld * clipSpaceCorners[i];

        // Homogeneous to cartesian conversion
        outPoints[i] /= outPoints[i].w();
    }
}

AABBData Frustum::FrustomBoundingBox(const Matrix4x4g & viewMatrix, const Matrix4x4g & projectionMatrix)
{
    // Get frustum points in world space
    std::array<Vector4, 8> points;
    ExtractFrustumPoints(viewMatrix, projectionMatrix, points);

    AABBData aabbData;
    aabbData.resize(points);
    return aabbData;
}

Frustum::Frustum() :
    CollidingGeometry(kFrustum)
{
}

Frustum::Frustum(const Frustum & other):
    CollidingGeometry(kFrustum),
    m_planes(other.m_planes)
{
}

Frustum::Frustum(const Matrix4x4g& viewMatrix, const Matrix4x4g& projectionMatrix):
    CollidingGeometry(kFrustum)
{
    initialize(viewMatrix, projectionMatrix);
}

Frustum::~Frustum()
{
}

Frustum & Frustum::operator=(const Frustum & other)
{
    m_planes = other.m_planes;
    return *this;
}

void Frustum::normalizePlanes()
{
    for (BoundingPlane& plane : m_planes) {
        plane.normalize();
    }
}

bool Frustum::intersects(const CollidingGeometry & geometry) const
{
    bool inside = false;
    BoundingPlane::Halfspace result = BoundingPlane::kPositive;
    switch (geometry.geometryType()) {
    case CollidingGeometry::kAABB: {
        const AABB& aabb = static_cast<const AABB&>(geometry);

        // Returns if either entirely on wrong side of plane, or if intersects with one
        for (int i = 0; i < 6; i++) {
            // Return if the aabb is outside of any of the planes
            result = m_planes[i].classifyGeometry(aabb);
            if (result == BoundingPlane::kNegative)
            {
                break;
            }
        }
        inside = (result != BoundingPlane::kNegative);
        break;
    }
    case CollidingGeometry::kBoundingSphere: {
        // Returns if either entirely on wrong side of plane, or if intersects with one
        const BoundingSphere& sphere = static_cast<const BoundingSphere&>(geometry);
        for (int i = 0; i < 6; i++) {
            // Return if the sphere is outside of any of the planes
            result = m_planes[i].classifyGeometry(sphere);
            if (result == BoundingPlane::kNegative)
            {
                break;
            }
        }
        inside = (result != BoundingPlane::kNegative);
        break;
    }
    case CollidingGeometry::kPoint: {
        return contains(static_cast<const CollidingPoint&>(geometry).point());
    }
    default:
        Logger::Throw("Error, type of geometry not supported");
    }
    return inside;
}

void Frustum::recalculateBounds(const TransformInterface & transform, CollidingGeometry & out) const
{
    Q_UNUSED(transform);
    Q_UNUSED(out);

    // TODO: implement
    Logger::Throw("unimplemented");
}

bool Frustum::contains(const CollidingGeometry & geometry) const
{
    return intersects(geometry);
}

bool Frustum::contains(const Vector3 & point) const
{
    // Returns true if the point is in the positive direction or intersects for all frustum planes
    BoundingPlane::Halfspace result;
    for (int i = 0; i < 6; i++) {
        result = m_planes.at(i).classifyGeometry(point);
        if (result != BoundingPlane::kPositive)
            // Returns if either entirely on wrong side of plane, or if intersects with one
            return result != BoundingPlane::kNegative;
    }
    return true;
}

void Frustum::initialize(const Matrix4x4g & viewMatrix, const Matrix4x4g & projectionMatrix)
{
    Matrix4x4g viewProjection = projectionMatrix * viewMatrix;

    // Left clipping plane
    Vector4 row4 = viewProjection.getRow(3);
    m_planes[kLeft].setA(row4[0] + viewProjection(0, 0));
    m_planes[kLeft].setB(row4[1] + viewProjection(0, 1));
    m_planes[kLeft].setC(row4[2] + viewProjection(0, 2));
    m_planes[kLeft].setD(row4[3] + viewProjection(0, 3));

    // Right clipping plane
    m_planes[kRight].setA(row4[0] - viewProjection(0, 0));    
    m_planes[kRight].setB(row4[1] - viewProjection(0, 1));
    m_planes[kRight].setC(row4[2] - viewProjection(0, 2));
    m_planes[kRight].setD(row4[3] - viewProjection(0, 3));

    // Top clipping plane
    m_planes[kTop].setA(row4[0] - viewProjection(1, 0));
    m_planes[kTop].setB(row4[1] - viewProjection(1, 1));
    m_planes[kTop].setC(row4[2] - viewProjection(1, 2));
    m_planes[kTop].setD(row4[3] - viewProjection(1, 3));

    // Bottom clipping plane
    m_planes[kBottom].setA(row4[0] + viewProjection(1, 0));
    m_planes[kBottom].setB(row4[1] + viewProjection(1, 1));
    m_planes[kBottom].setC(row4[2] + viewProjection(1, 2));
    m_planes[kBottom].setD(row4[3] + viewProjection(1, 3));

    // Near clipping plane
    m_planes[kNear].setA(row4[0] + viewProjection(2, 0));
    m_planes[kNear].setB(row4[1] + viewProjection(2, 1));
    m_planes[kNear].setC(row4[2] + viewProjection(2, 2));
    m_planes[kNear].setD(row4[3] + viewProjection(2, 3));

    // Far clipping plane
    m_planes[kFar].setA(row4[0] - viewProjection(2, 0));
    m_planes[kFar].setB(row4[1] - viewProjection(2, 1));
    m_planes[kFar].setC(row4[2] - viewProjection(2, 2));
    m_planes[kFar].setD(row4[3] - viewProjection(2, 3));

    // Normalize planes
    normalizePlanes();
}



    
// End namespaces
}
