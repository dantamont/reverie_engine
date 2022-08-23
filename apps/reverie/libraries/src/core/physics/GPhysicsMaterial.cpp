#include "core/physics/GPhysicsMaterial.h"

#include "fortress/system/memory/GPointerTypes.h"
#include "core/physics/GPhysicsManager.h"
#include "core/scene/GScene.h"

namespace rev {



// Physics Material

std::shared_ptr<PhysicsMaterial> PhysicsMaterial::Create(const nlohmann::json& json)
{
    std::shared_ptr<PhysicsMaterial> mtl = prot_make_shared<PhysicsMaterial>(json);
    AddToManager(mtl);
    return mtl;
}

std::shared_ptr<PhysicsMaterial> PhysicsMaterial::Create(const GString & name, float staticFriction, float dynamicFriction, float restitution)
{
    std::shared_ptr<PhysicsMaterial> mtl = prot_make_shared<PhysicsMaterial>(name, staticFriction, dynamicFriction, restitution);
    AddToManager(mtl);
    return mtl;
}

PhysicsMaterial::PhysicsMaterial(const nlohmann::json& json):
    m_material(nullptr)
{
    json.get_to(*this);
}

PhysicsMaterial::PhysicsMaterial(const GString& name,
    float staticFriction, float dynamicFriction, float restitution):
    PhysicsBase(name),
    m_material(PhysicsManager::Physics()->createMaterial(staticFriction, 
        dynamicFriction, 
        restitution)) 
{
}

void PhysicsMaterial::AddToManager(std::shared_ptr<PhysicsMaterial> mtl)
{
    // TODO: See where empty material is being added
    const GString& name = mtl->getName();

    PhysicsManager::Materials().push_back(mtl);
}

PhysicsMaterial::~PhysicsMaterial()
{
    PX_RELEASE(m_material);
}

void to_json(json& orJson, const PhysicsMaterial& korObject)
{
    ToJson<PhysicsBase>(orJson, korObject);
    orJson["static_friction"] = korObject.m_material->getStaticFriction();
    orJson["dynamic_friction"] = korObject.m_material->getDynamicFriction();
    orJson["restitution"] = korObject.m_material->getRestitution();
}

void from_json(const json& korJson, PhysicsMaterial& orObject)
{
    FromJson<PhysicsBase>(korJson, orObject);
    
    float staticFriction = korJson.at("static_friction").get<Float32_t>();
    float dynamicFriction = korJson.at("dynamic_friction").get<Float32_t>();
    float restitution = korJson.at("restitution").get<Float32_t>();

    if (orObject.m_material) {
        orObject.setStaticFriction(staticFriction);
        orObject.setDynamicFriction(dynamicFriction);
        orObject.setRestitution(restitution);
    }else{
        orObject.m_material = PhysicsManager::Physics()->createMaterial(staticFriction,
            dynamicFriction,
            restitution);
    }
}






}