#include "core/physics/GPhysicsShapePrefab.h"

#include "core/physics/GPhysicsShape.h"
#include "core/physics/GPhysicsActor.h"
#include "core/physics/GPhysicsManager.h"
#include "core/physics/GPhysicsMaterial.h"
#include "core/physics/GPhysicsGeometry.h"
#include "core/scene/GScene.h"
#include "fortress/system/memory/GPointerTypes.h"

namespace rev {


PhysicsShapePrefab::PhysicsShapePrefab()
{
}

PhysicsShapePrefab::PhysicsShapePrefab(const GString & name):
    PhysicsBase(name)
{
}

PhysicsShapePrefab* PhysicsShapePrefab::Create(const nlohmann::json& json)
{
    auto shape = prot_make_unique<PhysicsShapePrefab>();
    json.get_to(*shape);
    return AddToManager(std::move(shape));
}

PhysicsShapePrefab* PhysicsShapePrefab::Create(const GString & name, std::unique_ptr<PhysicsGeometry> geometry, const std::shared_ptr<PhysicsMaterial>& material)
{
    auto shape = prot_make_unique<PhysicsShapePrefab>(name);
    shape->setGeometry(std::move(geometry));
    shape->addMaterial(material);
    return AddToManager(std::move(shape));
}

PhysicsShapePrefab::~PhysicsShapePrefab()
{
    //delete m_geometry;

    // Clear references from all instances to avoid crash on PhysicsShape destructor
    for (PhysicsShape* shape : m_instances) {
        shape->prepareForDelete();
    }
}

void PhysicsShapePrefab::setGeometry(std::unique_ptr<PhysicsGeometry> geometry)
{
    m_geometry = std::move(geometry);
    updateInstances();
}

void PhysicsShapePrefab::updateInstances()
{
    for (PhysicsShape* shape : m_instances) {
        shape->reinitialize();
    }
}

physx::PxShape* PhysicsShapePrefab::createExclusive(RigidBody& rigidBody) const
{
    return physx::PxRigidActorExt::createExclusiveShape(
        *rigidBody.as<physx::PxRigidActor>(),
        m_geometry->getGeometry(),
        *(m_materials[0]->getMaterial()));
}

void PhysicsShapePrefab::addMaterial(const std::shared_ptr<PhysicsMaterial>& mtl)
{
    m_materials.push_back(mtl);
}

void to_json(json& orJson, const PhysicsShapePrefab& korObject)
{
    ToJson<PhysicsBase>(orJson, korObject);

    orJson["geometry"] = korObject.m_geometry->asJson();

    json materials = json::array();
    for (const std::shared_ptr<PhysicsMaterial>& mtl: korObject.m_materials) {
        materials.push_back(mtl->getName().c_str());
    }
    orJson["materials"] = materials;
}

void from_json(const json& korJson, PhysicsShapePrefab& orObject)
{
    FromJson<PhysicsBase>(korJson, orObject);

    // Load geometry
    orObject.m_geometry = std::move(PhysicsGeometry::CreateGeometry(korJson.at("geometry")));

    // Load materials
    const json& materials = korJson.at("materials");
    for (const json& mtlJson : materials) {
        GString mtlName;
        mtlJson.get_to(mtlName);
        orObject.addMaterial(PhysicsManager::Material(mtlName));
    }
}

PhysicsShapePrefab* PhysicsShapePrefab::AddToManager(std::unique_ptr<PhysicsShapePrefab> shape)
{
    PhysicsManager::s_shapes.push_back(std::move(shape));
    return PhysicsManager::s_shapes.back().get();
}

void PhysicsShapePrefab::addInstance(PhysicsShape * instance)
{
    m_instances.emplace_back(instance);
}

void PhysicsShapePrefab::removeInstance(PhysicsShape * instance)
{
    auto iter = std::find_if(m_instances.begin(), m_instances.end(),
        [instance](PhysicsShape* shape) {
        return instance->getUuid() == shape->getUuid();
    });

    if (iter == m_instances.end()) {
        Logger::Throw("Error, shape instance not found");
    }
    m_instances.erase(iter);
}





}