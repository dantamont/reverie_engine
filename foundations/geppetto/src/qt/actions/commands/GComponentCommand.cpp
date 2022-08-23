#include "geppetto/qt/actions/commands/GComponentCommand.h"

#include "ripple/network/gateway/GMessageGateway.h"

#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/tree/GComponentTreeWidget.h"
#include "geppetto/qt/widgets/tree/GSceneTreeWidget.h"

namespace rev {


ComponentCommand::ComponentCommand(WidgetManager* widgetManager, 
    GSceneType sceneType,
    const json& json, const QString& text, QUndoCommand* parent) :
    UndoCommand(widgetManager, text, parent),
    m_sceneType(sceneType),
    m_componentJson(json)
{
}

ComponentCommand::~ComponentCommand()
{
}


AddComponentCommand::AddComponentCommand(WidgetManager* widgetManager, Int32_t sceneObjectId, const json& json, const QString& text, QUndoCommand* parent):
    ComponentCommand(widgetManager, GSceneType(ESceneType::eSceneObject), json, text, parent),
    m_sceneObjectId(sceneObjectId)
{
    m_addComponentMessage.setCommandId(getId());
    m_addComponentMessage.setJsonBytes(GJson::ToBytes(json));
    m_addComponentMessage.setSceneType(GSceneType(ESceneType::eSceneObject));
    m_addComponentMessage.setSceneObjectId(sceneObjectId);

    m_removeComponentMessage.setCommandId(getId());
    m_removeComponentMessage.setJsonBytes(GJson::ToBytes(json));
    m_removeComponentMessage.setSceneType(GSceneType(ESceneType::eSceneObject));
    m_removeComponentMessage.setSceneObjectId(sceneObjectId);
}

AddComponentCommand::AddComponentCommand(WidgetManager* widgetManager, Int32_t sceneObjectId, GSceneObjectComponentType componentType, const QString& text, QUndoCommand* parent):
    ComponentCommand(widgetManager, GSceneType(ESceneType::eSceneObject), json{ (Int32_t)componentType }, text, parent),
    m_sceneObjectId(sceneObjectId)
{
    m_addComponentMessage.setCommandId(getId());
    m_addComponentMessage.setComponentType(componentType);
    m_addComponentMessage.setSceneType(GSceneType(ESceneType::eSceneObject));
    m_addComponentMessage.setSceneObjectId(sceneObjectId);

    m_removeComponentMessage.setCommandId(getId());
    m_removeComponentMessage.setSceneType(GSceneType(ESceneType::eSceneObject));
    m_removeComponentMessage.setSceneObjectId(sceneObjectId);
}

AddComponentCommand::AddComponentCommand(WidgetManager* widgetManager, GSceneComponentType componentType, const QString& text, QUndoCommand* parent):
    ComponentCommand(widgetManager, GSceneType(ESceneType::eScene), json{ (Int32_t)componentType }, text, parent)
{
    m_addComponentMessage.setCommandId(getId());
    m_addComponentMessage.setComponentType(componentType);
    m_addComponentMessage.setSceneType(GSceneType(ESceneType::eScene));

    m_removeComponentMessage.setCommandId(getId());
    m_removeComponentMessage.setSceneType(GSceneType(ESceneType::eScene));
}

void AddComponentCommand::redo()
{
    // Send message to add component
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_addComponentMessage);

    if (ESceneType::eScene == m_sceneType)
    {
        // Scene
        m_widgetManager->componentWidget()->addItem(GSceneType(ESceneType::eScene), m_componentJson);
    }
}

void AddComponentCommand::undo()
{
    // Send message to remove component
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_removeComponentMessage);

    // Send signal to remove tree widget item for the script
    if (ESceneType::eSceneObject == m_sceneType)
    {
        // Scene object
        Int32_t currentSceneObjectItem = m_widgetManager->sceneTreeWidget()->getCurrentSceneObjectId();
        m_widgetManager->componentWidget()->selectSceneObject(currentSceneObjectItem);
    }
    else {
        // Scene
        Uuid componentID = m_componentJson["id"];
        if (m_widgetManager->sceneTreeWidget()->getCurrentSceneId() > -1) {
            m_widgetManager->componentWidget()->removeItem(componentID);
        }
    }
}



DeleteComponentCommand::DeleteComponentCommand(WidgetManager* widgetManager, Int32_t sceneObjectId, const json& json, const QString& text, QUndoCommand* parent):
    ComponentCommand(widgetManager, GSceneType(ESceneType::eSceneObject), json, text, parent)
{
    m_addComponentMessage.setCommandId(getId());
    m_addComponentMessage.setJsonBytes(GJson::ToBytes(json));
    if (sceneObjectId >= 0) {
        m_addComponentMessage.setSceneType(GSceneType(ESceneType::eSceneObject));
        m_addComponentMessage.setSceneObjectId(sceneObjectId);
    }
    else {
        m_addComponentMessage.setSceneType(GSceneType(ESceneType::eScene));
    }

    m_removeComponentMessage.setCommandId(getId());
    m_removeComponentMessage.setJsonBytes(GJson::ToBytes(json));
    if (sceneObjectId >= 0) {
        m_removeComponentMessage.setSceneType(GSceneType(ESceneType::eSceneObject));
        m_removeComponentMessage.setSceneObjectId(sceneObjectId);
    }
    else {
        m_removeComponentMessage.setSceneType(GSceneType(ESceneType::eScene));
    }
}

void DeleteComponentCommand::redo()
{
    // Send message to remove component
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_removeComponentMessage);

    // Send signal to remove tree widget item for the script
    if (ESceneType::eSceneObject == m_sceneType)
    {
        // Scene object
        Int32_t currentSceneObjectItem = m_widgetManager->sceneTreeWidget()->getCurrentSceneObjectId();
        m_widgetManager->componentWidget()->selectSceneObject(currentSceneObjectItem);
    }
    else {
        // Scene
        Uuid componentID = m_componentJson["id"];
        if (m_widgetManager->sceneTreeWidget()->getCurrentSceneId() > -1) {
            m_widgetManager->componentWidget()->removeItem(componentID);
        }
    }
}

void DeleteComponentCommand::undo()
{
    // Send message to add component
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_addComponentMessage);
}




} // End namespaces