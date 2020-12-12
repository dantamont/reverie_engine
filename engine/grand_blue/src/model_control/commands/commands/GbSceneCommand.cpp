#include "GbSceneCommand.h"

#include <QMessageBox>

#include "../../../core/scene/GbScenario.h"
#include "../../../core/scene/GbScene.h"
#include "../../../core/scene/GbSceneObject.h"

#include "../../../core/GbCoreEngine.h"
#include "../../../core/rendering/renderer/GbMainRenderer.h"

#include "../../../view/GbWidgetManager.h"
#include "../../../GbMainWindow.h"
#include "../../../view/tree/GbSceneTreeWidget.h"
#include "../../models/GbSceneModels.h"

#include "../../../core/readers/GbJsonReader.h"
#include "../../../core/loop/GbSimLoop.h"

namespace Gb {


/////////////////////////////////////////////////////////////////////////////////////////////
SceneCommand::SceneCommand(CoreEngine* core, const GString & text, QUndoCommand * parent):
    UndoCommand(core, text, parent)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
SceneCommand::SceneCommand(CoreEngine* core, QUndoCommand * parent) :
    UndoCommand(core, parent)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
SceneCommand::~SceneCommand()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SceneObject> SceneCommand::getSceneObject(const QJsonValue & json) const
{
    QJsonObject object = json.toObject();
    if (!object.contains("name")) return nullptr;

    // Object is non-empty
    QString sceneName = object["scene"].toString();
    QString objectName = object["name"].toString();

    if (sceneName.isEmpty() || objectName.isEmpty()) {
        throw("Error, scene or object name is empty");
    }

    auto scene = m_engine->scenario()->getSceneByName(sceneName);
    std::shared_ptr<SceneObject> sceneObject = scene->getSceneObjectByName(objectName);
    return sceneObject;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SceneObject> SceneCommand::createSceneObject(const QJsonValue & json,
    const QJsonValue& sceneValue) const
{
    QJsonObject object = json.toObject();
    QJsonObject sceneJson = sceneValue.toObject();

    QString sceneName = sceneJson["name"].toString();
    auto scene = m_engine->scenario()->getSceneByName(sceneName);
    std::shared_ptr<SceneObject> sceneObject = SceneObject::create(scene);
    if (!object.isEmpty()) {
        // If scene object JSON is not empty
        sceneObject->loadFromJson(object);
    }

    return sceneObject;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Scene> SceneCommand::getScene(const QJsonValue & sceneJson) const
{
    QString sceneName;
    if (sceneJson.isString()) {
        sceneName = sceneJson.toString();
    }
    else {
        sceneName = sceneJson["name"].toString();
        if (sceneName.isEmpty()) {
            throw("Error, scene name is empty");
        }
    }

    auto scene = m_engine->scenario()->getSceneByName(sceneName);
    return scene;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Scene> SceneCommand::createScene(const QJsonValue & sceneJson) const
{
    auto scene = m_engine->scenario()->addScene();
    QJsonObject object = sceneJson.toObject();
    if(!object.isEmpty())
        scene->loadFromJson(sceneJson);
    return scene;
}



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Add Scenario
AddScenarioCommand::AddScenarioCommand(CoreEngine* core, const GString & text, QUndoCommand * parent) :
    SceneCommand(core, text, parent)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
AddScenarioCommand::AddScenarioCommand(CoreEngine* core, QUndoCommand * parent):
    SceneCommand(core, parent)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
AddScenarioCommand::~AddScenarioCommand()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AddScenarioCommand::redo()
{
    // Create scenario
    m_lastScenario = m_engine->scenario()->asJson().toObject();
    m_lastScenarioPath = m_engine->scenario()->getPath();
    m_engine->setNewScenario();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AddScenarioCommand::undo()
{
    // Revert to last scenario
    auto scenario = std::make_shared<Scenario>(m_engine);
    m_engine->setScenario(scenario);
    scenario->loadFromJson(m_lastScenario);
    scenario->setPath(m_lastScenarioPath);
}



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// AddSceneCommand
AddSceneCommand::AddSceneCommand(CoreEngine* core, const GString & text, QUndoCommand * parent) :
    SceneCommand(core, text, parent)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
AddSceneCommand::AddSceneCommand(CoreEngine* core, QUndoCommand * parent):
    SceneCommand(core, parent)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
AddSceneCommand::~AddSceneCommand()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AddSceneCommand::redo()
{
    // Create scene, ensuring that scenes from prior actions are overwritten
    // with the stored scene
    std::shared_ptr<Scene> scene = m_engine->scenario()->addScene();
    m_sceneName = scene->getName();

    // Create tree widget item for scene
    m_engine->sceneTreeWidget()->addSceneTreeItem(scene);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AddSceneCommand::undo()
{
    std::shared_ptr<Scene> scene = m_engine->scenario()->getSceneByName(m_sceneName);
    m_engine->scenario()->removeScene(scene);

    // Remove scene from tree widget
    m_engine->sceneTreeWidget()->removeTreeItem(scene);
}




/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// RemoveScene
/////////////////////////////////////////////////////////////////////////////////////////////
RemoveSceneCommand::RemoveSceneCommand(CoreEngine * core, const std::shared_ptr<Scene>& scene, const GString & text, QUndoCommand * parent) :
    SceneCommand(core, text, parent),
    m_scene(scene)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
RemoveSceneCommand::RemoveSceneCommand(CoreEngine * core, const std::shared_ptr<Scene>& scene, QUndoCommand * parent):
    SceneCommand(core, parent),
    m_scene(scene)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
RemoveSceneCommand::~RemoveSceneCommand()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RemoveSceneCommand::redo()
{
    bool wasPaused = m_engine->simulationLoop()->isPaused();
    if (!wasPaused)
        m_engine->simulationLoop()->pause();

    if (!m_scene) {
        m_scene = m_engine->scenario()->getScene(m_undoSceneID);
    }

    // Remove from tree widget selection
    m_engine->sceneTreeWidget()->clearSelectedItems();

    m_sceneJson = m_scene->asJson();
    QString json = JsonReader::ToQString(m_sceneJson);
    m_engine->scenario()->removeScene(m_scene);
    m_scene = nullptr;
    emit m_engine->scenarioChanged();
    emit m_engine->scenarioNeedsRedraw();

    if (!wasPaused)
        m_engine->simulationLoop()->play();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RemoveSceneCommand::undo()
{
    bool wasPaused = m_engine->simulationLoop()->isPaused();
    if (!wasPaused)
        m_engine->simulationLoop()->pause();

    auto scene = m_engine->scenario()->addScene();
    scene->loadFromJson(m_sceneJson);
    m_undoSceneID = scene->getUuid();

    emit m_engine->scenarioChanged();
    emit m_engine->scenarioNeedsRedraw();

    if (!wasPaused)
        m_engine->simulationLoop()->play();
}



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// AddSceneObject
AddSceneObjectCommand::AddSceneObjectCommand(CoreEngine* core, const std::shared_ptr<Scene>& scene, const GString & text, std::shared_ptr<SceneObject> parentObject, QUndoCommand * parent) :
    SceneCommand(core, text, parent)
{
    if (scene) {
        m_scene = scene->getName().c_str();
    }
    if (parentObject) {
        m_parentSceneObject = parentObject->asJson().toObject();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
AddSceneObjectCommand::AddSceneObjectCommand(CoreEngine* core, const std::shared_ptr<Scene>& scene, std::shared_ptr<SceneObject> parentObject, QUndoCommand * parent) :
    SceneCommand(core, parent)
{
    if (scene) {
        m_scene = scene->getName().c_str();
    }
    if (parentObject) {
        m_parentSceneObject = parentObject->asJson().toObject();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
AddSceneObjectCommand::~AddSceneObjectCommand()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AddSceneObjectCommand::redo()
{
    // Create scene object
    std::shared_ptr<Scene> scene = getScene(m_scene);
    std::shared_ptr<SceneObject> sceneObject = SceneObject::create(scene);
    m_uuid = sceneObject->getUuid();
    if (!m_parentSceneObject.isEmpty()) {
        auto parentSceneObject = getSceneObject(m_parentSceneObject);
        sceneObject->setParent(parentSceneObject);
    }

    // Create tree widget item for scene object
    m_engine->sceneTreeWidget()->addSceneObjectTreeItem(sceneObject);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AddSceneObjectCommand::undo()
{
    std::shared_ptr<Scene> scene = getScene(m_scene);
    std::shared_ptr<SceneObject> sceneObject = scene->getSceneObject(m_uuid);
    if (!m_parentSceneObject.isEmpty()) {
        // Remove object from parent scene object
        auto parentSceneObject = getSceneObject(m_parentSceneObject);
        parentSceneObject->removeChild(sceneObject);
        SceneObject::EraseFromNodeMap(sceneObject->getUuid());
    }
    else {
        // Remove object from the scene (and from the node map)
        scene->removeObject(sceneObject, true);
    }

    // Remove tree widget item for scene object
    m_engine->sceneTreeWidget()->removeTreeItem(sceneObject);
}


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// RemoveSceneObjectCommand
/////////////////////////////////////////////////////////////////////////////////////////////
RemoveSceneObjectCommand::RemoveSceneObjectCommand(CoreEngine * core,
    const std::shared_ptr<SceneObject>& sceneObject, const GString & text):
    SceneCommand(core, text, nullptr),
    m_sceneObject(sceneObject)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
RemoveSceneObjectCommand::RemoveSceneObjectCommand(CoreEngine * core, 
    const std::shared_ptr<SceneObject>& sceneObject) :
    SceneCommand(core, nullptr),
    m_sceneObject(sceneObject)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
RemoveSceneObjectCommand::~RemoveSceneObjectCommand()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RemoveSceneObjectCommand::redo()
{
    bool wasPaused = m_engine->simulationLoop()->isPaused();
    if(!wasPaused)
        m_engine->simulationLoop()->pause();

    // Retrieve scene object if it is not set for the command
    if (m_sceneObject) {
        m_sceneID = m_sceneObject->getUuid();
    }
    else {
        auto scene = m_engine->scenario()->getScene(m_sceneID);
        m_sceneObject = m_engine->scenario()->getScene(m_sceneID)->getSceneObject(m_addedObjectUuid);
    }

    if (m_sceneObject->isPythonGenerated()) {
        QMessageBox::about(m_engine->widgetManager()->mainWindow(),
            "Warning",
            "Manual deletion of a python-generated object will cause undefined behavior");
    }

    // Remove from tree widget selection
    if (m_engine->sceneTreeWidget()->getCurrentSceneObject()) {
        if (m_sceneObject->getUuid() == m_engine->sceneTreeWidget()->getCurrentSceneObject()->getUuid()) {
            m_engine->sceneTreeWidget()->clearSelectedItems();
        }
    }

    // Get engine
    m_sceneObjectJson = m_sceneObject->asJson();
    m_sceneObject->removeFromScene();

    // Remove tree widget item for scene object
    //m_engine->sceneTreeWidget()->removeTreeItem(m_sceneObject);
    m_sceneObject = nullptr;

    // Emit signal that scenario has changed
    emit m_engine->scenarioChanged();

    if (!wasPaused)
        m_engine->simulationLoop()->play();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RemoveSceneObjectCommand::undo()
{
    bool wasPaused = m_engine->simulationLoop()->isPaused();
    if (!wasPaused)
        m_engine->simulationLoop()->pause();

    // Reconstruct scene object from JSON
    auto sceneObject = SceneObject::create(m_engine, m_sceneObjectJson);
    m_uuid = sceneObject->getUuid();

    // Create tree widget item for scene object
    m_engine->sceneTreeWidget()->addSceneObjectTreeItem(sceneObject);

    if (!wasPaused)
        m_engine->simulationLoop()->play();
}



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// ChangeNameCommand
/////////////////////////////////////////////////////////////////////////////////////////////
ChangeNameCommand::ChangeNameCommand(const GString& name, 
    CoreEngine* core,
    const std::shared_ptr<Object>& object) :
    UndoCommand(core, nullptr),
    m_object(object),
    m_name(name)
{

}
/////////////////////////////////////////////////////////////////////////////////////////////
ChangeNameCommand::~ChangeNameCommand()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ChangeNameCommand::redo()
{
    if (!object()) return;

    // Cache previous object name
    m_previousName = object()->getName();

    // Set name in scene object and in item
    object()->setName(m_name);

    // Refresh text in the tree item
    auto* treeItem = m_engine->sceneTreeWidget()->getItem(object());
    treeItem->refreshText();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ChangeNameCommand::undo()
{
    if (!object()) return;

    // Set name in scene object and in item
    object()->setName(m_previousName);

    // Refresh text in the tree item
    auto* treeItem = m_engine->sceneTreeWidget()->getItem(object());
    treeItem->refreshText();
}


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// ReparentCommand
ReparentSceneObjectCommand::ReparentSceneObjectCommand(const std::shared_ptr<SceneObject>& newParent,
    CoreEngine* core, 
    std::shared_ptr<SceneObject> object,
    std::shared_ptr<Scene> newScene) :
    UndoCommand(core, nullptr),
    m_parent(newParent),
    m_object(object),
    m_scene(newScene)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
ReparentSceneObjectCommand::~ReparentSceneObjectCommand()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ReparentSceneObjectCommand::redo()
{
    if (!object()) return;

    // Reparent widget item
    reparentWidgetItem(parent(), scene());

    // Cache previous parent and scene
    m_prevParent = object()->parent();
    m_prevScene = object()->scene();

    // Set new parent for the object
    object()->setParent(parent());
    
    // Set new scene for the object
    if (scene()) {
        object()->switchScene(scene());
    }

}
/////////////////////////////////////////////////////////////////////////////////////////////
void ReparentSceneObjectCommand::undo()
{
    // Reparent widget item
    reparentWidgetItem(prevParent(), prevScene());

    // Set object parent to previous
    object()->setParent(prevParent());

    // Set object scene to previous
    if (scene()) {
        object()->switchScene(prevScene());
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ReparentSceneObjectCommand::reparentWidgetItem(const std::shared_ptr<SceneObject>& newParentObject, 
    const std::shared_ptr<Scene>& newScene)
{
    // Fallback for any weirdness
    if (!object()) return;

    // Remove from current parent in widget
    auto* item = m_engine->sceneTreeWidget()->getItem(object());
    auto* oldParent = item->parent();
    if (oldParent) {
        oldParent->removeChild(item);
    }

    // Add to new parent in tree widget
    QTreeWidgetItem* newParent;
    if (newParentObject) {
        newParent = m_engine->sceneTreeWidget()->getItem(newParentObject);
    }
    else {
        if (newScene) {
            newParent = m_engine->sceneTreeWidget()->getItem(newScene);
        }
        else {
            newParent = m_engine->sceneTreeWidget()->getItem(object()->scene());
        }
    }
    newParent->addChild(item);
}


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces