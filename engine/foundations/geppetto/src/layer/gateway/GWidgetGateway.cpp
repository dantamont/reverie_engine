#include "geppetto/layer/gateway/GWidgetGateway.h"

#include "fortress/math/GMath.h"
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

#include "ripple/network/messages/GScenarioModifiedMessage.h"
#include "ripple/network/messages/GSceneObjectAddedMessage.h"
#include "ripple/network/messages/GSceneObjectSelectedMessage.h"
#include "ripple/network/messages/GSceneObjectRemovedMessage.h"
#include "ripple/network/messages/GSceneObjectCopiedMessage.h"
#include "ripple/network/messages/GGetScenarioJsonMessage.h"
#include "ripple/network/messages/GScenarioJsonMessage.h"
#include "ripple/network/messages/GAddSceneObjectRenderLayerMessage.h"
#include "ripple/network/messages/GRemoveSceneObjectRenderLayerMessage.h"
#include "ripple/network/messages/GOnSceneComponentAddedMessage.h"
#include "ripple/network/messages/GOnSceneComponentRemovedMessage.h"
#include "ripple/network/messages/GOnUpdateJsonMessage.h"
#include "ripple/network/messages/GRenderSettingsInfoMessage.h"
#include "ripple/network/messages/GAnimationStateMachinesMessage.h"
#include "ripple/network/messages/GAnimationComponentDataMessage.h"
#include "ripple/network/messages/GAnimationDataMessage.h"
#include "ripple/network/messages/GCubemapsDataMessage.h"
#include "ripple/network/messages/GModelDataMessage.h"
#include "ripple/network/messages/GCanvasComponentDataMessage.h"
#include "ripple/network/messages/GAudioResourceDataMessage.h"
#include "ripple/network/messages/GAudioResourcesDataMessage.h"
#include "ripple/network/messages/GResourcesDataMessage.h"

#include "ripple/network/messages/GResourceAddedMessage.h"
#include "ripple/network/messages/GResourceModifiedMessage.h"
#include "ripple/network/messages/GResourceRemovedMessage.h"

#include "ripple/network/messages/GDisplayWarningMessage.h"
#include "ripple/network/messages/GResourceDataMessage.h"
#include "ripple/network/messages/GBlueprintsDataMessage.h"
#include "ripple/network/messages/GAdvanceProgressWidgetMessage.h"
#include "ripple/network/messages/GRequestAddTexturesToMaterialMessage.h"


namespace rev {

/// @todo Replace with static function
#define FULL_DELETE_RESOURCE Flags<ResourceDeleteFlag>(ResourceDeleteFlag::kDeleteHandle | ResourceDeleteFlag::kForce )

WidgetGateway::WidgetGateway(WidgetManager* widgetManager):
    m_widgetManager(widgetManager)
{
}

void WidgetGateway::processMessages()
{
    std::vector<TransportMessageInterface*> messages;
    while (TransportMessageInterface* msg = m_mailbox.retrieve()) {
        GMessage* message = static_cast<GMessage*>(msg);
        processMessage(message);
        messages.push_back(message);
    }

    for (Uint32_t i = 0; i < messages.size(); i++) {
        delete messages[i];
    }

    // Perform garbage collection on sent messages that are no longer in use
    m_garbageCollector.deleteStaleMessages();
}

void WidgetGateway::processMessage(GPlaybackDataMessage* message)
{
    m_widgetManager->playbackWidget()->processMessage(message);
}

void WidgetGateway::processMessage(GSceneObjectAddedMessage* message)
{
    // Create tree widget item for scene object
    json sceneObjectJson = GJson::FromBytes(message->getJsonBytes());
    m_widgetManager->sceneTreeWidget()->addSceneObjectTreeItem(sceneObjectJson);

    // Send ID to command that added scene object
    /// @todo This assumes that most recent command hasn't changed since the scene object was added,
    /// use a unique ID instead
    UndoCommand* command = m_widgetManager->actionManager()->getAction(message->getCommandId());
    if (command->undoCommandType() == UndoCommand::eAddSceneObject) {
        AddSceneObjectCommand* action = static_cast<AddSceneObjectCommand*>(command);
        action->m_objectId = sceneObjectJson["id"];
    }

    // Queue message for new scenario JSON to update tree widget
    static GGetScenarioJsonMessage s_message;
    copyAndQueueMessageForSend(s_message);
}

void WidgetGateway::processMessage(GSceneObjectRemovedMessage* message)
{
    json sceneObjectJson = GJson::FromBytes(message->getJsonBytes());
    Uint32_t id = sceneObjectJson["id"];

    // Remove from tree widget selection
    if (id == m_widgetManager->sceneTreeWidget()->getCurrentSceneObjectId()) {
        m_widgetManager->sceneTreeWidget()->clearSelectedItems();
    }

    // Send JSON to command that removed scene object
    UndoCommand* command = m_widgetManager->actionManager()->getAction(message->getCommandId());
    if (command->undoCommandType() == UndoCommand::eRemoveSceneObject) {
        RemoveSceneObjectCommand* action = static_cast<RemoveSceneObjectCommand*>(command);
        action->m_sceneObjectJson = sceneObjectJson;
        action->m_addSceneObjectMessage.setJsonBytes(GJson::ToBytes(sceneObjectJson));
    }
}

void WidgetGateway::processMessage(GSceneObjectCopiedMessage* message)
{
    UndoCommand* command = m_widgetManager->actionManager()->getAction(message->getCommandId());

    CopySceneObjectCommand* action = static_cast<CopySceneObjectCommand*>(command);
    action->m_sceneObjectCopyJson = GJson::FromBytes(message->getJsonBytes());

    // Create tree widget item for scene object
    m_widgetManager->sceneTreeWidget()->addSceneObjectTreeItem(action->m_sceneObjectCopyJson, 
        action->m_childIndex);
}

void WidgetGateway::processMessage(GScenarioModifiedMessage* message)
{
    // Clear widgets on scenario modification
    json j = GJson::FromBytes(message->getJsonBytes());
    m_widgetManager->sceneTreeWidget()->setScenarioTreeItem(j);
    m_widgetManager->componentWidget()->clear();
}

void WidgetGateway::processMessage(GSceneObjectSelectedMessage* message)
{
    Int32_t sceneObjectId = message->getSceneObjectId();
    SceneRelatedItem* item = m_widgetManager->sceneTreeWidget()->getSceneObjectItem(sceneObjectId);
    m_widgetManager->sceneTreeWidget()->setCurrentItem(item);
    m_widgetManager->sceneTreeWidget()->m_selectedSceneObjectSignal.emitForAll(sceneObjectId);
}

void WidgetGateway::processMessage(GOnSceneComponentAddedMessage* message)
{    
    // Send component ID to command that removed scene object
    UndoCommand* command = m_widgetManager->actionManager()->getAction(message->getCommandId());
    std::vector<Uuid> componentDependencyIds;
    std::vector<json> componentDependenciesAdded;
    if (command->undoCommandType() == UndoCommand::eAddComponent) {
        // Set component dependencies so they are removed on undo
        AddComponentCommand* action = static_cast<AddComponentCommand*>(command);
        componentDependencyIds = message->getComponentDependencyIds();
        componentDependenciesAdded = message->getComponentDependencies();
        action->m_removeComponentMessage.setComponentDependencyIds(componentDependencyIds);
        action->m_removeComponentMessage.setComponentDependencies(componentDependenciesAdded);
    }
    else if (command->undoCommandType() == UndoCommand::eRemoveComponent) {
        DeleteComponentCommand* action = static_cast<DeleteComponentCommand*>(command);
        action->m_addedComponentId = message->getComponentId();
    }

    // Create tree widget item for the component
    if (ESceneType::eSceneObject == message->getSceneType())
    {
        /// Scene object
        for (Uint32_t i = 0; i < componentDependenciesAdded.size(); i++) {
            // Make widgets for added component dependencies
            m_widgetManager->componentWidget()->addItem(GSceneType(ESceneType::eSceneObject), componentDependenciesAdded[i]);
        }

        // Make widget for main component added
        m_widgetManager->componentWidget()->addItem(GSceneType(ESceneType::eSceneObject), message->getJsonBytes());
    }

}

void WidgetGateway::processMessage(GOnUpdateJsonMessage* message)
{
    // Update the parameter widget with a matching Uuid
    json j = GJson::FromBytes(message->getJsonBytes());
    Uuid widgetId = j["widgetId"];
    m_widgetManager->parameterWidget(widgetId)->update(message);
}

void WidgetGateway::processMessage(GScenarioJsonMessage* message)
{
    json metadata = GJson::FromBytes(message->getMetadataJsonBytes());
    json scenarioJson = GJson::FromBytes(message->getJsonBytes());

    // Update scenario json in widget manager
    m_widgetManager->setScenarioJson(scenarioJson);

    if (metadata.contains("widgetType")) {
        GWidgetType widgetType = static_cast<GWidgetType>(metadata["widgetType"].get<Int32_t>());
        GWidgetActionType widgetActionType = static_cast<GWidgetActionType>(metadata["widgetAction"].get<Int32_t>());
        switch ((EWidgetType)widgetType) {
        case EWidgetType::eComponentTree:
        {
            switch ((EWidgetActionType)widgetActionType) {
            case EWidgetActionType::eSelectScene:
            {
                const std::string& sceneName = metadata["sceneName"].get_ref<const std::string&>();
                m_widgetManager->componentWidget()->onSelectScene(sceneName);
                break;
            }
            case EWidgetActionType::eSelectSceneObject:
            {
                Uint32_t sceneObjectId = metadata["sceneObjectId"].get<Uint32_t>();
                m_widgetManager->componentWidget()->onSelectSceneObject(sceneObjectId);
                break;
            }
            default:
                assert(false && "Unrecognized widget action");
            }
            break;
        }
        case EWidgetType::ePhysicsShapePrefab:
        {
            // Update the physics shape widget requested by the metadata
            PhysicsShapeWidget* widget = nullptr;
            QTreeWidgetItemIterator it(m_widgetManager->componentWidget());
            while (*it) {
                if (ComponentItem* componentItem = dynamic_cast<ComponentItem*>(*it)) {
                    // Check if physics widget
                    if (componentItem->componentType() == (Int32_t)ESceneObjectComponentType::eRigidBody) {
                        RigidBodyWidget* rigidBodyWidget = dynamic_cast<RigidBodyWidget*>(componentItem->componentWidget());
                        rigidBodyWidget->shapeWidget()->repopulatePrefabs();
                        rigidBodyWidget->shapeWidget()->repopulateMaterials();
                        break;
                    }
                }
                ++it;
            }
            break;
        }
        default:
            assert(false && "Unrecognized widget type");
        }
    }

    emit m_widgetManager->receivedScenarioJsonMessage(*message);
}

void WidgetGateway::processMessage(GTransformMessage* message)
{
    emit m_widgetManager->receivedTransformMessage(*message);
}

void WidgetGateway::processMessage(GRenderSettingsInfoMessage* message)
{
    // Retrieve light widget, which is currently the only widget that needs render settings
    QTreeWidgetItemIterator it(m_widgetManager->componentWidget());
    while (*it) {
        if (ComponentItem* componentItem = dynamic_cast<ComponentItem*>(*it)) {
            // Check if light component widget
            if (componentItem->componentType() == (Int32_t)ESceneObjectComponentType::eLight) {
                LightComponentWidget* lightWidget = dynamic_cast<LightComponentWidget*>(componentItem->componentWidget());
                lightWidget->update(*message);
                break;
            }
        }
        ++it;
    }
}

void WidgetGateway::processMessage(GAnimationStateMachinesMessage* message)
{
    // Retrieve animation widget, which is currently the only widget that needs animation state settings
    QTreeWidgetItemIterator it(m_widgetManager->componentWidget());
    while (*it) {
        if (ComponentItem* componentItem = dynamic_cast<ComponentItem*>(*it)) {
            if (componentItem->componentType() == (Int32_t)ESceneObjectComponentType::eBoneAnimation) {
                AnimationComponentWidget* animWidget = dynamic_cast<AnimationComponentWidget*>(componentItem->componentWidget());
                animWidget->update(*message);
                break;
            }
        }
        ++it;
    }
}

void WidgetGateway::processMessage(GAnimationComponentDataMessage* message)
{
    // Retrieve animation widget, which is currently the only widget that needs animation state settings
    QTreeWidgetItemIterator it(m_widgetManager->componentWidget());
    while (*it) {
        if (ComponentItem* componentItem = dynamic_cast<ComponentItem*>(*it)) {
            if (componentItem->componentType() == (Int32_t)ESceneObjectComponentType::eBoneAnimation) {
                AnimationComponentWidget* animWidget = dynamic_cast<AnimationComponentWidget*>(componentItem->componentWidget());
                animWidget->nodeWidget()->update(*message);
                break;
            }
        }
        ++it;
    }
}

void WidgetGateway::processMessage(GAnimationDataMessage* message)
{
    // Retrieve animation widget, which is currently the only widget that needs animation settings
    QTreeWidgetItemIterator it(m_widgetManager->componentWidget());
    while (*it) {
        if (ComponentItem* componentItem = dynamic_cast<ComponentItem*>(*it)) {
            if (componentItem->componentType() == (Int32_t)ESceneObjectComponentType::eBoneAnimation) {
                AnimationComponentWidget* animWidget = dynamic_cast<AnimationComponentWidget*>(componentItem->componentWidget());
                animWidget->nodeWidget()->update(*message);
                break;
            }
        }
        ++it;
    }
}

void WidgetGateway::processMessage(GCubemapsDataMessage* message)
{
    emit m_widgetManager->receivedCubemapsDataMessage(*message);
}

void WidgetGateway::processMessage(GModelDataMessage* message)
{
    emit m_widgetManager->receivedModelDataMessage(*message);
}

void WidgetGateway::processMessage(GCanvasComponentDataMessage* message)
{
    emit m_widgetManager->receivedCanvasDataMessage(*message);
}

void WidgetGateway::processMessage(GAudioResourceDataMessage* message)
{
    emit m_widgetManager->receivedAudioResourceDataMessage(*message);
}

void WidgetGateway::processMessage(GAudioResourcesDataMessage* message)
{
    emit m_widgetManager->receivedAudioResourcesDataMessage(*message);
}

void WidgetGateway::processMessage(GResourcesDataMessage* message)
{
    // Update resources JSON in widget manager
    json scenarioJson = m_widgetManager->scenarioJson();
    scenarioJson["resourceCache"]["resources"] = GJson::FromBytes(message->getResourcesJsonBytes());
    m_widgetManager->setScenarioJson(scenarioJson);
    emit m_widgetManager->receivedResourcesDataMessage(*message);
}

void WidgetGateway::processMessage(GResourceAddedMessage* message)
{
    emit m_widgetManager->receivedResourceAddedMessage(*message);
}

void WidgetGateway::processMessage(GResourceModifiedMessage* message)
{
    emit m_widgetManager->receivedResourceModifiedMessage(*message);
}

void WidgetGateway::processMessage(GResourceRemovedMessage* message)
{
    emit m_widgetManager->receivedResourceRemovedMessage(*message);
}

void WidgetGateway::processMessage(GDisplayWarningMessage* message)
{
    GString title = message->getTitle().data();
    GString warning = message->getText().data();
    QMessageBox::warning(nullptr, title.c_str(), warning.c_str());
}

void WidgetGateway::processMessage(GResourceDataMessage* message)
{
    emit m_widgetManager->receivedResourceDataMessage(*message);
}

void WidgetGateway::processMessage(GBlueprintsDataMessage* message)
{
    emit m_widgetManager->receivedBlueprintsDataMessage(*message);
}

void WidgetGateway::processMessage(GAdvanceProgressWidgetMessage* message)
{
    emit m_widgetManager->receivedAdvanceProgressWidgetMessage(*message);
}

void WidgetGateway::processMessage(GRequestAddTexturesToMaterialMessage* message)
{
    emit m_widgetManager->receivedRequestAddTexturesToMaterialMessage(*message);
}


} // End namespace