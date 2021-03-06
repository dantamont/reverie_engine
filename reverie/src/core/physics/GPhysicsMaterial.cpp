#include "GPhysicsMaterial.h"

#include "../utils/GMemoryManager.h"
#include "../physics/GPhysicsManager.h"
#include "../scene/GScene.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// Physics Material
//////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<PhysicsMaterial> PhysicsMaterial::Create(const QJsonValue & json)
{
    std::shared_ptr<PhysicsMaterial> mtl = prot_make_shared<PhysicsMaterial>(json);
    AddToManager(mtl);
    return mtl;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<PhysicsMaterial> PhysicsMaterial::Create(const GString & name, float staticFriction, float dynamicFriction, float restitution)
{
    std::shared_ptr<PhysicsMaterial> mtl = prot_make_shared<PhysicsMaterial>(name, staticFriction, dynamicFriction, restitution);
    AddToManager(mtl);
    return mtl;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsMaterial::PhysicsMaterial(const QJsonValue & json):
    m_material(nullptr)
{
    loadFromJson(json);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsMaterial::PhysicsMaterial(const GString& name,
    float staticFriction, float dynamicFriction, float restitution):
    PhysicsBase(name),
    m_material(PhysicsManager::Physics()->createMaterial(staticFriction, 
        dynamicFriction, 
        restitution)) 
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsMaterial::AddToManager(std::shared_ptr<PhysicsMaterial> mtl)
{
    // TODO: See where empty material is being added
    const GString& name = mtl->getName();

    PhysicsManager::Materials().push_back(mtl);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsMaterial::~PhysicsMaterial()
{
    PX_RELEASE(m_material);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue PhysicsMaterial::asJson(const SerializationContext& context) const
{
    QJsonObject object = PhysicsBase::asJson(context).toObject();
    object.insert("static_friction", m_material->getStaticFriction());
    object.insert("dynamic_friction", m_material->getDynamicFriction());
    object.insert("restitution", m_material->getRestitution());

    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsMaterial::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context);

    PhysicsBase::loadFromJson(json);
    QJsonObject object = json.toObject();
    float staticFriction = object.value("static_friction").toDouble();
    float dynamicFriction = object.value("dynamic_friction").toDouble();
    float restitution = object.value("restitution").toDouble();

    if (m_material) {
        setStaticFriction(staticFriction);
        setDynamicFriction(dynamicFriction);
        setRestitution(restitution);
    }else{
        m_material = PhysicsManager::Physics()->createMaterial(staticFriction,
            dynamicFriction,
            restitution);
    }
}




//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
}