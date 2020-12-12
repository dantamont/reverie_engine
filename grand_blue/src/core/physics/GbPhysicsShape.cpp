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
PhysicsShapePrefab::PhysicsShapePrefab(const GString& name,
    const std::shared_ptr<PhysicsGeometry>& geometry,
    const std::shared_ptr<PhysicsMaterial>& material):
    PhysicsBase(name),
    m_geometry(geometry)
{
    addMaterial(material);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<PhysicsShapePrefab> PhysicsShapePrefab::get(const GString & name)
{
    std::shared_ptr<PhysicsShapePrefab> shape;
    if (!Map::HasKey(PhysicsManager::ShapePrefabs(), name)) {
        throw("Error, shape with the specified name not found");
    }
    else {
        shape = PhysicsManager::ShapePrefabs().at(name);
    }

    return shape;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<PhysicsShapePrefab> PhysicsShapePrefab::get(const QJsonValue & json)
{
    std::shared_ptr<PhysicsShapePrefab> shape;
    GString name = json.toObject()["name"].toString();
    if (!Map::HasKey(PhysicsManager::ShapePrefabs(), name)) {
        shape = prot_make_shared<PhysicsShapePrefab>(json);
        addToManager(shape);
    }
    else {
        shape = PhysicsManager::ShapePrefabs().at(name);
    }

    return shape;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<PhysicsShapePrefab> PhysicsShapePrefab::get(const GString & name,
    const std::shared_ptr<PhysicsGeometry>& geometry, 
    const std::shared_ptr<PhysicsMaterial>& material)
{
    if (!Map::HasKey(PhysicsManager::ShapePrefabs(), name)) {
        PhysicsShapePrefab::create(name, geometry, material);
    }

    return PhysicsManager::ShapePrefabs().at(name);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<PhysicsShapePrefab> PhysicsShapePrefab::create(const QJsonValue & json)
{
    auto shape = prot_make_shared<PhysicsShapePrefab>(json);
    addToManager(shape);
    return shape;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<PhysicsShapePrefab> PhysicsShapePrefab::create(const GString & name, 
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
    for (PhysicsShape* shape : m_instances) {
        shape->prepareForDelete();
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShapePrefab::setGeometry(const std::shared_ptr<PhysicsGeometry>& geometry)
{
    m_geometry = geometry;
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
        *rigidBody.rigidActor(),
        m_geometry->getGeometry(),
        *(m_materials[0]->getMaterial()));
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShapePrefab::addMaterial(const std::shared_ptr<PhysicsMaterial>& mtl)
{
    m_materials.push_back(mtl);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue PhysicsShapePrefab::asJson() const
{
    QJsonObject object = PhysicsBase::asJson().toObject();
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
    m_geometry = PhysicsGeometry::createGeometry(
        object.value("geometry").toObject());

    // Load materials
    QJsonArray materials = object.value("materials").toArray();
    for (const QJsonValueRef& mtlJson : materials) {
        QString mtlName = mtlJson.toString();
        addMaterial(PhysicsManager::Materials().at(mtlName));
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShapePrefab::addToManager(std::shared_ptr<PhysicsShapePrefab> shape)
{
    const GString& name = shape->getName();
    if (Map::HasKey(PhysicsManager::ShapePrefabs(), name)) {
#ifdef DEBUG_MODE
        Object().logInfo("Re-adding a shape with the name " + name);
#endif
    }
    PhysicsManager::s_shapes.emplace(name, shape);
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
// PhysicsShape
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsShape::PhysicsShape() :
    m_pxShape(nullptr),
    m_prefab(nullptr) {
}
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
    if (!m_body) {
        throw("Error, no body associated with shape");
    }

    if (m_body->actor()) {
        m_body->rigidActor()->detachShape(*m_pxShape, true);
    }
#ifdef DEBUG_MODE
    else {
        logWarning("Body should always have actor");
    }
#endif
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
    if (m_prefab->getUuid() == prefab.getUuid()) {
        return;
    }

    // Remove as an instance from previous prefab
    if (removeFromOldPrefab && m_prefab) {
        m_prefab->removeInstance(this);
    }

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