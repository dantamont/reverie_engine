#include "GCharacterController.h"

#include "../containers/GFlags.h"
#include "../GCoreEngine.h"
#include "../scene/GScenario.h"
#include "../scene/GScene.h"
#include "../scene/GSceneObject.h"
#include "../loop/GSimLoop.h"

#include "GPhysicsManager.h"
#include "GPhysicsShape.h"
#include "GPhysicsGeometry.h"
#include "GPhysicsMaterial.h"
#include "GPhysicsManager.h"
#include "GPhysicsScene.h"
#include "../geometry/GTransform.h"
#include "../components/GTransformComponent.h"
#include "../utils/GMemoryManager.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////////////////////
// Controller Filters
//////////////////////////////////////////////////////////////////////////////////////////////////
ControllerFilters::ControllerFilters()
{
    mFilterFlags = (physx::PxQueryFlag::Enum)QueryFlag::kStatic | (physx::PxQueryFlag::Enum)QueryFlag::kDynamic;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Controller Description
//////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ControllerDescription> ControllerDescription::create(const QJsonValue & json)
{
    std::shared_ptr<ControllerDescription> desc;
    ControllerType type = ControllerType(json["controllerType"].toInt());
    switch (type) {
    case kBox:
        desc = std::make_shared<BoxControllerDescription>();
        break;
    case kCapsule:
        desc = std::make_shared<CapsuleControllerDescription>();
        break;
    default:
        throw("Error, invalid controller type");
        break;
    }
    desc->loadFromJson(json);
    return desc;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
ControllerDescription::ControllerDescription(ControllerType type):
    m_type(type)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
ControllerDescription::~ControllerDescription()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue ControllerDescription::asJson(const SerializationContext& context) const
{
    QJsonObject object;
    object.insert("initPos", m_initialPosition.asJson());
    object.insert("up", m_upDirection.asJson());
    object.insert("slopeLim", m_slopeLimit);
    object.insert("invWallHeight", m_invisibleWallHeight);
    object.insert("maxJumpHeight", m_maxJumpHeight);
    object.insert("contactOffset", m_contactOffset);
    object.insert("stepOffset", m_stepOffset);
    object.insert("density", m_density);
    object.insert("scaleCoeff", m_scaleCoeff);
    object.insert("volGrowth", m_volumeGrowth);
    if (m_reportCallback) 
        throw("JSON serialization not implemented");
    if (m_behaviorCallback) 
        throw("JSON serialization not implemented");
    object.insert("registerDeletion", m_registerDeletionListener);
    if (m_material)
        object.insert("mat", m_material->getName().c_str());
    object.insert("unwalkableMode", m_unwalkableMode);
    if (m_userData)
        throw("JSON serialization not implemented");
    object.insert("controllerType", (int)m_type);

    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void ControllerDescription::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    QJsonObject object = json.toObject();

    m_initialPosition = Vector3(object["initPos"]);
    m_upDirection = Vector3(object["up"]);
    m_slopeLimit = (float)object["slopeLim"].toDouble();
    m_invisibleWallHeight = (float)object["invWallHeight"].toDouble();
    m_maxJumpHeight = (float)object["maxJumpHeight"].toDouble();
    m_contactOffset = (float)object["contactOffset"].toDouble();
    m_stepOffset = (float)object["stepOffset"].toDouble();
    m_density = (float)object["density"].toDouble();
    m_scaleCoeff = (float)object["scaleCoeff"].toDouble();
    m_volumeGrowth = (float)object["volGrowth"].toDouble();

    if (object.contains("reportCallback")) {
        throw("Error, unimplemented");
    }

    if (object.contains("behaviorCallback")) {
        throw("Error, unimplemented");
    }

    m_registerDeletionListener = object["registerDeletion"].toBool();

    if (object.contains("mat"))
        m_material = PhysicsManager::Material(object["mat"].toString());

    m_unwalkableMode = object["unwalkableMode"].toInt();
    if (object.contains("userData"))
        throw("Error, unimplemented");
    m_type = (ControllerType)object["controllerType"].toInt();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void ControllerDescription::loadIntoPhysx(std::shared_ptr<physx::PxControllerDesc> pxDesc) const
{
    pxDesc->position = physx::PxExtendedVec3(m_initialPosition.x(), m_initialPosition.y(), m_initialPosition.z());
    pxDesc->upDirection = PhysicsManager::toPhysX(m_upDirection);
    pxDesc->slopeLimit = m_slopeLimit;
    pxDesc->invisibleWallHeight = m_invisibleWallHeight;
    pxDesc->maxJumpHeight = m_maxJumpHeight;
    pxDesc->contactOffset = m_contactOffset;
    pxDesc->stepOffset = m_stepOffset;
    pxDesc->density = m_density;
    pxDesc->scaleCoeff = m_scaleCoeff;
    pxDesc->volumeGrowth = m_volumeGrowth;
    pxDesc->reportCallback = m_reportCallback;
    pxDesc->behaviorCallback = m_behaviorCallback;
    pxDesc->registerDeletionListener = m_registerDeletionListener;
    if (m_material)
        pxDesc->material = m_material->getMaterial();
    else
        pxDesc->material = nullptr;
    pxDesc->nonWalkableMode = physx::PxControllerNonWalkableMode::Enum(m_unwalkableMode);
    pxDesc->userData = m_userData;
}




//////////////////////////////////////////////////////////////////////////////////////////////////
// Box Controller Description
//////////////////////////////////////////////////////////////////////////////////////////////////
BoxControllerDescription::BoxControllerDescription():
    ControllerDescription(kBox)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
BoxControllerDescription::~BoxControllerDescription()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<physx::PxControllerDesc> BoxControllerDescription::toPhysX() const
{
    auto desc = std::make_shared<physx::PxBoxControllerDesc>();
    loadIntoPhysx(desc);
    desc->halfHeight = m_halfHeight;
    desc->halfSideExtent = m_halfSideExtent;
    desc->halfForwardExtent = m_halfForwardExtent;
    return desc;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue BoxControllerDescription::asJson(const SerializationContext& context) const
{
    QJsonObject object = ControllerDescription::asJson(context).toObject();
    object.insert("hh", m_halfHeight);
    object.insert("hs", m_halfSideExtent);
    object.insert("hf", m_halfForwardExtent);
    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void BoxControllerDescription::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    ControllerDescription::loadFromJson(json);
    QJsonObject object = json.toObject();
    m_halfHeight = (float)object["hh"].toDouble();
    m_halfSideExtent = (float)object["hs"].toDouble();
    m_halfForwardExtent = (float)object["hf"].toDouble();
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// Capsule Controller Description
//////////////////////////////////////////////////////////////////////////////////////////////////
CapsuleControllerDescription::CapsuleControllerDescription() :
    ControllerDescription(kCapsule)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
CapsuleControllerDescription::~CapsuleControllerDescription()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<physx::PxControllerDesc> CapsuleControllerDescription::toPhysX() const
{
    auto desc = std::make_shared<physx::PxCapsuleControllerDesc>();
    loadIntoPhysx(desc);
    desc->radius = m_radius;
    desc->height = m_height;
    desc->climbingMode = physx::PxCapsuleClimbingMode::Enum(m_climbingMode);
    return desc;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue CapsuleControllerDescription::asJson(const SerializationContext& context) const
{
    QJsonObject object = ControllerDescription::asJson(context).toObject();
    object.insert("r", m_radius);
    object.insert("h", m_height);
    object.insert("climbingMode", (int)m_climbingMode);
    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CapsuleControllerDescription::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    ControllerDescription::loadFromJson(json);
    QJsonObject object = json.toObject();
    m_radius = (float)object["r"].toDouble();
    m_height = (float)object["h"].toDouble();
    m_climbingMode = (size_t)object["climbingMode"].toInt();
}



//////////////////////////////////////////////////////////////////////////////////////////////////
// Character Controller
//////////////////////////////////////////////////////////////////////////////////////////////////
CharacterController::CharacterController(std::shared_ptr<ControllerDescription> desc,
    const std::shared_ptr<SceneObject>& sceneObject):
    m_sceneObject(sceneObject),
    m_description(desc),
    m_controller(nullptr)
{
    initialize();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
CoreEngine * CharacterController::getEngine() const
{
    return sceneObject()->scene()->engine();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
const std::shared_ptr<CCTManager>& CharacterController::getManager() const
{
    return sceneObject()->scene()->physics()->cctManager();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CharacterController::initialize()
{
    if (m_controller) {
        PX_RELEASE(m_controller);
    }

    std::shared_ptr<SceneObject> so = sceneObject();
    auto pxDesc = m_description->toPhysX();
    if (pxDesc->isValid()) {
        // Create physx character controller and set user data to be a pointer to this encapsulating controller
        m_controller = 
            sceneObject()->scene()->physics()->cctManager()->m_controllerManager->createController(
            *pxDesc
        );
        m_controller->setUserData(this);
    }
    else {
#ifdef DEBUG_MODE
        throw("Description for character controller is invalid");
#else
        logError("Description for character controller is invalid");
#endif
    }

    // Set initial position of the controller
    Vector3 initPos = so->transform().getPosition() + m_heightOffset * m_description->m_upDirection.asReal();
    setPosition(initPos);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
CharacterController::~CharacterController()
{
    PX_RELEASE(m_controller);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
ControllerCollisionFlags CharacterController::move(const Vector3 & displacement)
{
    return move(displacement.asDouble(), ControllerFilters());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
ControllerCollisionFlags CharacterController::move(const Vector3d & displacement)
{
    return move(displacement, ControllerFilters());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
ControllerCollisionFlags CharacterController::move(const Vector3d & disp, 
    const ControllerFilters & filters,
    const physx::PxObstacleContext * obstacles)
{
    float newTime = getEngine()->simulationLoop()->elapsedTime() / 1000.0f;
    //float elapsedTime = newTime - m_lastTimeInSecs;
    float elapsedTime = getEngine()->simulationLoop()->fixedStep() / 1000.0f;
    m_lastTimeInSecs = newTime;
    physx::PxVec3 vec = PhysicsManager::toPhysX(disp);
    physx::PxControllerCollisionFlags out =
        m_controller->move(vec,
            m_minDistance,
            elapsedTime,
            filters,
            obstacles);
    auto outFlags = Flags<CollisionType>::toQFlags<uint16_t>(uint16_t(out));

    // Set the grounded flag
    m_isGrounded = outFlags.testFlag(kCollisionDown);

    // Move scene object with the controller
    sceneObject()->transform().setPosition(
        getPosition() - m_description->m_upDirection * m_heightOffset
    );

    return outFlags;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CharacterController::updateFallVelocity(float dt)
{
    m_fallVelocity += dt * m_gravity;

    if (m_fallVelocity.length() > m_terminalVelocity) {
        m_fallVelocity.normalize();
        m_fallVelocity *= m_terminalVelocity;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue CharacterController::asJson(const SerializationContext& context) const
{
    QJsonObject object = PhysicsBase::asJson(context).toObject();
    object.insert("description", m_description->asJson());
    object.insert("sceneObject", sceneObject()->getName().c_str());
    object.insert("heightOffset", m_heightOffset);
    object.insert("gravity", m_gravity.asJson());
    object.insert("fallVelocity", m_fallVelocity.asJson());
    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CharacterController::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    PhysicsBase::loadFromJson(json);
    QJsonObject object = json.toObject();
    // Description and scene object are already constructed from JSON from all code paths leading to this function
    //m_description->loadFromJson(object["description"]);
    //m_sceneObject = SceneObject::getByName(object["sceneObject"].toString());
    m_heightOffset = (float)object["heightOffset"].toDouble();
    m_gravity = Vector3(object["gravity"]);
    m_fallVelocity = Vector3(object["fallVelocity"]);

    // Set initial position of the controller
    Vector3 initPos = m_description->m_initialPosition + m_heightOffset * m_description->m_upDirection;
    setPosition(initPos);
    // initialize();
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// Box Controller
//////////////////////////////////////////////////////////////////////////////////////////////////
BoxController::BoxController(std::shared_ptr<BoxControllerDescription> desc, 
    const std::shared_ptr<SceneObject>& sceneObject):
    CharacterController(desc, sceneObject)
{
}



//////////////////////////////////////////////////////////////////////////////////////////////////
// Capsule Controller
//////////////////////////////////////////////////////////////////////////////////////////////////
CapsuleController::CapsuleController(std::shared_ptr<CapsuleControllerDescription> desc,
    const std::shared_ptr<SceneObject>& sceneObject) :
    CharacterController(desc, sceneObject)
{
}



//////////////////////////////////////////////////////////////////////////////////////////////////
// CCTManager
//////////////////////////////////////////////////////////////////////////////////////////////////
CCTManager::CCTManager(PhysicsScene& scene):
    m_scene(scene.scene())
{
    m_controllerManager = PxCreateControllerManager(*scene.pxScene());

    // Use overlap recovery
    //toggleOverlapRecovery(true);
    //togglePreciseSweeps(false);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
CCTManager::~CCTManager()
{
    // Not necessary apparently (causes crash)
    //PX_RELEASE(m_controllerManager);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
CharacterController* CCTManager::CreateController(const QJsonValue & json)
{
    if (!json.isObject()) throw("Error, JSON must be an object");
    QJsonObject object = json.toObject();
    if (!object.contains("description") || !object.contains("sceneObject")) {
        throw("Error, object does not contain sufficient information to construct CCT");
    }
    auto description = ControllerDescription::create(object["description"]);
    auto sceneObject = SceneObject::getByName(object["sceneObject"].toString());

    CharacterController* controller = CreateController(description, sceneObject);
    controller->loadFromJson(json);
    return controller;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
CharacterController* CCTManager::CreateController(const std::shared_ptr<ControllerDescription>& desc,
    const std::shared_ptr<SceneObject>& so)
{
    std::unique_ptr<CharacterController> controller;
    
    switch (desc->m_type) {
    case ControllerDescription::kBox:
    {
        auto boxDescription = std::static_pointer_cast<BoxControllerDescription>(desc);
        controller = prot_make_unique<BoxController>(boxDescription, so);
        break;
    }
    case ControllerDescription::kCapsule:
    {
        auto capsuleDescription = std::static_pointer_cast<CapsuleControllerDescription>(desc);
        controller = prot_make_unique<CapsuleController>(capsuleDescription, so);
        break;
    }
    default:
        throw("Error, controller description type is invalid");
    }

    // Add controller to map
    m_controllers.push_back(std::move(controller));

    return m_controllers.back().get();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CCTManager::removeController(const CharacterController& controller)
{
    auto iter = std::find_if(m_controllers.begin(), m_controllers.end(),
        [&controller](std::unique_ptr<CharacterController>& c) {return controller.getUuid() == c->getUuid(); });

    if (iter == m_controllers.end()) {
        throw("Error, controller not found");
    }
    else {
        m_controllers.erase(iter);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue CCTManager::asJson(const SerializationContext& context) const
{
    QJsonObject object = PhysicsBase::asJson(context).toObject();
    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CCTManager::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)
    PhysicsBase::loadFromJson(json);
    //QJsonObject object = json.toObject();
}













//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
} // rev