#include "GbPhysicsShape.h"

#include "GbPhysicsActor.h"
#include "GbPhysicsManager.h"
#include "GbPhysicsMaterial.h"
#include "GbPhysicsGeometry.h"
#include "../scene/GbScene.h"
#include "../utils/GbMemoryManager.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// Physics Material
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsShapePrefab::PhysicsShapePrefab(const QJsonValue & json)
{
    loadFromJson(json);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsShapePrefab::PhysicsShapePrefab(const QString& name,
    const std::shared_ptr<PhysicsGeometry>& geometry,
    const std::shared_ptr<PhysicsMaterial>& material):
    PhysicsBase(name),
    m_geometry(geometry)
{
    addMaterial(material);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<PhysicsShapePrefab> PhysicsShapePrefab::get(const QString & name)
{
    std::shared_ptr<PhysicsShapePrefab> shape;
    if (!Map::HasKey(PhysicsManager::shapes(), name)) {
        throw("Error, shape with the specified name not found");
    }
    else {
        shape = PhysicsManager::shapes().at(name);
    }

    return shape;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<PhysicsShapePrefab> PhysicsShapePrefab::get(const QJsonValue & json)
{
    std::shared_ptr<PhysicsShapePrefab> shape;
    QString name = json.toObject()["name"].toString();
    if (!Map::HasKey(PhysicsManager::shapes(), name)) {
        shape = prot_make_shared<PhysicsShapePrefab>(json);
        addToManager(shape);
    }
    else {
        shape = PhysicsManager::shapes().at(name);
    }

    return shape;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<PhysicsShapePrefab> PhysicsShapePrefab::get(const QString & name,
    const std::shared_ptr<PhysicsGeometry>& geometry, 
    const std::shared_ptr<PhysicsMaterial>& material)
{
    if (!Map::HasKey(PhysicsManager::shapes(), name)) {
        PhysicsShapePrefab::create(name, geometry, material);
    }

    return PhysicsManager::shapes().at(name);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<PhysicsShapePrefab> PhysicsShapePrefab::create(const QJsonValue & json)
{
    auto shape = prot_make_shared<PhysicsShapePrefab>(json);
    addToManager(shape);
    return shape;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<PhysicsShapePrefab> PhysicsShapePrefab::create(const QString & name, 
    const std::shared_ptr<PhysicsGeometry>& geometry,
    const std::shared_ptr<PhysicsMaterial>& material)
{
    auto shape = prot_make_shared<PhysicsShapePrefab>(name, geometry, material);
    addToManager(shape);
    return shape;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsShapePrefab::~PhysicsShapePrefab()
{
    //delete m_geometry;

    // Clear references from all instances to avoid crash on PhysicsShape destructor
    for (const auto& shapePair : m_instances) {
        shapePair.second->prepareForDelete();
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShapePrefab::setGeometry(const std::shared_ptr<PhysicsGeometry>& geometry)
{
    m_geometry = geometry;
    for (const auto& instancePair : m_instances) {
        instancePair.second->reinitialize();
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
physx::PxShape* PhysicsShapePrefab::createExclusive(RigidBody& rigidBody) const
{
    return physx::PxRigidActorExt::createExclusiveShape(
        *rigidBody.rigidActor(),
        m_geometry->getGeometry(),
        *(m_materials.begin()->second)->getMaterial());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShapePrefab::addMaterial(const std::shared_ptr<PhysicsMaterial>& mtl)
{
    m_materials.emplace(mtl->getName(), mtl);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue PhysicsShapePrefab::asJson() const
{
    QJsonObject object = PhysicsBase::asJson().toObject();
    object.insert("geometry", m_geometry->asJson());

    QJsonArray materials;
    for (const std::pair<QString, std::shared_ptr<PhysicsMaterial>>& mtlPair: m_materials) {
        materials.append(mtlPair.first);
    }
    object.insert("materials", materials);
    
    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShapePrefab::loadFromJson(const QJsonValue & json)
{
    PhysicsBase::loadFromJson(json);
    QJsonObject object = json.toObject();

    // Load geometry
    m_geometry = PhysicsGeometry::createGeometry(
        object.value("geometry").toObject());

    // Load materials
    QJsonArray materials = object.value("materials").toArray();
    for (const QJsonValueRef& mtlJson : materials) {
        QString mtlName = mtlJson.toString();
        addMaterial(PhysicsManager::materials().at(mtlName));
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShapePrefab::addToManager(std::shared_ptr<PhysicsShapePrefab> shape)
{
    const QString& name = shape->getName();
    if (Map::HasKey(PhysicsManager::shapes(), name)) {
#ifdef DEBUG_MODE
        qDebug() << "Re-adding a shape with the name " + name;
#endif
    }
    PhysicsManager::s_shapes.emplace(name, shape);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShapePrefab::addInstance(PhysicsShape * instance)
{
    m_instances[instance->getUuid()] = instance;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShapePrefab::removeInstance(PhysicsShape * instance)
{
    m_instances.erase(instance->getUuid());
}



//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// PhysicsShape
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsShape::PhysicsShape(PhysicsShapePrefab & prefab, RigidBody * body) :
    m_prefab(&prefab),
    m_pxShape(prefab.createExclusive(*body)),
    m_body(body)
{
    m_prefab->addInstance(this);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsShape::~PhysicsShape()
{
    // Shapes should be released when detached from actors
    //PX_RELEASE(m_pxShape);
    if (m_prefab) {
        m_prefab->removeInstance(this);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShape::detach() const
{
    if (!m_body) throw("Error, no body associated with shape");

    if(m_body->actor())
        m_body->rigidActor()->detachShape(*m_pxShape, true);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShape::reinitialize()
{
    // Delete old PxShape and replace on rigid actor with new one
    m_body->rigidActor()->detachShape(*m_pxShape, true);
    m_pxShape = m_prefab->createExclusive(*m_body);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShape::setPrefab(PhysicsShapePrefab & prefab, bool removeFromOldPrefab)
{
    if (m_prefab->getUuid() == prefab.getUuid()) return;

    // Remove as an instance from previous prefab
    if(removeFromOldPrefab)
        m_prefab->removeInstance(this);

    // Set new prefab and reinitialize
    m_prefab = &prefab;
    m_prefab->addInstance(this);
    reinitialize();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShape::prepareForDelete()
{
    m_prefab = nullptr;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
}