#include "GPhysicsShapePrefab.h"

#include "GPhysicsShape.h"
#include "GPhysicsActor.h"
#include "GPhysicsManager.h"
#include "GPhysicsMaterial.h"
#include "GPhysicsGeometry.h"
#include "../scene/GScene.h"
#include "../utils/GMemoryManager.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsShapePrefab::PhysicsShapePrefab()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsShapePrefab::PhysicsShapePrefab(const GString & name):
    PhysicsBase(name)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//PhysicsShapePrefab::PhysicsShapePrefab(const QJsonValue & json)
//{
//    loadFromJson(json);
//}
////////////////////////////////////////////////////////////////////////////////////////////////////
//PhysicsShapePrefab::PhysicsShapePrefab(const GString& name,
//    const std::shared_ptr<PhysicsGeometry>& geometry,
//    const std::shared_ptr<PhysicsMaterial>& material):
//    PhysicsBase(name),
//    m_geometry(geometry)
//{
//    addMaterial(material);
//}
////////////////////////////////////////////////////////////////////////////////////////////////////
//PhysicsShapePrefab* PhysicsShapePrefab::Get(const GString & name, std::unique_ptr<PhysicsGeometry> geometry, const std::shared_ptr<PhysicsMaterial>& material)
//{
//    if (!Map::HasKey(PhysicsManager::ShapePrefabs(), name)) {
//        PhysicsShapePrefab::Create(name, std::move(geometry), std::move(material));
//    }
//
//    return PhysicsManager::ShapePrefabs().at(name).get();
//}
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsShapePrefab* PhysicsShapePrefab::Create(const QJsonValue & json)
{
    auto shape = prot_make_unique<PhysicsShapePrefab>();
    shape->loadFromJson(json);
    return AddToManager(std::move(shape));
}
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsShapePrefab* PhysicsShapePrefab::Create(const GString & name, std::unique_ptr<PhysicsGeometry> geometry, const std::shared_ptr<PhysicsMaterial>& material)
{
    auto shape = prot_make_unique<PhysicsShapePrefab>(name);
    shape->setGeometry(std::move(geometry));
    shape->addMaterial(material);
    return AddToManager(std::move(shape));
}
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsShapePrefab::~PhysicsShapePrefab()
{
    //delete m_geometry;

    // Clear references from all instances to avoid crash on PhysicsShape destructor
    for (PhysicsShape* shape : m_instances) {
        shape->prepareForDelete();
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShapePrefab::setGeometry(std::unique_ptr<PhysicsGeometry> geometry)
{
    m_geometry = std::move(geometry);
    updateInstances();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShapePrefab::updateInstances()
{
    for (PhysicsShape* shape : m_instances) {
        shape->reinitialize();
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
physx::PxShape* PhysicsShapePrefab::createExclusive(RigidBody& rigidBody) const
{
    return physx::PxRigidActorExt::createExclusiveShape(
        *rigidBody.as<physx::PxRigidActor>(),
        m_geometry->getGeometry(),
        *(m_materials[0]->getMaterial()));
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShapePrefab::addMaterial(const std::shared_ptr<PhysicsMaterial>& mtl)
{
    m_materials.push_back(mtl);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue PhysicsShapePrefab::asJson(const SerializationContext& context) const
{
    QJsonObject object = PhysicsBase::asJson(context).toObject();
    object.insert("geometry", m_geometry->asJson());

    QJsonArray materials;
    for (const std::shared_ptr<PhysicsMaterial>& mtl: m_materials) {
        materials.append(mtl->getName().c_str());
    }
    object.insert("materials", materials);
    
    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShapePrefab::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context);

    PhysicsBase::loadFromJson(json);
    QJsonObject object = json.toObject();

    // Load geometry
    m_geometry = std::move(PhysicsGeometry::CreateGeometry(object.value("geometry").toObject()));

    // Load materials
    QJsonArray materials = object.value("materials").toArray();
    for (const QJsonValueRef& mtlJson : materials) {
        QString mtlName = mtlJson.toString();
        addMaterial(PhysicsManager::Material(mtlName));
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsShapePrefab* PhysicsShapePrefab::AddToManager(std::unique_ptr<PhysicsShapePrefab> shape)
{
    PhysicsManager::s_shapes.push_back(std::move(shape));
    return PhysicsManager::s_shapes.back().get();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShapePrefab::addInstance(PhysicsShape * instance)
{
    m_instances.emplace_back(instance);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShapePrefab::removeInstance(PhysicsShape * instance)
{
    auto iter = std::find_if(m_instances.begin(), m_instances.end(),
        [instance](PhysicsShape* shape) {
        return instance->getUuid() == shape->getUuid();
    });

    if (iter == m_instances.end()) {
        throw("Error, shape instance not found");
    }
    m_instances.erase(iter);
}



//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
}