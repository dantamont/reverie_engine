#include "core/layer/gateway/GApplicationGateway.h"

#include "core/GCoreEngine.h"

#include "core/components/GAnimationComponent.h"
#include "core/components/GAudioListenerComponent.h"
#include "core/components/GAudioSourceComponent.h"
#include "core/components/GCanvasComponent.h"
#include "core/components/GCameraComponent.h"
#include "core/components/GCharControlComponent.h"
#include "core/components/GComponent.h"
#include "core/components/GCubeMapComponent.h"
#include "core/components/GLightComponent.h"
#include "core/components/GListenerComponent.h"
#include "core/components/GModelComponent.h"
#include "core/components/GPhysicsSceneComponent.h"
#include "core/components/GRigidBodyComponent.h"
#include "core/components/GScriptComponent.h"
#include "core/components/GShaderComponent.h"
#include "core/components/GTransformComponent.h"

#include "core/debugging/GDebugManager.h"

#include "core/rendering/shaders/GShaderProgram.h"

#include "core/canvas/GGlyph.h"
#include "core/canvas/GIcon.h"
#include "core/canvas/GLabel.h"
#include "core/canvas/GSprite.h"

#include "core/events/GEventListener.h"

#include "core/physics/GPhysicsActor.h"
#include "core/physics/GPhysicsGeometry.h"
#include "core/physics/GPhysicsManager.h"
#include "core/physics/GPhysicsMaterial.h"
#include "core/physics/GPhysicsShapePrefab.h"
#include "core/physics/GPhysicsShape.h"

#include "core/processes/GProcessManager.h"

#include "core/animation/GAnimationManager.h"
#include "core/animation/GAnimationState.h"
#include "core/animation/GAnimMotion.h"
#include "core/animation/GAnimStateMachine.h"
#include "core/animation/GAnimTransition.h"

#include "core/layer/view/widgets/graphics/GGLWidget.h"

#include "core/loop/GSimLoop.h"

#include "core/rendering/models/GModel.h"
#include "core/rendering/geometry/GMesh.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"

#include "core/rendering/materials/GMaterial.h"

#include "core/resource/GResourceCache.h"
#include "core/resource/GResource.h"

#include "core/scene/GScenario.h"
#include "core/scene/GScene.h"
#include "core/scene/GSceneObject.h"

#include "fortress/math/GMath.h"
#include "fortress/system/memory/GGarbageCollector.h"
#include "fortress/system/path/GFile.h"
#include "fortress/system/path/GPath.h"
#include "fortress/types/GStringFixedSize.h"

#include "geppetto/qt/actions/GActionManager.h"
#include "geppetto/qt/actions/commands/GComponentCommand.h"
#include "geppetto/qt/actions/commands/GSceneCommand.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/components/GAnimationComponentWidget.h"
#include "geppetto/qt/widgets/components/GLightComponentWidget.h"
#include "geppetto/qt/widgets/components/GRigidBodyComponentWidget.h"
#include "geppetto/qt/widgets/components/GPhysicsWidgets.h"
#include "geppetto/qt/widgets/nodes/animation/GAnimationNodeWidget.h"
#include "geppetto/qt/widgets/playback/player/GPlayer.h"
#include "geppetto/qt/widgets/tree/GComponentTreeItems.h"
#include "geppetto/qt/widgets/tree/GComponentTreeWidget.h"
#include "geppetto/qt/widgets/tree/GSceneTreeWidget.h"
#include "geppetto/qt/widgets/types/GParameterWidgets.h"

#include "enums/GWidgetActionTypeEnum.h"
#include "enums/GWidgetTypeEnum.h"


#include "ripple/network/messages/GCreatePolygonMeshMessage.h"
#include "ripple/network/messages/GPlaybackDataMessage.h"
#include "ripple/network/messages/GRenameModelMessage.h"

// Scene commands
#include "ripple/network/messages/GAddScenarioMessage.h"
#include "ripple/network/messages/GRestorePreviousScenarioMessage.h"
#include "ripple/network/messages/GAddSceneObjectMessage.h"
#include "ripple/network/messages/GRemoveSceneObjectMessage.h"
#include "ripple/network/messages/GReparentSceneObjectMessage.h"
#include "ripple/network/messages/GCopySceneObjectMessage.h"

#include "ripple/network/messages/GScenarioAddedMessage.h"
#include "ripple/network/messages/GScenarioModifiedMessage.h"
#include "ripple/network/messages/GSceneObjectAddedMessage.h"
#include "ripple/network/messages/GSceneObjectSelectedMessage.h"
#include "ripple/network/messages/GPreviousScenarioRestoredMessage.h"
#include "ripple/network/messages/GSceneObjectRemovedMessage.h"
#include "ripple/network/messages/GSceneObjectCopiedMessage.h"

#include "ripple/network/messages/GGetScenarioJsonMessage.h"
#include "ripple/network/messages/GScenarioJsonMessage.h"

#include "ripple/network/messages/GRenameScenarioMessage.h"
#include "ripple/network/messages/GRenameSceneMessage.h"
#include "ripple/network/messages/GRenameSceneObjectMessage.h"

// Scene object messages
#include "ripple/network/messages/GAddSceneObjectRenderLayerMessage.h"
#include "ripple/network/messages/GRemoveSceneObjectRenderLayerMessage.h"

// Component commands
#include "ripple/network/messages/GAddSceneComponentMessage.h"
#include "ripple/network/messages/GOnSceneComponentAddedMessage.h"
#include "ripple/network/messages/GRemoveSceneComponentMessage.h"
#include "ripple/network/messages/GOnSceneComponentRemovedMessage.h"

#include "ripple/network/messages/GUpdateJsonMessage.h"
#include "ripple/network/messages/GOnUpdateJsonMessage.h"

#include "ripple/network/messages/GToggleComponentMessage.h"
#include "ripple/network/messages/GResetPythonScriptMessage.h"

#include "ripple/network/messages/GRequestTransformMessage.h"
#include "ripple/network/messages/GTransformMessage.h"
#include "ripple/network/messages/GTransformUpdateMessage.h"

#include "ripple/network/messages/GSelectSceneObjectShaderPresetMessage.h"

#include "ripple/network/messages/GUpdateRigidBodyComponentMessage.h"
#include "ripple/network/messages/GCreatePhysicsShapePrefabMessage.h"
#include "ripple/network/messages/GDeletePhysicsShapePrefabMessage.h"
#include "ripple/network/messages/GUpdatePhysicsShapePrefabMessage.h"

#include "ripple/network/messages/GUpdateLightComponentMessage.h"
#include "ripple/network/messages/GSetLightComponentTypeMessage.h"
#include "ripple/network/messages/GToggleLightComponentShadowsMessage.h"
#include "ripple/network/messages/GRequestRenderSettingsInfoMessage.h"
#include "ripple/network/messages/GRenderSettingsInfoMessage.h"

#include "ripple/network/messages/GRequestAnimationStateMachinesMessage.h"
#include "ripple/network/messages/GAnimationStateMachinesMessage.h"
#include "ripple/network/messages/GSetAnimationComponentStateMachineMessage.h"
#include "ripple/network/messages/GRenameAnimationStateMachineMessage.h"
#include "ripple/network/messages/GAddAnimationStateMachineMessage.h"

#include "ripple/network/messages/GAddAnimationStateMachineConnectionMessage.h"
#include "ripple/network/messages/GRemoveAnimationStateMachineConnectionMessage.h"
#include "ripple/network/messages/GRequestAnimationComponentDataMessage.h"
#include "ripple/network/messages/GAnimationComponentDataMessage.h"
#include "ripple/network/messages/GAddAnimationComponentMotionMessage.h"
#include "ripple/network/messages/GAddAnimationStateMachineStateMessage.h"
#include "ripple/network/messages/GRemoveAnimationStateMachineStateMessage.h"
#include "ripple/network/messages/GAddAnimationStateMachineTransitionMessage.h"
#include "ripple/network/messages/GModifyAnimationStateMachineTransitionMessage.h"
#include "ripple/network/messages/GModifyAnimationComponentMotionMessage.h"
#include "ripple/network/messages/GAddAnimationClipToStateMessage.h"
#include "ripple/network/messages/GRemoveAnimationClipFromStateMessage.h"

#include "ripple/network/messages/GAddedAnimationMotionMessage.h"
#include "ripple/network/messages/GRemovedAnimationMotionMessage.h"
#include "ripple/network/messages/GModifyAnimationClipMessage.h"
#include "ripple/network/messages/GRequestAnimationDataMessage.h"
#include "ripple/network/messages/GAnimationDataMessage.h"

#include "ripple/network/messages/GAddCameraRenderLayerMessage.h"
#include "ripple/network/messages/GRemoveCameraRenderLayerMessage.h"
#include "ripple/network/messages/GModifyCameraClearColorMessage.h"
#include "ripple/network/messages/GModifyCameraOptionFlagsMessage.h"
#include "ripple/network/messages/GModifyCameraViewportMessage.h"
#include "ripple/network/messages/GModifyCameraRenderProjectionMessage.h"
#include "ripple/network/messages/GModifyCameraCubemapMessage.h"

#include "ripple/network/messages/GRequestCubemapsDataMessage.h"
#include "ripple/network/messages/GCubemapsDataMessage.h"

#include "ripple/network/messages/GSetSceneObjectModelMessage.h"
#include "ripple/network/messages/GRequestModelDataMessage.h"
#include "ripple/network/messages/GModelDataMessage.h"

#include "ripple/network/messages/GUpdateRenderSettingsMessage.h"

#include "ripple/network/messages/GModifyCubemapColorMessage.h"
#include "ripple/network/messages/GModifyCubemapNameMessage.h"
#include "ripple/network/messages/GModifyCubemapTextureMessage.h"
#include "ripple/network/messages/GModifyDefaultCubemapMessage.h"

#include "ripple/network/messages/GModifyCanvasBillboardFlagsMessage.h"
#include "ripple/network/messages/GModifyCanvasGlyphModeMessage.h"

#include "ripple/network/messages/GModifyGlyphAlignmentMessage.h"
#include "ripple/network/messages/GModifyLabelTextMessage.h"
#include "ripple/network/messages/GModifyLabelFontMessage.h"
#include "ripple/network/messages/GModifyLabelSpacingMessage.h"
#include "ripple/network/messages/GModifyLabelColorMessage.h"
#include "ripple/network/messages/GModifyIconFontSizeMessage.h"
#include "ripple/network/messages/GModifyIconNameMessage.h"
#include "ripple/network/messages/GModifyIconColorMessage.h"
#include "ripple/network/messages/GAddGlyphMessage.h"
#include "ripple/network/messages/GReparentGlyphMessage.h"
#include "ripple/network/messages/GRemoveGlyphMessage.h"
#include "ripple/network/messages/GCanvasComponentDataMessage.h"

#include "ripple/network/messages/GModifyAudioComponentResourceMessage.h"
#include "ripple/network/messages/GModifyAudioComponentSourceFlagsMessage.h"
#include "ripple/network/messages/GModifyAudioComponentSourceAttenuationMessage.h"
#include "ripple/network/messages/GModifyAudioComponentSourceVolumeMessage.h"
#include "ripple/network/messages/GModifyAudioComponentSettingsMessage.h"
#include "ripple/network/messages/GRequestAudioResourceDataMessage.h"
#include "ripple/network/messages/GAudioResourceDataMessage.h"
#include "ripple/network/messages/GRequestAudioResourcesDataMessage.h"
#include "ripple/network/messages/GAudioResourcesDataMessage.h"
#include "ripple/network/messages/GRequestAudioResourcesDataMessage.h"

#include "ripple/network/messages/GRequestResourcesDataMessage.h"
#include "ripple/network/messages/GResourcesDataMessage.h"

#include "ripple/network/messages/GResourceAddedMessage.h"
#include "ripple/network/messages/GResourceModifiedMessage.h"
#include "ripple/network/messages/GResourceRemovedMessage.h"

#include "ripple/network/messages/GModifyListenerScriptMessage.h"

#include "ripple/network/messages/GModifyAudioListenerMessage.h"

#include "ripple/network/messages/GDisplayWarningMessage.h"

#include "ripple/network/messages/GModifyResourceMessage.h"
#include "ripple/network/messages/GLoadTextureResourceMessage.h"
#include "ripple/network/messages/GUnloadTextureResourceMessage.h"
#include "ripple/network/messages/GAddMaterialResourceMessage.h"
#include "ripple/network/messages/GAddTextureToMaterialMessage.h"
#include "ripple/network/messages/GRemoveMaterialResourceMessage.h"
#include "ripple/network/messages/GCopyMaterialResourceMessage.h"
#include "ripple/network/messages/GRemoveMeshResourceMessage.h"
#include "ripple/network/messages/GAddModelResourceMessage.h"
#include "ripple/network/messages/GRemoveModelResourceMessage.h"
#include "ripple/network/messages/GRenameModelMessage.h"
#include "ripple/network/messages/GLoadModelMessage.h"
#include "ripple/network/messages/GCopyModelResourceMessage.h"
#include "ripple/network/messages/GLoadAudioResourceMessage.h"
#include "ripple/network/messages/GRemoveAudioResourceMessage.h"
#include "ripple/network/messages/GAddShaderResourceMessage.h"
#include "ripple/network/messages/GLoadShaderResourceMessage.h"
#include "ripple/network/messages/GRemoveShaderResourceMessage.h"
#include "ripple/network/messages/GReloadResourceMessage.h"
#include "ripple/network/messages/GGetResourceDataMessage.h"
#include "ripple/network/messages/GResourceDataMessage.h"

#include "ripple/network/messages/GCreateBlueprintMessage.h"
#include "ripple/network/messages/GBlueprintsDataMessage.h"
#include "ripple/network/messages/GModifyBlueprintMessage.h"
#include "ripple/network/messages/GRequestBlueprintsDataMessage.h"

#include "ripple/network/messages/GAdvanceProgressWidgetMessage.h"
#include "ripple/network/messages/GRequestAddTexturesToMaterialMessage.h"

#include "ripple/network/messages/GAddShaderPresetMessage.h"
#include "ripple/network/messages/GRemoveShaderPresetMessage.h"

#include "ripple/network/messages/GReorderRenderLayersMessage.h"
#include "ripple/network/messages/GAddRenderLayerMessage.h"
#include "ripple/network/messages/GRemoveRenderLayerMessage.h"

#include "ripple/network/messages/GReorderScriptProcessesMessage.h"
#include "ripple/network/messages/GAddScriptProcessLayerMessage.h"
#include "ripple/network/messages/GRemoveScriptProcessLayerMessage.h"

namespace rev {

/// @todo Replace with static function
#define FULL_DELETE_RESOURCE Flags<ResourceDeleteFlag>(ResourceDeleteFlag::kDeleteHandle | ResourceDeleteFlag::kForce )

ApplicationGateway::ApplicationGateway(CoreEngine* engine):
    m_engine(engine)
{
}

void ApplicationGateway::scenarioLoaded()
{
    GScenarioJsonMessage* scenarioMessage = m_garbageCollector.createCollectedMessage<GScenarioJsonMessage>();
    json scenarioJson = m_engine->getScenarioJson();
    scenarioMessage->setJsonBytes(GJson::ToBytes(scenarioJson));
    queueMessageForSend(scenarioMessage);
}

void ApplicationGateway::sceneObjectSelected(Int32_t id)
{
    GSceneObjectSelectedMessage* message = m_garbageCollector.createCollectedMessage<GSceneObjectSelectedMessage>();
    message->setSceneObjectId(id);
    queueMessageForSend(message);
}

void ApplicationGateway::processMessages()
{
    // Process received messages
    std::vector<TransportMessageInterface*> messages;
    while (TransportMessageInterface* msg = m_mailbox.retrieve()) {
        GMessage* message = static_cast<GMessage*>(msg);
        processMessage(message);
        messages.push_back(message);
    }

    // Delete received messages
    for (Uint32_t i = 0; i < messages.size(); i++) {
        delete messages[i];
    }

    // Perform garbage collection on sent messages that are no longer in use
    m_garbageCollector.deleteStaleMessages();
}

void ApplicationGateway::initializeConnections()
{
    m_engine->s_scenarioLoaded.connect(this, &ApplicationGateway::scenarioLoaded);

    /// @note Each of these routines garbage collects the sent messages after 5sec

    /// @todo Move this. For now, connecting resource added signal to widgets here
    ResourceCache::Instance().m_resourceAdded.connect(
        [this](const Uuid& uuid) {
            GResourceAddedMessage* resourceAddedMessage = m_garbageCollector.createCollectedMessage<GResourceAddedMessage>();

            auto resourceHandle = ResourceCache::Instance().getHandle(uuid);
            resourceAddedMessage->setUuid(uuid);
            resourceAddedMessage->setResourceType(static_cast<Int32_t>(resourceHandle->getResourceType()));
            resourceAddedMessage->setResourceJsonBytes(GJson::ToBytes(resourceHandle->asJson()));
            queueMessageForSend(resourceAddedMessage);
        }
    );
    ResourceCache::Instance().m_resourceDeleted.connect(
        [this](const Uuid& uuid) {
            GResourceRemovedMessage* resourceRemovedMessage = m_garbageCollector.createCollectedMessage<GResourceRemovedMessage>();
            auto resourceHandle = ResourceCache::Instance().getHandle(uuid);
            resourceRemovedMessage->setUuid(uuid);
            resourceRemovedMessage->setResourceJsonBytes(GJson::ToBytes(resourceHandle->asJson()));
            queueMessageForSend(resourceRemovedMessage);
        }
    );
    ResourceCache::Instance().m_resourceChanged.connect(
        [this](const Uuid& uuid) {
            GResourceModifiedMessage* resourceModifiedMessage = m_garbageCollector.createCollectedMessage<GResourceModifiedMessage>();
            auto resourceHandle = ResourceCache::Instance().getHandle(uuid);
            resourceModifiedMessage->setUuid(uuid);
            resourceModifiedMessage->setResourceJsonBytes(GJson::ToBytes(resourceHandle->asJson()));
            queueMessageForSend(resourceModifiedMessage);
        }
    );
}

void ApplicationGateway::processMessage(GRequestPlaybackDataMessage* message)
{
    // Send playback data message in response
    GPlaybackDataMessage* response = m_garbageCollector.createCollectedMessage<GPlaybackDataMessage>();
    response->setPlaybackMode(m_engine->simulationLoop()->getPlayMode());
    response->setPlaybackState(m_engine->simulationLoop()->getPlayState());
    queueMessageForSend(response);
}

void ApplicationGateway::processMessage(GTogglePlaybackMessage* message)
{
    switch ((ESimulationPlayState)m_engine->simulationLoop()->getPlayState()) {
    case ESimulationPlayState::eStoppedState:
    case ESimulationPlayState::ePausedState:
        // Play the loop
        m_engine->simulationLoop()->play();
        break;
    case ESimulationPlayState::ePlayedState:
        // Pause the loop
        m_engine->simulationLoop()->pause();
        break;
    }

    // Update widget with new play state
    GPlaybackDataMessage* response = m_garbageCollector.createCollectedMessage<GPlaybackDataMessage>();
    response->setPlaybackState(m_engine->simulationLoop()->getPlayState());
    queueMessageForSend(response);
}

void ApplicationGateway::processMessage(GTogglePlaybackModeMessage* message)
{
    m_engine->simulationLoop()->setPlayMode(message->getPlaybackMode());
}

void ApplicationGateway::processMessage(GLoadModelMessage* message)
{
    // Load model from file
    // TODO: Check file extension, creating a new resource folder if not MDL
    const GStringFixedSize<>& filePath = message->getFilePath();
    if (GPath::Exists(filePath)) {
        // Load model file if path exists
        ResourceCache::Instance().guaranteeHandleWithPath(
            filePath.c_str(),
            EResourceType::eModel)->getUuid();
    }

}

void ApplicationGateway::processMessage(GCopyModelResourceMessage* message)
{
    json modelJson = GJson::FromBytes(message->getJsonBytes());

    // Add model to resource cache
    auto handle = Model::CreateHandle(m_engine);
    handle->setUuid(message->getUuid());
    handle->setName(modelJson["name"].get<GString>());
    handle->setRuntimeGenerated(true);
    handle->setCachedResourceJson(modelJson);
    handle->loadResource();
}

void ApplicationGateway::processMessage(GCreatePolygonMeshMessage* message)
{
    const std::shared_ptr<PolygonCache>& cache = ResourceCache::Instance().polygonCache();
    Mesh* mesh{ nullptr };
    switch ((EBasicPolygonType)message->getPolygonType()) {
    case EBasicPolygonType::eLatLonSphere:
        mesh = cache->getSphere(message->getStackCount(), message->getSectorCount());
        break;
    case EBasicPolygonType::eGridPlane:
        mesh = cache->getGridPlane(message->getGridSpacing(), message->getNumHalfSpaces());
        break;
    case EBasicPolygonType::eGridCube:
        mesh = cache->getGridCube(message->getGridSpacing(), message->getNumHalfSpaces());
        break;
    case EBasicPolygonType::eCylinder:
        mesh = cache->getCylinder(
            message->getBaseRadius(),
            message->getTopRadius(),
            message->getHeight(),
            message->getSectorCount(),
            message->getStackCount());
        break;
    case EBasicPolygonType::eCapsule:
        mesh = cache->getCapsule(message->getRadius(), message->getHalfHeight());
        break;
    case EBasicPolygonType::eRectangle:
    case EBasicPolygonType::eCube:
    default:
        assert(false && "Error, shape type not recognized");
    }

    // Make sure UUID matches that specified
    if (!message->getUuid().isNull()) {
        mesh->handle()->setUuid(message->getUuid());
    }
}


void ApplicationGateway::processMessage(GAddScenarioMessage* message)
{
    // Create scenario
    m_previousLoadedScenario = m_engine->getScenarioJson();
    m_previousScenarioPath = m_engine->scenario()->getPath();
    m_engine->setNewScenario();
}

void ApplicationGateway::processMessage(GRestorePreviousScenarioMessage* message)
{
    // Revert to last scenario
    auto scenario = std::make_shared<Scenario>(m_engine);
    m_engine->setScenario(scenario);
    m_previousLoadedScenario.get_to(*scenario);
    scenario->setPath(m_previousScenarioPath);
}

void ApplicationGateway::processMessage(GAddSceneObjectMessage* message)
{
    // Create scene object
    std::shared_ptr<SceneObject> sceneObject = SceneObject::Create(&m_engine->scenario()->scene());
    if (message->getLoadFromJson()) {
        // Load scene object from JSON if flag set to true
        json j = GJson::FromBytes(message->getJsonBytes());
        from_json(j, *sceneObject);
    }
    
    if (message->getParentId() >= 0) {
        auto parentSceneObject = SceneObject::Get(message->getParentId());
        sceneObject->setParent(parentSceneObject);
    }

    // Send scene object JSON back to widget
    GSceneObjectAddedMessage* response = m_garbageCollector.createCollectedMessage<GSceneObjectAddedMessage>();
    response->setJsonBytes(GJson::ToBytes(*sceneObject));
    queueMessageForSend(response);
}

void ApplicationGateway::processMessage(GRemoveSceneObjectMessage* message)
{
    // Retrieve scene object 
    auto sceneObject = SceneObject::Get(message->getSceneObjectId());

    if (sceneObject->isScriptGenerated()) {
        QMessageBox::about(m_engine->widgetManager()->mainWindow(),
            "Warning",
            "Manual deletion of a python-generated object will cause undefined behavior");
        return;
    }

    // Cache JSON and delete scene object
    json sceneObjectJson = *sceneObject;
    sceneObject->removeFromScene();

    // Send message that scenario has been updated to widgets
    GSceneObjectRemovedMessage* response = m_garbageCollector.createCollectedMessage<GSceneObjectRemovedMessage>();
    queueMessageForSend(response);
}

void ApplicationGateway::processMessage(GRenameScenarioMessage* message)
{
    m_engine->scenario()->setName(message->getNewName().c_str());
}

void ApplicationGateway::processMessage(GRenameSceneMessage* message)
{
    m_engine->scenario()->scene().setName(message->getNewName().c_str());
}

void ApplicationGateway::processMessage(GRenameSceneObjectMessage* message)
{
    auto sceneObject = m_engine->scenario()->scene().getSceneObjectByName(message->getPreviousName().c_str());
    sceneObject->setName(message->getNewName().c_str());
}

void ApplicationGateway::processMessage(GReparentSceneObjectMessage* message)
{
    // Cache previous parent and scene
    std::shared_ptr<SceneObject> sceneObject = SceneObject::Get(message->getSceneObjectId());
    std::shared_ptr<SceneObject> newParent = nullptr;

    if (message->getNewParentId() >= 0) {
        newParent = SceneObject::Get(message->getNewParentId());
    }
    
    sceneObject->setParent(newParent);
}

void ApplicationGateway::processMessage(GCopySceneObjectMessage* message)
{
    // Duplicate object
    std::shared_ptr<SceneObject> originalObject = SceneObject::Get(message->getSceneObjectId());
    std::shared_ptr<SceneObject> sceneObjectCopy = SceneObject::Create(*originalObject);

    // Set new name for duplicate object
    sceneObjectCopy->setName(sceneObjectCopy->getName() + "_copy");

    // Emit message that object was copied
    GSceneObjectCopiedMessage* response = m_garbageCollector.createCollectedMessage<GSceneObjectCopiedMessage>();
    response->setJsonBytes(GJson::ToBytes(*sceneObjectCopy));
    queueMessageForSend(response);
}

void ApplicationGateway::processMessage(GAddSceneComponentMessage* message)
{
    json j = GJson::FromBytes(message->getJsonBytes());

    switch ((ESceneType)message->getSceneType()) {
    case ESceneType::eSceneObject:
    {
        // Load component
        Component* component = Component::create(SceneObject::Get(message->getSceneObjectId()), j);

        std::vector<Uuid> dependencyIds;
        std::vector<json> dependencies;
        if (message->getAddComponentDependencies()) {
            component->addRequiredComponents(dependencyIds, dependencies);
        }

        GOnSceneComponentAddedMessage* objectAddedMessage = m_garbageCollector.createCollectedMessage<GOnSceneComponentAddedMessage>();
        objectAddedMessage->setJsonBytes(component->toJson());
        objectAddedMessage->setSceneObjectId(message->getSceneObjectId());
        objectAddedMessage->setSceneType(message->getSceneType());
        objectAddedMessage->setCommandId(message->getCommandId());
        objectAddedMessage->setComponentId(component->getUuid());
        objectAddedMessage->setComponentDependencyIds(dependencyIds);
        objectAddedMessage->setComponentDependencies(dependencies);
        queueMessageForSend(objectAddedMessage);
        break;
    }
    case ESceneType::eScene:
    {
        // Create scene component
        Component* component = Component::Create(m_engine->scenario()->scene(), j);
        component->enable(); /// @todo See if this is necessary
    }
    default:
        assert(false && "Invalid scene type");
    }
}

void ApplicationGateway::processMessage(GRemoveSceneComponentMessage* message)
{
    // Get component JSON
    json componentJson = GJson::FromBytes(message->getJsonBytes());
    Uuid componentId = componentJson["id"];

    GSceneType sceneType = message->getSceneType();

    if (ESceneType::eSceneObject == sceneType) {
        /// Scene object

        // Remove component, and delete
        auto so = SceneObject::Get(message->getSceneObjectId());
        so->removeComponent(componentId);

        // Remove any other added components
        for (const Uuid& id : message->getComponentDependencyIds()) {
            so->removeComponent(id);
        }
    }
    else {
        /// Scene
        // Remove rigid body component, but do not delete
        m_engine->scenario()->scene().removeComponent(componentId);
    }

    // Might not be needed since data is already on widget side
    //// Return message that component was removed
    //m_componentRemovedMessage.setCommandId(message->getCommandId());
    //m_componentRemovedMessage.setJsonBytes(message->getJsonBytes());
    //m_componentRemovedMessage.setSceneType(sceneType);
}

void ApplicationGateway::processMessage(GUpdateJsonMessage* message)
{
    json jsonFromMessage = GJson::FromBytes(message->getJsonBytes());
    const json& metadata = jsonFromMessage["metadata"];
    const json& objectJson = jsonFromMessage["object"];
    if (metadata.contains("isBlueprintItem")) {
        /// Process updated JSON for a blueprint

        // Find matching blueprint
        Uuid blueprintId = objectJson["uuid"];
        std::vector<Blueprint>& blueprints = m_engine->scenario()->blueprints();
        auto bpIter = std::find_if(blueprints.begin(), blueprints.end(),
            [blueprintId](const Blueprint& bp) {
                return bp.getUuid() == blueprintId;
            }
        );

#ifdef DEBUG_MODE
        assert(bpIter != blueprints.end() && "Error, blueprint is not in list");
#endif

        // Set JSON for the blueprint
        Blueprint& bp = *bpIter;
        bp = jsonFromMessage;
    }
    else if (metadata.contains("isComponentItem")) {
        Uuid componentId = objectJson["id"];
        if (metadata.contains("sceneObjectType")) {
            // Is a scene object component
            Uint32_t soId = metadata["sceneObjectId"].get<Uint32_t>();
            auto so = SceneObject::Get(soId);

            // Retrieve component
            ESceneObjectComponentType componentType = ESceneObjectComponentType(metadata["sceneObjectType"].get<Int32_t>());
            Component* comp = so->getComponent((ComponentType)Int32_t(componentType));

            // Load component from JSON
            switch (componentType) {
            case ESceneObjectComponentType::eCharacterController:
            {
                CharControlComponent* charComp = static_cast<CharControlComponent*>(comp);
                *charComp = objectJson;
                break;
            }
            default:
                assert(false && "Unsupported type");
            }
        }
        else {
            // Is a scene component
            Scene& scene = m_engine->scenario()->scene();
            GSceneComponentType componentType(metadata["sceneType"].get<Int32_t>());

            // Load physics scene from json
            if (componentType != ESceneComponentType::ePhysicsScene) {
                assert(false && "Unsupported type");
            }
            *scene.physicsComponent() = objectJson;
        }
    }
    else if(metadata.contains("isAnimationMotionWidget")) {
        // Obtain the scene object
        Uint32_t soId = metadata["sceneObjectId"].get<Uint32_t>();
        auto so = SceneObject::Get(soId);
        BoneAnimationComponent* comp = so->getComponent<BoneAnimationComponent>(ComponentType::kBoneAnimation);
        GString motionName = objectJson["name"].get_ref<const std::string&>().c_str();
        Motion* motion = comp->animationController().getMotion(motionName);
        from_json(objectJson, *motion);
    }
    else if (metadata.contains("isSpriteWidget")) {
        // Obtain the scene object
        Uint32_t soId = metadata["sceneObjectId"].get<Uint32_t>();
        Uuid glyphId = objectJson["uuid"].get<Uuid>();
        auto so = SceneObject::Get(soId);
        CanvasComponent* comp = so->getComponent<CanvasComponent>(ComponentType::kCanvas);
        Sprite* sprite = comp->getGlyph<Sprite>(glyphId);
        from_json(objectJson, *sprite);
    }
    else if (metadata.contains("isDebugSettings")) {
        objectJson.get_to(m_engine->debugManager()->debugSettings());
    }
    else if (metadata.contains("isShaderJsonWidget")) {
        GString name = objectJson["name"];
        bool created;
        ShaderPreset& preset = *m_engine->scenario()->settings().getShaderPreset(name, created);
        assert(!created && "Preset should already exist");
        objectJson.get_to(preset);
    }
    else if (metadata.contains("isMainRenderLayerWidget")) {
        SortingLayers& layers = m_engine->scenario()->settings().renderLayers();
        Uint32_t layerId = objectJson["id"].get<Int32_t>();
        Int32_t layerOrder = objectJson.at("order").get<Int32_t>();
        std::string layerName = objectJson.at("label").get_ref<const std::string&>();
        layers.setLayerOrder(layerId, layerOrder);
        layers.setLayerNameFromId(layerId, layerName);

        // Update render layer widget with new scenario JSON
        /// @todo This is overkill, send a smaller message
        GScenarioJsonMessage* scenarioMessage = m_garbageCollector.createCollectedMessage<GScenarioJsonMessage>();
        json scenarioJson = m_engine->getScenarioJson();
        scenarioMessage->setJsonBytes(GJson::ToBytes(scenarioJson));
        queueMessageForSend(scenarioMessage);
    }
    else if (metadata.contains("isScriptOrderWidget")) {
        SortingLayers& layers = m_engine->processManager()->processQueue().sortingLayers();
        Uint32_t layerId = objectJson["id"].get<Int32_t>();
        Int32_t layerOrder = objectJson.at("order").get<Int32_t>();
        std::string layerName = objectJson.at("label").get_ref<const std::string&>();
        layers.setLayerOrder(layerId, layerOrder);
        layers.setLayerNameFromId(layerId, layerName);

        // Update script order widget with new scenario JSON
        /// @todo This is overkill, send a smaller message
        GScenarioJsonMessage* scenarioMessage = m_garbageCollector.createCollectedMessage<GScenarioJsonMessage>();
        json scenarioJson = m_engine->getScenarioJson();
        scenarioMessage->setJsonBytes(GJson::ToBytes(scenarioJson));
        queueMessageForSend(scenarioMessage);
    }
}

void ApplicationGateway::processMessage(GGetScenarioJsonMessage* message)
{
    // Need to offload this to a different thread so it's non-blocking
    auto sendScenarioJson = [this](std::vector<Uint8_t> metadataBytes)
    {
        // Allow access from multiple threads
        static std::mutex s_reentrantMutex;
        std::unique_lock lock(s_reentrantMutex);

        while (ResourceCache::Instance().isLoadingResources()) {
            // Sleep until resources are loaded
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        GScenarioJsonMessage* response = m_garbageCollector.createCollectedMessage<GScenarioJsonMessage>();
        json scenarioJson = m_engine->getScenarioJson();
        response->setJsonBytes(GJson::ToBytes(scenarioJson));
        response->setMetadataJsonBytes(metadataBytes);
        queueMessageForSend(response);
    };

    m_threadPool.addTask(sendScenarioJson, message->getJsonBytes());
}

void ApplicationGateway::processMessage(GToggleComponentMessage* message)
{
    Component* component = nullptr;
    if (message->getSceneType() == ESceneType::eSceneObject) {
        SceneObject& so = *SceneObject::Get(message->getSceneOrObjectId());
        component = so.getComponent((ComponentType)message->getComponentType());
    }
    else {
        /// @todo Handle debug scene
        Scene& scene = m_engine->scenario()->scene();
        component = scene.getComponent((ComponentType)message->getComponentType());
    }
    component->toggle(message->getToggleState());
}

void ApplicationGateway::processMessage(GResetPythonScriptMessage* message)
{
    // Retrieve component
    SceneObject& so = *SceneObject::Get(message->getSceneOrObjectId());
    ScriptComponent* component = so.getComponent<ScriptComponent>(ComponentType::kPythonScript);

    if (!message->getNewFilePath().isEmpty()) {
        component->initializeBehavior(message->getNewFilePath().c_str());
    }
    else {
        component->reset();
    }
}

void ApplicationGateway::processMessage(GRequestTransformMessage* message)
{
    GTransformMessage* response = m_garbageCollector.createCollectedMessage<GTransformMessage>();
    json transformJson;
    const std::vector<Uint8_t>& metadataBytes = message->getMetadataJsonBytes();
    json metadata = GJson::FromBytes(metadataBytes);
    SceneObject& so = *SceneObject::Get(message->getSceneOrObjectId());
    response->setSceneOrObjectId(message->getSceneOrObjectId());

    if (!metadata.contains("glyphId")) {
        // Regular scene object transform
        const TransformComponent& transform = so.transform();

        // Populate response with transform JSON, and send
        transformJson = transform;
    }
    else {
        // Glyph transform
        CanvasComponent* canvasComp = so.getComponent<CanvasComponent>(ComponentType::kCanvas);
        //Glyph::GlyphType glyphType = static_cast<Glyph::GlyphType>(metadata["glyphType"].get<Int32_t>());
        Uuid glyphId = metadata["glyphId"];
        Glyph* glyph{ nullptr };
        auto iter = std::find_if(canvasComp->glyphs().begin(), canvasComp->glyphs().end(),
            [&](const std::shared_ptr<Glyph>& g) {
                return g->getUuid() == glyphId;
            }
        );
        if (iter != canvasComp->glyphs().end()) {
            glyph = (*iter).get();
        }
#ifdef DEBUG_MODE
        assert(glyph && "Glyph not found");
#endif
        transformJson = glyph->transform();
    }

    response->setMetadataJsonBytes(metadataBytes);
    response->setTransformJsonBytes(GJson::ToBytes(transformJson));
    queueMessageForSend(response);
}

void ApplicationGateway::processMessage(GTransformUpdateMessage* message)
{
    // Assumes that the currently selected scene object is being transformed
    SceneObject& so = *SceneObject::Get(message->getSceneOrObjectId());
    json transformJson = GJson::FromBytes(message->getTransformJsonBytes());

    json metadata = GJson::FromBytes(message->getMetadataJsonBytes());

    if(metadata.contains("glyphType")) {
#ifdef DEBUG_MODE
        assert(metadata.contains("glyphId") && "Insufficient metadata");
#endif
        CanvasComponent* canvasComp = so.getComponent<CanvasComponent>(ComponentType::kCanvas);
        Uuid glyphId = metadata["glyphId"];
        Glyph* glyph{ nullptr };
        auto iter = std::find_if(canvasComp->glyphs().begin(), canvasComp->glyphs().end(),
            [&](const std::shared_ptr<Glyph>& g) {
                return g->getUuid() == glyphId;
            }
        );
#ifdef DEBUG_MODE
        assert(glyph && "Glyph not found");
#endif
        glyph->transform() = transformJson;
    }
    else {
        // Setting scene object transform
        TransformComponent& transform = so.transform();
        transform.set(transformJson); // Set is important because needs to load only the transform-specific fields
    }
}

void ApplicationGateway::processMessage(GSelectSceneObjectShaderPresetMessage* message)
{
    // Verify validity of preset
    GString presetName = message->getShaderPresetName().c_str();
    bool wasCreated;
    std::shared_ptr<ShaderPreset> shaderPreset = m_engine->scenario()->settings().getShaderPreset(presetName, wasCreated);
    if (wasCreated) {
        Logger::Throw("Error, shader preset must exist");
    }
    if (!shaderPreset) {
        Logger::Throw("Error, preset does not exist");
    }

    // Set shader preset in scene object
    auto soPtr = SceneObject::Get(message->getSceneObjectId());
    SceneObject& so = *soPtr;
    so.getComponent<ShaderComponent>(ComponentType::kShader)->setShaderPreset(shaderPreset);
}

void ApplicationGateway::processMessage(GUpdatePhysicsShapePrefabMessage* message)
{
    // New prefab JSON
    json newPrefabJson = GJson::FromBytes(message->getJsonBytes());

    // Get prefab
    PhysicsShapePrefab* prefab = PhysicsManager::Shape(newPrefabJson["name"].get_ref<const std::string&>().c_str());

    // Update geometry
    GPhysicsGeometryType type = GPhysicsGeometryType(newPrefabJson["geometry"]["type"].get<Int32_t>());
    const json& geometryJson = newPrefabJson["geometry"];

    switch ((EPhysicsGeometryType)type) {
    case EPhysicsGeometryType::eBox: {
        auto boxGeometry = std::make_unique<BoxGeometry>(
            geometryJson["hx"],
            geometryJson["hy"],
            geometryJson["hz"]
            );
        prefab->setGeometry(std::move(boxGeometry));
        break;
    }
    case EPhysicsGeometryType::eSphere: {
        auto sphereGeometry = std::make_unique<SphereGeometry>(
            geometryJson["radius"]
            );
        prefab->setGeometry(std::move(sphereGeometry));
        break;
    }
    case EPhysicsGeometryType::ePlane: {
        prefab->setGeometry(std::move(std::make_unique<PlaneGeometry>()));
        break;
    }
    default:
        Logger::Throw("Geometry type not recognized");
    }

    if (!message->getUpdateGeometryOnly()) {

    }

}

void ApplicationGateway::processMessage(GUpdateRigidBodyComponentMessage* message)
{
    // New rigid body JSON
    json newRigidBodyJson = GJson::FromBytes(message->getBodyJsonBytes());

    // Get the relevant rigid body component
    Int32_t sceneObjectId = message->getSceneObjectId();
    Uuid componentId = message->getComponentId();
    Component* comp = SceneObject::Get(sceneObjectId)->getComponent(ComponentType::kRigidBody);
    RigidBodyComponent& rigidBodyComponent = *static_cast<RigidBodyComponent*>(comp);

    if (rigidBodyComponent.body()) {
        // Update the body's type if it has changed
        GRigidBodyType rigidType = GRigidBodyType(newRigidBodyJson["rigidType"].get<Int32_t>());
        if (rigidBodyComponent.body()->rigidType() != rigidType) {
            rigidBodyComponent.body()->setRigidType(rigidType);
            rigidBodyComponent.body()->reinitialize();
        }

        // Update the physics shape used by the body
        GString prefabShapeName = newRigidBodyJson["shapes"][0].get_ref<const std::string&>().c_str();
        if (rigidBodyComponent.body()->shape(0).prefab().getName() != prefabShapeName) {
            rigidBodyComponent.body()->shape(0).setPrefab(*PhysicsManager::Shape(prefabShapeName));
        }

        // Update whether or not the body is kinematic
        bool newIsKinematic = newRigidBodyJson["isKinematic"].get<bool>();
        if (newIsKinematic != rigidBodyComponent.body()->isKinematic()) {
            rigidBodyComponent.body()->setKinematic(newIsKinematic, true);
        }

        // Update the density if changed
        Float32_t newDensity = newRigidBodyJson["density"].get<Float32_t>();
        bool densityChanged = !math::fuzzyIsNull(newDensity - rigidBodyComponent.body()->density());
        if (densityChanged) {
            rigidBodyComponent.body()->setDensity(newDensity);
        }
    }
}

void ApplicationGateway::processMessage(GCreatePhysicsShapePrefabMessage* message)
{
    // Add prefab
    GString name = message->getName();
    json pJson = *PhysicsManager::DefaultShape();
    pJson["name"] = name;
    PhysicsShapePrefab::Create(pJson);

    // Respond with message that updates physics shape widget
    GScenarioJsonMessage* response = m_garbageCollector.createCollectedMessage<GScenarioJsonMessage>();
    json scenarioJson = m_engine->getScenarioJson();
    json metadatajson = { {"metadata", {"widgetType", Int32_t(EWidgetType::ePhysicsShapePrefab)}} };
    response->setMetadataJsonBytes(GJson::ToBytes(metadatajson));
    response->setJsonBytes(GJson::ToBytes(scenarioJson));
    queueMessageForSend(response);
}

void ApplicationGateway::processMessage(GDeletePhysicsShapePrefabMessage* message)
{
    PhysicsShapePrefab* prefab = PhysicsManager::Shape(message->getName().c_str());
    PhysicsManager::RemoveShape(prefab);

    // Respond with message that updates physics shape widget
    GScenarioJsonMessage* response = m_garbageCollector.createCollectedMessage<GScenarioJsonMessage>();
    json scenarioJson = m_engine->getScenarioJson();
    json metadatajson = { {"metadata", {"widgetType", Int32_t(EWidgetType::ePhysicsShapePrefab)}} };
    metadatajson["metadata"]["widgetType"] = Int32_t(EWidgetType::ePhysicsShapePrefab);
    response->setMetadataJsonBytes(GJson::ToBytes(metadatajson));
    response->setJsonBytes(GJson::ToBytes(scenarioJson));
    queueMessageForSend(response);
}

void ApplicationGateway::processMessage(GAddSceneObjectRenderLayerMessage* message)
{
    // Add render layer to a scene object
    auto soPtr = SceneObject::Get(message->getSceneObjectId());
    SceneObject& so = *soPtr;
    so.addRenderLayer(message->getRenderLayerId());
}

void ApplicationGateway::processMessage(GRemoveSceneObjectRenderLayerMessage* message)
{
    // Remove render layer from a scene object
    auto soPtr = SceneObject::Get(message->getSceneObjectId());
    SceneObject& so = *soPtr;

    // Use the ID to remove layer from the scene object
    Int32_t layerId = message->getRenderLayerId();
    so.removeRenderLayer(layerId);
}

void ApplicationGateway::processMessage(GAddCameraRenderLayerMessage* message)
{
    // Add render layer to a scene object's camera
    auto soPtr = SceneObject::Get(message->getSceneObjectId());
    SceneObject& so = *soPtr;
    CameraComponent* camera = so.getComponent<CameraComponent>(ComponentType::kCamera);
    camera->camera().addRenderLayer(message->getRenderLayerId());
}

void ApplicationGateway::processMessage(GRemoveCameraRenderLayerMessage* message)
{
    // Remove render layer from a scene object
    auto soPtr = SceneObject::Get(message->getSceneObjectId());
    SceneObject& so = *soPtr;
    CameraComponent* camera = so.getComponent<CameraComponent>(ComponentType::kCamera);

    // Use the ID to remove layer from the scene object
    Int32_t layerId = message->getRenderLayerId();
    camera->camera().removeRenderLayer(layerId);
}

void ApplicationGateway::processMessage(GUpdateLightComponentMessage* message)
{
    // Remove render layer from a scene object
    auto soPtr = SceneObject::Get(message->getSceneObjectId());
    SceneObject& so = *soPtr;

    LightComponent* lightComp = static_cast<LightComponent*>(so.getComponent(ComponentType::kLight));
    json compJson = GJson::FromBytes(message->getJsonBytes());
    if (message->getUpdateShadows()) {
        from_json(compJson, *lightComp);
    }
    else {
        lightComp->loadLightFromJson(compJson["light"]);
    }
}

void ApplicationGateway::processMessage(GSetLightComponentTypeMessage* message)
{
    auto so = SceneObject::Get(message->getSceneObjectId());
    LightComponent* lightComponent = so->getComponent<LightComponent>(ComponentType::kLight);
    lightComponent->setLightType(Light::LightType(message->getLightType()));
}

void ApplicationGateway::processMessage(GToggleLightComponentShadowsMessage* message)
{
    //QMutexLocker lock(&m_widgetManager->openGlRenderer()->drawMutex());

    if (!m_engine->openGlRenderer()->renderContext().isCurrent()) {
        m_engine->openGlRenderer()->renderContext().makeCurrent();
    }

    auto so = SceneObject::Get(message->getSceneObjectId());
    LightComponent* lightComponent = so->getComponent<LightComponent>(ComponentType::kLight);
    if (message->getEnable()) {
        // Enabling shadows for a light
        LightingSettings& settings = m_engine->openGlRenderer()->renderContext().lightingSettings();
        if (settings.canAddShadow(lightComponent->getLightType())){
            lightComponent->enableShadowCasting(true);
        }
    }
    else {
        // Disabling shadows for a light
        // TODO: Configure so that shadow map is preserved on disable
        lightComponent->disableShadowCasting(true);
    }
}

void ApplicationGateway::processMessage(GRequestRenderSettingsInfoMessage* message)
{
    LightingSettings& lightSettings = m_engine->openGlRenderer()->renderContext().lightingSettings();
    GRenderSettingsInfoMessage* renderMessage = m_garbageCollector.createCollectedMessage<GRenderSettingsInfoMessage>();
    renderMessage->setCanAddPointLightShadow(lightSettings.canAddShadow(Light::kPoint));
    renderMessage->setCanAddDirectionalLightShadow(lightSettings.canAddShadow(Light::kDirectional));
    renderMessage->setCanAddSpotLightShadow(lightSettings.canAddShadow(Light::kSpot));
    queueMessageForSend(renderMessage);
}

void ApplicationGateway::processMessage(GRequestAnimationStateMachinesMessage* message)
{
    GAnimationStateMachinesMessage* response = m_garbageCollector.createCollectedMessage<GAnimationStateMachinesMessage>();
    json responseJson = json::array();

    std::vector<AnimationStateMachine*> machines = m_engine->animationManager()->stateMachines();
    for (AnimationStateMachine* machine : machines) {
        responseJson.push_back(*machine);
    }

    response->setJsonBytes(GJson::ToBytes(responseJson));
    queueMessageForSend(response);
}

void ApplicationGateway::processMessage(GSetAnimationComponentStateMachineMessage* message)
{
    AnimationStateMachine* selected = m_engine->animationManager()->getStateMachine(message->getNewStateMachineId());
    auto so = SceneObject::Get(message->getSceneObjectId());
    BoneAnimationComponent* comp = so->getComponent<BoneAnimationComponent>(ComponentType::kBoneAnimation);
    comp->animationController().setStateMachine(selected);
}

void ApplicationGateway::processMessage(GRenameAnimationStateMachineMessage* message)
{
    GRequestAnimationStateMachinesMessage* requestMessage = m_garbageCollector.createCollectedMessage<GRequestAnimationStateMachinesMessage>();

    Uuid machineId = message->getStateMachineId();
    
    // Rename the state machine, and update the widgets that were used to rename the state machine
    AnimationStateMachine * selected = m_engine->animationManager()->getStateMachine(machineId);
    if (selected) {
        GString newName = message->getNewName();
        selected->setName(newName);
        
        // Trigger callback to update animation widgets
        /// @todo Handle this in a more fine-tuned way using data from widget side, without needing a message
        processMessage(requestMessage);
    }
}

void ApplicationGateway::processMessage(GAddAnimationStateMachineMessage* message)
{
    GRequestAnimationStateMachinesMessage* requestMessage = m_garbageCollector.createCollectedMessage<GRequestAnimationStateMachinesMessage>();

    m_engine->animationManager()->addStateMachine();

    // Trigger callback to update animation widgets
    /// @todo Handle this in a more fine-tuned way using data from widget side, without needing a message
    processMessage(requestMessage);
}

void ApplicationGateway::processMessage(GAddAnimationStateMachineConnectionMessage* message)
{
    AnimationStateMachine* sm = m_engine->animationManager()->getStateMachine(message->getStateMachineId());
    BaseAnimationState* startState = sm->getState(message->getStartStateId());
    BaseAnimationState* endState = sm->getState(message->getEndStateId());
    StateConnection* sConnection = sm->addConnection(startState, endState);
    sConnection->setUuid(message->getConnectionId());
}

void ApplicationGateway::processMessage(GRemoveAnimationStateMachineConnectionMessage* message)
{
    AnimationStateMachine* sm = m_engine->animationManager()->getStateMachine(message->getStateMachineId());
    StateConnection* sConnection = sm->getConnection(message->getConnectionId());
    sm->removeConnection(sConnection);
}

void ApplicationGateway::processMessage(GRequestAnimationComponentDataMessage* message)
{
    GAnimationComponentDataMessage* response = m_garbageCollector.createCollectedMessage<GAnimationComponentDataMessage>();
    AnimationStateMachine* sm = m_engine->animationManager()->getStateMachine(message->getStateMachineId());
    json animationsJson;
    json motionsJson;
    json statesJson;
    json transitionsJson;
    json connectionsJson;

    if (message->getSceneObjectId() >= 0) {
        auto so = SceneObject::Get(message->getSceneObjectId());

        // Add animations JSON to message
        BoneAnimationComponent* animationComponent = so->getComponent<BoneAnimationComponent>(ComponentType::kBoneAnimation);
        std::shared_ptr<ResourceHandle> modelHandle = animationComponent->modelHandle();
        if (modelHandle) {
            // Iterate through animations to add 
            for (const std::shared_ptr<ResourceHandle>& child : modelHandle->children()) {
                // Skip resources that are not animations
                if (child->getResourceType() != EResourceType::eAnimation) {
                    continue;
                }

                // Get animation
                Animation* animation = child->resourceAs<Animation>();
                if (animation) {
                    animationsJson[child->getUuid().asString().c_str()] = *animation;

                    // Add name for use by widget
                    animationsJson[child->getUuid().asString().c_str()]["name"] = child->getName();
                }
            }
        }
        response->setAnimationsJsonBytes(GJson::ToBytes(animationsJson));

        // Add motions json
        const std::vector<Motion>& motions = animationComponent->animationController().motions();
        for (const Motion& motion : motions) {
            motionsJson[motion.getUuid().asString().c_str()] = motion;
        }
        response->setMotionsJsonBytes(GJson::ToBytes(motionsJson));

    }

    // Add states JSON
    for (const AnimationState* state : sm->states()) {
        statesJson[state->getUuid().asString().c_str()] = *state;
    }
    response->setStatesJsonBytes(GJson::ToBytes(statesJson));

    // Add transitions JSON
    for (const AnimationTransition* t : sm->transitions()) {
        transitionsJson[t->getUuid().asString().c_str()] = *t;
    }
    response->setTransitionsJsonBytes(GJson::ToBytes(transitionsJson));

    // Add connections JSON
    for (const StateConnection& sc : sm->connections()) {
        connectionsJson[sc.getUuid().asString().c_str()] = sc;
    }
    response->setConnectionsJsonBytes(GJson::ToBytes(connectionsJson));

    // Send response
    queueMessageForSend(response);
}

void ApplicationGateway::processMessage(GAddAnimationComponentMotionMessage* message)
{
    // Add motion to an animation component
    auto so = SceneObject::Get(message->getSceneObjectId());
    BoneAnimationComponent* animationComponent = so->getComponent<BoneAnimationComponent>(ComponentType::kBoneAnimation);
    BaseAnimationState* state = animationComponent->animationController().stateMachine()->getState(message->getAnimationStateId());
    animationComponent->animationController().addMotion(state);
}

void ApplicationGateway::processMessage(GAddAnimationStateMachineTransitionMessage* message)
{
    static std::atomic<int> count = 0;

    AnimationStateMachine* sm = m_engine->animationManager()->getStateMachine(message->getStateMachineId());

    StateConnection* conn = sm->getConnection(message->getConnectionId());
    AnimationTransition* transition = new AnimationTransition(sm, conn->machineIndex());
    transition->setName("transition" + GString::FromNumber((size_t)count));
    sm->addTransition(transition);

    count++;
}

void ApplicationGateway::processMessage(GModifyAnimationStateMachineTransitionMessage* message)
{
    // Reload settings for the transition
    AnimationStateMachine* sm = m_engine->animationManager()->getStateMachine(message->getStateMachineId());
    AnimationTransition* transition = sm->getTransition(message->getTransitionId());
    transition->settings() = GJson::FromBytes(message->getSettingsJsonBytes());
}

void ApplicationGateway::processMessage(GModifyAnimationComponentMotionMessage* message)
{
    auto so = SceneObject::Get(message->getSceneObjectId());
    BoneAnimationComponent* animationComponent = so->getComponent<BoneAnimationComponent>(ComponentType::kBoneAnimation);
    Motion* motion = animationComponent->animationController().getMotion(message->getMotionName().c_str());
    motion->setName(message->getNewMotionName().c_str());
}

void ApplicationGateway::processMessage(GAddAnimationClipToStateMessage* message)
{
    AnimationStateMachine* sm = m_engine->animationManager()->getStateMachine(message->getStateMachineId());
    AnimationState* state = static_cast<AnimationState*>(sm->getState(message->getStateId()));
    state->addClip(nullptr);
}

void ApplicationGateway::processMessage(GRemoveAnimationClipFromStateMessage* message)
{
    AnimationStateMachine* sm = m_engine->animationManager()->getStateMachine(message->getStateMachineId());
    AnimationState* state = static_cast<AnimationState*>(sm->getState(message->getStateId()));
    state->removeClip(message->getClipId());
}

void ApplicationGateway::processMessage(GAddedAnimationMotionMessage* message)
{
    throw("unimplemented");
    //static GRequestAnimationComponentDataMessage response;
    //response.setSceneObjectId(message->getSceneObjectId());
    //response.setStateMachineId(message->getStateMachineId());

    //// Signal animation widgets to update
    //queueMessageForSend(&response);
}

void ApplicationGateway::processMessage(GRemovedAnimationMotionMessage* message)
{
    throw("unimplemented");

    //static GRequestAnimationComponentDataMessage response;
    //response.setSceneObjectId(message->getSceneObjectId());
    //response.setStateMachineId(message->getStateMachineId());

    //// Signal animation widgets to update
    //queueMessageForSend(&response);
}

void ApplicationGateway::processMessage(GAddAnimationStateMachineStateMessage* message)
{
    AnimationStateMachine* sm = m_engine->animationManager()->getStateMachine(message->getStateMachineId());
    sm->addState(new AnimationState(Uuid().createUniqueName("state_"), sm));
}

void ApplicationGateway::processMessage(GRemoveAnimationStateMachineStateMessage* message)
{
    // Remove state from a state machine
    AnimationStateMachine* sm = m_engine->animationManager()->getStateMachine(message->getStateMachineId());
    AnimationState* state = static_cast<AnimationState*>(sm->getState(message->getStateId()));
    if (!state) {
        assert(false && "Error, no selected state");
    }
    sm->removeState(state);
}

void ApplicationGateway::processMessage(GModifyAnimationClipMessage* message)
{
    AnimationStateMachine* sm = m_engine->animationManager()->getStateMachine(message->getStateMachineId());
    AnimationState* state = static_cast<AnimationState*>(sm->getState(message->getStateId()));
    AnimationClip& clip = state->getClip(message->getClipId());
    json clipJson = GJson::FromBytes(message->getJsonBytes());
    if (clipJson.contains("newName")) {
        const std::string& name = clipJson["newName"].get_ref<const std::string&>();
        if (!name.empty()) {
            clip.setName(name);
        }
    }
    if (clipJson.contains("speedFactor")) {
        Float64_t speedFactor = clipJson["speedFactor"];
        clip.settings().m_speedFactor = speedFactor;
    }
    if (clipJson.contains("tickDelay")) {
        Float64_t tickDelay = clipJson["tickDelay"];
        clip.settings().m_tickOffset = tickDelay;
    }
    if (clipJson.contains("timeDelay")) {
        Float64_t timeDelay = clipJson["timeDelay"];
        clip.settings().m_timeOffsetSec = timeDelay;
    }
}

void ApplicationGateway::processMessage(GRequestAnimationDataMessage* message)
{
    AnimationStateMachine* sm = m_engine->animationManager()->getStateMachine(message->getStateMachineId());
    AnimationState* state = static_cast<AnimationState*>(sm->getState(message->getStateId()));
    AnimationClip& clip = state->getClip(message->getClipId());
    Animation* animation = clip.animationHandle()->resourceAs<Animation>();

    GAnimationDataMessage* response = m_garbageCollector.createCollectedMessage<GAnimationDataMessage>();
    response->setAnimationName(clip.animationHandleName().c_str());
    response->setAnimationDurationSec(animation->getTimeDuration());
    queueMessageForSend(response);
}

void ApplicationGateway::processMessage(GModifyCameraClearColorMessage* message)
{
    auto so = SceneObject::Get(message->getSceneObjectId());
    CameraComponent* cameraComp = so->getComponent<CameraComponent>(ComponentType::kCamera);
    cameraComp->camera().setClearColor(message->getColor());
}

void ApplicationGateway::processMessage(GModifyCameraOptionFlagsMessage* message)
{
    auto so = SceneObject::Get(message->getSceneObjectId());
    CameraComponent* cameraComp = so->getComponent<CameraComponent>(ComponentType::kCamera);
    cameraComp->camera().cameraOptions() = message->getOptions();
}

void ApplicationGateway::processMessage(GModifyCameraViewportMessage* message)
{
    json viewportJson = GJson::FromBytes(message->getJsonBytes());
    auto so = SceneObject::Get(message->getSceneObjectId());
    CameraComponent* cameraComp = so->getComponent<CameraComponent>(ComponentType::kCamera);
    from_json(viewportJson, cameraComp->camera().viewport());

    /// @todo Only do this on width or height change
    const std::shared_ptr<OpenGlRenderer>& renderer = m_engine->openGlRenderer();
    if (!renderer->renderContext().isCurrent()) {
        renderer->renderContext().makeCurrent();
    }
    cameraComp->camera().resizeFrame(renderer->widget()->width(), renderer->widget()->height());
    cameraComp->camera().lightClusterGrid().onResize();
}

void ApplicationGateway::processMessage(GModifyCameraRenderProjectionMessage* message)
{
    json projectionJson = GJson::FromBytes(message->getJsonBytes());
    auto so = SceneObject::Get(message->getSceneObjectId());
    CameraComponent* cameraComp = so->getComponent<CameraComponent>(ComponentType::kCamera);
    from_json(projectionJson, cameraComp->camera().renderProjection());
}

void ApplicationGateway::processMessage(GModifyCameraCubemapMessage* message)
{
    auto so = SceneObject::Get(message->getSceneObjectId());
    CameraComponent* cameraComp = so->getComponent<CameraComponent>(ComponentType::kCamera);
    cameraComp->setCubeMapID(message->getCubemapId());
}

void ApplicationGateway::processMessage(GRequestCubemapsDataMessage* message)
{
    std::vector<Uuid> cubemapIds;
    std::vector<GStringFixedSize<>> cubemapNames;
    std::vector<CubeMapComponent*>& cubemaps = m_engine->scenario()->scene().cubeMaps();
    size_t count = cubemaps.size();
    for (size_t i = 0; i < count; i++) {
        CubeMapComponent* cm = cubemaps[i];
        cubemapNames.push_back(cm->getName().c_str());
        cubemapIds.push_back(cm->getUuid());
    }

    GCubemapsDataMessage* response = m_garbageCollector.createCollectedMessage<GCubemapsDataMessage>();
    queueMessageForSend(response);
}

void ApplicationGateway::processMessage(GSetSceneObjectModelMessage* message)
{
    std::shared_ptr<ResourceHandle> modelHandle = ResourceCache::Instance().getHandle(message->getModelId());
    auto so = SceneObject::Get(message->getSceneObjectId());
    ModelComponent* modelComp = so->getComponent<ModelComponent>(ComponentType::kModel);
    if (modelHandle->getResourceType() != EResourceType::eModel) {
        Logger::Throw("Error, handle is not a model");
    }
    if (!modelHandle) {
        Logger::Throw("Error, no model with specified name found");
    }

    modelComp->setModelHandle(modelHandle);
}

void ApplicationGateway::processMessage(GRequestModelDataMessage* message)
{
    // Models can only be a top-level resource
    std::vector<Uuid> modelIds;
    std::vector<GStringFixedSize<>> modelNames;
    for (const auto& resource : ResourceCache::Instance().topLevelResources()) {
        if (!resource->isConstructed()) { continue; }
        if (resource->getResourceType() != EResourceType::eModel) { continue; }
        modelIds.push_back(resource->getUuid());
        modelNames.push_back(resource->getName().c_str());
    }

    // Send message with model data
    GModelDataMessage* response = m_garbageCollector.createCollectedMessage<GModelDataMessage>();
    response->setModelIds(modelIds);
    response->setModelNames(modelNames);
    queueMessageForSend(response);
}

void ApplicationGateway::processMessage(GUpdateRenderSettingsMessage* message)
{
    auto so = SceneObject::Get(message->getSceneObjectId());
    if (message->getUpdatingModel()) {
        // Load render settings from message JSON
        ModelComponent* modelComp = so->getComponent<ModelComponent>(ComponentType::kModel);
        modelComp->renderSettings() = GJson::FromBytes(message->getJsonBytes());
    }
    else {
        assert(false && "Unimplemented");
    }
}

void ApplicationGateway::processMessage(GModifyCubemapColorMessage* message)
{
    auto so = SceneObject::Get(message->getSceneObjectId());
    CubeMapComponent* cubemapComp = so->getComponent<CubeMapComponent>(ComponentType::kCubeMap);
    cubemapComp->setDiffuseColor(message->getDiffuseColor());
}

void ApplicationGateway::processMessage(GModifyCubemapNameMessage* message)
{
    auto so = SceneObject::Get(message->getSceneObjectId());
    CubeMapComponent* cubemapComp = so->getComponent<CubeMapComponent>(ComponentType::kCubeMap);
    cubemapComp->setName(message->getCubemapName().c_str());
}

void ApplicationGateway::processMessage(GModifyCubemapTextureMessage* message)
{
    auto so = SceneObject::Get(message->getSceneObjectId());
    CubeMapComponent* cubemapComp = so->getComponent<CubeMapComponent>(ComponentType::kCubeMap);
    cubemapComp->setCubeTexture(message->getTextureFilePath().c_str());
}

void ApplicationGateway::processMessage(GModifyDefaultCubemapMessage* message)
{
    auto so = SceneObject::Get(message->getSceneObjectId());
    CubeMapComponent* cubemapComp = so->getComponent<CubeMapComponent>(ComponentType::kCubeMap);
    if (message->getSetDefault()) {
        cubemapComp->setDefault();
    }
    else {
        m_engine->scenario()->scene().setDefaultCubeMap(nullptr);
    }
}

void ApplicationGateway::processMessage(GModifyCanvasBillboardFlagsMessage* message)
{
    auto so = SceneObject::Get(message->getSceneObjectId());
    CanvasComponent* canvasComp = so->getComponent<CanvasComponent>(ComponentType::kCanvas);
    canvasComp->flags() = message->getBillboardFlags();
}

void ApplicationGateway::processMessage(GModifyCanvasGlyphModeMessage* message)
{
    auto so = SceneObject::Get(message->getSceneObjectId());
    CanvasComponent* canvasComp = so->getComponent<CanvasComponent>(ComponentType::kCanvas);
    canvasComp->setGlyphMode((GlyphMode)message->getGlyphMode());
}

void ApplicationGateway::processMessage(GModifyGlyphAlignmentMessage* message)
{
    auto so = SceneObject::Get(message->getSceneObjectId());
    CanvasComponent* canvasComp = so->getComponent<CanvasComponent>(ComponentType::kCanvas);
    Glyph* glyph = canvasComp->getGlyph(message->getGlyphId());
    glyph->setVerticalAlignment(static_cast<Glyph::VerticalAlignment>(message->getVerticalAlignment()));
    glyph->setHorizontalAlignment(static_cast<Glyph::HorizontalAlignment>(message->getHorizontalAlignment()));
}

void ApplicationGateway::processMessage(GModifyLabelTextMessage* message)
{
    m_engine->setGLContext();
    auto so = SceneObject::Get(message->getSceneObjectId());
    CanvasComponent* canvasComp = so->getComponent<CanvasComponent>(ComponentType::kCanvas);
    Label* label = canvasComp->getGlyph<Label>(message->getGlyphId());
    GString labelText(message->getText().data());
    label->setText(labelText.c_str());
}

void ApplicationGateway::processMessage(GModifyLabelFontMessage* message)
{
    m_engine->setGLContext();
    auto so = SceneObject::Get(message->getSceneObjectId());
    CanvasComponent* canvasComp = so->getComponent<CanvasComponent>(ComponentType::kCanvas);
    Label* label = canvasComp->getGlyph<Label>(message->getGlyphId());
    label->setFontSize(message->getFontSize());
    label->setFontFace(message->getFontName().c_str());

}

void ApplicationGateway::processMessage(GModifyLabelSpacingMessage* message)
{
    m_engine->setGLContext();
    auto so = SceneObject::Get(message->getSceneObjectId());
    CanvasComponent* canvasComp = so->getComponent<CanvasComponent>(ComponentType::kCanvas);
    Label* label = canvasComp->getGlyph<Label>(message->getGlyphId());
    label->setLineMaxSize(message->getMaxLineWidth());
    label->setLineSpacing(message->getVerticalLineSpacing());
}

void ApplicationGateway::processMessage(GModifyLabelColorMessage* message)
{
    m_engine->setGLContext();
    auto so = SceneObject::Get(message->getSceneObjectId());
    CanvasComponent* canvasComp = so->getComponent<CanvasComponent>(ComponentType::kCanvas);
    Label* label = canvasComp->getGlyph<Label>(message->getGlyphId());
    label->setColor(message->getColor());
}

void ApplicationGateway::processMessage(GModifyIconFontSizeMessage* message)
{
    m_engine->setGLContext();
    auto so = SceneObject::Get(message->getSceneObjectId());
    CanvasComponent* canvasComp = so->getComponent<CanvasComponent>(ComponentType::kCanvas);
    Icon* icon = canvasComp->getGlyph<Icon>(message->getGlyphId());
    icon->setFontSize(message->getFontSize());
}

void ApplicationGateway::processMessage(GModifyIconNameMessage* message)
{
    m_engine->setGLContext();
    auto so = SceneObject::Get(message->getSceneObjectId());
    CanvasComponent* canvasComp = so->getComponent<CanvasComponent>(ComponentType::kCanvas);
    Icon* icon = canvasComp->getGlyph<Icon>(message->getGlyphId());
    icon->setFontAwesomeIcon(message->getIconName().c_str());
}

void ApplicationGateway::processMessage(GModifyIconColorMessage* message)
{
    m_engine->setGLContext();
    auto so = SceneObject::Get(message->getSceneObjectId());
    CanvasComponent* canvasComp = so->getComponent<CanvasComponent>(ComponentType::kCanvas);
    Icon* icon = canvasComp->getGlyph<Icon>(message->getGlyphId());
    icon->setColor(message->getColor());
}

void ApplicationGateway::processMessage(GAddGlyphMessage* message)
{
    auto so = SceneObject::Get(message->getSceneObjectId());
    CanvasComponent* canvasComp = so->getComponent<CanvasComponent>(ComponentType::kCanvas);

    // Create the correct type of glyph, and add it to the canvas
    std::shared_ptr<Glyph> newGlyph{ nullptr };
    Glyph::GlyphType glyphType = static_cast<Glyph::GlyphType>(message->getGlyphType());

    switch (glyphType) {
    case Glyph::GlyphType::kLabel:
        newGlyph = Glyph::Create<Label>(canvasComp);
        break;
    case Glyph::GlyphType::kIcon:
        newGlyph = Glyph::Create<Icon>(canvasComp);
        break;
    case Glyph::GlyphType::kSprite:
        newGlyph = Glyph::Create<Sprite>(canvasComp);
        break;
    default:
        assert(false && "Invalid glyph type");
    }
    newGlyph->reload();
    canvasComp->addGlyph(newGlyph);

    // Send data to repopulate canvas widget
    GCanvasComponentDataMessage* canvasDataMessage = m_garbageCollector.createCollectedMessage<GCanvasComponentDataMessage>();
    json canvasJson = *canvasComp;
    canvasDataMessage->setSceneObjectId(message->getSceneObjectId());
    canvasDataMessage->setJsonBytes(GJson::ToBytes(canvasJson));
    queueMessageForSend(canvasDataMessage);
}

void ApplicationGateway::processMessage(GReparentGlyphMessage* message)
{
    auto so = SceneObject::Get(message->getSceneObjectId());
    CanvasComponent* canvasComp = so->getComponent<CanvasComponent>(ComponentType::kCanvas);
    Glyph* glyph = canvasComp->getGlyph(message->getGlyphId());
    Uuid parentId = message->getParentGlyphId();

    if (parentId.isNull()) {
        glyph->setParent(canvasComp);
    }
    else {
        Glyph* parentGlyph = canvasComp->getGlyph(parentId);
        glyph->setParent(parentGlyph);
    }
}

void ApplicationGateway::processMessage(GRemoveGlyphMessage* message)
{
    auto so = SceneObject::Get(message->getSceneObjectId());
    CanvasComponent* canvasComp = so->getComponent<CanvasComponent>(ComponentType::kCanvas);
    Glyph* glyph = canvasComp->getGlyph(message->getGlyphId());
    canvasComp->removeGlyph(*glyph);
    
    // Send data to repopulate canvas widget
    GCanvasComponentDataMessage* canvasDataMessage = m_garbageCollector.createCollectedMessage<GCanvasComponentDataMessage>();
    json canvasJson = *canvasComp;
    canvasDataMessage->setSceneObjectId(message->getSceneObjectId());
    canvasDataMessage->setJsonBytes(GJson::ToBytes(canvasJson));
    queueMessageForSend(canvasDataMessage);
}

void ApplicationGateway::processMessage(GModifyAudioComponentResourceMessage* message)
{
    auto so = SceneObject::Get(message->getSceneObjectId());
    AudioSourceComponent* audioComp = so->getComponent<AudioSourceComponent>(ComponentType::kAudioSource);

    GStringFixedSize path = message->getResourcePath();
    std::shared_ptr<ResourceHandle> handle = nullptr;
    if (!path.isEmpty()) {
        handle = ResourceCache::Instance().getTopLevelHandleWithPath(path.c_str());
        if (!handle) {
            Logger::Throw("Error, handle with path " + path + "not found");
        }
    }
    audioComp->setAudioHandle(handle);

    // Make sure that source widgets get updated
    GRequestAudioResourceDataMessage* requestAudioResourceMessage = m_garbageCollector.createCollectedMessage<GRequestAudioResourceDataMessage>();
    requestAudioResourceMessage->setSceneObjectId(message->getSceneObjectId());
    processMessage(requestAudioResourceMessage);
}

void ApplicationGateway::processMessage(GModifyAudioComponentSourceFlagsMessage* message)
{
    auto so = SceneObject::Get(message->getSceneObjectId());
    AudioSourceComponent* audioComp = so->getComponent<AudioSourceComponent>(ComponentType::kAudioSource);

    // Modify settings in audio handle
    const auto& handle = audioComp->audioHandle();
    if (!handle) {
        return;
    }
    AudioResource& audioResource = *handle->resourceAs<AudioResource>();
    audioResource.audioSourceSettings().m_audioSourceFlags = message->getAudioSourceFlags();
    audioResource.cacheSettings();
}

void ApplicationGateway::processMessage(GModifyAudioComponentSourceAttenuationMessage* message)
{
    auto so = SceneObject::Get(message->getSceneObjectId());
    AudioSourceComponent* audioComp = so->getComponent<AudioSourceComponent>(ComponentType::kAudioSource);

    // Modify settings in audio handle
    const auto& handle = audioComp->audioHandle();
    if (!handle) {
        return;
    }

    AudioResource& audioResource = *handle->resourceAs<AudioResource>();
    audioResource.audioSourceSettings().m_minDistance = message->getMinDistance();
    audioResource.audioSourceSettings().m_maxDistance = message->getMaxDistance();
    audioResource.audioSourceSettings().m_attenuationModel = message->getAttenuationModel();
    audioResource.audioSourceSettings().m_rolloff = message->getAttenuationRolloff();
    audioResource.cacheSettings();
}

void ApplicationGateway::processMessage(GModifyAudioComponentSourceVolumeMessage* message)
{
    auto so = SceneObject::Get(message->getSceneObjectId());
    AudioSourceComponent* audioComp = so->getComponent<AudioSourceComponent>(ComponentType::kAudioSource);

    // Modify settings in audio handle
    const auto& handle = audioComp->audioHandle();
    if (!handle) {
        return;
    }
    AudioResource& audioResource = *handle->resourceAs<AudioResource>();
    audioResource.audioSourceSettings().m_defaultVolume = message->getVolume();
    audioResource.cacheSettings();
}

void ApplicationGateway::processMessage(GModifyAudioComponentSettingsMessage* message)
{
    auto so = SceneObject::Get(message->getSceneObjectId());
    AudioSourceComponent* audioComp = so->getComponent<AudioSourceComponent>(ComponentType::kAudioSource);
    AudioComponentSettings& settings = audioComp->voiceProperties();

    /// @todo Capture pause status in widget, instead of preserving here
    bool isPaused = settings.testFlag(VoiceFlag::kPaused);
    settings.m_voiceFlags = message->getVoiceFlags();
    settings.setFlag(VoiceFlag::kPaused, isPaused);
    
    settings.m_volume = message->getVolume();
    settings.m_pan = message->getPan();
    settings.m_relativePlaySpeed = message->getRelativePlaySpeed();
}

void ApplicationGateway::processMessage(GRequestAudioResourceDataMessage* message)
{
    GAudioResourceDataMessage* resourceDataMessage = m_garbageCollector.createCollectedMessage<GAudioResourceDataMessage>();
    resourceDataMessage->setSceneObjectId(message->getSceneObjectId());

    auto so = SceneObject::Get(message->getSceneObjectId());
    AudioSourceComponent* audioComp = so->getComponent<AudioSourceComponent>(ComponentType::kAudioSource);

    // Modify settings in audio handle
    const auto& handle = audioComp->audioHandle();
    if (!handle) {
        return;
    }

    // Make sure that source widgets get updated
    if (handle) {
        // Populate JSON with resource data used by widgets
        json resourceJson;
        json handleJson = *handle;
        resourceJson["handle"] = handleJson;
        resourceJson["audioSettings"] = handle->resourceAs<AudioResource>()->audioSourceSettings();
        resourceDataMessage->setJsonBytes(GJson::ToBytes(resourceJson));
    }
    else {
        resourceDataMessage->setJsonBytes(std::vector<Uint8_t>());
    }
    queueMessageForSend(resourceDataMessage);
}

void ApplicationGateway::processMessage(GRequestAudioResourcesDataMessage* message)
{
    GAudioResourcesDataMessage* resourcesMessage = m_garbageCollector.createCollectedMessage<GAudioResourcesDataMessage>();
    resourcesMessage->setSceneObjectId(message->getSceneObjectId());

    json resourceNames = json::array();
    json resourcePaths = json::array();

    for (const auto& resource : ResourceCache::Instance().topLevelResources()) {
        if (resource->getResourceType() != EResourceType::eAudio) { continue; }
        resourceNames.push_back(resource->getName().c_str());
        resourcePaths.push_back(resource->getPath().c_str());
    }

    json responseJson = { {"names", resourceNames}, {"paths", resourcePaths} };
    resourcesMessage->setJsonBytes(GJson::ToBytes(responseJson));
    queueMessageForSend(resourcesMessage);
}

void ApplicationGateway::processMessage(GModifyListenerScriptMessage* message)
{
    auto so = SceneObject::Get(message->getSceneObjectId());
    ListenerComponent* listenerComp = so->getComponent<ListenerComponent>(ComponentType::kListener);
    const std::vector<Int32_t>& eventTypes = message->getEventTypes();

    // Verify that filepath is valid
    if (!message->getScriptPath().isEmpty()) {
        GFile scriptFile(message->getScriptPath().c_str());
        if (scriptFile.exists()) {
            // Path is valid, create listener
            listenerComp->initializeListener(message->getScriptPath().c_str());
        }
        else {
            assert(false && "Script file does not exist. Failed to load.");
        }
    }

    listenerComp->reset();
    listenerComp->listener()->setEventTypes(eventTypes);
}

void ApplicationGateway::processMessage(GModifyAudioListenerMessage* message)
{
    auto so = SceneObject::Get(message->getSceneObjectId());
    AudioListenerComponent* listenerComp = so->getComponent<AudioListenerComponent>(ComponentType::kAudioListener);
    listenerComp->setVelocity(Vector3(message->getVelocityX(), message->getVelocityY(), message->getVelocityZ()));
    listenerComp->setSpeedOfSound(message->getSpeedofSound());
}

void ApplicationGateway::processMessage(GLoadTextureResourceMessage* message)
{
    // Load texture from file
    GFile textureFile(message->getFilePath().c_str());
    if (textureFile.exists()) {
        // Load texture file if path exists, and set given UUID
        auto handle = ResourceCache::Instance().guaranteeHandleWithPath(
            message->getFilePath().c_str(),
            EResourceType::eTexture);

        if (!message->getUuid().isNull()) {
            handle->setUuid(message->getUuid());
        }
    }

}

void ApplicationGateway::processMessage(GUnloadTextureResourceMessage* message)
{
    auto handle = ResourceCache::Instance().getHandle(message->getUuid());
    ResourceCache::Instance().remove(handle, FULL_DELETE_RESOURCE);
}

void ApplicationGateway::processMessage(GAddMaterialResourceMessage* message)
{
    // Add material to resource cache
    GString name;
    if (message->getName().isEmpty()) {
        name = "material_" + message->getUuid().asString();;
    }
    else {
        name = message->getName().c_str();
    }
    auto handle = Material::CreateHandle(m_engine, name);
    handle->setUuid(message->getUuid());

    if (!message->getFilePath().isEmpty()) {
        // Logic from sprite sheet widget is only path that sends a filepath, so this check should
        // really be "Is this doing sprite sheet widget stuff"
        handle->setPath(message->getFilePath().c_str());
        handle->setStatusFlags(ResourceStatusFlag::kIsLoading);
        ResourceCache::Instance().incrementLoadCount();

        // Set sprite info for material
        SpriteSheetInfo spriteInfo;
        GJson::FromBytes(message->getSpriteSheetInfoJsonBytes()).get_to(spriteInfo);
        handle->resourceAs<Material>()->setSpriteInfo(spriteInfo);
    }

    // Load from JSON if given
    if (message->getMaterialJsonBytes().size()) {
        json materialJson = GJson::FromBytes(message->getMaterialJsonBytes());
        materialJson.get_to(*handle->resourceAs<Material>());
    }

    if (!message->getFilePath().isEmpty()) {
        // This is also just a switch for sprite sheet widget stuff
        GAdvanceProgressWidgetMessage* advanceMessage = m_garbageCollector.createCollectedMessage<GAdvanceProgressWidgetMessage>();
        queueMessageForSend(advanceMessage);

        // Also want to load textures
        GRequestAddTexturesToMaterialMessage* requestMessage = m_garbageCollector.createCollectedMessage<GRequestAddTexturesToMaterialMessage>();
        queueMessageForSend(requestMessage);
    }

    // Emit message that material was created
    GResourceAddedMessage* resourceAddedMessage = m_garbageCollector.createCollectedMessage<GResourceAddedMessage>();
    resourceAddedMessage->setResourceType(static_cast<Int32_t>(handle->getResourceType()));
    queueMessageForSend(resourceAddedMessage);
}

void ApplicationGateway::processMessage(GAddTextureToMaterialMessage* message)
{
    // This is currently only for the sprite sheet widget, so this will work
    GAdvanceProgressWidgetMessage* advanceMessage = m_garbageCollector.createCollectedMessage<GAdvanceProgressWidgetMessage>();
    advanceMessage->setClose(false);
    queueMessageForSend(advanceMessage);

    std::shared_ptr<ResourceHandle> handle = ResourceCache::Instance().getHandle(message->getMaterialUuid());

    // Save textures and create as resources
    std::vector<GStringFixedSize<>> texturePaths = message->getImageFilePaths();
    std::vector<Int32_t> textureTypes = message->getTextureTypes();
    Int32_t count = texturePaths.size();
    for (size_t i = 0; i < count; i++) {
        const char* filepath = texturePaths[i].c_str();
        TextureUsageType texType = static_cast<TextureUsageType>(textureTypes[i]);

        // Create texture handle, loading it serially
        std::shared_ptr<ResourceHandle> textureHandle = Texture::CreateHandle(m_engine, GString(filepath), texType, true);

        // Make texture a child of the material
        handle->addChild(textureHandle);

        queueMessageForSend(advanceMessage);
    }
    queueMessageForSend(advanceMessage);
    
    advanceMessage->setClose(true);
    queueMessageForSend(advanceMessage);

    emit ResourceCache::Instance().doneLoadingResource(handle->getUuid());

}

void ApplicationGateway::processMessage(GRemoveMaterialResourceMessage* message)
{
    auto handle = ResourceCache::Instance().getHandle(message->getUuid());
    ResourceCache::Instance().remove(handle, FULL_DELETE_RESOURCE);
}

void ApplicationGateway::processMessage(GCopyMaterialResourceMessage* message)
{
    // Add material to resource cache
    GString name = "material_" + Uuid().asString();
    auto matHandle = Material::CreateHandle(m_engine, name);
    json materialJson = GJson::FromBytes(message->getJsonBytes());
    materialJson.get_to(*matHandle->resourceAs<Material>());

    // Emit message that material was created
    GResourceAddedMessage* resourceAddedMessage = m_garbageCollector.createCollectedMessage<GResourceAddedMessage>();
    resourceAddedMessage->setResourceType(static_cast<Int32_t>(matHandle->getResourceType()));
    queueMessageForSend(resourceAddedMessage);
}

void ApplicationGateway::processMessage(GRemoveMeshResourceMessage* message)
{
    auto handle = ResourceCache::Instance().getHandle(message->getUuid());
    ResourceCache::Instance().remove(handle, FULL_DELETE_RESOURCE);
}

void ApplicationGateway::processMessage(GAddModelResourceMessage* message)
{
    // Add model to resource cache
    auto handle = Model::CreateHandle(m_engine);
    handle->setUuid(message->getUuid());
    handle->setRuntimeGenerated(true);
    handle->setIsLoading(true);

    // Get name
    GString uniqueName;
    if (message->getName().isEmpty()) {
        uniqueName = Uuid::UniqueName("model_");
    }
    else {
        uniqueName = message->getName().c_str();
    }

    // Get JSON if sent
    if (message->getModelJsonBytes().size()) {
        handle->setCachedResourceJson(GJson::FromBytes(message->getModelJsonBytes()));
    }

    auto model = std::make_unique<Model>(*handle, handle->cachedResourceJson());
    handle->setName(uniqueName);
    handle->setResource(std::move(model), false);
    handle->setConstructed(true);

    // Emit message that model was created
    GResourceAddedMessage* resourceAddedMessage = m_garbageCollector.createCollectedMessage<GResourceAddedMessage>();
    resourceAddedMessage->setResourceType(static_cast<Int32_t>(handle->getResourceType()));
    queueMessageForSend(resourceAddedMessage);
}

void ApplicationGateway::processMessage(GRemoveModelResourceMessage* message)
{
    auto handle = ResourceCache::Instance().getHandle(message->getUuid());
    ResourceCache::Instance().remove(handle, FULL_DELETE_RESOURCE);
}

void ApplicationGateway::processMessage(GLoadAudioResourceMessage* message)
{
    GDisplayWarningMessage* warningMessage = m_garbageCollector.createCollectedMessage<GDisplayWarningMessage>();
    GString filePath = message->getFilePath().c_str();
    std::shared_ptr<ResourceHandle> audio = ResourceCache::Instance().getTopLevelHandleWithPath(filePath);

    if (audio) {
        // If the audio file is already loaded, return with a warning
        static const GString titleStr("Audio Already Loaded");
        static const std::vector<Uint8_t> title = std::vector<Uint8_t>(titleStr.begin(), titleStr.end());

        GString warningStr = "Did not load audio file, audio at " + filePath + " already loaded.";
        std::vector<Uint8_t> warning = std::vector<Uint8_t>(warningStr.begin(), warningStr.end());
        warningMessage->setTitle(title);
        warningMessage->setText(warning);
        queueMessageForSend(warningMessage);
        return;
    }

    GFile audioFile(filePath);
    if (!audioFile.exists()) {
        // If the audio file does not exist, return with a warning
        static const GString titleStr("Audio Not Loaded");
        static const std::vector<Uint8_t> title = std::vector<Uint8_t>(titleStr.begin(), titleStr.end());

        GString warningStr = "Did not load audio, " + filePath + " does not exist.";
        std::vector<Uint8_t> warning = std::vector<Uint8_t>(warningStr.begin(), warningStr.end());
        warningMessage->setTitle(title);
        warningMessage->setText(warning);
        queueMessageForSend(warningMessage);
        return;
    }

    // Load audio
    AudioResource::SourceType sourceType = message->getStreamAudio() ? AudioResource::SourceType::kWavStream : AudioResource::SourceType::kWav;
    AudioResource::CreateHandle(m_engine, filePath, sourceType);

}

void ApplicationGateway::processMessage(GModifyResourceMessage* message)
{
    auto handle = ResourceCache::Instance().getHandle(message->getUuid());
    if (!message->getNewName().isEmpty()) {
        handle->setName(message->getNewName().c_str());
    }
    if (!message->getNewUuid().isNull()) {
        message->setNewUuid(message->getNewUuid());
    }
}

void ApplicationGateway::processMessage(GRemoveAudioResourceMessage* message)
{
    auto handle = ResourceCache::Instance().getHandle(message->getUuid());
    ResourceCache::Instance().remove(handle, FULL_DELETE_RESOURCE);
}

void ApplicationGateway::processMessage(GLoadShaderResourceMessage* message)
{
    auto handle = ResourceCache::Instance().guaranteeHandleWithPath(
        { message->getVertexFilePath().c_str(), 
          message->getFragmentFilePath().c_str(),
          "", 
          message->getComputeFilePath().c_str()
        },
        EResourceType::eShaderProgram);
    handle->setUuid(message->getUuid());

    // Emit message that shader was created
    GResourceAddedMessage* resourceAddedMessage = m_garbageCollector.createCollectedMessage<GResourceAddedMessage>();
    resourceAddedMessage->setResourceType(static_cast<Int32_t>(handle->getResourceType()));
    queueMessageForSend(resourceAddedMessage);
}

void ApplicationGateway::processMessage(GAddShaderResourceMessage* message)
{
    // Add shader back to resource cache
    json shaderJson = GJson::FromBytes(message->getShaderJsonBytes());
    auto handle = ShaderProgram::CreateHandle(m_engine, shaderJson, message->getName().c_str());

    // Emit message that shader was created
    GResourceAddedMessage* resourceAddedMessage = m_garbageCollector.createCollectedMessage<GResourceAddedMessage>();
    resourceAddedMessage->setResourceType(static_cast<Int32_t>(handle->getResourceType()));
    queueMessageForSend(resourceAddedMessage);
}

void ApplicationGateway::processMessage(GRemoveShaderResourceMessage* message)
{
    auto handle = ResourceCache::Instance().getHandle(message->getUuid());
    ResourceCache::Instance().remove(handle, FULL_DELETE_RESOURCE);
}

void ApplicationGateway::processMessage(GReloadResourceMessage* message)
{
    std::shared_ptr<ResourceHandle> handle = ResourceCache::Instance().getHandle(message->getUuid());
    if (!handle) {
        Logger::Throw("Error, somehow no handle found");
    }

    // Copy name prior to reload
    GString name = handle->getName();

    // Edit resource via JSON
    json resourceBytes = GJson::FromBytes(message->getResourceJsonBytes());
    handle->loadResourceJson(resourceBytes);

    // If object name has changed, emit signal to repopulate widget
    if (name != handle->getName()) {
        GResourceAddedMessage* resourceAddedMessage = m_garbageCollector.createCollectedMessage<GResourceAddedMessage>();
        resourceAddedMessage->setResourceType(static_cast<Int32_t>(handle->getResourceType()));
        queueMessageForSend(resourceAddedMessage);
    }
}

void ApplicationGateway::processMessage(GGetResourceDataMessage* message)
{
    // Respond by sending out resource data message
    GResourceDataMessage* response = m_garbageCollector.createCollectedMessage<GResourceDataMessage>();
    auto resourceHandle = ResourceCache::Instance().getHandle(message->getUuid());
    response->setName(resourceHandle->getName().c_str());
    response->setUuid(message->getUuid());
    response->setResourceJsonBytes(GJson::ToBytes(resourceHandle->asJson()));
    queueMessageForSend(response);
}

void ApplicationGateway::processMessage(GRequestResourcesDataMessage* message)
{
    GResourcesDataMessage* response = m_garbageCollector.createCollectedMessage<GResourcesDataMessage>();
    json resourceCacheJson = ResourceCache::Instance();
    response->setResourcesJsonBytes(GJson::ToBytes(resourceCacheJson["resources"]));
    queueMessageForSend(response);
}

void ApplicationGateway::processMessage(GCreateBlueprintMessage* message)
{
    const std::shared_ptr<SceneObject>& so = m_engine->scenario()->scene().getSceneObject(message->getSceneObjectId());
    so->createBlueprint();
}

void ApplicationGateway::processMessage(GRequestBlueprintsDataMessage* message)
{
    GBlueprintsDataMessage* response = m_garbageCollector.createCollectedMessage<GBlueprintsDataMessage>();
    json blueprintsJson = m_engine->getScenarioJson()["blueprints"];
    response->setBlueprintsJsonBytes(GJson::ToBytes(blueprintsJson));
    queueMessageForSend(response);
}

void ApplicationGateway::processMessage(GModifyBlueprintMessage* message)
{
    std::vector<Blueprint>& blueprints = m_engine->scenario()->blueprints();
    auto iter = std::find_if(blueprints.begin(), blueprints.end(),
        [&](const Blueprint& blueprint) {
            return blueprint.getUuid() == message->getUuid();
        }
    );
    assert(iter != blueprints.end() && "Failed to find blueprint");
    Blueprint& bp = *iter;

    if (!message->getNewUuid().isNull()) {
        bp.setUuid(message->getNewUuid());
    }
    if (!message->getNewName().isEmpty()) {
        bp.setName(message->getNewName().c_str());
    }
}

void ApplicationGateway::processMessage(GAddShaderPresetMessage* message)
{
    bool created;
    m_engine->scenario()->settings().getShaderPreset(Uuid::UniqueName("preset_"), created);
    if (!created) {
        Logger::Throw("Error, resource not created");
    }
}

void ApplicationGateway::processMessage(GRemoveShaderPresetMessage* message)
{
    m_engine->scenario()->settings().removeShaderPreset(message->getName().c_str());

}

void ApplicationGateway::processMessage(GReorderRenderLayersMessage* message)
{
    m_engine->scenario()->settings().renderLayers().setLayerOrder(message->getId(), message->getNewOrder());

    GScenarioJsonMessage* response = m_garbageCollector.createCollectedMessage<GScenarioJsonMessage>();
    json scenarioJson = m_engine->getScenarioJson();
    response->setJsonBytes(GJson::ToBytes(scenarioJson));
    queueMessageForSend(response);
}

void ApplicationGateway::processMessage(GAddRenderLayerMessage* message)
{
    /// @todo Remove this if safe, should be
    //QMutex& mutex = static_cast<GLWidget*>(m_engine->widgetManager()->mainGLWidget())->renderer()->drawMutex();
    //QMutexLocker lock(&mutex);

    // Add sorting layer
    m_engine->scenario()->settings().renderLayers().addLayer();

    GScenarioJsonMessage* response = m_garbageCollector.createCollectedMessage<GScenarioJsonMessage>();
    json scenarioJson = m_engine->getScenarioJson();
    response->setJsonBytes(GJson::ToBytes(scenarioJson));
    queueMessageForSend(response);
}

void ApplicationGateway::processMessage(GRemoveRenderLayerMessage* message)
{
    /// @todo Remove this if safe, should be
    //QMutex& mutex = static_cast<GLWidget*>(m_engine->widgetManager()->mainGLWidget())->renderer()->drawMutex();
    //QMutexLocker lock(&mutex);

    // Remove sorting layer
    m_engine->scenario()->settings().renderLayers().removeLayer(
        message->getId(),
        [this](uint32_t layerId) {
            m_engine->scenario()->settings().onRemoveRenderLayer(layerId);
        });

    GScenarioJsonMessage* response = m_garbageCollector.createCollectedMessage<GScenarioJsonMessage>();
    json scenarioJson = m_engine->getScenarioJson();
    response->setJsonBytes(GJson::ToBytes(scenarioJson));
    queueMessageForSend(response);
}

void ApplicationGateway::processMessage(GReorderScriptProcessesMessage* message)
{
    m_engine->processManager()->processQueue().sortingLayers().setLayerOrder(message->getId(), message->getNewOrder());
    m_engine->processManager()->processQueue().refreshProcessOrder();

    GScenarioJsonMessage* response = m_garbageCollector.createCollectedMessage<GScenarioJsonMessage>();
    json scenarioJson = m_engine->getScenarioJson();
    response->setJsonBytes(GJson::ToBytes(scenarioJson));
    queueMessageForSend(response);
}

void ApplicationGateway::processMessage(GAddScriptProcessLayerMessage* message)
{
    // Add sorting layer
    m_engine->processManager()->processQueue().sortingLayers().addLayer();

    GScenarioJsonMessage* response = m_garbageCollector.createCollectedMessage<GScenarioJsonMessage>();
    json scenarioJson = m_engine->getScenarioJson();
    response->setJsonBytes(GJson::ToBytes(scenarioJson));
    queueMessageForSend(response);
}

void ApplicationGateway::processMessage(GRemoveScriptProcessLayerMessage* message)
{
    // Remove sorting layer
    m_engine->processManager()->processQueue().sortingLayers().removeLayer(message->getId(),
        [this](size_t layerId) {
            m_engine->processManager()->processQueue().onRemoveSortingLayer(layerId); 
        }
    );

    GScenarioJsonMessage* response = m_garbageCollector.createCollectedMessage<GScenarioJsonMessage>();
    json scenarioJson = m_engine->getScenarioJson();
    response->setJsonBytes(GJson::ToBytes(scenarioJson));
    queueMessageForSend(response);
}

} // End namespace