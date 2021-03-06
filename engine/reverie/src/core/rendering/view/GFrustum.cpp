#include "GFrustum.h"
#include "GFrustum.h"
#include "GFrustum.h"
#include "GFrustum.h"

#include "../../containers/GFlags.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// Frustum
//////////////////////////////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////////////////////////////
AABBData Frustum::FrustomBoundingBox(const Matrix4x4g & viewMatrix, const Matrix4x4g & projectionMatrix)
{
    // Get frustum points in world space
    std::array<Vector4, 8> points;
    ExtractFrustumPoints(viewMatrix, projectionMatrix, points);

    AABBData aabbData;
    aabbData.resize(points);
    return aabbData;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Frustum::Frustum() :
    CollidingGeometry(kFrustum)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Frustum::Frustum(const Frustum & other):
    CollidingGeometry(kFrustum),
    m_planes(other.m_planes)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Frustum::Frustum(const Matrix4x4g& viewMatrix, const Matrix4x4g& projectionMatrix):
    CollidingGeometry(kFrustum)
{
    initialize(viewMatrix, projectionMatrix);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Frustum::~Frustum()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Frustum & Frustum::operator=(const Frustum & other)
{
    m_planes = other.m_planes;
    return *this;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Frustum::normalizePlanes()
{
    for (BoundingPlane& planePair : m_planes) {
        planePair.normalize();
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
bool Frustum::intersects(const CollidingGeometry & geometry) const
{
    bool inside = false;
    BoundingPlane::Halfspace result = BoundingPlane::kPositive;
    switch (geometry.geometryType()) {
    case CollidingGeometry::kAABB: {
        const AABB& aabb = static_cast<const AABB&>(geometry);

        for (int i = 0; i < 6; i++) {
            // Return if the aabb is outside of any of the planes
            result = m_planes[i].classifyGeometry(aabb);
            if (result != BoundingPlane::kPositive)
            {
                // Returns if either entirely on wrong side of plane, or if intersects with one
                return result != BoundingPlane::kNegative;
            }
        }
        inside = result != BoundingPlane::kNegative;
        break;
    }
    case CollidingGeometry::kBoundingSphere: {
        const BoundingSphere& sphere = static_cast<const BoundingSphere&>(geometry);
        for (int i = 0; i < 6; i++) {
            // Return if the sphere is outside of any of the planes
            result = m_planes[i].classifyGeometry(sphere);
            if (result != BoundingPlane::kPositive)
            {
                // Returns if either entirely on wrong side of plane, or if intersects with one
                return result != BoundingPlane::kNegative;
            }
        }
        inside = result != BoundingPlane::kNegative;
        break;
    }
    case CollidingGeometry::kPoint: {
        return contains(static_cast<const Vector3&>(static_cast<const CollidingPoint&>(geometry)));
    }
    default:
        throw("Error, type of geometry not supported");
    }
    return inside;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Frustum::recalculateBounds(const Transform & transform, CollidingGeometry & out) const
{
    Q_UNUSED(transform);
    Q_UNUSED(out);

    // TODO: implement
    throw("unimplemented");
}
//////////////////////////////////////////////////////////////////////////////////////////////////
bool Frustum::contains(const CollidingGeometry & geometry) const
{
    return intersects(geometry);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////////////////////////////
void Frustum::initialize(const Matrix4x4g & viewMatrix, const Matrix4x4g & projectionMatrix)
{
    Matrix4x4g viewProjection = projectionMatrix * viewMatrix;

    // Left clipping plane
    m_planes[kLeft] = BoundingPlane();
    m_planes[kLeft].setA(viewProjection(3, 0) + viewProjection(0, 0));
    m_planes[kLeft].setB(viewProjection(3, 1) + viewProjection(0, 1));
    m_planes[kLeft].setC(viewProjection(3, 2) + viewProjection(0, 2));
    m_planes[kLeft].setD(viewProjection(3, 3) + viewProjection(0, 3));

    // Right clipping plane
    m_planes[kRight] = BoundingPlane();
    m_planes[kRight].setA(viewProjection(3, 0) - viewProjection(0, 0));    
    m_planes[kRight].setB(viewProjection(3, 1) - viewProjection(0, 1));
    m_planes[kRight].setC(viewProjection(3, 2) - viewProjection(0, 2));
    m_planes[kRight].setD(viewProjection(3, 3) - viewProjection(0, 3));

    // Top clipping plane
    m_planes[kTop] = BoundingPlane();
    m_planes[kTop].setA(viewProjection(3, 0) - viewProjection(1, 0));
    m_planes[kTop].setB(viewProjection(3, 1) - viewProjection(1, 1));
    m_planes[kTop].setC(viewProjection(3, 2) - viewProjection(1, 2));
    m_planes[kTop].setD(viewProjection(3, 3) - viewProjection(1, 3));

    // Bottom clipping plane
    m_planes[kBottom] = BoundingPlane();
    m_planes[kBottom].setA(viewProjection(3, 0) + viewProjection(1, 0));
    m_planes[kBottom].setB(viewProjection(3, 1) + viewProjection(1, 1));
    m_planes[kBottom].setC(viewProjection(3, 2) + viewProjection(1, 2));
    m_planes[kBottom].setD(viewProjection(3, 3) + viewProjection(1, 3));

    // Near clipping plane
    m_planes[kNear] = BoundingPlane();
    m_planes[kNear].setA(viewProjection(3, 0) + viewProjection(2, 0));
    m_planes[kNear].setB(viewProjection(3, 1) + viewProjection(2, 1));
    m_planes[kNear].setC(viewProjection(3, 2) + viewProjection(2, 2));
    m_planes[kNear].setD(viewProjection(3, 3) + viewProjection(2, 3));

    // Far clipping plane
    m_planes[kFar] = BoundingPlane();
    m_planes[kFar].setA(viewProjection(3, 0) - viewProjection(2, 0));
    m_planes[kFar].setB(viewProjection(3, 1) - viewProjection(2, 1));
    m_planes[kFar].setC(viewProjection(3, 2) - viewProjection(2, 2));
    m_planes[kFar].setD(viewProjection(3, 3) - viewProjection(2, 3));

    // Normalize planes
    normalizePlanes();

    //for (const std::pair<Plane, BoundingPlane>& planePair : m_planes) {
    //    logInfo(QString::number(int(planePair.first))
    //        + " "+ QString(planePair.second.normal().normalized()));
    //}

    //logInfo(QString::number((int)contains(Vector3(0, 0, 0))));

    // Test box
    //AABB box;
    //box.setMinX(0);
    //box.setMaxX(10);
    //box.setMinY(0);
    //box.setMaxY(10);
    //box.setMinZ(0);
    //box.setMaxZ(10);
    //logInfo(QString::number((int)contains(box)));
    //std::vector<Vector3g> vec = { Vector3g(10, 0, 0), Vector3g(-5, 9, 2), Vector3g(3, -12, -17) };
    //AABB box(vec);
}



//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}
