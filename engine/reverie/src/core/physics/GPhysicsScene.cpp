#include "GPhysicsScene.h"

#include "../utils/GMemoryManager.h"
#include "../GCoreEngine.h"
#include "GPhysicsManager.h"
#include "GPhysicsActor.h"
#include "GPhysicsQuery.h"
#include "GCharacterController.h"
#include "../scene/GScene.h"
#include "../scene/GSceneObject.h"

namespace rev {
//////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<PhysicsScene> PhysicsScene::create(Scene* scene)
{
    auto pScene = prot_make_shared<PhysicsScene>(scene);
    PhysicsManager::Scenes().emplace_back(pScene);
    return pScene;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//std::shared_ptr<PhysicsScene> PhysicsScene::create(const Vector3f& gravity)
//{
//    auto pScene = prot_make_shared<PhysicsScene>(gravity);
//    PhysicsManager::scenes().emplace_back(pScene);
//    return pScene;
//}
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsScene::PhysicsScene(Scene* scene) :
    //m_scene(scene),
    m_description(PhysicsManager::Physics()->getTolerancesScale()),
    m_pxScene(nullptr)
{
    initializeDescription();

    // Create physx scene
    initialize(scene);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsScene::~PhysicsScene()
{
    onDelete();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsScene::createCctManager()
{
    m_cctManager = std::make_shared<CCTManager>(*this);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
bool PhysicsScene::raycast(PhysicsRaycast & cast) const
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
    const std::vector<std::shared_ptr<SceneObject>>& topObjects = scene()->topLevelSceneObjects();
    for (const std::shared_ptr<SceneObject>& so : topObjects) {
        so->updatePhysics();
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Vector3f PhysicsScene::getGravity() const
{
    if (m_pxScene) {
        return PhysicsManager::toVector3d(m_pxScene->getGravity()).asFloat();
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
void PhysicsScene::addActor(PhysicsActor* act)
{
    physx::PxActor* actor = act->actor();
    m_pxScene->addActor(*actor);
    act->enableGravity();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsScene::removeActor(PhysicsActor * act)
{
    m_pxScene->removeActor(*act->actor());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue PhysicsScene::asJson(const SerializationContext& context) const
{
    QJsonObject object;
    QJsonValue g = PhysicsManager::toVector3d(m_pxScene->getGravity()).asJson();
    object.insert("g", g);
    if (m_cctManager) {
        object.insert("cct", m_cctManager->asJson());
    }
    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsScene::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context);

    QJsonObject object = json.toObject();
    auto gravity = Vector3f(object.value("g"));
    setGravity(gravity);

    if (object.contains("cct")) {
        m_cctManager = std::make_shared<CCTManager>(*this);
        m_cctManager->loadFromJson(object);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsScene::initialize(Scene* scene)
{
    // Create scene
    m_pxScene = PhysicsManager::Physics()->createScene(m_description);
    m_pxScene->userData = scene;

    // Create character controller manager
    createCctManager();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsScene::initializeDescription()
{
    m_description.cpuDispatcher = PhysicsManager::Dispatcher();
    m_description.filterShader = physx::PxDefaultSimulationFilterShader;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsScene::onDelete()
{
    // Delete physX scene
    PX_RELEASE(m_pxScene);

    // Remove from physics manager
    PhysicsManager* manager = scene()->m_engine->physicsManager();
    auto it = std::find_if(manager->Scenes().begin(), manager->Scenes().end(),
        [&](std::shared_ptr<PhysicsScene>& scene) {
        return getUuid() == scene->getUuid();
    });

    // Remove scene from physics manager if not already cleared
    if (it != manager->Scenes().end()) {
        manager->Scenes().erase(it);
    }   
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
}