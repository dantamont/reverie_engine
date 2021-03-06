#ifndef GB_RAYCAST_H
#define GB_RAYCAST_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Standard
#include <type_traits>

// QT

// Internal
#include "../containers/GFlags.h"
#include "../geometry/GVector.h"
#include "../containers/GByteArray.h"
#include "../encoding/GUUID.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class Camera;
class MainRenderer;
class SceneObject;
class AABB;
class Mesh;

//////////////////////////////////////////////////////////////////////////////////
// Macro Definitions
//////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Type Definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Struct containing identifying information about the hit
struct HitInfo {
    size_t m_sceneObjectId;
    Uuid m_meshId;
};

/// @brief Represents a collision between a ray and an object
struct WorldRayHit {

    WorldRayHit(HitInfo&& info, real_g distance, const Vector4& hitPoint, const Vector4& normal);

    /// @brief Metadata associated with the hit
    HitInfo m_hitInfo;

    /// @brief The distance from the hit to the ray origin, to avoid recalculating
    real_g m_distance;

    /// @brief The position of the hit in world-space
    Vector4 m_position = Vector4::EmptyVector();

    /// @brief The normal at the hit in world space
    Vector4 m_normal = Vector4::EmptyVector();
};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Flags for controlling raycast
enum class RaycastFlag {
    kUseDistance = 1 << 0, // If not set, raycast will have infinite length
    kTestTriangles = 1 << 1, // If set, iterates over mesh triangles as part of raycast
    kSingleHit = 1 << 2, // If set, only check for a single hit
    kSingleHitPerObject = 1 << 3, // If set, only check for a single hit per scene object
};
typedef Flags<RaycastFlag> RaycastFlags;


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class Raycast
/// @brief For raycasting without relying on the physics backend
class WorldRay {
public:

    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Create a raycast from mouse widget position
    static WorldRay FromMousePosition(const Vector2& widgetPos, const Camera& camera, const MainRenderer& renderer);

    /// @brief Determine an intersection with a triangle
    /// @note See: https://gamedev.stackexchange.com/questions/114955/m%C3%B6ller-trumbore-intersection-point
    /// @param[in] orig The origin of the raycast
    /// @param[in] dir The direction of the raycast
    /// @param[in] t the distance from P (the intersection point) to the ray origin
    /// @param[in] u the barycentric coordinate value such that P = v0 + u*v0v2 + v*v0v1
    /// @param[in] v the barycentric coordinate value such that P = v0 + u*v0v2 + v*v0v1
    /// @param[out] v0v1
    /// @param[out] v0v2
    static bool TriangleIntersect(
        const Vector3 &orig, const Vector3 &dir,
        const Vector3 &v0, const Vector3 &v1, const Vector3 &v2,
        float &t, float &u, float &v, 
        Vector3 & out_v0v1, Vector3 &out_v0v2);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    WorldRay();
    WorldRay(const Vector3& origin, const Vector3& direction, real_g maxDistance = (float)1e30, RaycastFlags flags = 0);
    ~WorldRay();

    /// @}


    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    void setRaycastFlags(RaycastFlags flags) {
        m_raycastFlags = flags;
    }

    const Vector3& origin() const { return m_origin; }
    Vector3& origin() { return m_origin; }
    void setOrigin(const Vector3& origin) { m_origin = origin; }

    const Vector3& direction() const { return m_direction; }
    Vector3& direction() { return m_direction; }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    void initialize();

    /// @brief Test if the ray hits the specified scene object
    bool cast(const SceneObject& so, std::vector<WorldRayHit>& outHits) const;

    /// @brief Test if the ray hits the specified bounding box
    /// @details When tmin < 0, the ray origin is inside the box
    /// @param[out] tmax The distance to the intersection point, such that when the ray is inside the AABB, tmax*ray.m_direction + ray.m_origin is the intersection point
    /// @param[out] tmin The distance to the intersection point, such that when the ray is outside the AABB, tmin*ray.m_direction + ray.m_origin is the intersection point
    bool cast(const AABB& aabb, double& tmin, double& tmax) const;

    /// @brief Test if the ray hits the specified mesh
    /// @param[in] objectOrigin the origin of the ray in the scene object's local object space
    /// @param[in] objectOrigin the direction of the ray in the scene object's local object space
    bool cast(const SceneObject & so, const Mesh& mesh, const Vector3& objectOrigin, const Vector3& objectDir, std::vector<WorldRayHit>& outHits) const;

    /// @}

    //bool hadHit() const {
    //    return m_hits.getNumHits() != 0;
    //}

    //RaycastHit firstHit() const {
    //    return m_hits.getHit(0);
    //}

private:

    //--------------------------------------------------------------------------------------------
    /// @name Private methods
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Private members
    /// @{

    RaycastFlags m_raycastFlags;

    real_g m_maxDistance = (float)1e30;

    Vector3 m_origin;

    /// @brief Unit direction of the ray
    Vector3 m_direction;

    /// @brief Component-wise inverse of direction of the ray
    Vector3 m_dirReciprocal;

    static float s_errorTolerance;

    /// @}
};




//////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif