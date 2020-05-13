#include "GbComponentCommand.h"

#include "../../../GbMainWindow.h"
#include "../../../core/scene/GbScene.h"
#include "../../../core/scene/GbSceneObject.h"
#include "../../../core/components/GbComponent.h"
#include "../../../core/components/GbScriptComponent.h"
#include "../../../core/components/GbRendererComponent.h"
#include "../../../core/components/GbCamera.h"
#include "../../../core/components/GbLight.h"
#include "../../../core/components/GbModelComponent.h"
#include "../../../core/components/GbListenerComponent.h"
#include "../../../core/components/GbCanvasComponent.h"
#include "../../../core/components/GbPhysicsComponents.h"
#include "../../../core/components/GbPhysicsSceneComponent.h"

#include "../../../core/rendering/renderer/GbMainRenderer.h"

#include "../../../core/GbCoreEngine.h"
#include "../../../core/events/GbEventManager.h"
#include "../../../core/loop/GbSimLoop.h"

#include "../../../view/tree/GbComponentWidget.h"
#include "../../models/GbComponentModels.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
ComponentCommand::ComponentCommand(CoreEngine* core, 
    const std::shared_ptr<SceneObject>& object, const QString & text, QUndoCommand * parent) :
    UndoCommand(core, text, parent),
    m_object(object)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
ComponentCommand::ComponentCommand(CoreEngine* core, 
    const std::shared_ptr<Scene>& object, const QString & text, QUndoCommand * parent):
    UndoCommand(core, text, parent),
    m_object(object)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
ComponentCommand::~ComponentCommand()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SceneObject> ComponentCommand::sceneObject()
{
    return std::static_pointer_cast<SceneObject>(m_object);
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Scene> ComponentCommand::scene()
{
    return std::static_pointer_cast<Scene>(m_object);
}


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Delete Component
/////////////////////////////////////////////////////////////////////////////////////////////

DeleteComponentCommand::DeleteComponentCommand(CoreEngine * core, 
    std::shared_ptr<SceneObject> object, 
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
//        if (!m_component->sceneObject()) {
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
    QMutexLocker lock(&m_engine->simulationLoop()->userInterfaceMutex());
    
    // Remove component, and delete
    Component::ParentType parentType = m_component->parentType();
    m_componentJson = m_component->asJson();
    sceneObject()->removeComponent(m_component);
    m_component = nullptr;

    // Send signal to remove tree widget item for the script
    emit m_engine->eventManager()->deletedComponent(sceneObject()->getUuid(), parentType);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DeleteComponentCommand::undo()
{
    QMutexLocker lock(&m_engine->simulationLoop()->userInterfaceMutex());

    // Create and add component to scene object
    QJsonObject componentJsonObject = m_componentJson.toObject();

    // Load component
    m_component = Component::create(sceneObject(), componentJsonObject);

    // Create tree widget item for the renderer
    m_engine->componentWidget()->addItem(m_component);
}


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Add Component
/////////////////////////////////////////////////////////////////////////////////////////////
AddSceneObjectComponent::AddSceneObjectComponent(CoreEngine * core, 
    const std::shared_ptr<SceneObject>& object,
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
//    if (!m_component->sceneObject()) {
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
    QMutexLocker lock(&m_engine->simulationLoop()->userInterfaceMutex());

    // Create and add component to scene object
    if (m_component) throw("Error, component should not exist");
    m_component = Component::create(sceneObject(), Component::ComponentType(m_componentType));

    // Create tree widget item for the renderer
    m_engine->componentWidget()->addItem(m_component);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AddSceneObjectComponent::undo()
{
    QMutexLocker lock(&m_engine->simulationLoop()->userInterfaceMutex());

    Component::ParentType parentType = m_component->parentType();

    // Remove script component, but do not delete
    sceneObject()->removeComponent(m_component);
    m_component = nullptr;

    // Send signal to remove tree widget item for the script
    emit m_engine->eventManager()->deletedComponent(sceneObject()->getUuid(), parentType);
}


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
AddPhysicsSceneCommand::AddPhysicsSceneCommand(CoreEngine * core, 
    const std::shared_ptr<Scene>& object,
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
    QMutexLocker lock(&m_engine->simulationLoop()->userInterfaceMutex());

    // Create and add model to scene
    if (!m_physicsComponent) {
        m_physicsComponent = new PhysicsSceneComponent(scene());
    }
    else {
        // Set scene object of model component, which was removed on undo
        m_physicsComponent->setScene(scene());
        scene()->addComponent(m_physicsComponent);
    }
    m_physicsComponent->enable();

    // Create tree widget item for the script
    m_engine->componentWidget()->addItem(m_physicsComponent);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AddPhysicsSceneCommand::undo()
{
    QMutexLocker lock(&m_engine->simulationLoop()->userInterfaceMutex());

    // Remove rigid body component, but do not delete
    scene()->removeComponent(m_physicsComponent, false);
    m_physicsComponent->setScene(nullptr);

    // Disable the model component 
    m_physicsComponent->disable();

    // Remove tree widget item for the script
    m_engine->componentWidget()->removeItem(m_physicsComponent);

}







/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces