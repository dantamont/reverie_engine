#include "GbPhysicsScene.h"

#include "../utils/GbMemoryManager.h"
#include "../GbCoreEngine.h"
#include "GbPhysicsManager.h"
#include "GbPhysicsActor.h"
#include "GbPhysicsQuery.h"
#include "GbCharacterController.h"
#include "../scene/GbScene.h"
#include "../scene/GbSceneObject.h"

namespace Gb {
//////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<PhysicsScene> PhysicsScene::create(std::shared_ptr<Scene> scene)
{
    auto pScene = prot_make_shared<PhysicsScene>(scene);
    PhysicsManager::scenes().emplace_back(pScene);
    return pScene;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<PhysicsScene> PhysicsScene::create(const Vector3f& gravity)
{
    auto pScene = prot_make_shared<PhysicsScene>(gravity);
    PhysicsManager::scenes().emplace_back(pScene);
    return pScene;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SceneObject> PhysicsScene::bodyObject(physx::PxActor* actor)
{
    return SceneObject::get(ActorMap[actor]);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsScene::PhysicsScene(std::shared_ptr<Scene> scene) :
    m_scene(scene),
    m_description(PhysicsManager::physics()->getTolerancesScale()),
    m_pxScene(nullptr)
{
    initializeDescription();

    // Create physx scene
    initialize();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsScene::PhysicsScene(const Vector3f& gravity):
    m_description(PhysicsManager::physics()->getTolerancesScale()),
    m_pxScene(nullptr)
{
    // Initialize scene description
    initializeDescription();
    m_description.gravity = physx::PxVec3(gravity.x(), gravity.y(), gravity.z());

    // Create physx scene
    initialize();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsScene::~PhysicsScene()
{
    onDelete();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Scene> PhysicsScene::scene() const
{
    if (std::shared_ptr<Scene> sc = m_scene.lock()) {
        return sc;
    }
    else {
        return nullptr;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsScene::createCctManager()
{
    m_cctManager = std::make_shared<CCTManager>(*this);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
bool PhysicsScene::raycast(Raycast & cast) const
{
    return m_pxScene->raycast(
        PhysicsManager::toPhysX(cast.m_origin),
        PhysicsManager::toPhysX(cast.m_direction),
        cast.m_maxDistance,
        cast.m_hits.m_hitBuffer
    );
}
//////////////////////////////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<physx::PxActor*> PhysicsScene::dynamicActors()
{
    physx::PxU32 numActors = m_pxScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC);
    physx::PxActor** actArray = new physx::PxActor*[numActors];
    //physx::PxU32 numWritten = m_pxScene->getActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC, actArray, numActors);
    auto out = std::vector<physx::PxActor*>(actArray, actArray + int(numActors));
    delete[] actArray;

    return out;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<physx::PxActor*> PhysicsScene::activeActors()
{
    physx::PxU32 numActors;
    physx::PxActor** actors = m_pxScene->getActiveActors(numActors);
    return std::vector<physx::PxActor*>(actors, actors + int(numActors));
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsScene::fetchResults(bool block)
{
    physx::PxU32 errorState;
    bool fetched = m_pxScene->fetchResults(block, &errorState);
    if (!fetched || int(errorState) != 0) {
        throw("Error, failed to fetch physics results");
    }

    // Iterate through scene objects with rigid bodies and set transforms
    const std::shared_ptr<Scene>& sc = scene();
    const std::multiset<std::shared_ptr<SceneObject>, CompareByRenderLayer>& topObjects
        = sc->topLevelSceneObjects();
    for (const std::shared_ptr<SceneObject>& so : topObjects) {
        so->updatePhysics();
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Vector3f PhysicsScene::getGravity() const
{
    if (m_pxScene) {
        return PhysicsManager::toVec3(m_pxScene->getGravity()).asFloat();
    }
    return Vector3f();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsScene::setGravity(const Vector3f & gravity)
{
    physx::PxVec3 g = PhysicsManager::toPhysX(gravity);
    m_description.gravity = g;
    if(m_pxScene) m_pxScene->setGravity(g);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsScene::addActor(PhysicsActor* act, const std::shared_ptr<SceneObject>& so)
{
    physx::PxActor* actor = act->actor();
    m_pxScene->addActor(*actor);
    act->enableGravity();
    if (!Map::HasKey(ActorMap, actor))
        ActorMap[actor] = so->getUuid();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsScene::removeActor(PhysicsActor * act)
{
    ActorMap.erase(act->actor());
    m_pxScene->removeActor(*act->actor());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue PhysicsScene::asJson() const
{
    QJsonObject object;
    QJsonValue g = PhysicsManager::toVec3(m_pxScene->getGravity()).asJson();
    object.insert("g", g);
    if (m_cctManager) {
        object.insert("cct", m_cctManager->asJson());
    }
    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsScene::loadFromJson(const QJsonValue & json)
{
    QJsonObject object = json.toObject();
    auto gravity = Vector3f(object.value("g"));
    setGravity(gravity);

    if (object.contains("cct")) {
        m_cctManager = std::make_shared<CCTManager>(*this);
        m_cctManager->loadFromJson(object);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsScene::initialize()
{
    // Create scene
    m_pxScene = PhysicsManager::physics()->createScene(m_description);

    // Create character controller manager
    createCctManager();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsScene::initializeDescription()
{
    m_description.cpuDispatcher = PhysicsManager::dispatcher();
    m_description.filterShader = physx::PxDefaultSimulationFilterShader;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsScene::onDelete()
{
    // Delete physX scene
    PX_RELEASE(m_pxScene);

    // Remove from physics manager
    PhysicsManager* manager = scene()->m_engine->physicsManager();
    auto it = std::find_if(manager->scenes().begin(), manager->scenes().end(),
        [&](std::shared_ptr<PhysicsScene>& scene) {
        return getUuid() == scene->getUuid();
    });

    // Remove scene from physics manager if not already cleared
    if (it != manager->scenes().end()) {
        manager->scenes().erase(it);
    }   
}

std::unordered_map<physx::PxActor*, Uuid> PhysicsScene::ActorMap;

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
}