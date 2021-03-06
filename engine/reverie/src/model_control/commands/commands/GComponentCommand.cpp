#include "GComponentCommand.h"

#include "../../../GMainWindow.h"
#include "../../../core/scene/GScene.h"
#include "../../../core/scene/GSceneObject.h"
#include "../../../core/components/GComponent.h"
#include "../../../core/components/GScriptComponent.h"
#include "../../../core/components/GShaderComponent.h"
#include "../../../core/components/GCameraComponent.h"
#include "../../../core/components/GLightComponent.h"
#include "../../../core/components/GModelComponent.h"
#include "../../../core/components/GListenerComponent.h"
#include "../../../core/components/GCanvasComponent.h"
#include "../../../core/components/GCharControlComponent.h"
#include "../../../core/components/GRigidBodyComponent.h"
#include "../../../core/components/GPhysicsSceneComponent.h"

#include "../../../core/rendering/renderer/GMainRenderer.h"

#include "../../../core/GCoreEngine.h"
#include "../../../core/events/GEventManager.h"
#include "../../../core/loop/GSimLoop.h"

#include "../../../view/tree/GComponentWidget.h"
#include "../../models/GComponentModels.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
ComponentCommand::ComponentCommand(CoreEngine* core, 
    SceneObject* object, const QString & text, QUndoCommand * parent) :
    UndoCommand(core, text, parent),
    m_sceneObject(object),
    m_scene(nullptr)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
ComponentCommand::ComponentCommand(CoreEngine* core, 
    Scene* object, const QString & text, QUndoCommand * parent):
    UndoCommand(core, text, parent),
    m_sceneObject(nullptr),
    m_scene(object)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
ComponentCommand::~ComponentCommand()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Delete Component
/////////////////////////////////////////////////////////////////////////////////////////////

DeleteComponentCommand::DeleteComponentCommand(CoreEngine * core, 
    SceneObject* object,
    Component* component,
    const QString & text,
    QUndoCommand * parent) :
    ComponentCommand(core, object, text, parent),
    m_component(component)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
DeleteComponentCommand::~DeleteComponentCommand()
{
    // Should never be necessary (and causes crash)
//    if (m_component) {
//        if (!m_component->m_sceneObject) {
//#ifdef DEBUG_MODE
//            logInfo("Deleting DeleteCommand, and associated component pointer");
//#endif
//            // Delete renderer component if not on any scene object
//            delete m_component;
//        }
//    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DeleteComponentCommand::redo()
{
    QMutexLocker lock(&m_engine->simulationLoop()->updateMutex());
    
    // Remove component, and delete
    m_componentJson = m_component->asJson();
    m_sceneObject->removeComponent(m_component);
    m_component = nullptr;

    // Send signal to remove tree widget item for the script
    emit m_engine->eventManager()->deletedSceneObjectComponent(m_sceneObject->id());
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DeleteComponentCommand::undo()
{
    QMutexLocker lock(&m_engine->simulationLoop()->updateMutex());

    // Create and add component to scene object
    QJsonObject componentJsonObject = m_componentJson.toObject();

    // Load component
    m_component = Component::create(SceneObject::Get(m_sceneObject->id()), componentJsonObject);

    // Create tree widget item for the renderer
    m_engine->componentWidget()->addItem(m_component);
}


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Add Component
/////////////////////////////////////////////////////////////////////////////////////////////
AddSceneObjectComponent::AddSceneObjectComponent(CoreEngine * core, 
    SceneObject* object,
    const QString & text,
    int componentType,
    QUndoCommand * parent):
    ComponentCommand(core, object, text, parent),
    m_componentType(componentType)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
AddSceneObjectComponent::~AddSceneObjectComponent()
{
    // Should never be necessary (and causes crash)
//    if (!m_component->m_sceneObject) {
//#ifdef DEBUG_MODE
//        logInfo("Deleting AddSceneObjectComponent, and associated component pointer");
//#endif
//        // Delete camera component if not on any scene object
//        delete m_component;
//    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AddSceneObjectComponent::redo()
{
    QMutexLocker lock(&m_engine->simulationLoop()->updateMutex());

    // Create and add component to scene object
    if (m_component) throw("Error, component should not exist");
    m_component = Component::create(SceneObject::Get(m_sceneObject->id()), ComponentType(m_componentType));
    m_component->addRequiredComponents();

    // Create tree widget item for the renderer
    m_engine->componentWidget()->addItem(m_component);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AddSceneObjectComponent::undo()
{
    // TODO: Remove objects added by "addRequiredComponents"
    QMutexLocker lock(&m_engine->simulationLoop()->updateMutex());

    // Remove script component, but do not delete
    m_sceneObject->removeComponent(m_component);
    m_component = nullptr;

    // Send signal to remove tree widget item for the script
    emit m_engine->eventManager()->deletedSceneObjectComponent(m_sceneObject->id());
}


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
AddPhysicsSceneCommand::AddPhysicsSceneCommand(CoreEngine * core, 
    Scene* object,
    const QString & text,
    QUndoCommand * parent) :
    ComponentCommand(core, object, text, parent),
    m_physicsComponent(nullptr)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
AddPhysicsSceneCommand::~AddPhysicsSceneCommand()
{
    if (!m_physicsComponent->scene()) {
#ifdef DEBUG_MODE
        logInfo("Deleting AddPhysicsSceneCommand, and associated component pointer");
#endif
        // Delete model component if not on any scene object
        delete m_physicsComponent;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AddPhysicsSceneCommand::redo()
{
    QMutexLocker lock(&m_engine->simulationLoop()->updateMutex());

    // Create and add model to scene
    if (!m_physicsComponent) {
        m_physicsComponent = new PhysicsSceneComponent(m_scene);
    }
    else {
        // Set scene object of model component, which was removed on undo
        m_physicsComponent->setScene(m_scene);
        m_scene->addComponent(m_physicsComponent);
    }
    m_physicsComponent->enable();

    // Create tree widget item for the script
    m_engine->componentWidget()->addItem(m_physicsComponent);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AddPhysicsSceneCommand::undo()
{
    QMutexLocker lock(&m_engine->simulationLoop()->updateMutex());

    // Remove rigid body component, but do not delete
    m_scene->removeComponent(m_physicsComponent, false);
    m_physicsComponent->setScene(nullptr);

    // Disable the model component 
    m_physicsComponent->disable();

    // Remove tree widget item for the script
    m_engine->componentWidget()->removeItem(m_physicsComponent);

}







/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces