#include "geppetto/qt/actions/commands/GSceneCommand.h"

#include <QMessageBox>

#include "ripple/network/gateway/GMessageGateway.h"

#include "geppetto/qt/actions/GActionManager.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/tree/GSceneTreeWidget.h"

#include "fortress/json/GJson.h"

namespace rev {



SceneCommand::SceneCommand(WidgetManager* manager, const GString & text, QUndoCommand * parent):
    UndoCommand(manager, text.c_str(), parent)
{
}

SceneCommand::SceneCommand(WidgetManager* manager, QUndoCommand * parent) :
    UndoCommand(manager, parent)
{
}

SceneCommand::~SceneCommand()
{
}



AddScenarioCommand::AddScenarioCommand(WidgetManager* manager, const GString & text, QUndoCommand * parent) :
    SceneCommand(manager, text, parent)
{
}

AddScenarioCommand::AddScenarioCommand(WidgetManager* manager, QUndoCommand * parent):
    SceneCommand(manager, parent)
{
}

AddScenarioCommand::~AddScenarioCommand()
{
}

void AddScenarioCommand::redo()
{
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_addScenarioMessage);
}

void AddScenarioCommand::undo()
{
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_restorePreviousScenarioMessage);
}



// AddSceneObject
AddSceneObjectCommand::AddSceneObjectCommand(WidgetManager* manager, const GString& text, Int32_t parentObjectId, QUndoCommand* parent) :
    SceneCommand(manager, text, parent)
{
    if (parentObjectId <= 0) {
        m_parentSceneObjectId = parentObjectId;
    }
    m_addSceneObjectMessage.setCommandId(m_id);
    m_removeSceneObjectMessage.setCommandId(m_id);
}

AddSceneObjectCommand::AddSceneObjectCommand(WidgetManager* manager, const GString& text, const json& sceneObjectJson, Int32_t parentObjectId, QUndoCommand* parent) :
    SceneCommand(manager, text, parent)
{
    if (parentObjectId <= 0) {
        m_parentSceneObjectId = parentObjectId;
    }
    m_addSceneObjectMessage.setCommandId(m_id);
    m_addSceneObjectMessage.setLoadFromJson(true);
    m_addSceneObjectMessage.setJsonBytes(GJson::ToBytes(sceneObjectJson));
    m_removeSceneObjectMessage.setCommandId(m_id);
}

AddSceneObjectCommand::AddSceneObjectCommand(WidgetManager* manager, Int32_t parentObjectId, QUndoCommand* parent) :
    SceneCommand(manager, parent)
{
    if (parentObjectId <= 0) {
        m_parentSceneObjectId = parentObjectId;
    }
    m_addSceneObjectMessage.setCommandId(m_id);
    m_removeSceneObjectMessage.setCommandId(m_id);
}

AddSceneObjectCommand::~AddSceneObjectCommand()
{
}

void AddSceneObjectCommand::redo()
{
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_addSceneObjectMessage);
}

void AddSceneObjectCommand::undo()
{
    // Object ID should be updated before this gets called
    assert(m_objectId >= 0 && "Invalid scene object ID");
    m_removeSceneObjectMessage.setSceneObjectId(m_objectId);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_removeSceneObjectMessage);
}




// RemoveSceneObjectCommand

RemoveSceneObjectCommand::RemoveSceneObjectCommand(WidgetManager* manager, Uint32_t sceneObjectId, const GString & text):
    SceneCommand(manager, text, nullptr),
    m_sceneObjectId(sceneObjectId)
{
    m_addSceneObjectMessage.setCommandId(m_id);
    m_removeSceneObjectMessage.setCommandId(m_id);
    m_removeSceneObjectMessage.setSceneObjectId(m_sceneObjectId);
}


RemoveSceneObjectCommand::RemoveSceneObjectCommand(WidgetManager* manager, Uint32_t sceneObjectId) :
    SceneCommand(manager, nullptr),
    m_sceneObjectId(sceneObjectId)
{
    m_addSceneObjectMessage.setCommandId(m_id);
    m_removeSceneObjectMessage.setCommandId(m_id);
    m_removeSceneObjectMessage.setSceneObjectId(m_sceneObjectId);
}

RemoveSceneObjectCommand::~RemoveSceneObjectCommand()
{
}

void RemoveSceneObjectCommand::redo()
{
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_removeSceneObjectMessage);
}

void RemoveSceneObjectCommand::undo()
{
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_addSceneObjectMessage);
}




CopySceneObjectCommand::CopySceneObjectCommand(WidgetManager* manager,
    Uint32_t sceneObjectId,
    int childIndex,
    const GString& text, QUndoCommand* parent) :
    SceneCommand(manager, text, parent),
    m_sceneObjectId(m_sceneObjectId),
    m_childIndex(childIndex)
{
    m_copySceneObjectMessage.setSceneObjectId(sceneObjectId);
}

CopySceneObjectCommand::~CopySceneObjectCommand()
{
}

void CopySceneObjectCommand::redo()
{
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_copySceneObjectMessage);
}

void CopySceneObjectCommand::undo()
{
    /// @todo Set up response that sets m_sceneObjectCopyJson
    Uint32_t copyId = m_sceneObjectCopyJson["id"];
    m_removeSceneObjectMessage.setCommandId(getId());
    m_removeSceneObjectMessage.setSceneObjectId(copyId);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_removeSceneObjectMessage);

    /// @todo Might already be handled by removal callback, so commented out for now
    // Remove tree widget item for scene object
    //m_widgetManager->sceneTreeWidget()->removeTreeItem(m_sceneObjectCopyJson);
}



ChangeNameCommand::ChangeNameCommand(WidgetManager* manager,
    const GString& name, const GString& previousName,
    SceneRelatedItem::SceneType sceneType) :
    UndoCommand(manager, nullptr),
    m_name(name),
    m_previousName(previousName),
    m_sceneType(sceneType)
{
}

ChangeNameCommand::~ChangeNameCommand()
{
}

void ChangeNameCommand::redo()
{
    // Refresh text in the tree item
    SceneRelatedItem* treeItem =
        m_widgetManager->sceneTreeWidget()->getItem(m_sceneType, m_previousName);
    treeItem->data().set("name", m_name.c_str());
    treeItem->refreshText();

    switch (m_sceneType) {
    case SceneRelatedItem::kScenario:
        m_renameScenarioMessage.setPreviousName(m_previousName.c_str());
        m_renameScenarioMessage.setNewName(m_name.c_str());
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_renameScenarioMessage);
        break;
    case SceneRelatedItem::kScene:
        m_renameSceneMessage.setPreviousName(m_previousName.c_str());
        m_renameSceneMessage.setNewName(m_name.c_str());
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_renameSceneMessage);
        break;
    case SceneRelatedItem::kSceneObject:
        m_renameSceneObjectMessage.setPreviousName(m_previousName.c_str());
        m_renameSceneObjectMessage.setNewName(m_name.c_str());
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_renameSceneObjectMessage);
        break;
    default:
        assert(false && "oh no");
        break;
    }
}

void ChangeNameCommand::undo()
{
    // Refresh text in the tree item
    SceneRelatedItem* treeItem =
        m_widgetManager->sceneTreeWidget()->getItem(m_sceneType, m_name);
    treeItem->data().set("name", m_previousName.c_str());
    treeItem->refreshText();

    switch (m_sceneType) {
    case SceneRelatedItem::kScenario:
        m_renameScenarioMessage.setPreviousName(m_name.c_str());
        m_renameScenarioMessage.setNewName(m_previousName.c_str());
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_renameScenarioMessage);
        break;
    case SceneRelatedItem::kScene:
        m_renameSceneMessage.setPreviousName(m_name.c_str());
        m_renameSceneMessage.setNewName(m_previousName.c_str());
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_renameSceneMessage);
        break;
    case SceneRelatedItem::kSceneObject:
        m_renameSceneObjectMessage.setPreviousName(m_name.c_str());
        m_renameSceneObjectMessage.setNewName(m_previousName.c_str());
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_renameSceneObjectMessage);
        break;
    default:
        assert(false && "oh no");
        break;
    }
}


// ReparentCommand
ReparentSceneObjectCommand::ReparentSceneObjectCommand(WidgetManager* manager, 
    Uint32_t sceneObjectId,
    Int32_t previousParentId, 
    Int32_t newParentId,
    int childIndex) :
    UndoCommand(manager, nullptr),
    m_previousParentId(previousParentId),
    m_newParentId(newParentId),
    m_sceneObjectId(sceneObjectId),
    m_childIndex(childIndex)
{
    m_reparentMessage.setSceneObjectId(sceneObjectId);
}

ReparentSceneObjectCommand::~ReparentSceneObjectCommand()
{
}

void ReparentSceneObjectCommand::redo()
{
    // Reparent widget item
    reparentWidgetItem(m_newParentId);

    m_reparentMessage.setNewParentId(m_newParentId);
    m_reparentMessage.setPreviousParentId(m_previousParentId);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_reparentMessage);
}

void ReparentSceneObjectCommand::undo()
{
    // Reparent widget item
    reparentWidgetItem(m_previousParentId);

    m_reparentMessage.setNewParentId(m_previousParentId);
    m_reparentMessage.setPreviousParentId(m_newParentId);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_reparentMessage);
}

void ReparentSceneObjectCommand::reparentWidgetItem(Int32_t newParentId)
{
    // Get tree item for scene object
    auto* item = m_widgetManager->sceneTreeWidget()->getSceneObjectItem(m_sceneObjectId);
    SceneRelatedItem* oldParent = dynamic_cast<SceneRelatedItem*>(item->parent());

    // Remove from current parent in widget
    if (oldParent) {
        oldParent->removeChild(item);
    }

    // Add to new parent in tree widget
    QTreeWidgetItem* newParent;
    if (newParentId >= 0) {
        // If has a parent, make a child
        auto* newParent= m_widgetManager->sceneTreeWidget()->getSceneObjectItem(newParentId);

        // Add at correct widget index
        if (m_childIndex < 0) {
            // No child index specified
            newParent->addChild(item);
        }
        else {
            newParent->insertChild(m_childIndex, item);
        }

        if (newParent->parent()) {
            // Resize to fit additional parent levels
            m_widgetManager->sceneTreeWidget()->resizeColumns();
        }
    }
    else {
        // Else, add to scene level if wasn't already at it
        /// @todo Make it such that name string doesn't need to be passed for scene
        newParent = m_widgetManager->sceneTreeWidget()->getItem(SceneRelatedItem::kScene, "");

        if (m_childIndex < 0) {
            // No child index specified
            newParent->addChild(item);
        }
        else {
            newParent->insertChild(m_childIndex, item);
        }
        m_widgetManager->sceneTreeWidget()->resizeColumns();
    }
}


} // End namespaces