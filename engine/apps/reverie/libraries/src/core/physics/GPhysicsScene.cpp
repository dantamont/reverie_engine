#include "core/physics/GPhysicsScene.h"

#include "fortress/system/memory/GPointerTypes.h"
#include "core/GCoreEngine.h"
#include "core/physics/GPhysicsManager.h"
#include "core/physics/GPhysicsActor.h"
#include "core/physics/GPhysicsQuery.h"
#include "core/physics/GCharacterController.h"
#include "core/scene/GScene.h"
#include "core/scene/GSceneObject.h"
#include "core/converters/GPhysxConverter.h"
#include "logging/GLogger.h"

namespace rev {

std::shared_ptr<PhysicsScene> PhysicsScene::create(Scene* scene)
{
    auto pScene = prot_make_shared<PhysicsScene>(scene);
    PhysicsManager::Scenes().emplace_back(pScene);
    return pScene;
}

//std::shared_ptr<PhysicsScene> PhysicsScene::create(const Vector3f& gravity)
//{
//    auto pScene = prot_make_shared<PhysicsScene>(gravity);
//    PhysicsManager::scenes().emplace_back(pScene);
//    return pScene;
//}

PhysicsScene::PhysicsScene(Scene* scene) :
    //m_scene(scene),
    m_description(PhysicsManager::Physics()->getTolerancesScale()),
    m_pxScene(nullptr)
{
    initializeDescription();

    // Create physx scene
    initialize(scene);
}

//PhysicsScene::PhysicsScene(const Vector3f& gravity):
//    m_description(PhysicsManager::Physics()->getTolerancesScale()),
//    m_pxScene(nullptr)
//{
//    // Initialize scene description
//    initializeDescription();
//    m_description.gravity = physx::PxVec3(gravity.x(), gravity.y(), gravity.z());
//
//    // Create physx scene
//    initialize(nullptr);
//}

PhysicsScene::~PhysicsScene()
{
    onDelete();
}

void PhysicsScene::createCctManager()
{
    m_cctManager = std::make_shared<CCTManager>(*this);
}

bool PhysicsScene::raycast(PhysicsRaycast & cast) const
{
    return m_pxScene->raycast(
        PhysxConverter::ToPhysX(cast.m_origin),
        PhysxConverter::ToPhysX(cast.m_direction),
        cast.m_maxDistance,
        cast.m_hits.m_hitBuffer
    );
}

std::vector<physx::PxActor*> PhysicsScene::actors()
{
    physx::PxActorTypeFlags flags = physx::PxActorTypeFlag::eRIGID_DYNAMIC;
    flags |= physx::PxActorTypeFlag::eRIGID_STATIC;
    physx::PxU32 numActors = m_pxScene->getNbActors(flags);
    physx::PxActor** actArray = new physx::PxActor*[numActors];
    //physx::PxU32 numWritten = m_pxScene->getActors(flags,
    //    actArray, 
    //    numActors);
    auto out = std::vector<physx::PxActor*>(actArray, actArray + int(numActors));
    delete[] actArray;

    return out;
}

std::vector<physx::PxActor*> PhysicsScene::dynamicActors()
{
    physx::PxU32 numActors = m_pxScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC);
    physx::PxActor** actArray = new physx::PxActor*[numActors];
    //physx::PxU32 numWritten = m_pxScene->getActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC, actArray, numActors);
    auto out = std::vector<physx::PxActor*>(actArray, actArray + int(numActors));
    delete[] actArray;

    return out;
}

std::vector<physx::PxActor*> PhysicsScene::activeActors()
{
    physx::PxU32 numActors;
    physx::PxActor** actors = m_pxScene->getActiveActors(numActors);
    return std::vector<physx::PxActor*>(actors, actors + int(numActors));
}

void PhysicsScene::fetchResults(bool block)
{
    physx::PxU32 errorState;
    bool fetched = m_pxScene->fetchResults(block, &errorState);
    if (!fetched || int(errorState) != 0) {
        Logger::Throw("Error, failed to fetch physics results");
    }

    // Iterate through scene objects with rigid bodies and set transforms
    const std::vector<std::shared_ptr<SceneObject>>& topObjects = scene()->topLevelSceneObjects();
    for (const std::shared_ptr<SceneObject>& so : topObjects) {
        so->updatePhysics();
    }
}

Vector3f PhysicsScene::getGravity() const
{
    if (m_pxScene) {
        return PhysxConverter::FromPhysX(m_pxScene->getGravity()).asFloat();
    }
    return Vector3f();
}

void PhysicsScene::setGravity(const Vector3f & gravity)
{
    physx::PxVec3 g = PhysxConverter::ToPhysX(gravity);
    m_description.gravity = g;
    if(m_pxScene) m_pxScene->setGravity(g);
}

void PhysicsScene::addActor(PhysicsActor* act)
{
    physx::PxActor* actor = act->actor();
    m_pxScene->addActor(*actor);
    act->enableGravity();
}

void PhysicsScene::removeActor(PhysicsActor * act)
{
    m_pxScene->removeActor(*act->actor());
}

void to_json(json& orJson, const PhysicsScene& korObject)
{
    json g = PhysxConverter::FromPhysX(korObject.m_pxScene->getGravity());
    orJson["g"] = g;
    if (korObject.m_cctManager) {
        orJson["cct"] = *korObject.m_cctManager;
    }
}

void from_json(const json& korJson, PhysicsScene& orObject)
{
    Vector3f gravity;
    korJson.at("g").get_to(gravity);
    orObject.setGravity(gravity);

    if (korJson.contains("cct")) {
        orObject.m_cctManager = std::make_shared<CCTManager>(orObject);
        korJson.at("cct").get_to(*orObject.m_cctManager);
    }
}

void PhysicsScene::initialize(Scene* scene)
{
    // Create scene
    m_pxScene = PhysicsManager::Physics()->createScene(m_description);
    m_pxScene->userData = scene;

    // Create character controller manager
    createCctManager();
}

void PhysicsScene::initializeDescription()
{
    m_description.cpuDispatcher = PhysicsManager::Dispatcher();
    m_description.filterShader = physx::PxDefaultSimulationFilterShader;
}

void PhysicsScene::onDelete()
{
    // Remove from physics manager
    Scene* scene = this->scene();
    PhysicsManager* manager = scene->m_engine->physicsManager();
    auto it = std::find_if(manager->Scenes().begin(), manager->Scenes().end(),
        [&](std::shared_ptr<PhysicsScene>& scene) {
        return getUuid() == scene->getUuid();
    });

    // Remove scene from physics manager if not already cleared
    if (it != manager->Scenes().end()) {
        manager->Scenes().erase(it);
    }   

    // Delete physX scene
    // Note, since the scene is stored as metadate in the physX scene, this comes last
    PX_RELEASE(m_pxScene);
}




}