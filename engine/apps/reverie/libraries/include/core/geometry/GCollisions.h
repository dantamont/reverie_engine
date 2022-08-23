#pragma once

// Standard
#include <vector>

// QT

// Internal
#include "fortress/types/GSizedTypes.h"
#include "fortress/containers/math/GMatrix.h"

namespace rev {

class Frustum;
class TransformInterface;

/// @brief Abstract class for collidable geometry
/// @see https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection
class CollidingGeometry {
public:
    /// @name Static
    /// @{

    enum GeometryType{
        kPoint,
        kAABB,
        kBoundingSphere,
        kPlane,
        kFrustum
    };

    /// @}

    /// @name Constructor/Destructor
    /// @{

    CollidingGeometry(GeometryType type);
    virtual ~CollidingGeometry();

    /// @}

    /// @name Public Methods
    /// @{

    virtual bool intersects(const CollidingGeometry& other) const = 0;
    virtual void recalculateBounds(const TransformInterface& transform, CollidingGeometry& out) const = 0;

    /// @}

    /// @name Properties
    /// @{

    GeometryType geometryType() const { return m_geometryType; }

    /// @}

protected:

    /// @name Protected Members 
    /// @{

    GeometryType m_geometryType;

    /// @}

};


/// @brief Represents dimensions for a bounding box
struct AABBData {
    /// @name Constructor/Destructor
    /// @{

    AABBData();
    ~AABBData();

    /// @}

    /// @name Properties
    /// @{

    const Vector4& min() const {
        return m_min;
    }
    const Vector4& max() const {
        return m_max;
    }

    float getMinMaxX(bool max) const { if (max) return m_max.x(); else return m_min.x(); }
    float getMinMaxY(bool max) const { if (max) return m_max.y(); else return m_min.y(); }
    float getMinMaxZ(bool max) const { if (max) return m_max.z(); else return m_min.z(); }

    float minX() const { return m_min.x(); }
    float maxX() const { return m_max.x(); }
    float minY() const { return m_min.y(); }
    float maxY() const { return m_max.y(); }
    float minZ() const { return m_min.z(); }
    float maxZ() const { return m_max.z(); }

    void setMinX(float x) { m_min[0] = x; }
    void setMaxX(float x) { m_max[0] = x; }
    void setMinY(float y) { m_min[1] = y; }
    void setMaxY(float y) { m_max[1] = y; }
    void setMinZ(float z) { m_min[2] = z; }
    void setMaxZ(float z) { m_max[2] = z; }

    /// @}

    /// @name Operators
    /// @{

    AABBData& operator=(const AABBData& other);

    /// @}

    /// @name Public Methods
    /// @{

    bool isInitialized() const{
        return m_min.x() != std::numeric_limits<Real_t>::infinity();
    }

    /// @brief Convert to cube that contains current AABB
    void toContainingCube(){
        float minDim = std::min(std::min(m_min.x(), m_min.y()), m_min.z());
        float maxDim = std::max(std::max(m_max.x(), m_max.y()), m_max.z());
        m_min = Vector4::Ones() * minDim;
        m_max = Vector4::Ones() * maxDim;
    }

    /// @brief Resize given a set of points
    template<typename D, size_t N>
    void resize(const std::vector<Vector<D, N>>& points) {
        static_assert(N > 2, "Vector is too small to resize");
        for (const Vector<D, N>& point : points) {
            float x = (float)point.x();
            float y = (float)point.y();
            float z = (float)point.z();
            if (x > m_max.x()) {
                m_max[0] = x;
            }
            if (x < m_min.x()) {
                m_min[0] = x;
            }
            if (y > m_max.y()) {
                m_max[1] = y;
            }
            if (y < m_min.y()) {
                m_min[1] = y;
            }
            if (z > m_max.z()) {
                m_max[2] = z;
            }
            if (z < m_min.z()) {
                m_min[2] = z;
            }
        }
    }

    template<typename D, size_t N>
    void resize(const std::array<Vector<D, N>, 8>& points) {
        static_assert(N >= 3, "Error, vector not large enough");
        for (const Vector<D, N>& point : points) {
            float x = (float)point.x();
            float y = (float)point.y();
            float z = (float)point.z();
            if (x > m_max.x()) {
                m_max[0] = x;
            }
            if (x < m_min.x()) {
                m_min[0] = x;
            }
            if (y > m_max.y()) {
                m_max[1] = y;
            }
            if (y < m_min.y()) {
                m_min[1] = y;
            }
            if (z > m_max.z()) {
                m_max[2] = z;
            }
            if (z < m_min.z()) {
                m_min[2] = z;
            }
        }
    }

    /// @brief Return dimensions in world-space x, y, z directions
    Vector3 getDimensions() const;

    /// @brief Get center of the cube
    Vector3 getOrigin() const;

    /// @brief Get the points constituting the bounding box
    template<typename D>
    void getPoints(std::vector<Vector<D, 3>>& points) const {
        points.reserve(8);
        points.assign({
            Vector<D, 3>{m_min.x(), m_min.y(), m_min.z()},
            Vector<D, 3>{m_max.x(), m_min.y(), m_min.z()},
            Vector<D, 3>{m_min.x(), m_max.y(), m_min.z()},
            Vector<D, 3>{m_max.x(), m_max.y(), m_min.z()},
            Vector<D, 3>{m_min.x(), m_min.y(), m_max.z()},
            Vector<D, 3>{m_min.x(), m_max.y(), m_max.z()},
            Vector<D, 3>{m_max.x(), m_min.y(), m_max.z()},
            Vector<D, 3>{m_max.x(), m_max.y(), m_max.z()}
            });
    }

    template<typename D>
    void getPoints(std::vector<Vector<D, 4>>& points) const {
        points.reserve(8);
        points.assign({
            Vector<D, 4>{m_min.x(), m_min.y(), m_min.z(), 1},
            Vector<D, 4>{m_max.x(), m_min.y(), m_min.z(), 1},
            Vector<D, 4>{m_min.x(), m_max.y(), m_min.z(), 1},
            Vector<D, 4>{m_max.x(), m_max.y(), m_min.z(), 1},
            Vector<D, 4>{m_min.x(), m_min.y(), m_max.z(), 1},
            Vector<D, 4>{m_min.x(), m_max.y(), m_max.z(), 1},
            Vector<D, 4>{m_max.x(), m_min.y(), m_max.z(), 1},
            Vector<D, 4>{m_max.x(), m_max.y(), m_max.z(), 1}
            });
    }

    /// @}

    /// @name Public Members 
    /// @{

    Vector4 m_min = std::numeric_limits<Real_t>::infinity() * Vector4(1, 1, 1, 1);
    Vector4 m_max = -std::numeric_limits<Real_t>::infinity() * Vector4(1, 1, 1, 1);

    /// @}

};



/// @class AABB
/// @brief Axis-Aligned Bounding Box
// See: https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection
class AABB: public CollidingGeometry {
public:
    /// @name Constructor/Destructor
    /// @{

    AABB();
    AABB(const AABBData& data);

    template<typename D>
    AABB(const std::vector<Vector<D, 3>>& points):
        CollidingGeometry(kAABB) 
    {
        resize(points);
    }

    ~AABB();

    /// @}

    /// @name Operators
    /// @{

    AABB& operator=(const AABB& other);

    /// @}

    /// @name Properties
    /// @{

    AABBData& boxData() { return m_boxData; }
    const AABBData& boxData() const { return m_boxData; }
    
    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Resize given a set of points
    template<typename D, size_t N>
    void resize(const std::vector<Vector<D, N>>& points) {
        m_boxData.resize(points);
    }

    virtual bool intersects(const CollidingGeometry& other) const override;
    virtual void recalculateBounds(const TransformInterface& transform, CollidingGeometry& out) const override;
    void recalculateBounds(const Matrix4x4g& transform, CollidingGeometry& out) const;

    /// @}

protected:

    /// @name Protected Members 
    /// @{

    AABBData m_boxData;

    /// @}

};


struct BoundingSphereData {
    float m_radius;
    Vector3 m_origin;
};

/// @class BoundingSphere
/// @see https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection
class BoundingSphere : public CollidingGeometry {
public:
    /// @name Constructor/Destructor
    /// @{

    BoundingSphere();
    ~BoundingSphere();

    /// @}

    /// @name Properties
    /// @{

    BoundingSphereData& data() { return m_data; }

    const Vector3& origin() const { return m_data.m_origin; }
    void setOrigin(const Vector3& origin) { m_data.m_origin = origin; }

    const float& radius() const { return m_data.m_radius; }
    void setRadius(const float& radius) { m_data.m_radius = radius; }

    /// @}

    /// @name Public Methods
    /// @{

    virtual bool intersects(const CollidingGeometry& other) const override;
    virtual void recalculateBounds(const TransformInterface& transform, CollidingGeometry& out) const override;

    /// @}

protected:

    /// @name Protected Members 
    /// @{

    BoundingSphereData m_data;

    /// @}

};


/// @class CollidingPoint
/// @see https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection
class CollidingPoint : public CollidingGeometry{
public:
    /// @name Constructor/Destructor
    /// @{

    CollidingPoint();
    ~CollidingPoint();

    /// @}

    /// @name Public Methods
    /// @{

    Vector3& point() { return m_point; }
    const Vector3& point() const { return m_point; }

    virtual bool intersects(const CollidingGeometry& other) const override;
    virtual void recalculateBounds(const TransformInterface& transform, CollidingGeometry& out) const override;

    /// @}

private:

    Vector3 m_point;
};


/// @class BoundingPlane
/// @see https://www.gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
/// @see https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection
/// @see https://www.scratchapixel.com/lessons/advanced-rendering/introduction-acceleration-structure/introduction
class BoundingPlane : public CollidingGeometry {
public:
    /// @name Static
    /// @{

    enum Halfspace{ 
        kNegative = -1,
        kIntersects = 0,
        kPositive = 1, 
    };

    /// @}

    /// @name Constructor/Destructor
    /// @{

    BoundingPlane();
    ~BoundingPlane();

    /// @}

    /// @name Properties
    /// @{

    const Vector3& normal() const { return m_normal; }

    /// @}

    /// @name Public Methods
    /// @{

    virtual bool intersects(const CollidingGeometry& other) const override;
    virtual void recalculateBounds(const TransformInterface& transform, CollidingGeometry& out) const override;

    Halfspace classifyGeometry(const Vector3& pt)  const;

    Halfspace classifyGeometry(const BoundingSphere& sphere) const;

    /// @see http://www.lighthouse3d.com/tutorials/view-frustum-culling/geometric-approach-testing-boxes-ii/
    Halfspace classifyGeometry(const AABB& aabb) const;

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
    float distanceToPoint(const Vector3& point);

    /// @brief Whether or not the plane is normalixed
    bool isNormalized() const {
        return m_normal.lengthSquared() == 1;
    }

    /// @brief Normalize the plane
    void normalize();

    /// @}

protected:

    /// @name Protected Methods 
    /// @{

    /// @brief Find the point most positive from (p) and point most negative from (n) this plane of the given AABB
    /// @see http://www.lighthouse3d.com/tutorials/view-frustum-culling/geometric-approach-testing-boxes-ii/
    void findNP(const AABB& aabb, Vector3& n, Vector3& p) const;

    /// @}

    /// @name Protected Members 
    /// @{

    /// @details ax + by + cz + d = 0
    Vector3 m_normal; ///< Coefficients of the plane equation describing this plane
    float m_d; ///< -normal.dot(planePosition)

    /// @}

};


/// @brief An aggregate class that includes one or more types of collision geometry
template<typename G>
class BoundingGeometry{
public:
    static_assert((std::is_base_of<AABB, G>::value ||
        std::is_base_of<BoundingSphere, G>::value ||
        std::is_base_of<CollidingPoint, G>::value), "Invalid geometry type");

    /// @name Constructors/Destructors
    /// @{

    BoundingGeometry() {}
    ~BoundingGeometry() {}

    /// @}

    /// @name Properties
    /// @{

    size_t count() const { return m_geometry.size(); }
    std::vector<G>& geometry() { return m_geometry; }
    const std::vector<G>& geometry() const { return m_geometry; }

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Transform the bounding geometry
    void transformBounds(const TransformInterface& transform, BoundingGeometry<G>& transformedGeomtry) const;

    /// @brief Transform the bounding geometry, and append to specified geometry
    void addTransformedBounds(const TransformInterface& transform, BoundingGeometry<G>& transformedGeometry) const;

    /// @brief Check whether or not the bounding geometry intersects the given piece of geometry
    bool inShape(const CollidingGeometry & shape) const;
    bool inShape(const CollidingGeometry & shape, size_t startIndex, size_t numChecks) const;

    /// @brief Whether or not the bounds are empty
    bool isEmpty() const { return m_geometry.size() == 0; }

    /// @}

protected:
    /// @name Protected Members 
    /// @{

    /// @brief Base geometry
    std::vector<G> m_geometry;

    /// @}

};
typedef BoundingGeometry<AABB> BoundingBoxes;




template<typename G>
inline void BoundingGeometry<G>::transformBounds(const TransformInterface& transform, BoundingGeometry<G>& transformedGeometry) const
{
    size_t originalSize = transformedGeometry.m_geometry.size();
    if (originalSize != m_geometry.size()) {
        transformedGeometry.m_geometry.resize(m_geometry.size());
    }

    for (size_t i = 0; i < m_geometry.size(); i++) {
        G& outG = transformedGeometry.m_geometry[i];
        //outG = AABB();
        m_geometry[i].recalculateBounds(transform, outG);
    }
}


template<typename G>
inline void BoundingGeometry<G>::addTransformedBounds(const TransformInterface& transform, BoundingGeometry<G>& transformedGeometry) const
{
    std::vector<G>& tg = transformedGeometry.m_geometry;
    tg.reserve(tg.size() + m_geometry.size());
    for (const G& geometry: m_geometry) {
        tg.emplace_back();
        G& outG = tg.back();
        geometry.recalculateBounds(transform, outG);
    }
}


template<typename G>
inline bool BoundingGeometry<G>::inShape(const CollidingGeometry & shape) const
{
    if (!m_geometry.size()) {
        // If no geometry, always pass test
        return true;
    }

    for (const G& geometry : m_geometry) {
        if (geometry.intersects(shape)) {
            return true;
        }
    }
    return false;
}


template<typename G>
inline bool BoundingGeometry<G>::inShape(const CollidingGeometry & shape, size_t startIndex, size_t numChecks) const
{
    if (!m_geometry.size()) {
        // If no geometry, always pass test
        return true;
    }

    size_t endIndex = startIndex + numChecks;
    for (size_t i = startIndex; i < endIndex; i++) {
        if (m_geometry[i].intersects(shape)) {
            return true;
        }
    }
    return false;
}



} // End namespaces
