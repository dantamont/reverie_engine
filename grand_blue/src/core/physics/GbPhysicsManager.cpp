#include "GbPhysicsManager.h"

#include "../GbCoreEngine.h"
#include "GbPhysicsActor.h"
#include "GbPhysicsScene.h"
#include "GbPhysicsShape.h"
#include "GbCharacterController.h"
#include "GbPhysicsMaterial.h"
#include "GbPhysicsGeometry.h"

#define PX_USE_PVD
#define PVD_HOST "127.0.0.1"

using namespace physx;

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// Error callback
//////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCallback::ErrorCallback()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCallback::~ErrorCallback()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void ErrorCallback::reportError(physx::PxErrorCode::Enum e, 
    const char * message, const char * file, int line)
{
    const char* errorCode = NULL;

    switch (e){
    case PxErrorCode::eNO_ERROR:
        errorCode = "no error";
        break;
    case PxErrorCode::eINVALID_PARAMETER:
        errorCode = "invalid parameter";
        break;
    case PxErrorCode::eINVALID_OPERATION:
        errorCode = "invalid operation";
        break;
    case PxErrorCode::eOUT_OF_MEMORY:
        errorCode = "out of memory";
        break;
    case PxErrorCode::eDEBUG_INFO:
        errorCode = "info";
        break;
    case PxErrorCode::eDEBUG_WARNING:
        errorCode = "warning";
        break;
    case PxErrorCode::ePERF_WARNING:
        errorCode = "performance warning";
        break;
    case PxErrorCode::eABORT:
        errorCode = "abort";
        break;
    case PxErrorCode::eINTERNAL_ERROR:
        errorCode = "internal error";
        break;
    case PxErrorCode::eMASK_ALL:
        errorCode = "unknown error";
        break;
    }

    if (e != PxErrorCode::eNO_ERROR) {
#ifdef DEBUG_MODE
        logError("physx error");
#endif
    }

    PX_ASSERT(errorCode);
    if (errorCode)
    {
        char buffer[1024];
        sprintf(buffer, "%s (%d) : %s : %s\n", file, line, errorCode, message);
        QString errorBuffer(buffer);
        logError(errorBuffer);

        // in debug builds halt execution for abort codes
        PX_ASSERT(e != PxErrorCode::eABORT);

        // in release builds we also want to halt execution 
        // and make sure that the error message is flushed  
        while (e == PxErrorCode::eABORT)
        {
            logError(errorBuffer);
            //physx::shdfnd::Thread::sleep(1000);
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// PhysicsManager
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsManager::RemoveShape(const std::shared_ptr<PhysicsShapePrefab>& prefab)
{
    if (!Map::HasKey(s_shapes, prefab->getName()))
        throw("Error, no prefab with given name found in map: " + prefab->getName());

    // Set instances of this shape to default shape
    for (PhysicsShape* shape : prefab->m_instances) {
        shape->setPrefab(*PhysicsManager::DefaultShape(), false);
    }
    prefab->m_instances.clear();

    // Remove shape
    s_shapes.erase(prefab->getName());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsManager::RenameShape(const std::shared_ptr<PhysicsShapePrefab>& prefab, const GString & name)
{
    if (!Map::HasKey(s_shapes, prefab->getName()))
        throw("Error, no prefab with given name found in map: " + prefab->getName());

    s_shapes.erase(prefab->getName());
    prefab->setName(name);
    s_shapes[name] = prefab;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Vector3d PhysicsManager::toVector3d(const physx::PxVec3 & vec)
{
    return Vector3d(vec.x, vec.y, vec.z);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Vector3 PhysicsManager::toVector3(const physx::PxVec3 & vec)
{
    return Vector3(vec.x, vec.y, vec.z);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
physx::PxVec3 PhysicsManager::toPhysX(const Vector3d & vec3)
{
    return physx::PxVec3(vec3.x(), vec3.y(), vec3.z());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
physx::PxVec3 PhysicsManager::toPhysX(const Vector3f & vec3)
{
    return physx::PxVec3(vec3.x(), vec3.y(), vec3.z());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion PhysicsManager::toQuaternion(const physx::PxQuat & quat)
{
    return Quaternion(quat.x, quat.y, quat.z, quat.w);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsManager::clear()
{
    // Clear geometry
    s_geometry.clear();

    // Clear materials
    tsl::robin_map<GString, std::shared_ptr<PhysicsMaterial>>::iterator mit;
    for (mit = s_materials.begin(); mit != s_materials.end();) {
        if (mit->first != s_defaultMaterialKey) {
            mit = s_materials.erase(mit);
        }
        else {
            ++mit;
        }
    }

    // Clear shapes
    tsl::robin_map<GString, std::shared_ptr<PhysicsShapePrefab>>::iterator sit;
    for (sit = s_shapes.begin(); sit != s_shapes.end();) {
        if (sit->first != s_defaultShapeKey) {
            sit = s_shapes.erase(sit);
        }
        else {
            ++sit;
        }
    }

    // Clear scenes
    m_scenes.clear();

    // Clear static maps
    // TODO: Check that there isn't a memory leak from failed removal of actors
    PhysicsScene::actorMap().clear();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsManager::PhysicsManager(CoreEngine* core):
    Manager(core, "PhysicsManager")
{
    initialize();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsManager::~PhysicsManager()
{
    onDelete();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsManager::step(float dt)
{
    // Remove any deleted scenes
    // First move all safe elements to the front of the vector
    auto end = std::remove_if(m_scenes.begin(), m_scenes.end(),
        [](std::shared_ptr<PhysicsScene>& scene) {
        return !scene;
    });

    // Then erase all scenes that are null
    m_scenes.erase(end, m_scenes.end());

    for (size_t i = 0; i < m_scenes.size(); i++) {
        const std::shared_ptr<PhysicsScene>& scene = m_scenes[i];
        // Move character controllers with gravity
        // FIXME: Is grounded flag is not getting preserved
        auto& controllers = scene->cctManager()->controllers();
        for (const auto& controllerPair : controllers) {
            const std::shared_ptr<CharacterController>& controller = controllerPair.second;
            if (!controller->isGrounded()) {
                // Controller is falling
                float g = controller->getGravity().length();
                if (g > 0) {
                    // Move controller with gravity
                    const Vector3& fallVel = controller->getFallVelocity();
                    Vector3d delta = dt * fallVel.asDouble();
                    controller->move(delta);

                    // Update fall velocity
                    controller->updateFallVelocity(dt);
                }
            }
            else {
                // Controller is grounded
                if (controller->getFallVelocity().length() > 0) {
                    controller->setFallVelocity(Vector3(0.0f, 0.0f, 0.0f));
                }
            }
        }

        // Step simulation forward by dt
        scene->simulate(dt);

        // Fetch results of simulation, blocking until finished
        // I.e., even if simulation is done, queries won't be accurate until this is called
        scene->fetchResults(true);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue PhysicsManager::asJson() const
{
    QJsonObject object;

    QJsonObject geometry;
    for (const auto& geoPair : s_geometry) {
        geometry.insert(geoPair.first, geoPair.second->asJson());
    }
    object.insert("geometry", geometry);

    QJsonObject materials;
    for (const auto& mtlPair : s_materials) {
        if (mtlPair.first == s_defaultMaterialKey) continue;
        materials.insert(mtlPair.first, mtlPair.second->asJson());
    }
    object.insert("materials", materials);

    QJsonObject shapes;
    for (const auto& shapePair : s_shapes) {
        if (shapePair.first == s_defaultShapeKey) continue;
        shapes.insert(shapePair.first, shapePair.second->asJson());
    }
    object.insert("shapes", shapes);

    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsManager::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context);

    QJsonObject object = json.toObject();

    QJsonObject geometry = object.value("geometry").toObject();
    for (const QString& key : geometry.keys()) {
        std::shared_ptr<PhysicsGeometry> geo = PhysicsGeometry::createGeometry(geometry.value(key).toObject());
        s_geometry[key] = geo;
    }

    QJsonObject materials = object.value("materials").toObject();
    QStringList keys = materials.keys();
    for (const QString& key : keys) {
        if (s_materials.count(key))
            continue;
        PhysicsMaterial::create(materials.value(key));
    }

    QJsonObject shapes = object.value("shapes").toObject();
    QStringList shapeKeys = shapes.keys();
    for (const QString& key : shapeKeys) {
        if (s_shapes.count(key) ) {
            if(GString(key) != s_defaultShapeKey) throw("Error, reloading key");
            continue;
        }
        PhysicsShapePrefab::create(shapes.value(key));
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsManager::initialize()
{
    if (!m_foundation) {
        m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_allocator, m_errorCallback);
    }
    if (!m_dispatcher) {
        m_dispatcher = PxDefaultCpuDispatcherCreate(std::thread::hardware_concurrency() / 2);
    }
    if (!m_physics) {
        bool recordMemoryAllocations = true;

#ifdef PX_USE_PVD

        m_pvd = PxCreatePvd(*m_foundation);
        PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
        m_pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
#endif

        m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, 
            *m_foundation, 
            PxTolerancesScale(), 
            recordMemoryAllocations, m_pvd);

        if (!PxInitExtensions(*m_physics, m_pvd))
            throw("Error, PxInitExtensions failed");
    }
    if (!m_foundation) {
        throw("PxCreateFoundation failed!");
    }
    if (!m_physics) {
        throw("PxCreatePhysics failed!");
    }

    // Initialize default shape, material
    std::shared_ptr<PhysicsMaterial> defaultMaterial = PhysicsMaterial::create(s_defaultMaterialKey,
        0.5f, 0.5f, 0.2f);
    std::shared_ptr<PhysicsShapePrefab> defaultShape = PhysicsShapePrefab::create(s_defaultShapeKey,
        std::make_shared<BoxGeometry>(),
        defaultMaterial
    );
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsManager::onDelete()
{
    PX_RELEASE(m_dispatcher);
    PX_RELEASE(m_physics);
    PX_RELEASE(m_foundation);
#ifdef DEBUG_MODE
    logInfo("Cleaned up physics");
#endif

    // Close down extensions
    PxCloseExtensions();

    if (m_pvd) {
        // If using physx visual debugger, properly close down
        PxPvdTransport* transport = m_pvd->getTransport();
        m_pvd->release();	
        m_pvd = NULL;
        PX_RELEASE(transport);
    }

    clear();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
physx::PxFoundation* PhysicsManager::m_foundation = nullptr;

physx::PxPhysics* PhysicsManager::m_physics = nullptr;

physx::PxDefaultCpuDispatcher* PhysicsManager::m_dispatcher = nullptr;

std::vector<std::shared_ptr<PhysicsScene>> PhysicsManager::m_scenes;

tsl::robin_map<GString, std::shared_ptr<PhysicsShapePrefab>> PhysicsManager::s_shapes;

tsl::robin_map<GString, std::shared_ptr<PhysicsGeometry>> PhysicsManager::s_geometry;

tsl::robin_map<GString, std::shared_ptr<PhysicsMaterial>> PhysicsManager::s_materials;

//////////////////////////////////////////////////////////////////////////////////////////////////
GString PhysicsManager::s_defaultShapeKey = "defaultShape";
//////////////////////////////////////////////////////////////////////////////////////////////////
GString PhysicsManager::s_defaultMaterialKey = "defaultMaterial";




//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
}