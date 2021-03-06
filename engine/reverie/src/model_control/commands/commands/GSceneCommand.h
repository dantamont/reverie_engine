#ifndef GB_SCENE_COMMAND_H
#define GB_SCENE_COMMAND_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Qt

// Internal
#include "../GUndoCommand.h"
#include <view/tree/GSceneTreeWidget.h>

namespace rev {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class Object;
class CoreEngine;
class Scenario;
class Scene;
class SceneObject;
class Component;
namespace View {
class SceneTreeWidget;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class SceneCommand
/// @brief Abstract representing a scene-related command
class SceneCommand : public UndoCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static methods
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    SceneCommand(CoreEngine* core, const GString &text, QUndoCommand *parent = nullptr);
    SceneCommand(CoreEngine* core, QUndoCommand *parent = nullptr);
    ~SceneCommand();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the add scenario command
    void redo() override {}

    /// @brief Undoes the add scenario command
    void undo() override {}

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "SceneCommand"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "rev::SceneCommand"; }
    /// @}
protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief Obtain objects from their JSON representation
    SceneObject* getSceneObject(const QJsonValue& object) const;
    SceneObject* createSceneObject(const QJsonValue& scene, const QJsonValue& object) const;

    /// @}

};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class AddScenarioCommand
class AddScenarioCommand : public SceneCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    AddScenarioCommand(CoreEngine* core, const GString &text, QUndoCommand *parent = nullptr);
    AddScenarioCommand(CoreEngine* core, QUndoCommand *parent = nullptr);
    ~AddScenarioCommand();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the add scenario command
    void redo() override;

    /// @brief Undoes the add scenario command
    void undo() override;

    /// @}
protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief The JSON for the scenario overwritten by the newly created scenario
    QJsonObject m_lastScenario;
    GString m_lastScenarioPath;

    /// @}
    
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class AddSceneObjectCommand
class AddSceneObjectCommand : public SceneCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    AddSceneObjectCommand(CoreEngine* core, Scene* scene, 
        const GString &text, SceneObject* parentObject = nullptr, QUndoCommand *parent = nullptr);
    AddSceneObjectCommand(CoreEngine* core, Scene* scene,
        SceneObject* parentObject = nullptr, QUndoCommand *parent = nullptr);
    ~AddSceneObjectCommand();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the add scenario command
    void redo() override;

    /// @brief Undoes the add scenario command
    void undo() override;

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief The ID of the scene object added by this command
    size_t m_objectId;

    /// @brief The parent to the scene object added by this command
    QJsonObject m_parentSceneObject;

    /// @brief The scene which the scene object belongs to
    QJsonValue m_scene;
    //std::shared_ptr<Scene> m_scene;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class CopySceneObjectCommand
class CopySceneObjectCommand : public SceneCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    CopySceneObjectCommand(CoreEngine* core, Scene* scene, SceneObject* object, int childIndex,
        const GString &text,
        QUndoCommand *parent = nullptr);
    ~CopySceneObjectCommand();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the add scenario command
    void redo() override;

    /// @brief Undoes the add scenario command
    void undo() override;

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief The object that is being copied
    SceneObject* m_object;

    /// @brief The parent of the object
    SceneObject* m_parent;

    /// @brief The scene of the object
    Scene* m_scene;

    /// @brief The ID of the copied object
    size_t m_copyId;

    /// @brief Index at which to add as tree widget child
    int m_childIndex;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class RemoveSceneObjectCommand
class RemoveSceneObjectCommand : public SceneCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    RemoveSceneObjectCommand(CoreEngine* core, SceneObject* sceneObject, const GString &text);
    RemoveSceneObjectCommand(CoreEngine* core, SceneObject* sceneObject);
    ~RemoveSceneObjectCommand();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the remove scenario command
    void redo() override;

    /// @brief Undoes the remove scenario command
    void undo() override;

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief The scene object removed by this command
    SceneObject* m_sceneObject;
    size_t m_sceneObjectId;

    /// @brief The UUID for the scene object added by undoing this command
    Uuid m_sceneID;

    /// @brief The json value representing the scene object
    QJsonValue m_sceneObjectJson;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class ChangeNameCommand
class ChangeNameCommand : public UndoCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    ChangeNameCommand(const GString& name, CoreEngine* core, SceneObject* object);
    ChangeNameCommand(const GString& name, CoreEngine* core, Scene* object);
    ChangeNameCommand(const GString& name, CoreEngine* core, Scenario* object);
    ~ChangeNameCommand();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the add scenario command
    void redo() override;

    /// @brief Undoes the add scenario command
    void undo() override;

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    Nameable* nameable() const {
        return dynamic_cast<Nameable*>(m_object);
    }

    Object* object() const {
        return m_object;
    }

    // TODO: Remove this and emit signal
    void refreshTreeItem();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    View::SceneRelatedItem::SceneType m_sceneType;

    /// @brief The object that has name changed by this command
    Object* m_object;

    /// @brief Name to set for the object
    GString m_name;

    /// @brief Previous name of the object
    GString m_previousName;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class ReparentCommand
class ReparentSceneObjectCommand : public UndoCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    ReparentSceneObjectCommand(SceneObject* newParent,
        CoreEngine* core, 
        SceneObject* object,
        Scene* newScene,
        int childIndex);
    ~ReparentSceneObjectCommand();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the add scenario command
    void redo() override;

    /// @brief Undoes the add scenario command
    void undo() override;

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief Handle reparenting of widget component
    void reparentWidgetItem(SceneObject* newParentObject, Scene* newScene);

    SceneObject* object() const {
        return m_object;
    }

    SceneObject* prevParent() const {
        return m_prevParent;
    }

    Scene* prevScene() const {
        return m_prevScene;
    }

    SceneObject* parent() const {
        return m_parent;
    }

    Scene* scene() const {
        return m_scene;
    }



    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief The object that is being reparented
    SceneObject* m_object;

    /// @brief The previous parent of the object
    SceneObject* m_prevParent;

    /// @brief The previous scene of the object
    Scene* m_prevScene;

    /// @brief The new parent of the object
    SceneObject* m_parent;

    /// @brief The new scene of the object
    Scene* m_scene;

    /// @brief Index at which to add as tree widget child
    int m_childIndex;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif