#pragma once

// External
#include "ripple/network/messages/GAddScenarioMessage.h"
#include "ripple/network/messages/GRestorePreviousScenarioMessage.h"
#include "ripple/network/messages/GAddSceneObjectMessage.h"
#include "ripple/network/messages/GRemoveSceneObjectMessage.h"
#include "ripple/network/messages/GReparentSceneObjectMessage.h"
#include "ripple/network/messages/GCopySceneObjectMessage.h"

#include "ripple/network/messages/GScenarioAddedMessage.h"
#include "ripple/network/messages/GSceneObjectAddedMessage.h"
#include "ripple/network/messages/GPreviousScenarioRestoredMessage.h"
#include "ripple/network/messages/GSceneObjectRemovedMessage.h"
#include "ripple/network/messages/GSceneObjectCopiedMessage.h"

#include "ripple/network/messages/GRenameScenarioMessage.h"
#include "ripple/network/messages/GRenameSceneMessage.h"
#include "ripple/network/messages/GRenameSceneObjectMessage.h"

// Internal
#include "geppetto/qt/actions/commands/GUndoCommand.h"
#include "geppetto/qt/widgets/tree/GSceneTreeWidget.h"

namespace rev {

class WidgetManager;
class SceneTreeWidget;


/// @class SceneCommand
/// @brief Abstract representing a scene-related command
class SceneCommand : public UndoCommand {
public:
    /// @name Constructors/Destructor
    /// @{
    SceneCommand(WidgetManager* manager, const GString &text, QUndoCommand *parent = nullptr);
    SceneCommand(WidgetManager* manager, QUndoCommand *parent = nullptr);
    ~SceneCommand();
    /// @}
};

/// @class AddScenarioCommand
class AddScenarioCommand : public SceneCommand {
public:
    /// @name Constructors/Destructor
    /// @{
    AddScenarioCommand(WidgetManager* manager, const GString &text, QUndoCommand *parent = nullptr);
    AddScenarioCommand(WidgetManager* manager, QUndoCommand *parent = nullptr);
    ~AddScenarioCommand();
    /// @}

    /// @name Public Methods
    /// @{

    virtual ECommandType undoCommandType() const { return UndoCommand::eAddScenario; }

    /// @brief Redoes the add scenario command
    void redo() override;

    /// @brief Undoes the add scenario command
    void undo() override;

    /// @}
protected:
    /// @name Protected members
    /// @{

    json m_lastScenario; ///< The JSON for the scenario overwritten by the newly created scenario
    GString m_lastScenarioPath;

    GAddScenarioMessage m_addScenarioMessage; ///< The message for adding a scenario
    GRestorePreviousScenarioMessage m_restorePreviousScenarioMessage; ///< The message for restoring a scenario

    /// @}
    
};

/// @class AddSceneObjectCommand
class AddSceneObjectCommand : public SceneCommand {
public:
    /// @name Constructors/Destructor
    /// @{
    AddSceneObjectCommand(WidgetManager* manager, const GString &text, Int32_t parentObjectId = -1, QUndoCommand *parent = nullptr);
    AddSceneObjectCommand(WidgetManager* manager, const GString& text, const json& sceneObjectJson, Int32_t parentObjectId = -1, QUndoCommand* parent = nullptr);
    AddSceneObjectCommand(WidgetManager* manager, Int32_t parentObjectId = -1, QUndoCommand *parent = nullptr);
    ~AddSceneObjectCommand();
    /// @}

    /// @name Public Methods
    /// @{

    virtual ECommandType undoCommandType() const { return UndoCommand::eAddSceneObject; }

    /// @brief Redoes the add scenario command
    void redo() override;

    /// @brief Undoes the add scenario command
    void undo() override;

    /// @}

    /// @name Members
    /// @{

    Int32_t m_objectId{ -1 }; ///< The ID of the scene object added by this command
    Int32_t m_parentSceneObjectId{ -1 }; ///< The ID of the parent to the scene object added by this command

    GAddSceneObjectMessage m_addSceneObjectMessage; ///< Message for adding a scene object
    GRemoveSceneObjectMessage m_removeSceneObjectMessage; ///< Message for removing a scene object

    /// @}
};

/// @class CopySceneObjectCommand
class CopySceneObjectCommand : public SceneCommand {
public:
    /// @name Constructors/Destructor
    /// @{
    CopySceneObjectCommand(WidgetManager* manager, Uint32_t sceneObjectId, int childIndex,
        const GString &text,
        QUndoCommand *parent = nullptr);
    ~CopySceneObjectCommand();
    /// @}

    /// @name Public Methods
    /// @{

    virtual ECommandType undoCommandType() const { return UndoCommand::eCopySceneObject; }

    /// @brief Redoes the add scenario command
    void redo() override;

    /// @brief Undoes the add scenario command
    void undo() override;

    /// @}

    /// @name members
    /// @{

    Int32_t m_sceneObjectId{ -1 }; ///< The ID of the object to be copied
    json m_sceneObjectCopyJson; ///< The json value representing the copy created by this command
    Int32_t m_childIndex{ -1 }; ///< The index at which the scene object widget is to be inserted under its parent

    GCopySceneObjectMessage m_copySceneObjectMessage; ///< Message for copying a scene object
    GRemoveSceneObjectMessage m_removeSceneObjectMessage; ///< Message for removing a scene object

    /// @}
};

/// @class RemoveSceneObjectCommand
class RemoveSceneObjectCommand : public SceneCommand {
public:
    /// @name Constructors/Destructor
    /// @{
    RemoveSceneObjectCommand(WidgetManager* manager, Uint32_t sceneObjectId, const GString &text);
    RemoveSceneObjectCommand(WidgetManager* manager, Uint32_t sceneObjectId);
    ~RemoveSceneObjectCommand();
    /// @}

    /// @name Public Methods
    /// @{

    virtual ECommandType undoCommandType() const { return UndoCommand::eRemoveSceneObject; }

    /// @brief Redoes the remove scenario command
    void redo() override;

    /// @brief Undoes the remove scenario command
    void undo() override;

    /// @}

    /// @name members
    /// @{

    Uint32_t m_sceneObjectId; ///< The scene object removed by this command
    json m_sceneObjectJson; ///< The json value representing the scene object removed by this command

    GAddSceneObjectMessage m_addSceneObjectMessage; ///< Message for adding a scene object
    GRemoveSceneObjectMessage m_removeSceneObjectMessage; ///< Message for removing a scene object

    /// @}
};

/// @class ChangeNameCommand
class ChangeNameCommand : public UndoCommand {
public:
    /// @name Constructors/Destructor
    /// @{
    ChangeNameCommand(WidgetManager* manager, const GString& name, const GString& previousName, SceneRelatedItem::SceneType sceneType);
    ~ChangeNameCommand();
    /// @}

    /// @name Public Methods
    /// @{

    virtual ECommandType undoCommandType() const { return UndoCommand::eChangeSceneRelatedName; }

    /// @brief Redoes the add scenario command
    void redo() override;

    /// @brief Undoes the add scenario command
    void undo() override;

    /// @}

protected:

    /// @name Protected members
    /// @{

    SceneRelatedItem::SceneType m_sceneType; ///< The type of scene-related object
    GString m_name; ///< Name to set for the object
    GString m_previousName; ///< Previous name of the object

    GRenameScenarioMessage m_renameScenarioMessage;
    GRenameSceneMessage m_renameSceneMessage;
    GRenameSceneObjectMessage m_renameSceneObjectMessage;

    /// @}
};


/// @class ReparentCommand
class ReparentSceneObjectCommand : public UndoCommand {
public:
    /// @name Constructors/Destructor
    /// @{
    ReparentSceneObjectCommand(WidgetManager* manager, 
        Uint32_t sceneObjectId,
        Int32_t previousParentId, 
        Int32_t newParentId,
        int childIndex);
    ~ReparentSceneObjectCommand();
    /// @}

    /// @name Public Methods
    /// @{

    virtual ECommandType undoCommandType() const { return UndoCommand::eRemoveSceneObject; }

    /// @brief Redoes the add scenario command
    void redo() override;

    /// @brief Undoes the add scenario command
    void undo() override;

    /// @}

protected:
    /// @name Protected methods
    /// @{

    /// @brief Handle reparenting of widget component
    void reparentWidgetItem(Int32_t newParentId);

    /// @}

    /// @name Protected members
    /// @{

    GReparentSceneObjectMessage m_reparentMessage; ///< Message to reparent the scene object

    Int32_t m_previousParentId{ -1 }; ///< ID of the previous parent
    Int32_t m_newParentId{ -1 }; ///< ID of the previous parent
    Uint32_t m_sceneObjectId{ 0 }; ///< ID of the scene object being reparented
    int m_childIndex{ -1 }; ///< Index at which to add as tree widget child

    /// @}
};

} // End namespaces
