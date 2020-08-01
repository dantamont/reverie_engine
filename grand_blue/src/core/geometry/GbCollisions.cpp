#include "GbCollisions.h"

#include <algorithm>
#include "GbVector.h"
#include "../geometry/GbTransform.h"

namespace Gb{

//////////////////////////////////////////////////////////////////////////////////////////////////
// CollidingGeometry
//////////////////////////////////////////////////////////////////////////////////////////////////
CollidingGeometry::CollidingGeometry(GeometryType type):
    m_geometryType(type)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
CollidingGeometry::~CollidingGeometry()
{
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// AABB
//////////////////////////////////////////////////////////////////////////////////////////////////
AABB::AABB(): CollidingGeometry(kAABB)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
AABB::~AABB()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Vector3g AABB::getDimensions() const
{
    return Vector3g(m_maxX - m_minX, m_maxY - m_minY, m_maxZ - m_minZ);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Vector3g AABB::getOrigin() const
{
    return Vector3g((m_maxX + m_minX) / 2, (m_maxY + m_minY) / 2, (m_maxZ + m_minZ) / 2);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void AABB::getPoints(std::vector<Vector3g>& points) const
{
    points.reserve(8);
    points.assign({
        {m_minX, m_minY, m_minZ},
        {m_maxX, m_minY, m_minZ},
        {m_minX, m_maxY, m_minZ},
        {m_maxX, m_maxY, m_minZ},
        {m_minX, m_minY, m_maxZ},
        {m_minX, m_maxY, m_maxZ},
        {m_maxX, m_minY, m_maxZ},
        {m_maxX, m_maxY, m_maxZ}
        });
}
//////////////////////////////////////////////////////////////////////////////////////////////////
bool AABB::intersects(const CollidingGeometry & other)
{
    bool hit;
    switch (other.geometryType()) {
    case kAABB:
    {
        const AABB& aabb = *static_cast<const AABB*>(&other);
        hit = (minX() <= aabb.maxX() && maxX() >= aabb.minX()) &&
              (minY() <= aabb.maxY() && maxY() >= aabb.minY()) &&
              (minZ() <= aabb.maxZ() && maxZ() >= aabb.minZ());
        break;
    }
    case kBoundingSphere:
    {
        const BoundingSphere& sphere = *static_cast<const BoundingSphere*>(&other);

        // Get box closest point to sphere center by clamping        
        Vector3g closestPoint = { std::max(m_minX, std::min(sphere.origin().x(), m_maxX)),
            std::max(m_minY, std::min(sphere.origin().y(), m_maxY)),
            std::max(m_minZ, std::min(sphere.origin().z(), m_maxZ))
        };

        float distance = (closestPoint - sphere.origin()).length();

        hit = distance <= sphere.radius();
        break;
    }
    case kPoint:
    {
        const CollidingPoint& point = *static_cast<const CollidingPoint*>(&other);
        hit = (point.x() >= m_minX && point.x() <= m_maxX) &&
              (point.y() >= m_minY && point.y() <= m_maxY) &&
              (point.z() >= m_minZ && point.z() <= m_maxZ);
        break;
    }
    default:
        throw("Invalid");
        break;
    }
    return hit;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void AABB::recalculateBounds(const Transform & transform, CollidingGeometry & out) const
{
    std::vector<Vector3g> outPoints;
    getPoints(outPoints);
    for (Vector3g& point : outPoints) {
        point = transform.worldMatrix().multPoint(point);
    }
    AABB& outCast = static_cast<AABB&>(out);
    outCast.resize(outPoints);
}




//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// Bounding Sphere
//////////////////////////////////////////////////////////////////////////////////////////////////
BoundingSphere::BoundingSphere() : CollidingGeometry(kBoundingSphere)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
BoundingSphere::~BoundingSphere()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
bool BoundingSphere::intersects(const CollidingGeometry & other)
{
    bool hit;
    switch (other.geometryType()) {
    case kAABB:
    {
        const AABB& aabb = *static_cast<const AABB*>(&other);

        // Get box closest point to sphere center by clamping        
        Vector3g closestPoint = { std::max(aabb.minX(), std::min(m_origin.x(), aabb.maxX())),
            std::max(aabb.minY(), std::min(m_origin.y(), aabb.maxY())),
            std::max(aabb.minZ(), std::min(m_origin.z(), aabb.maxZ()))
        };

        float distance = (closestPoint - m_origin).length();

        hit = distance <= m_radius;
        break;
    }
    case kBoundingSphere:
    {
        const BoundingSphere& sphere = *static_cast<const BoundingSphere*>(&other);
        float distance = (m_origin - sphere.m_origin).length();
        hit = distance <= m_radius + sphere.m_radius;
        break;
    }
    case kPoint:
    {
        const CollidingPoint& point = *static_cast<const CollidingPoint*>(&other);
        float distance = (m_origin - point).length();
        hit = distance <= m_radius;
        break;
    }
    default:
        throw("Invalid");
        break;
    }
    return hit;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void BoundingSphere::recalculateBounds(const Transform & transform, CollidingGeometry & out) const
{
    BoundingSphere& sphere = dynamic_cast<BoundingSphere&>(out);
    sphere.m_origin += transform.getPosition().asReal();
    sphere.m_radius *= std::max(std::max(transform.getScaleVec()[0], transform.getScaleVec()[1]), transform.getScaleVec()[2]);
}



//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// Colliding Point
//////////////////////////////////////////////////////////////////////////////////////////////////
CollidingPoint::CollidingPoint() : CollidingGeometry(kPoint)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
CollidingPoint::~CollidingPoint()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
bool CollidingPoint::intersects(const CollidingGeometry & other)
{
    bool hit;
    switch (other.geometryType()) {
    case kAABB:
    {
        const AABB& aabb = *static_cast<const AABB*>(&other);
        hit = (x() >= aabb.minX() && x() <= aabb.maxX()) &&
              (y() >= aabb.minY() && y() <= aabb.maxY()) &&
              (z() >= aabb.minZ() && z() <= aabb.maxZ());
        break;
    }
    case kBoundingSphere:
    {
        const BoundingSphere& sphere = *static_cast<const BoundingSphere*>(&other);
        float distance = (*this - sphere.origin()).length();
        hit =  distance <= sphere.radius();
        break;
    }
    case kPoint:
    {
        const CollidingPoint& point = *static_cast<const CollidingPoint*>(&other);
        hit = *this == point;
        break;
    }
    default:
        throw("Invalid");
        break;
    }
    return hit;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CollidingPoint::recalculateBounds(const Transform & transform, CollidingGeometry & out) const
{
    CollidingPoint& point = dynamic_cast<CollidingPoint&>(out);
    Vector3g newPoint = transform.worldMatrix().multPoint(point.asReal());
    point.array().swap(newPoint.array());
}



//////////////////////////////////////////////////////////////////////////////////////////////////
// Bounding Plane
//////////////////////////////////////////////////////////////////////////////////////////////////
BoundingPlane::BoundingPlane(): CollidingGeometry(kPlane)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
BoundingPlane::~BoundingPlane()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
bool BoundingPlane::intersects(const CollidingGeometry & other)
{
    bool hit;
    switch (other.geometryType()) {
    case kAABB:
    {
        hit = classifyGeometry(static_cast<const AABB&>(other)) == kIntersects;
        break;
    }
    case kBoundingSphere:
    {
        hit = classifyGeometry(static_cast<const BoundingSphere&>(other)) == kIntersects;
        break;
    }
    case kPoint:
    {
        hit = classifyGeometry(static_cast<const CollidingPoint&>(other)) == kIntersects;
        break;
    }
    default:
        throw("Invalid");
        break;
    }
    return hit;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void BoundingPlane::recalculateBounds(const Transform & transform, CollidingGeometry & out) const
{
    // See: https://community.khronos.org/t/clever-way-to-transform-plane-by-matrix/49570/3
    BoundingPlane& plane = dynamic_cast<BoundingPlane&>(out);
    Vector4g planeEq = Vector4g(m_normal);
    planeEq[3] = m_d;
    
    // Transform plane equation
    planeEq = transform.worldMatrix().inversed().transposed() * planeEq;
    plane.m_normal = Vector3g(planeEq);
    plane.m_d = planeEq[3];
}
//////////////////////////////////////////////////////////////////////////////////////////////////
float BoundingPlane::distanceToPoint(const Vector3g & point)
{
    if(!isNormalized()) normalize();
    return m_normal.dot(point) + m_d; // m_normal.dot(point) is equivalent to ax + by + cz
}

//////////////////////////////////////////////////////////////////////////////////////////////////
} // Gb