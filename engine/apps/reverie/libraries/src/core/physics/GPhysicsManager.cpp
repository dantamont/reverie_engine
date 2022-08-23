#include "core/physics/GPhysicsManager.h"

#include "core/GCoreEngine.h"
#include "core/physics/GPhysicsActor.h"
#include "core/physics/GPhysicsScene.h"
#include "core/physics/GPhysicsShape.h"
#include "core/physics/GPhysicsShapePrefab.h"
#include "core/physics/GCharacterController.h"
#include "core/physics/GPhysicsMaterial.h"
#include "core/physics/GPhysicsGeometry.h"
#include "logging/GLogger.h"

#define PX_USE_PVD
#define PVD_HOST "127.0.0.1"

using namespace physx;

namespace rev {



// Error callback

ErrorCallback::ErrorCallback()
{
}

ErrorCallback::~ErrorCallback()
{
}

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
        Logger::LogError("physx error");
#endif
    }

    PX_ASSERT(errorCode);
    if (errorCode)
    {
        char buffer[1024];
        sprintf(buffer, "%s (%d) : %s : %s\n", file, line, errorCode, message);
        QString errorBuffer(buffer);
        Logger::LogError(errorBuffer.toStdString());

        // in debug builds halt execution for abort codes
        PX_ASSERT(e != PxErrorCode::eABORT);

        // in release builds we also want to halt execution 
        // and make sure that the error message is flushed  
        while (e == PxErrorCode::eABORT)
        {
            Logger::LogError(errorBuffer.toStdString());
            //physx::shdfnd::Thread::sleep(1000);
        }
    }
}




// PhysicsManager

PhysicsShapePrefab * PhysicsManager::DefaultShape()
{
    PhysicsShapePrefab* defaultShape =  Shape(s_defaultShapeKey);
    if (!defaultShape) {
        Logger::Throw("Error, no default shape found");
    }
    return defaultShape;
}

PhysicsShapePrefab * PhysicsManager::Shape(const GString& name)
{
    size_t prefabIndex = GetPrefabIndex(name);
    if (prefabIndex < 0) {
        return nullptr;
    }
    else {
        return s_shapes[prefabIndex].get();
    }
}

void PhysicsManager::RemoveShape(PhysicsShapePrefab* prefab)
{
    size_t prefabIndex = GetPrefabIndex(prefab->getUuid());
    if (prefabIndex < 0) {
        Logger::Throw("Error, no prefab with given Uuid found in map: " + prefab->getName());
    }

    // Set instances of this shape to default shape
    for (PhysicsShape* shape : prefab->m_instances) {
        shape->setPrefab(*PhysicsManager::DefaultShape(), false);
    }
    prefab->m_instances.clear();

    // Remove shape
    s_shapes.erase(s_shapes.begin() + prefabIndex);
}

std::shared_ptr<PhysicsMaterial> PhysicsManager::DefaultMaterial()
{
    const GString& defaultName = s_defaultMaterialKey;
    auto mat = Material(defaultName);
    if (!mat) {
        Logger::Throw("Error, no default material found");
    }
    return mat;
}

std::shared_ptr<PhysicsMaterial> PhysicsManager::Material(const GString& name)
{
    auto iter = std::find_if(s_materials.begin(), s_materials.end(),
        [&name](const auto& mat) {return mat->getName() == name; });
    if (iter == s_materials.end()) {
        return nullptr;
    }
    return *iter;
}


void PhysicsManager::Clear(bool clearDefault)
{
    //// Clear geometry
    //s_geometry.clear();


    // Clear shapes, except default
    if (clearDefault) {
        s_shapes.clear();
        s_materials.clear();
    }
    else {
        // Don't clear the default shape prefab
        int defaultPrefabIndex = GetPrefabIndex(s_defaultShapeKey);
        if (defaultPrefabIndex < 0) {
            Logger::Throw("Default shape prefab not found");
        }

        std::unique_ptr<PhysicsShapePrefab> defaultShapePrefab = std::move(s_shapes[defaultPrefabIndex]);
        s_shapes.clear();
        s_shapes.push_back(std::move(defaultShapePrefab));

        // Clear materials, except default
        std::shared_ptr<PhysicsMaterial> defaultMaterial = DefaultMaterial();
        s_materials.clear();
        s_materials.push_back(defaultMaterial);
    }

    // Clear scenes
    s_scenes.clear();

}

PhysicsManager::PhysicsManager(CoreEngine* core):
    Manager(core, "PhysicsManager")
{
    initialize();
}

PhysicsManager::~PhysicsManager()
{
    onDelete();
}

void PhysicsManager::step(float dt)
{
    // Remove any deleted scenes
    // First move all safe elements to the front of the vector
    auto end = std::remove_if(s_scenes.begin(), s_scenes.end(),
        [](std::shared_ptr<PhysicsScene>& scene) {
        return !scene;
    });

    // Then erase all scenes that are null
    s_scenes.erase(end, s_scenes.end());

    for (size_t i = 0; i < s_scenes.size(); i++) {
        const std::shared_ptr<PhysicsScene>& scene = s_scenes[i];
        // Move character controllers with gravity
        // FIXME: Is grounded flag is not getting preserved
        auto& controllers = scene->cctManager()->controllers();
        for (const std::unique_ptr<CharacterController>& controller : controllers) {
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

void to_json(json& orJson, const PhysicsManager& korObject)
{
    json materials;
    for (const auto& mat : korObject.s_materials) {
        if (mat->getName() != korObject.s_defaultMaterialKey) {
            materials[mat->getName().c_str()] = *mat;
        }
    }
    orJson["materials"] = materials;

    json shapes;
    for (const auto& shape : korObject.s_shapes) {
        if (shape->getName() != korObject.s_defaultShapeKey) {
            shapes[shape->getName().c_str()] = *shape;
        }
    }
    orJson["shapes"] = shapes;
}

void from_json(const json& korJson, PhysicsManager& orObject)
{
    const json& materials = korJson.at("materials");
    for (const auto& matPair : materials.items()) {
        PhysicsMaterial::Create(matPair.value());
    }

    const json& shapes = korJson.at("shapes");
    for (const auto& shapePair : shapes.items()) {
        PhysicsShapePrefab::Create(shapePair.value());
    }
}

void PhysicsManager::initialize()
{
    if (!s_foundation) {
        s_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_allocator, m_errorCallback);
    }
    if (!s_dispatcher) {
        s_dispatcher = PxDefaultCpuDispatcherCreate(std::thread::hardware_concurrency() / 2);
    }
    if (!s_physics) {
        bool recordMemoryAllocations = true;

#ifdef PX_USE_PVD

        m_pvd = PxCreatePvd(*s_foundation);
        PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
        m_pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
#endif

        s_physics = PxCreatePhysics(PX_PHYSICS_VERSION, 
            *s_foundation, 
            PxTolerancesScale(), 
            recordMemoryAllocations, m_pvd);

        if (!PxInitExtensions(*s_physics, m_pvd))
            Logger::Throw("Error, PxInitExtensions failed");
    }
    if (!s_foundation) {
        Logger::Throw("PxCreateFoundation failed!");
    }
    if (!s_physics) {
        Logger::Throw("PxCreatePhysics failed!");
    }

    // Initialize default shape, material
    std::shared_ptr<PhysicsMaterial> defaultMaterial = PhysicsMaterial::Create(s_defaultMaterialKey.c_str(),
        0.5f, 0.5f, 0.2f);
    PhysicsShapePrefab::Create(s_defaultShapeKey,
        std::move(std::make_unique<BoxGeometry>()),
        defaultMaterial
    );
}

void PhysicsManager::onDelete()
{
    // Clear everything, including default shape
    // Local objects must be cleared before physx shuts down
    Clear(true);

    PX_RELEASE(s_dispatcher);
    PX_RELEASE(s_physics);
    PX_RELEASE(s_foundation);
#ifdef DEBUG_MODE
    Logger::LogInfo("Cleaned up physics");
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
}

int PhysicsManager::GetPrefabIndex(const GString & name)
{
    auto iter = std::find_if(s_shapes.begin(), s_shapes.end(),
        [&name](const std::unique_ptr<PhysicsShapePrefab>& prefab) {
        return prefab->getName() == name; 
    });

    if (iter == s_shapes.end()) {
        return -1;
    }
    else {
        return iter - s_shapes.begin();
    }
}

int PhysicsManager::GetPrefabIndex(const Uuid & uuid)
{
    auto iter = std::find_if(s_shapes.begin(), s_shapes.end(),
        [&uuid](const std::unique_ptr<PhysicsShapePrefab>& prefab) {
        return prefab->getUuid() == uuid;
    });

    if (iter == s_shapes.end()) {
        return -1;
    }
    else {
        return iter - s_shapes.begin();
    }
}

physx::PxFoundation* PhysicsManager::s_foundation = nullptr;

physx::PxPhysics* PhysicsManager::s_physics = nullptr;

physx::PxDefaultCpuDispatcher* PhysicsManager::s_dispatcher = nullptr;

std::vector<std::shared_ptr<PhysicsScene>> PhysicsManager::s_scenes;

std::vector<std::unique_ptr<PhysicsShapePrefab>> PhysicsManager::s_shapes;

//tsl::robin_map<GString, std::shared_ptr<PhysicsGeometry>> PhysicsManager::s_geometry;

std::vector<std::shared_ptr<PhysicsMaterial>> PhysicsManager::s_materials;


GString PhysicsManager::s_defaultShapeKey = "defaultShape";

GString PhysicsManager::s_defaultMaterialKey = "defaultMaterial";







}