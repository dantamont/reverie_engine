#ifndef GB_SCENE_COMMAND_H
#define GB_SCENE_COMMAND_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Qt

// Internal
#include "../GbUndoCommand.h"

namespace Gb {

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
    virtual const char* namespaceName() const { return "Gb::SceneCommand"; }
    /// @}
protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief Obtain objects from their JSON representation
    std::shared_ptr<SceneObject> getSceneObject(const QJsonValue& object) const;
    std::shared_ptr<SceneObject> createSceneObject(const QJsonValue& scene, const QJsonValue& object) const;

    /// @brief Obtain scenes from theeir JSON representation
    std::shared_ptr<Scene> getScene(const QJsonValue& sceneJson) const;
    std::shared_ptr<Scene> createScene(const QJsonValue& sceneJson) const;


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
/// @class AddSceneCommand
class AddSceneCommand : public SceneCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    AddSceneCommand(CoreEngine* core, const GString &text, QUndoCommand *parent = nullptr);
    AddSceneCommand(CoreEngine* core, QUndoCommand *parent = nullptr);
    ~AddSceneCommand();
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

    /// @brief The name of the scene added by this command
    GString m_sceneName;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class RemoveSceneCommand
class RemoveSceneCommand : public SceneCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    RemoveSceneCommand(CoreEngine* core, const std::shared_ptr<Scene>& scene, const GString &text, QUndoCommand *parent = nullptr);
    RemoveSceneCommand(CoreEngine* core, const std::shared_ptr<Scene>& scene, QUndoCommand *parent = nullptr);
    ~RemoveSceneCommand();
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

    /// @brief The scene removed by this command
    std::shared_ptr<Scene> m_scene;

    /// @brief The UUID of the scene created by the undo command
    Uuid m_undoSceneID;

    /// @brief The json value representing the scene object removed by this command
    QJsonValue m_sceneJson;

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
    AddSceneObjectCommand(CoreEngine* core, const std::shared_ptr<Scene>& scene, const GString &text, std::shared_ptr<SceneObject> parentObject = nullptr, QUndoCommand *parent = nullptr);
    AddSceneObjectCommand(CoreEngine* core, const std::shared_ptr<Scene>& scene, std::shared_ptr<SceneObject> parentObject = nullptr, QUndoCommand *parent = nullptr);
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

    /// @brief The UUID of the scene object added by this command
    Uuid m_objectUuid;

    /// @brief The parent to the scene object added by this command
    QJsonObject m_parentSceneObject;

    /// @brief The scene which the scene object belongs to
    QJsonValue m_scene;
    //std::shared_ptr<Scene> m_scene;

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
    RemoveSceneObjectCommand(CoreEngine* core, const std::shared_ptr<SceneObject>& sceneObject, const GString &text);
    RemoveSceneObjectCommand(CoreEngine* core, const std::shared_ptr<SceneObject>& sceneObject);
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
    std::shared_ptr<SceneObject> m_sceneObject;

    /// @brief The UUID for the scene object added by undoing this command
    Uuid m_addedObjectUuid;
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
    ChangeNameCommand(const GString& name, CoreEngine* core, const std::shared_ptr<Object>& object);
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

    std::shared_ptr<Object> object() const {
        if (std::shared_ptr<Object> obj = m_object.lock()) {
            return obj;
        }
        else {
            return nullptr;
        }
    }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief The object that has name changed by this command
    std::weak_ptr<Object> m_object;

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
    ReparentSceneObjectCommand(const std::shared_ptr<SceneObject>& newParent, 
        CoreEngine* core, 
        std::shared_ptr<SceneObject> object,
        std::shared_ptr<Scene> newScene = nullptr);
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
    void reparentWidgetItem(const std::shared_ptr<SceneObject>& newParentObject, const std::shared_ptr<Scene>& newScene);

    std::shared_ptr<SceneObject> object() const {
        if (const std::shared_ptr<SceneObject>& obj = m_object.lock()) {
            return obj;
        }
        else {
            return nullptr;
        }
    }

    std::shared_ptr<SceneObject> prevParent() const {
        if (const std::shared_ptr<SceneObject>& obj = m_prevParent.lock()) {
            return obj;
        }
        else {
            return nullptr;
        }
    }

    std::shared_ptr<Scene> prevScene() const {
        if (std::shared_ptr<Scene> obj = m_prevScene.lock()) {
            return obj;
        }
        else {
            return nullptr;
        }
    }

    std::shared_ptr<SceneObject> parent() const {
        if (const std::shared_ptr<SceneObject>& obj = m_parent.lock()) {
            return obj;
        }
        else {
            return nullptr;
        }
    }

    std::shared_ptr<Scene> scene() const {
        if (std::shared_ptr<Scene> obj = m_scene.lock()) {
            return obj;
        }
        else {
            return nullptr;
        }
    }



    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief The object that is being reparented
    std::weak_ptr<SceneObject> m_object;

    /// @brief The previous parent of the object
    std::weak_ptr<SceneObject> m_prevParent;

    /// @brief The previous scene of the object
    std::weak_ptr<Scene> m_prevScene;

    /// @brief The new parent of the object
    std::weak_ptr<SceneObject> m_parent;

    /// @brief The new scene of the object
    std::weak_ptr<Scene> m_scene;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif