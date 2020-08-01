#ifndef GB_COLLISIONS_H
#define GB_COLLISIONS_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Standard
#include <vector>

// QT

// Internal
#include "GbMatrix.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class Frustum;
class Transform;

//////////////////////////////////////////////////////////////////////////////////
// Macro Definitions
//////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Type Definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////


/// @brief Abstract class for collidable geometry
// See: https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection
class CollidingGeometry {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum GeometryType{
        kPoint,
        kAABB,
        kBoundingSphere,
        kPlane
    };

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructor/Destructor
    /// @{

    CollidingGeometry(GeometryType type);
    virtual ~CollidingGeometry();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual bool intersects(const CollidingGeometry& other) = 0;
    virtual void recalculateBounds(const Transform& transform, CollidingGeometry& out) const = 0;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    GeometryType geometryType() const { return m_geometryType; }

    /// @}

protected:

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members 
    /// @{

    GeometryType m_geometryType;

    /// @}

};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class AABB
/// @brief Axis-Aligned Bounding Box
// See: https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection
class AABB: public CollidingGeometry {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructor/Destructor
    /// @{

    AABB();

    template<typename D>
    AABB(const std::vector<Vector<D, 3>>& points):
        CollidingGeometry(kAABB) 
    {
        resize(points);
    }

    ~AABB();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
    
    const float& getMinMaxX(bool max) const { if (max) return m_maxX; else return m_minX; }
    const float& getMinMaxY(bool max) const { if (max) return m_maxY; else return m_minZ; }
    const float& getMinMaxZ(bool max) const { if (max) return m_maxY; else return m_minZ; }

    const float& minX() const { return m_minX; }
    const float& maxX() const { return m_maxX; }
    const float& minY() const { return m_minY; }
    const float& maxY() const { return m_maxY; }
    const float& minZ() const { return m_minZ; }
    const float& maxZ() const { return m_maxZ; }

    void setMinX(float x) { m_minX = x; }
    void setMaxX(float x) { m_maxX = x; }
    void setMinY(float y) { m_minY = y; }
    void setMaxY(float y) { m_maxY = y; }
    void setMinZ(float z) { m_minZ = z; }
    void setMaxZ(float z) { m_maxZ = z; }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Resize given a set of points
    template<typename D>
    void resize(const std::vector<Vector<D, 3>>& points) {
        for (const Vector<D, 3>& point : points) {
            float x = (float)point.x();
            float y = (float)point.y();
            float z = (float)point.z();
            if (x > m_maxX) {
                m_maxX = x;
            }
            if (x < m_minX) {
                m_minX = x;
            }
            if (y > m_maxY) {
                m_maxY = y;
            }
            if (y < m_minY) {
                m_minY = y;
            }
            if (z > m_maxZ) {
                m_maxZ = z;
            }
            if (z < m_minZ) {
                m_minZ = z;
            }
        }
    }

    /// @brief Return dimensions in world-space x, y, z directions
    Vector3g getDimensions() const;

    /// @brief Get center of the cube
    Vector3g getOrigin() const;

    /// @brief Get the points constituting the bounding box
    void getPoints(std::vector<Vector3g>& points) const;

    virtual bool intersects(const CollidingGeometry& other) override;
    virtual void recalculateBounds(const Transform& transform, CollidingGeometry& out) const override;

    /// @}

protected:

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members 
    /// @{

    float m_minX = std::numeric_limits<float>::infinity();
    float m_maxX = -std::numeric_limits<float>::infinity();
    float m_minY = std::numeric_limits<float>::infinity();
    float m_maxY = -std::numeric_limits<float>::infinity();
    float m_minZ = std::numeric_limits<float>::infinity();
    float m_maxZ = -std::numeric_limits<float>::infinity();

    /// @}

};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class BoundingSphere
// See: https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection
class BoundingSphere : public CollidingGeometry {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructor/Destructor
    /// @{

    BoundingSphere();
    ~BoundingSphere();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    const Vector3g& origin() const { return m_origin; }
    void setOrigin(const Vector3g& origin) { m_origin = origin; }

    const float& radius() const { return m_radius; }
    void setRadius(const float& radius) { m_radius = radius; }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual bool intersects(const CollidingGeometry& other) override;
    virtual void recalculateBounds(const Transform& transform, CollidingGeometry& out) const override;

    /// @}

protected:

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members 
    /// @{

    Vector3g m_origin;
    float m_radius;

    /// @}

};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class CollidingPoint
// See: https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection
class CollidingPoint : public CollidingGeometry, public Vector3g {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructor/Destructor
    /// @{

    CollidingPoint();
    ~CollidingPoint();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual bool intersects(const CollidingGeometry& other) override;
    virtual void recalculateBounds(const Transform& transform, CollidingGeometry& out) const override;

    /// @}

protected:

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members 
    /// @{

    /// @}

};


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class BoundingPlane
// See: https://www.gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
// https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection
// https://www.scratchapixel.com/lessons/advanced-rendering/introduction-acceleration-structure/introduction
class BoundingPlane : public CollidingGeometry {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum Halfspace{ 
        kNegative = -1,
        kIntersects = 0,
        kPositive = 1, 
    };

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructor/Destructor
    /// @{

    BoundingPlane();
    ~BoundingPlane();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    const Vector3g& normal() const { return m_normal; }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual bool intersects(const CollidingGeometry& other) override;
    virtual void recalculateBounds(const Transform& transform, CollidingGeometry& out) const override;

    Halfspace classifyGeometry(const Vector3g& pt)  const
    { 
        float dist = m_normal.dot(pt) + m_d;
        if (dist < 0) return kNegative;
        if (dist > 0) return kPositive;
        return kIntersects;
    }

    Halfspace classifyGeometry(const BoundingSphere& sphere) const
    {
        // Assumes normalized plane
        float dist = m_normal.dot(sphere.origin()) + m_d;
        if (dist < -sphere.radius()) 
            return kNegative;
        else if (dist < sphere.radius()) 
            return kIntersects;
        else 
            return kPositive;
    }

    // See: http://www.lighthouse3d.com/tutorials/view-frustum-culling/geometric-approach-testing-boxes-ii/
    Halfspace classifyGeometry(const AABB& aabb) const
    {
        Vector3g p = Vector3g::EmptyVector();
        Vector3g n = Vector3g::EmptyVector();
        findNP(aabb, n, p);

        // Is the most positive vertex position outside?
        if (classifyGeometry(p) == kNegative)
            return kNegative;

        // Is the most negative vertex outside?
        else if (classifyGeometry(n) == kNegative)
            // Positive vertex is inside, but negative vertex is outside
            return kIntersects;
        else
            return kPositive;
    }

    /// @brief Plane equation coefficients
    const float& a() const { return m_normal[0]; }
    void setA(float a) { m_normal[0] = a; }

    const float& b() const { return m_normal[1]; }
    void setB(float b) { m_normal[1] = b; }

    const float& c() const { return m_normal[2]; }
    void setC(float c) { m_normal[2] = c; }

    const float& d() const { return m_d; }
    void setD(float d) { m_d = d; }

    /// @brief Obtain the shortest signed distance from the plane to a point
    float distanceToPoint(const Vector3g& point);

    /// @brief Whether or not the plane is normalixed
    bool isNormalized() const {
        return m_normal.lengthSquared() == 1;
    }

    /// @brief Normalize the plane
    void normalize() {
        float length = m_normal.length();
        if (length != 1) {
            m_normal.normalize();
            m_d /= length;
        }
    }

    /// @}

protected:

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods 
    /// @{

    /// @brief Find the point farthest (p) and point nearest (n) to this plane of the given AABB
    void findNP(const AABB& aabb, Vector3g& n, Vector3g& p) const {
        // Note: This relies on the assumption that the box is axis-aligned
        // If the box is not axis-aligned, then need to convert normal to box space and use that:
        // nb = (bx . n, by . n, bz . n), where bx, by, and bz are the box axes

        // Find p
        p = Vector3g(aabb.minX(), aabb.minY(), aabb.minZ());
        if (m_normal.x() >= 0)
            p[0] = aabb.maxX();
        if (m_normal.y() >= 0)
            p[1] = aabb.maxY();
        if (m_normal.z() >= 0)
            p[2] = aabb.maxZ();

        // Find n
        n = Vector3g(aabb.maxX(), aabb.maxY(), aabb.maxZ());
        if (m_normal.x() >= 0)
            n[0] = aabb.minX();
        if (m_normal.y() >= 0)
            n[1] = aabb.minY();
        if (m_normal.z() >= 0)
            n[2] = aabb.minZ();

    }


    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members 
    /// @{

    /// @brief Coefficients of the plane equation describing this plane
    /// @details ax + by + cz + d = 0
    Vector3g m_normal;
    float m_d; // -normal.dot(planePosition)

    /// @}

};


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief An aggregate class that includes one or more types of collision geometry
template<typename G>
class BoundingGeometry{
public:
    static_assert((std::is_base_of<AABB, G>::value ||
        std::is_base_of<BoundingSphere, G>::value ||
        std::is_base_of<CollidingPoint, G>::value), "Invalid geometry type");

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructors
    /// @{

    BoundingGeometry() {}
    ~BoundingGeometry() {}

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    std::vector<G>& geometry() { return m_geometry; }
    const std::vector<G>& geometry() const { return m_geometry; }
    //std::vector<G>& transformedGeometry() { return m_transformedGeometry; }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Transform the bounding geometry
    void recalculateBounds(const Transform& transform, BoundingGeometry<G>& transformedGeomtry);

    /// @brief Whether or not the bounding geometry is in the given frustum
    bool inFrustum(const Frustum& frustum) const;

    /// @brief Whether or not the bounds are empty
    bool isEmpty() const { return m_geometry.size() == 0; }

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected Members 
    /// @{

    /// @brief Base geometry
    std::vector<G> m_geometry;

    /// @}

};
typedef BoundingGeometry<AABB> BoundingBoxes;


//////////////////////////////////////////////////////////////////////////////////////////////////
template<typename G>
inline void BoundingGeometry<G>::recalculateBounds(const Transform & transform, BoundingGeometry<G>& transformedGeometry)
{
    size_t originalSize = transformedGeometry.m_geometry.size();
    if (originalSize != m_geometry.size()) {
        transformedGeometry.m_geometry.resize(m_geometry.size());
    }

    for (size_t i = 0; i < m_geometry.size(); i++) {
        G& outG = transformedGeometry.m_geometry[originalSize + i];
        outG = AABB();
        m_geometry[i].recalculateBounds(transform, outG);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
template<typename G>
inline bool BoundingGeometry<G>::inFrustum(const Frustum & frustum) const
{
    if (!m_geometry.size()) {
        // If no geometry, always pass frustum test
        return true;
    }

    bool inFrustum = false;
    for (const G& geometry : m_geometry) {
        if (frustum.contains(geometry))
            return true;
    }
    return inFrustum;
}



//////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif