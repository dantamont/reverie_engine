//////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_FRUSTUM_H
#define GB_FRUSTUM_H

// QT

// Internal
#include "../../geometry/GCollisions.h"
#include "GRenderProjection.h"
#include "GFrameBuffer.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////////////////////
class RenderProjection;
class FrameBuffer;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
//////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Describes the view frustum for a camera
/// @details Currently 36 bytes
class Frustum: public CollidingGeometry {
public:

    //---------------------------------------------------------------------------------------
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

    /// @brief Get the points describing the corners of a frustum with the specified view and projection matrices
    /// @note For other approaches, see: http://donw.io/post/frustum-point-extraction/
    static void ExtractFrustumPoints(const Matrix4x4g& viewMatrix, const Matrix4x4g& projectionMatrix, std::array<Vector4, 8>& outPoints);
    static AABBData FrustomBoundingBox(const Matrix4x4g& viewMatrix, const Matrix4x4g& projectionMatrix);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    Frustum();
    Frustum(const Frustum& other);
    Frustum(const Matrix4x4g& viewMatrix, const Matrix4x4g& projectionMatrix);
    ~Frustum();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    Frustum& operator=(const Frustum& other);

    /// @}


    //---------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief Initialize the frustum
    void initialize(const Matrix4x4g& viewMatrix, const Matrix4x4g& projectionMatrix);

    /// @brief Normalize the planes of the frustum
    void normalizePlanes();

    virtual bool intersects(const CollidingGeometry& other) const;
    virtual void recalculateBounds(const Transform& transform, CollidingGeometry& out) const;


    /// @brief Checks whether or not the given geometry is inside the frustum
    bool contains(const CollidingGeometry& geometry) const;
    bool contains(const Vector3& point) const;

    /// @}

private:
    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    /// @brief The culling planes for the frustum
    /// @details All planes have normals facing inside the frustum
    std::array<BoundingPlane, 6> m_planes;

    /// @}
};



//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}


#endif