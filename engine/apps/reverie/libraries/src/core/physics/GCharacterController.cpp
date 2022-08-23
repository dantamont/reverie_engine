#include "core/physics/GCharacterController.h"

#include "fortress/layer/framework/GFlags.h"
#include "core/GCoreEngine.h"
#include "core/scene/GScenario.h"
#include "core/scene/GScene.h"
#include "core/scene/GSceneObject.h"
#include "core/loop/GSimLoop.h"

#include "core/physics/GPhysicsManager.h"
#include "core/physics/GPhysicsShape.h"
#include "core/physics/GPhysicsGeometry.h"
#include "core/physics/GPhysicsMaterial.h"
#include "core/physics/GPhysicsManager.h"
#include "core/physics/GPhysicsScene.h"
#include "fortress/containers/math/GTransform.h"
#include "core/converters/GPhysxConverter.h"
#include "core/components/GTransformComponent.h"
#include "fortress/system/memory/GPointerTypes.h"
#include "logging/GLogger.h"

namespace rev {


// Controller Filters

ControllerFilters::ControllerFilters()
{
    mFilterFlags = (physx::PxQueryFlag::Enum)QueryFlag::kStatic | (physx::PxQueryFlag::Enum)QueryFlag::kDynamic;
}


// Controller Description

std::shared_ptr<ControllerDescription> ControllerDescription::create(const nlohmann::json& json)
{
    std::shared_ptr<ControllerDescription> desc;
    ControllerType type = ControllerType(json.at("controllerType").get<Int32_t>());
    switch (type) {
    case kBox:
        desc = std::make_shared<BoxControllerDescription>();
        FromJson<BoxControllerDescription>(json, *desc);
        break;
    case kCapsule:
        desc = std::make_shared<CapsuleControllerDescription>();
        FromJson<CapsuleControllerDescription>(json, *desc);
        break;
    default:
        Logger::Throw("Error, invalid controller type");
        break;
    }
    return desc;
}

ControllerDescription::ControllerDescription(ControllerType type):
    m_type(type)
{
}

ControllerDescription::~ControllerDescription()
{
}

json ControllerDescription::asJson() const
{
    json myJson;
    switch (m_type) {
    case kBox:
        ToJson<BoxControllerDescription>(myJson, *this);
        break;
    case kCapsule:
        ToJson<CapsuleControllerDescription>(myJson, *this);
        break;
    default:
        Logger::Throw("Error, invalid controller type");
        break;
    }
    return myJson;
}

std::shared_ptr<physx::PxControllerDesc> ControllerDescription::toPhysX() const
{
    return std::shared_ptr<physx::PxControllerDesc>();
}

void to_json(json& orJson, const ControllerDescription& korObject)
{
    orJson["initPos"] = korObject.m_initialPosition;
    orJson["up"] = korObject.m_upDirection;
    orJson["slopeLim"] = korObject.m_slopeLimit;
    orJson["invWallHeight"] = korObject.m_invisibleWallHeight;
    orJson["maxJumpHeight"] = korObject.m_maxJumpHeight;
    orJson["contactOffset"] = korObject.m_contactOffset;
    orJson["stepOffset"] = korObject.m_stepOffset;
    orJson["density"] = korObject.m_density;
    orJson["scaleCoeff"] = korObject.m_scaleCoeff;
    orJson["volGrowth"] = korObject.m_volumeGrowth;
    if (korObject.m_reportCallback)
        Logger::Throw("JSON serialization not implemented");
    if (korObject.m_behaviorCallback)
        Logger::Throw("JSON serialization not implemented");
    orJson["registerDeletion"] = korObject.m_registerDeletionListener;
    if (korObject.m_material)
        orJson["mat"] = korObject.m_material->getName().c_str();
    orJson["unwalkableMode"] = korObject.m_unwalkableMode;
    if (korObject.m_userData)
        Logger::Throw("JSON serialization not implemented");
    orJson["controllerType"] = (int)korObject.m_type;
}

void from_json(const json& korJson, ControllerDescription& orObject)
{
    korJson["initPos"].get_to(orObject.m_initialPosition);
    korJson["up"].get_to(orObject.m_upDirection);
    orObject.m_slopeLimit = (float)korJson.at("slopeLim").get<Float64_t>();
    orObject.m_invisibleWallHeight = (float)korJson.at("invWallHeight").get<Float64_t>();
    orObject.m_maxJumpHeight = (float)korJson.at("maxJumpHeight").get<Float64_t>();
    orObject.m_contactOffset = (float)korJson.at("contactOffset").get<Float64_t>();
    orObject.m_stepOffset = (float)korJson.at("stepOffset").get<Float64_t>();
    orObject.m_density = (float)korJson.at("density").get<Float64_t>();
    orObject.m_scaleCoeff = (float)korJson.at("scaleCoeff").get<Float64_t>();
    orObject.m_volumeGrowth = (float)korJson.at("volGrowth").get<Float64_t>();

    if (korJson.contains("reportCallback")) {
        Logger::Throw("Error, unimplemented");
    }

    if (korJson.contains("behaviorCallback")) {
        Logger::Throw("Error, unimplemented");
    }

    orObject.m_registerDeletionListener = korJson.at("registerDeletion").get<bool>();

    if (korJson.contains("mat"))
        orObject.m_material = PhysicsManager::Material(korJson["mat"].get_ref<const std::string&>().c_str());

    orObject.m_unwalkableMode = korJson.at("unwalkableMode").get<Int32_t>();
    if (korJson.contains("userData"))
        Logger::Throw("Error, unimplemented");
    orObject.m_type = (ControllerDescription::ControllerType)korJson.at("controllerType").get<Int32_t>();
}

void ControllerDescription::loadIntoPhysx(std::shared_ptr<physx::PxControllerDesc> pxDesc) const
{
    pxDesc->position = physx::PxExtendedVec3(m_initialPosition.x(), m_initialPosition.y(), m_initialPosition.z());
    pxDesc->upDirection = PhysxConverter::ToPhysX(m_upDirection);
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





// Box Controller Description

BoxControllerDescription::BoxControllerDescription():
    ControllerDescription(kBox)
{
}

BoxControllerDescription::~BoxControllerDescription()
{
}

std::shared_ptr<physx::PxControllerDesc> BoxControllerDescription::toPhysX() const
{
    auto desc = std::make_shared<physx::PxBoxControllerDesc>();
    loadIntoPhysx(desc);
    desc->halfHeight = m_halfHeight;
    desc->halfSideExtent = m_halfSideExtent;
    desc->halfForwardExtent = m_halfForwardExtent;
    return desc;
}

void to_json(json& orJson, const BoxControllerDescription& korObject)
{
    ToJson<ControllerDescription>(orJson, korObject);
    orJson["hh"] = korObject.m_halfHeight;
    orJson["hs"] = korObject.m_halfSideExtent;
    orJson["hf"] = korObject.m_halfForwardExtent;
}

void from_json(const json& korJson, BoxControllerDescription& orObject)
{
    FromJson<ControllerDescription>(korJson, orObject);
    orObject.m_halfHeight = (float)korJson.at("hh").get<Float64_t>();
    orObject.m_halfSideExtent = (float)korJson.at("hs").get<Float64_t>();
    orObject.m_halfForwardExtent = (float)korJson.at("hf").get<Float64_t>();
}



// Capsule Controller Description

CapsuleControllerDescription::CapsuleControllerDescription() :
    ControllerDescription(kCapsule)
{
}

CapsuleControllerDescription::~CapsuleControllerDescription()
{
}

std::shared_ptr<physx::PxControllerDesc> CapsuleControllerDescription::toPhysX() const
{
    auto desc = std::make_shared<physx::PxCapsuleControllerDesc>();
    loadIntoPhysx(desc);
    desc->radius = m_radius;
    desc->height = m_height;
    desc->climbingMode = physx::PxCapsuleClimbingMode::Enum(m_climbingMode);
    return desc;
}

void to_json(json& orJson, const CapsuleControllerDescription& korObject)
{
    ToJson<ControllerDescription>(orJson, korObject);
    orJson["r"] = korObject.m_radius;
    orJson["h"] = korObject.m_height;
    orJson["climbingMode"] = (int)korObject.m_climbingMode;
}

void from_json(const json& korJson, CapsuleControllerDescription& orObject)
{
    FromJson<ControllerDescription>(korJson, orObject);
    
    orObject.m_radius = (float)korJson.at("r").get<Float64_t>();
    orObject.m_height = (float)korJson.at("h").get<Float64_t>();
    orObject.m_climbingMode = (size_t)korJson.at("climbingMode").get<Int32_t>();
}




// Character Controller

CharacterController::CharacterController(std::shared_ptr<ControllerDescription> desc,
    const std::shared_ptr<SceneObject>& sceneObject):
    m_sceneObject(sceneObject),
    m_description(desc),
    m_controller(nullptr)
{
    initialize();
}

CoreEngine * CharacterController::getEngine() const
{
    return sceneObject()->scene()->engine();
}

const std::shared_ptr<CCTManager>& CharacterController::getManager() const
{
    return sceneObject()->scene()->physics()->cctManager();
}

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
        Logger::Throw("Description for character controller is invalid");
#else
        Logger::LogError("Description for character controller is invalid");
#endif
    }

    // Set initial position of the controller
    Vector3 initPos = so->transform().getPosition() + m_heightOffset * m_description->m_upDirection.asReal();
    setPosition(initPos);
}

CharacterController::~CharacterController()
{
    PX_RELEASE(m_controller);
}

CharacterController::ControllerCollisionFlags CharacterController::move(const Vector3 & displacement)
{
    return move(displacement.asDouble(), ControllerFilters());
}

CharacterController::ControllerCollisionFlags CharacterController::move(const Vector3d & displacement)
{
    return move(displacement, ControllerFilters());
}

CharacterController::ControllerCollisionFlags CharacterController::move(const Vector3d & disp,
    const ControllerFilters & filters,
    const physx::PxObstacleContext * obstacles)
{
    float newTime = getEngine()->simulationLoop()->elapsedTime();
    //float elapsedTime = newTime - m_lastTimeInSecs;
    float elapsedTime = getEngine()->simulationLoop()->fixedStep();
    m_lastTimeInSecs = newTime;
    physx::PxVec3 vec = PhysxConverter::ToPhysX(disp);
    physx::PxControllerCollisionFlags out =
        m_controller->move(vec,
            m_minDistance,
            elapsedTime,
            filters,
            obstacles);
    ControllerCollisionFlags outFlags = (Uint32_t(out));

    // Set the grounded flag
    m_isGrounded = outFlags.testFlag(kCollisionDown);

    // Move scene object with the controller
    sceneObject()->transform().setPosition(
        getPosition() - m_description->m_upDirection * m_heightOffset
    );

    return outFlags;
}

void CharacterController::updateFallVelocity(float dt)
{
    m_fallVelocity += dt * m_gravity;

    if (m_fallVelocity.length() > m_terminalVelocity) {
        m_fallVelocity.normalize();
        m_fallVelocity *= m_terminalVelocity;
    }
}

void to_json(json& orJson, const CharacterController& korObject)
{
    ToJson<PhysicsBase>(orJson, korObject);
    orJson["description"] = korObject.m_description->asJson(); // Needs appropriate cast to sub-type
    orJson["sceneObject"] = korObject.sceneObject()->getName().c_str();
    orJson["heightOffset"] = korObject.m_heightOffset;
    orJson["gravity"] = korObject.m_gravity;
    orJson["fallVelocity"] = korObject.m_fallVelocity;
}

void from_json(const json& korJson, CharacterController& orObject)
{
    FromJson<PhysicsBase>(korJson, orObject);
    
    // Description and scene object are already constructed from JSON from all code paths leading to this function
    //m_description->loadFromJson(object["description"]);
    //m_sceneObject = SceneObject::getByName(object["sceneObject"].toString());
    orObject.m_heightOffset = (float)korJson.at("heightOffset").get<Float64_t>();
    orObject.m_gravity = Vector3(korJson["gravity"]);
    orObject.m_fallVelocity = Vector3(korJson["fallVelocity"]);

    // Set initial position of the controller
    Vector3 initPos = orObject.m_description->m_initialPosition + 
        orObject.m_heightOffset * orObject.m_description->m_upDirection;
    orObject.setPosition(initPos);
    // initialize();
}



// Box Controller

BoxController::BoxController(std::shared_ptr<BoxControllerDescription> desc, 
    const std::shared_ptr<SceneObject>& sceneObject):
    CharacterController(desc, sceneObject)
{
}




// Capsule Controller

CapsuleController::CapsuleController(std::shared_ptr<CapsuleControllerDescription> desc,
    const std::shared_ptr<SceneObject>& sceneObject) :
    CharacterController(desc, sceneObject)
{
}




// CCTManager

CCTManager::CCTManager(PhysicsScene& scene):
    m_scene(scene.scene())
{
    m_controllerManager = PxCreateControllerManager(*scene.pxScene());

    // Use overlap recovery
    //toggleOverlapRecovery(true);
    //togglePreciseSweeps(false);
}

CCTManager::~CCTManager()
{
    // Not necessary apparently (causes crash)
    //PX_RELEASE(m_controllerManager);
}

CharacterController* CCTManager::CreateController(const nlohmann::json& json)
{
    if (!json.is_object()) Logger::Throw("Error, JSON must be an object");
    
    if (!json.contains("description") || !json.contains("sceneObject")) {
        Logger::Throw("Error, object does not contain sufficient information to construct CCT");
    }
    auto description = ControllerDescription::create(json["description"]);
    auto sceneObject = SceneObject::getByName(json["sceneObject"].get_ref<const std::string&>().c_str());

    CharacterController* controller = CreateController(description, sceneObject);
    json.get_to(*controller);
    return controller;
}

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
        Logger::Throw("Error, controller description type is invalid");
    }

    // Add controller to map
    m_controllers.push_back(std::move(controller));

    return m_controllers.back().get();
}

void CCTManager::removeController(const CharacterController& controller)
{
    auto iter = std::find_if(m_controllers.begin(), m_controllers.end(),
        [&controller](std::unique_ptr<CharacterController>& c) {return controller.getUuid() == c->getUuid(); });

    if (iter == m_controllers.end()) {
        Logger::Throw("Error, controller not found");
    }
    else {
        m_controllers.erase(iter);
    }
}

void to_json(json& orJson, const CCTManager& korObject)
{
    ToJson<PhysicsBase>(orJson, korObject);
}

void from_json(const json& korJson, CCTManager& orObject)
{
    FromJson<PhysicsBase>(korJson, orObject);
}















} // rev