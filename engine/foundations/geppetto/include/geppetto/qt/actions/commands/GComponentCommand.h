#pragma once

// External
#include "enums/GSceneComponentTypeEnum.h"
#include "enums/GSceneObjectComponentTypeEnum.h"
#include "enums/GSceneTypeEnum.h"

#include "ripple/network/messages/GAddSceneComponentMessage.h"
#include "ripple/network/messages/GOnSceneComponentAddedMessage.h"
#include "ripple/network/messages/GRemoveSceneComponentMessage.h"
#include "ripple/network/messages/GOnSceneComponentRemovedMessage.h"

// Internal
#include "geppetto/qt/actions/commands/GUndoCommand.h"

namespace rev {

class ComponentTreeWidget;
class WidgetManager;

/// @class ComponentCommand
/// @brief Abstract representing a component-related command
class ComponentCommand : public UndoCommand {
public:

    /// @name Constructors/Destructor
    /// @{
    ComponentCommand(WidgetManager* widgetManager, GSceneType sceneType, const json& json, const QString &text, QUndoCommand *parent = nullptr);
    ~ComponentCommand();
    /// @}

protected:

    /// @name Protected members
    /// @{

    GSceneType m_sceneType; ///< The type of scene item (scene or scene object) housing the component
    json m_componentJson; ///< The JSON data held by this command, representing a component

    /// @}

};


/// @class AddSceneObjectComponent
class AddComponentCommand : public ComponentCommand {
public:
    /// @name Constructors/Destructor
    /// @{
    AddComponentCommand(WidgetManager* widgetManager, Int32_t sceneObjectId, const json& componentJson, const QString& text, QUndoCommand* parent = nullptr);
    AddComponentCommand(WidgetManager* widgetManager, Int32_t sceneObjectId, GSceneObjectComponentType componentType, const QString& text, QUndoCommand* parent = nullptr);
    AddComponentCommand(WidgetManager* widgetManager, GSceneComponentType componentType, const QString& text, QUndoCommand* parent = nullptr);

    ~AddComponentCommand() = default;
    /// @}

    /// @name Public Methods
    /// @{

    virtual ECommandType undoCommandType() const override { return ECommandType::eAddComponent; };

    /// @brief Redoes the add renderer command
    virtual void redo() override;

    /// @brief Undoes the add renderer command
    virtual void undo() override;

    /// @}

    /// @name Protected members
    /// @{

    Int32_t m_sceneObjectId{ -1 };
    Uuid m_addedComponentId;
    std::vector<Uuid> m_componentDependenciesAdded;
    GAddSceneComponentMessage m_addComponentMessage;
    GRemoveSceneComponentMessage m_removeComponentMessage;

    /// @}

};


/// @class DeleteCommand
/// @brief Delete a component
class DeleteComponentCommand : public ComponentCommand {
public:
    /// @name Constructors/Destructor
    /// @{

    DeleteComponentCommand(WidgetManager* widgetManager, Int32_t sceneObjectId, const json& json, const QString& text, QUndoCommand* parent = nullptr);
    ~DeleteComponentCommand() = default;
    
    /// @}

    /// @name Public Methods
    /// @{

    virtual ECommandType undoCommandType() const override { return ECommandType::eRemoveComponent; };

    /// @brief Redoes the add scenario command
    virtual void redo() override;

    /// @brief Undoes the add scenario command
    virtual void undo() override;

    /// @}

    /// @name Protected members
    /// @{

    Uuid m_addedComponentId;
    GAddSceneComponentMessage m_addComponentMessage;
    GRemoveSceneComponentMessage m_removeComponentMessage;

    /// @}
};

} // End namespaces
