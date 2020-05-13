#ifndef GB_COLLISIONS_H
#define GB_COLLISIONS_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Standard

// QT

// Internal
#include "GbMatrix.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////

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
        kBoundingSphere
    };

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructor/Destructor
    /// @{

    CollidingGeometry(GeometryType type);
    ~CollidingGeometry();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual bool intersects(const CollidingGeometry& other) = 0;

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
    ~AABB();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    const double& minX() const { return m_minX; }
    const double& maxX() const { return m_maxX; }
    const double& minY() const { return m_minY; }
    const double& maxY() const { return m_maxY; }
    const double& minZ() const { return m_minZ; }
    const double& maxZ() const { return m_maxZ; }
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual bool intersects(const CollidingGeometry& other) override;

    /// @}

protected:

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members 
    /// @{

    double m_minX;
    double m_maxX;
    double m_minY;
    double m_maxY;
    double m_minZ;
    double m_maxZ;

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

    const Vector3& origin() const { return m_origin; }
    const double& radius() const { return m_radius; }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual bool intersects(const CollidingGeometry& other) override;

    /// @}

protected:

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members 
    /// @{

    Vector3 m_origin;
    double m_radius;

    /// @}

};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class CollidingPoint
// See: https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection
class CollidingPoint : public CollidingGeometry, public Vector3 {
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
// See: https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection
class BoundingPlane : public CollidingGeometry {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum Halfspace{ 
        kNegative = -1,
        kOnPlane = 0,
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
    /// @name Public Methods
    /// @{

    virtual bool intersects(const CollidingGeometry& other) override;

    HalfspaceClassifyPoint(constPlane & plane, constPoint & pt) 
    { 
        float d;   
        d = plane.a*pt.x + plane.b*pt.y + plane.c*pt.z + plane.d;  
        if (d < 0) return NEGATIVE;   
        if (d > 0) return POSITIVE;
        return ON_PLANE;
    }

    /// @}

protected:

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members 
    /// @{

    /// @}

};


//////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif