#ifndef GB_COMPONENT_COMMAND_H
#define GB_COMPONENT_COMMAND_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Qt

// Internal
#include "../GUndoCommand.h"


namespace rev {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class Object;
class CoreEngine;
class Scene;
class SceneObject;
class Component;
class CameraComponent;
class Light;
class ModelComponent;
class ShaderComponent;
class ScriptComponent;
class ListenerComponent;
class RigidBodyComponent;
class CanvasComponent;
class CharControlComponent;
class PhysicsSceneComponent;

namespace View {
class ComponentTreeWidget;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class ComponentCommand
/// @brief Abstract representing a component-related command
class ComponentCommand : public UndoCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static methods
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    ComponentCommand(CoreEngine* core, Scene* object, const QString &text, QUndoCommand *parent = nullptr);
    ComponentCommand(CoreEngine* core, SceneObject* object, const QString &text, QUndoCommand *parent = nullptr);
    ~ComponentCommand();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the command
    virtual void redo() override {}

    /// @brief Undoes the command
    virtual void undo() override {}

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "ComponentCommand"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "rev::ComponentCommand"; }
    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief Pointer to the scene or scene object relating to this component command
    SceneObject* m_sceneObject = nullptr;
    Scene* m_scene = nullptr;

    /// @}

};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class DeleteCommand
/// @brief Delete a component
class DeleteComponentCommand : public ComponentCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    DeleteComponentCommand(CoreEngine* core, 
        SceneObject* object,
        Component* component,
        const QString &text, 
        QUndoCommand *parent = nullptr);
    ~DeleteComponentCommand();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the add scenario command
    virtual void redo() override;

    /// @brief Undoes the add scenario command
    virtual void undo() override;

    /// @}
protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief The component removed by this command
    Component* m_component;

    /// @brief The JSON for the component removed by this command
    QJsonValue m_componentJson;

    /// @}

};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class AddSceneObjectComponent
class AddSceneObjectComponent : public ComponentCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    AddSceneObjectComponent(CoreEngine* core,
        SceneObject* object,
        const QString &text, 
        int componentType, 
        QUndoCommand *parent = nullptr);
    ~AddSceneObjectComponent();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the add renderer command
    virtual void redo() override;

    /// @brief Undoes the add renderer command
    virtual void undo() override;

    /// @}

protected:

    Component* m_component = nullptr;
    int m_componentType;
};



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class AddPhysicsSceneCommand
// TODO: One generic command for adding scene components
class AddPhysicsSceneCommand : public ComponentCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    AddPhysicsSceneCommand(CoreEngine* core, Scene* object,
        const QString &text, QUndoCommand *parent = nullptr);
    ~AddPhysicsSceneCommand();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the add scenario command
    virtual void redo() override;

    /// @brief Undoes the add scenario command
    virtual void undo() override;

    /// @}
protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// The physics component added by this object
    PhysicsSceneComponent* m_physicsComponent;

    /// @}

};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif