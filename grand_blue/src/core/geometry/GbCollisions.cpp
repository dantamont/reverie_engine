#include "GbCollisions.h"

#include <algorithm>
#include "GbVector.h"

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
        Vector3 closestPoint = { std::max(m_minX, std::min(sphere.origin().x(), m_maxX)),
            std::max(m_minY, std::min(sphere.origin().y(), m_maxY)),
            std::max(m_minZ, std::min(sphere.origin().z(), m_maxZ))
        };

        double distance = (closestPoint - sphere.origin()).length();

        hit = distance <= sphere.radius;
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
        Vector3 closestPoint = { std::max(aabb.minX(), std::min(m_origin.x(), aabb.maxX())),
            std::max(aabb.minY(), std::min(m_origin.y(), aabb.maxY())),
            std::max(aabb.minZ(), std::min(m_origin.z(), aabb.maxZ()))
        };

        double distance = (closestPoint - m_origin).length();

        hit = distance <= m_radius;
        break;
    }
    case kBoundingSphere:
    {
        const BoundingSphere& sphere = *static_cast<const BoundingSphere*>(&other);
        double distance = (m_origin - sphere.m_origin).length();
        hit = distance <= m_radius + sphere.m_radius;
        break;
    }
    case kPoint:
    {
        const CollidingPoint& point = *static_cast<const CollidingPoint*>(&other);
        double distance = (m_origin - point).length();
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
        double distance = (*this - sphere.origin()).length();
        hit =  distance <= sphere.radius;
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
} // Gb