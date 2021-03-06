#include "GSceneCommand.h"

#include <QMessageBox>

#include "../../../core/scene/GScenario.h"
#include "../../../core/scene/GScene.h"
#include "../../../core/scene/GSceneObject.h"

#include "../../../core/GCoreEngine.h"
#include "../../../core/rendering/renderer/GMainRenderer.h"

#include "../../../view/GWidgetManager.h"
#include "../../../GMainWindow.h"
#include "../../../view/tree/GSceneTreeWidget.h"

#include "../../../core/readers/GJsonReader.h"
#include "../../../core/loop/GSimLoop.h"

namespace rev {


/////////////////////////////////////////////////////////////////////////////////////////////
SceneCommand::SceneCommand(CoreEngine* core, const GString & text, QUndoCommand * parent):
    UndoCommand(core, text.c_str(), parent)
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
SceneObject* SceneCommand::getSceneObject(const QJsonValue & json) const
{
    QJsonObject object = json.toObject();
    if (!object.contains("name")) return nullptr;

    // Object is non-empty
    QString sceneName = object["scene"].toString();
    QString objectName = object["name"].toString();

    if (sceneName.isEmpty() || objectName.isEmpty()) {
        throw("Error, scene or object name is empty");
    }

    Scene& scene = m_engine->scenario()->scene();
    SceneObject* sceneObject = scene.getSceneObjectByName(objectName).get();
    return sceneObject;
}
/////////////////////////////////////////////////////////////////////////////////////////////
SceneObject* SceneCommand::createSceneObject(const QJsonValue & json,
    const QJsonValue& sceneValue) const
{
    QJsonObject object = json.toObject();
    QJsonObject sceneJson = sceneValue.toObject();

    QString sceneName = sceneJson["name"].toString();
    Scene& scene = m_engine->scenario()->scene();
    SceneObject* sceneObject = SceneObject::Create(&scene).get();
    if (!object.isEmpty()) {
        // If scene object JSON is not empty
        sceneObject->loadFromJson(object);
    }

    return sceneObject;
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
// AddSceneObject
AddSceneObjectCommand::AddSceneObjectCommand(CoreEngine* core, Scene* scene,
    const GString & text, SceneObject* parentObject, QUndoCommand * parent) :
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
AddSceneObjectCommand::AddSceneObjectCommand(CoreEngine* core, Scene* scene, 
    SceneObject* parentObject, QUndoCommand * parent) :
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
    std::shared_ptr<SceneObject> sceneObject = SceneObject::Create(&m_engine->scenario()->scene());
    m_objectId = sceneObject->id();
    if (!m_parentSceneObject.isEmpty()) {
        SceneObject* parentSceneObject = getSceneObject(m_parentSceneObject);
        sceneObject->setParent(parentSceneObject->sharedPtr());
    }

    // Create tree widget item for scene object
    m_engine->sceneTreeWidget()->addSceneObjectTreeItem(sceneObject.get());
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AddSceneObjectCommand::undo()
{
    Scene* scene = &m_engine->scenario()->scene();
    std::shared_ptr<SceneObject> sceneObject = scene->getSceneObject(m_objectId);
    if (!m_parentSceneObject.isEmpty()) {
        // Remove object from parent scene object
        auto parentSceneObject = getSceneObject(m_parentSceneObject);
        parentSceneObject->removeChild(sceneObject);
        SceneObject::EraseFromNodeVec(sceneObject->id());
    }
    else {
        // Remove object from the scene (and from the node map)
        scene->removeObject(sceneObject);
    }

    // Remove tree widget item for scene object
    m_engine->sceneTreeWidget()->removeTreeItem(sceneObject.get());
}


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// RemoveSceneObjectCommand
/////////////////////////////////////////////////////////////////////////////////////////////
RemoveSceneObjectCommand::RemoveSceneObjectCommand(CoreEngine * core, SceneObject* sceneObject, const GString & text):
    SceneCommand(core, text, nullptr),
    m_sceneObject(sceneObject),
    m_sceneObjectId(m_sceneObject->id()),
    m_sceneID(m_sceneObject->scene()->getUuid())
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
RemoveSceneObjectCommand::RemoveSceneObjectCommand(CoreEngine * core, SceneObject* sceneObject) :
    SceneCommand(core, nullptr),
    m_sceneObject(sceneObject),
    m_sceneObjectId(m_sceneObject->id()),
    m_sceneID(m_sceneObject->scene()->getUuid())
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
RemoveSceneObjectCommand::~RemoveSceneObjectCommand()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RemoveSceneObjectCommand::redo()
{
    //bool wasPaused = m_engine->simulationLoop()->isPaused();
    //if(!wasPaused)
    //    m_engine->simulationLoop()->pause();

    if (!m_sceneObject){
        // Retrieve scene object if it is not set for the command
        Scene* scene = &m_engine->scenario()->scene();
        m_sceneObject = scene->getSceneObject(m_sceneObjectId).get();
    }

    if (m_sceneObject->isScriptGenerated()) {
        QMessageBox::about(m_engine->widgetManager()->mainWindow(),
            "Warning",
            "Manual deletion of a python-generated object will cause undefined behavior");
    }

    // Remove from tree widget selection
    if (m_engine->sceneTreeWidget()->getCurrentSceneObject()) {
        if (m_sceneObject->id() == m_engine->sceneTreeWidget()->getCurrentSceneObject()->id()) {
            m_engine->sceneTreeWidget()->clearSelectedItems();
        }
    }

    // Cache JSON and delete scene object
    m_sceneObjectJson = m_sceneObject->asJson();
    m_sceneObject->removeFromScene();
    //m_engine->sceneTreeWidget()->removeTreeItem(m_sceneObject);
    m_sceneObject = nullptr;

    // Emit signal that scenario has changed to update tree widget
    emit m_engine->scenarioChanged();

    //if (!wasPaused)
    //    m_engine->simulationLoop()->play();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RemoveSceneObjectCommand::undo()
{
    //bool wasPaused = m_engine->simulationLoop()->isPaused();
    //if (!wasPaused)
    //    m_engine->simulationLoop()->pause();

    // Reconstruct scene object from JSON
    auto sceneObject = SceneObject::Create(m_engine, m_sceneObjectJson);
    m_sceneObjectId = sceneObject->id();

    // Create tree widget item for scene object
    // TODO: Emit a signal for this
    m_engine->sceneTreeWidget()->addSceneObjectTreeItem(sceneObject.get());

    //if (!wasPaused)
    //    m_engine->simulationLoop()->play();
}



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// ChangeNameCommand
/////////////////////////////////////////////////////////////////////////////////////////////
ChangeNameCommand::ChangeNameCommand(const GString& name, 
    CoreEngine* core,
    SceneObject* object) :
    UndoCommand(core, nullptr),
    m_object(dynamic_cast<Object*>(object)),
    m_name(name),
    m_sceneType(View::SceneRelatedItem::kSceneObject)
{

}
/////////////////////////////////////////////////////////////////////////////////////////////
ChangeNameCommand::ChangeNameCommand(const GString& name,
    CoreEngine* core,
    Scene* object) :
    UndoCommand(core, nullptr),
    m_object(dynamic_cast<Object*>(object)),
    m_name(name),
    m_sceneType(View::SceneRelatedItem::kScene)
{

}
/////////////////////////////////////////////////////////////////////////////////////////////
ChangeNameCommand::ChangeNameCommand(const GString& name,
    CoreEngine* core,
    Scenario* object) :
    UndoCommand(core, nullptr),
    m_object(dynamic_cast<Object*>(object)),
    m_name(name),
    m_sceneType(View::SceneRelatedItem::kScenario)
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
    m_previousName = nameable()->getName();

    // Set name in scene object and in item
    nameable()->setName(m_name);

    // Refresh text in the tree item
    // TODO: Send signal instead
    refreshTreeItem();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ChangeNameCommand::undo()
{
    if (!object()) return;

    // Set name in scene object and in item
    nameable()->setName(m_previousName);

    // Refresh text in the tree item
    // TODO: Send signal instead
    refreshTreeItem();;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ChangeNameCommand::refreshTreeItem()
{
    // TODO: Send signal instead
    View::SceneRelatedItem* treeItem;
    switch (m_sceneType) {
    case View::SceneRelatedItem::kScenario:
        treeItem = m_engine->sceneTreeWidget()->getItem(dynamic_cast<Scenario*>(object()));
        break;
    case View::SceneRelatedItem::kScene:
        treeItem = m_engine->sceneTreeWidget()->getItem(dynamic_cast<Scene*>(object()));
        break;
    case View::SceneRelatedItem::kSceneObject:
        treeItem = m_engine->sceneTreeWidget()->getItem(dynamic_cast<SceneObject*>(object()));
        break;
    default:
        throw("oh no");
        break;
    }
    treeItem->refreshText();
}


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// ReparentCommand
ReparentSceneObjectCommand::ReparentSceneObjectCommand(SceneObject* newParent,
    CoreEngine* core, 
    SceneObject* object,
    Scene* newScene,
    int childIndex) :
    UndoCommand(core, nullptr),
    m_parent(newParent),
    m_object(object),
    m_scene(newScene),
    m_childIndex(childIndex)
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
    reparentWidgetItem(m_parent, m_scene);

    // Cache previous parent and scene
    m_prevParent = object()->parent().get();
    m_prevScene = object()->scene();

    // Set new parent for the object
    if (m_parent) {
        size_t pid = m_parent->id();
        object()->setParent(SceneObject::Get(pid));
    }
    else {
        object()->setParent(nullptr);
    }
    
    // Set new scene for the object
    //if (scene()) {
    //    object()->switchScene(scene());
    //}

}
/////////////////////////////////////////////////////////////////////////////////////////////
void ReparentSceneObjectCommand::undo()
{
    // Reparent widget item
    reparentWidgetItem(m_prevParent, m_prevScene);

    // Set object parent to previous
    if (m_prevParent) {
        size_t pid = m_prevParent->id();
        object()->setParent(SceneObject::Get(pid));
    }
    else {
        object()->setParent(nullptr);
    }

    // Set object scene to previous
    //if (scene()) {
    //    object()->switchScene(prevScene());
    //}
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ReparentSceneObjectCommand::reparentWidgetItem(SceneObject* newParentObject,
    Scene* newScene)
{
    // Fallback for any weirdness
    if (!object()) return;

    auto* item = m_engine->sceneTreeWidget()->getItem(object());
    View::SceneRelatedItem* oldParent = dynamic_cast<View::SceneRelatedItem*>(item->parent());

    // Don't do anything if no new parent
    View::SceneRelatedItem* sceneItem = m_engine->sceneTreeWidget()->getItem(newScene);
    if (oldParent->object() == newScene && !newParentObject && m_childIndex < 0) {
        return;
    }

    // Remove from current parent in widget
    if (oldParent) {
        oldParent->removeChild(item);
    }

    // Add to new parent in tree widget
    QTreeWidgetItem* newParent;
    if (newParentObject) {
        // If has a parent, make a child
        newParent = m_engine->sceneTreeWidget()->getItem(newParentObject);
        if (m_childIndex < 0) {
            // No child index specified
            newParent->addChild(item);
        }

        else {
            newParent->insertChild(m_childIndex, item);
        }
        if (newParent->parent()) {
            // Resize to fit additional parent levels
            m_engine->sceneTreeWidget()->resizeColumns();
        }
    }
    else {
        // Else, add to scene level if wasn't already at it
        if (newScene) {
            newParent = m_engine->sceneTreeWidget()->getItem(newScene);
        }
        else {
            newParent = m_engine->sceneTreeWidget()->getItem(object()->scene());
        }

        if (m_childIndex < 0) {
            // No child index specified
            newParent->addChild(item);
        }
        else {
            newParent->insertChild(m_childIndex, item);
        }
        m_engine->sceneTreeWidget()->resizeColumns();
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////
CopySceneObjectCommand::CopySceneObjectCommand(CoreEngine * core, Scene * scene, 
    SceneObject * object,
    int childIndex, 
    const GString & text, QUndoCommand * parent):
    SceneCommand(core, text, parent),
    m_scene(scene),
    m_object(object),
    m_childIndex(childIndex)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
CopySceneObjectCommand::~CopySceneObjectCommand()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CopySceneObjectCommand::redo()
{
    // Duplicate object
    std::shared_ptr<SceneObject> sceneObject = SceneObject::Create(*m_object);

    // Set new name for duplicate object
    sceneObject->setName(sceneObject->getName() + "_copy");

    // Create tree widget item for scene object
    // TODO: Set order of scene object in scene (or as child), so that order is preserved
    // on repopulation
    m_engine->sceneTreeWidget()->addSceneObjectTreeItem(sceneObject.get(), m_childIndex);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CopySceneObjectCommand::undo()
{
    // Remove duplicate object
    Scene* scene = &m_engine->scenario()->scene();
    std::shared_ptr<SceneObject> sceneObject = scene->getSceneObject(m_copyId);

    // Remove tree widget item for scene object
    m_engine->sceneTreeWidget()->removeTreeItem(sceneObject.get());

    // Remove object from the scene (and from the node map)
    scene->removeObject(sceneObject);
}

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces