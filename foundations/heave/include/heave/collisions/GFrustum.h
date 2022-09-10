#pragma once

// Internal
#include "heave/collisions/GCollisions.h"

namespace rev {

/// @brief Describes the view frustum for a camera
/// @details Currently 36 bytes
class Frustum: public CollidingGeometry {
public:

    /// @name Static
    /// @{

    enum Plane {
        kLeft,
        kRight,
        kTop,
        kBottom,
        kNear,
        kFar
    };

    /// @brief The points of a frustum in NDC space
    static const std::array<Vector4, 8/*numPoints*/> s_ndcPoints;

    /// @brief Get the points describing the corners of a frustum with the specified view and projection matrices
    /// @note For other approaches, see: http://donw.io/post/frustum-point-extraction/
    static void ExtractFrustumPoints(const Matrix4x4g& viewMatrix, const Matrix4x4g& projectionMatrix, std::array<Vector4, 8>& outPoints);
    static AABBData FrustomBoundingBox(const Matrix4x4g& viewMatrix, const Matrix4x4g& projectionMatrix);

    /// @}

    /// @name Constructors/Destructor
    /// @{

    Frustum();
    Frustum(const Frustum& other);
    Frustum(const Matrix4x4g& viewMatrix, const Matrix4x4g& projectionMatrix);
    ~Frustum();

    /// @}

    /// @name Operators
    /// @{

    Frustum& operator=(const Frustum& other);

    /// @}


    /// @name Public methods
    /// @{

    /// @brief Initialize the frustum
    /// @see https://cgvr.cs.uni-bremen.de/teaching/cg_literatur/lighthouse3d_view_frustum_culling/index.html
    /// @see 
    void initialize(const Matrix4x4g& viewMatrix, const Matrix4x4g& projectionMatrix);

    /// @brief Normalize the planes of the frustum
    void normalizePlanes();

    virtual bool intersects(const CollidingGeometry& other) const;
    virtual void recalculateBounds(const TransformInterface& transform, CollidingGeometry& out) const;

    /// @brief Checks whether or not the given geometry is inside the frustum
    bool contains(const CollidingGeometry& geometry) const;
    bool contains(const Vector3& point) const;

    /// @}

private:
    /// @name Private Members
    /// @{

    /// @details All planes have normals facing inside the frustum
    std::array<BoundingPlane, 6> m_planes; ///< The culling planes for the frustum

    /// @}
};

    
// End namespaces
}
