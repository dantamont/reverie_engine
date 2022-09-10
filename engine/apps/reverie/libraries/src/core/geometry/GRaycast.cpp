#include "core/geometry/GRaycast.h"

#include "core/rendering/view/GCamera.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"
#include "core/GCoreEngine.h"
#include "core/scene/GScene.h"
#include "core/scene/GSceneObject.h"
#include "heave/collisions/GCollisions.h"
#include "core/components/GTransformComponent.h"
#include "core/components/GModelComponent.h"
#include "core/rendering/models/GModel.h"
#include "core/rendering/geometry/GMesh.h"

#define MOLLER_TRUMBORE
#define CULLING

namespace rev {


WorldRay WorldRay::FromMousePosition(const Vector2 & widgetPos, const Camera& camera, const OpenGlRenderer& renderer)
{
    WorldRay ray;
    ray.m_origin = camera.eye();
    camera.widgetToRayDirection(widgetPos, ray.m_direction, renderer);
    ray.initialize();
    return ray;
}

bool WorldRay::TriangleIntersect(const Vector3 & orig, const Vector3 & dir, const Vector3 & v0, const Vector3 & v1, const Vector3 & v2, float & t, float & u, float & v, Vector3 & v0v1, Vector3 &v0v2)
{
    /// \see https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
#ifdef MOLLER_TRUMBORE 
    v0v1 = v1 - v0;
    v0v2 = v2 - v0;
    Vector3 pvec = dir.cross(v0v2);
    float det = v0v1.dot(pvec);
#ifdef CULLING 
    // if the determinant is negative the triangle is backfacing
    // if the determinant is close to 0, the ray misses the triangle
    if (det < s_errorTolerance) return false;
#else 
    // ray and triangle are parallel if det is close to 0
    if (fabs(det) < s_errorTolerance) return false;
#endif 
    float invDet = 1 / det;

    Vector3 tvec = orig - v0;
    u = tvec.dot(pvec) * invDet;
    if (u < 0 || u > 1) return false;

    Vector3 qvec = tvec.cross(v0v1);
    v = dir.dot(qvec) * invDet;
    if (v < 0 || u + v > 1) return false;

    t = v0v2.dot(qvec) * invDet;

    return true;
#else 
    Logger::Throw("Unimplemented");
    return false;
#endif 
}

WorldRay::WorldRay()
{
}

WorldRay::WorldRay(const Vector3 & origin, const Vector3 & direction, Real_t maxDistance, RaycastFlags flags):
    m_origin(origin),
    m_direction(direction.normalized()),
    m_maxDistance(maxDistance),
    m_raycastFlags(flags)
{
    initialize();
}

WorldRay::~WorldRay()
{
}

bool WorldRay::cast(const SceneObject & so, std::vector<WorldRayHit>& outHits) const
{
    bool hit = false;

    ModelComponent* mc = so.getComponent<ModelComponent>(ComponentType::kModel);
    if (!mc) {
        return hit;
    }

    Model* model = mc->model();
    if (!model) {
        return hit;
    }
    std::vector<ModelChunk>& chunks = model->chunks();

    // Perform raycast on scene object bounds
    Matrix4x4g inverseWorld = Matrix4x4g::EmptyMatrix();
    //bool calculatedInverseWorld = false;
    Vector4 hitPoint;
    Vector4 hitNormal;
    double tmin;
    double tmax;
    uint32_t boxIndex = 0;

    bool testingTriangles = m_raycastFlags.testFlag(RaycastFlag::kTestTriangles);
    Vector4 objectRayOrigin = Vector4(m_origin, 1);
    Vector4 objectRayDir = Vector4(m_direction, 0);
    if (testingTriangles) {
        inverseWorld = so.transform().worldMatrix().inversed();
        objectRayOrigin = inverseWorld * objectRayOrigin;
        objectRayDir = inverseWorld * objectRayDir;
    }

    for (const AABB& box : so.worldBounds().geometry()) {
        if (cast(box, tmin, tmax)) {
            if (!testingTriangles) {
                // Determine distance from ray origin to hit point
                Real_t distance = (Real_t)tmin;
                if (tmin < 0) {
                    distance = (Real_t)tmax;
                }

                // Calculate hit point in world space
                hitPoint = Vector4(distance * m_direction + m_origin);
                hitPoint.setW(1);

                // Calculate hit normal in world space
                /// \see http://blog.johnnovak.net/2016/10/22/the-nim-raytracer-project-part-4-calculating-box-normals/
                Vector3 c = box.boxData().getOrigin();
                Vector3 p = hitPoint - c;
                Vector3 d = (box.boxData().min() - box.boxData().max()) * 0.5;
                Real_t bias = (Real_t)1.000001; // To fix floating point errors
                hitNormal = Vector4(Vector3(float(int(p.x() / abs(d.x()) * bias)),
                    float(int(p.y() / abs(d.y()) * bias)),
                    float(int(p.z() / abs(d.z()) * bias))).normalized(), 0);

                // Create hit
                outHits.emplace_back(HitInfo{so.id(), Uuid(false)}, distance, hitPoint, hitNormal);
                hit = true;

                if (m_raycastFlags.testFlag(RaycastFlag::kSingleHit) ||
                    m_raycastFlags.testFlag(RaycastFlag::kSingleHitPerObject)) {
                    // Break if only checking against first hit
                    break;
                }
            }
            else {
                /// @fixme Implement this.
                /// Meshes used to store vertex data, but not any longer!
                /// Color picking is the new approach for checking collisions. Need to add
                /// a flag to scene objects for whether or not they should have this style of
                /// collision check. 
                /// Could store an index/count of the mesh's vertices in a vector in ResourceCache,
                /// and handle these on a case-by-case basis

                // Otherwise, need to test triangles
                //const Mesh& mesh = *chunks[boxIndex].mesh();

                // Check if hit mesh
                //hit = cast(so, vertexData, objectRayOrigin, objectRayDir, outHits);

                if (hit && (m_raycastFlags.testFlag(RaycastFlag::kSingleHit) ||
                    m_raycastFlags.testFlag(RaycastFlag::kSingleHitPerObject))) {
                    // Break if only checking against first hit
                    break;
                }
            }
        }

        boxIndex++;
    }

    if (hit && m_raycastFlags.testFlag(RaycastFlag::kSingleHit)) {
        // Return if only checking for a single hit
        return hit;
    }

    // Perform raycast for all child objects
    for (const auto& child : so.children()) {
        cast(*child, outHits);
    }

    return hit;
}

bool WorldRay::cast(const AABB & aabb, double& tmin, double& tmax) const
{
    /// \see https://tavianator.com/2011/ray_box.html
    // https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
    // https://gdbooks.gitbooks.io/3dcollisions/content/Chapter3/raycast_aabb.html
    // Cyrus-Beck clipping
    const AABBData& b = aabb.boxData();
    double tx1 = (b.min().x() - m_origin.x())*m_dirReciprocal.x();
    double tx2 = (b.max().x() - m_origin.x())*m_dirReciprocal.x();

    tmin = std::min(tx1, tx2);
    tmax = std::max(tx1, tx2);

    double ty1 = (b.min().y() - m_origin.y())*m_dirReciprocal.y();
    double ty2 = (b.max().y() - m_origin.y())*m_dirReciprocal.y();

    tmin = std::max(tmin, std::min(ty1, ty2));
    tmax = std::min(tmax, std::max(ty1, ty2));

    double tz1 = (b.min().z() - m_origin.z())*m_dirReciprocal.z();
    double tz2 = (b.max().z() - m_origin.z())*m_dirReciprocal.z();

    tmin = std::max(tmin, std::min(tz1, tz2));
    tmax = std::min(tmax, std::max(tz1, tz2));

    if (tmax < 0) {
        // if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behind origin
        return false;
    }
    else {
        return tmax >= tmin;
    }
}

bool WorldRay::cast(const SceneObject & so, const MeshVertexAttributes& vertexData, const Vector3& objectOrigin, const Vector3& objectDir, std::vector<WorldRayHit>& outHits) const
{
    Vector3 v0, v1, v2, v0v1, v0v2;
    float t, u, v;
    bool anyHit = false;
    size_t numIndices = vertexData.get<MeshVertexAttributeType::kIndices>().size();
    const Matrix4x4& worldMatrix = so.transform().worldMatrix();
    Vector3 worldHitPoint;
    Vector4 objectNormal;
    Vector3 worldHitNormal;

    size_t hitCount = 0;
    for (size_t i = 0; i < numIndices; i+=3) {
        v0 = vertexData.get<MeshVertexAttributeType::kPosition>()[vertexData.get<MeshVertexAttributeType::kIndices>()[i]];
        v1 = vertexData.get<MeshVertexAttributeType::kPosition>()[vertexData.get<MeshVertexAttributeType::kIndices>()[i+1]];
        v2 = vertexData.get<MeshVertexAttributeType::kPosition>()[vertexData.get<MeshVertexAttributeType::kIndices>()[i+2]];
        bool hit = TriangleIntersect(objectOrigin, objectDir, v0, v1, v2, t, u, v, v0v1, v0v2);

        if (hit) {
            // There was a hit!
            anyHit = hit;

            // Create hit
            // Need to convert hit point from object to world space
            worldHitPoint = worldMatrix * Vector4(objectOrigin + objectDir * t, 1);

            // Need to convert ray direction (normal) as well, and normalize since scaling may
            // be non-uniform
            // Find normal
            /// \see https://math.stackexchange.com/questions/305642/how-to-find-surface-normal-of-a-triangle
            if (!vertexData.get<MeshVertexAttributeType::kNormal>().size()) {
                // Calculate normals from triangle if unspecified
                objectNormal = Vector4(
                    v0v1.y()*v0v2.z() - v0v1.z()*v0v2.y(),
                    v0v1.z()*v0v2.x() - v0v1.x()*v0v2.z(),
                    v0v1.x()*v0v2.y() - v0v1.y()*v0v2.x(),
                    0);
            }
            else {
                // Otherwise, average specified normals
                objectNormal = (vertexData.get<MeshVertexAttributeType::kNormal>()[vertexData.get<MeshVertexAttributeType::kIndices>()[i]]
                    + vertexData.get<MeshVertexAttributeType::kNormal>()[vertexData.get<MeshVertexAttributeType::kIndices>()[i + 1]]
                    + vertexData.get<MeshVertexAttributeType::kNormal>()[vertexData.get<MeshVertexAttributeType::kIndices>()[i + 2]]) / 3.0;
            }
            worldHitNormal = worldMatrix * objectNormal;

            // Need to normalize
            //worldHitNormal /= Vector3(worldHitNormal).length(); 
            worldHitNormal.normalize();

            // Get world distance
            /// @see https://gamedev.stackexchange.com/questions/72440/the-correct-way-to-transform-a-ray-with-a-matrix
            Real_t worldDistance = worldHitPoint.distance(m_origin);

            outHits.emplace_back(HitInfo{ so.id(), Uuid(false) }, 
                worldDistance, // distance 
                worldHitPoint, // hit point in world space
                worldHitNormal);

            if (m_raycastFlags.testFlag(RaycastFlag::kSingleHit) ||
                m_raycastFlags.testFlag(RaycastFlag::kSingleHitPerObject)) {
                break;
            }

            hitCount++;
        }
    }

    return anyHit;
}

void WorldRay::initialize()
{
    m_dirReciprocal = Vector3(1.0 / m_direction.x(), 1.0 / m_direction.y(), 1.0 / m_direction.z());
}


WorldRayHit::WorldRayHit(HitInfo && info, Real_t distance, const Vector4 & hitPoint, const Vector4& normal):
    m_hitInfo(info),
    m_distance(distance),
    m_position(hitPoint),
    m_normal(normal)
{
}


float WorldRay::s_errorTolerance = (float)1e-6;




}