#include "core/physics/GPhysicsGeometry.h"

#include "core/physics/GPhysicsManager.h"
#include "core/converters/GPhysxConverter.h"
#include "core/scene/GScene.h"

namespace rev {


std::unique_ptr<PhysicsGeometry> PhysicsGeometry::CreateGeometry(const json & json)
{
    GPhysicsGeometryType type = GPhysicsGeometryType(json.at("type").get<Int32_t>());
    std::unique_ptr<PhysicsGeometry> geometry = nullptr;
    switch ((EPhysicsGeometryType)type) {
    case EPhysicsGeometryType::eBox:
        geometry = std::make_unique<BoxGeometry>();
        FromJson<BoxGeometry>(json, *geometry);
        break;
    case EPhysicsGeometryType::eSphere:
        geometry = std::make_unique<SphereGeometry>();
        FromJson<SphereGeometry>(json, *geometry);
        break;
    case EPhysicsGeometryType::ePlane:
        geometry = std::make_unique<PlaneGeometry>();
        FromJson<PlaneGeometry>(json, *geometry);
        break;
    default:
        break;
    }

    return geometry;
}

PhysicsGeometry::PhysicsGeometry(GPhysicsGeometryType type):
    m_type(type)
{
}

PhysicsGeometry::~PhysicsGeometry()
{
}

json PhysicsGeometry::asJson() const
{
    json geometryJson;
    GPhysicsGeometryType type = getType();
    switch ((EPhysicsGeometryType)type) {
    case EPhysicsGeometryType::eBox:
        ToJson<BoxGeometry>(geometryJson, *this);
        break;
    case EPhysicsGeometryType::eSphere:
        ToJson<SphereGeometry>(geometryJson, *this);
        break;
    case EPhysicsGeometryType::ePlane:
        ToJson<PlaneGeometry>(geometryJson, *this);
        break;
    default:
        assert(false && "Unrecognized geometry type");
        break;
    }
    return geometryJson;
}

void to_json(json& orJson, const PhysicsGeometry& korObject)
{
    orJson["type"] = int(korObject.m_type);
}

void from_json(const json& korJson, PhysicsGeometry& orObject)
{
    orObject.m_type = GPhysicsGeometryType(korJson.at("type").get<Int32_t>());
}




BoxGeometry::BoxGeometry(const Vector3f & extents):
    PhysicsGeometry(GPhysicsGeometryType(EPhysicsGeometryType::eBox)),
    m_box(extents.x(), extents.y(), extents.z())
{
}

BoxGeometry::BoxGeometry(float hx, float hy, float hz):
    PhysicsGeometry(GPhysicsGeometryType(EPhysicsGeometryType::eBox)),
    m_box(hx, hy, hz)
{
}

BoxGeometry::~BoxGeometry()
{
}

Vector3d BoxGeometry::halfExtents() const
{
    return PhysxConverter::FromPhysX(m_box.halfExtents).asDouble();
}

void to_json(json& orJson, const BoxGeometry& korObject)
{
    ToJson<PhysicsGeometry>(orJson, korObject);
    orJson["hx"] = korObject.hx();
    orJson["hy"] = korObject.hy();
    orJson["hz"] = korObject.hz();
}

void from_json(const json& korJson, BoxGeometry& orObject)
{
    FromJson<PhysicsGeometry>(korJson, orObject);
    
    float hx = korJson.at("hx").get<Float32_t>();
    float hy = korJson.at("hy").get<Float32_t>();
    float hz = korJson.at("hz").get<Float32_t>();
    orObject.m_box = physx::PxBoxGeometry(hx, hy, hz);
}



SphereGeometry::SphereGeometry(): 
    PhysicsGeometry(GPhysicsGeometryType(EPhysicsGeometryType::eSphere))
{
}

SphereGeometry::SphereGeometry(float radius):
    PhysicsGeometry(GPhysicsGeometryType(EPhysicsGeometryType::eSphere)),
    m_sphere(radius)
{
}

SphereGeometry::~SphereGeometry()
{
}

void to_json(json& orJson, const SphereGeometry& korObject)
{
    ToJson<PhysicsGeometry>(orJson, korObject);
    orJson["radius"] = korObject.m_sphere.radius;
}

void from_json(const json& korJson, SphereGeometry& orObject)
{
    FromJson<PhysicsGeometry>(korJson, orObject);
    orObject.m_sphere = physx::PxSphereGeometry(korJson.at("radius").get<Float64_t>());
}




PlaneGeometry::PlaneGeometry(): 
    PhysicsGeometry(GPhysicsGeometryType(EPhysicsGeometryType::ePlane)),
    m_plane(physx::PxPlaneGeometry())
{
}

PlaneGeometry::~PlaneGeometry()
{
}

void to_json(json& orJson, const PlaneGeometry& korObject)
{
    ToJson<PhysicsGeometry>(orJson, korObject);
}

void from_json(const json& korJson, PlaneGeometry& orObject)
{
    FromJson<PhysicsGeometry>(korJson, orObject);
}





}