#include "GComponentWidget.h"
#include "GSceneTreeWidget.h"

#include "../../core/GCoreEngine.h"
#include "../../core/events/GEventManager.h"
#include "../../model_control/commands/GActionManager.h"
#include "../../model_control/commands/commands/GComponentCommand.h"
#include "../components/GComponentWidgets.h"

#include <core/geometry/GTransform.h>
#include <core/scene/GScenario.h>
#include <core/scene/GScene.h>
#include <core/scene/GSceneObject.h>
#include <core/components/GComponent.h>
#include <core/debugging/GDebugManager.h>
#include <core/components/GShaderComponent.h>
#include <core/components/GCameraComponent.h>
#include "../../model_control/models/GComponentModels.h"

#include "../../core/processes/GProcessManager.h"
#include "../../core/loop/GSimLoop.h"

#include "../GWidgetManager.h"

#include <view/list/GBlueprintView.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ComponentTreeWidget::ComponentTreeWidget(CoreEngine* engine, const QString & name, QWidget * parent) :
    AbstractService(name),
    QTreeWidget(parent),
    m_engine(engine)
{
    initializeWidget();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ComponentTreeWidget::~ComponentTreeWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentTreeWidget::selectScene(const Uuid& sceneID) {
    // Temporarily disable updates for a performance boost
    setUpdatesEnabled(false);

    clearSceneObject();

    // Set new scene object
    m_currentSceneObject.reset();

    Scene& scene = m_engine->scenario()->scene();
    m_currentScene = scene.getUuid() == sceneID ? &scene: m_engine->debugManager()->scene().get();

    // Return if no scene
    if (!m_currentScene) {
#ifdef DEBUG_MODE
        logWarning("selectSceneObject::Warning, no scene found");
#endif
        return;
    }

    // Populate with components of new scene
    for (const auto& compVec : m_currentScene->components()) {
        for (const auto& comp : compVec) {
            addItem(comp);
        }
    }

    // Reenable updates
    setUpdatesEnabled(true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentTreeWidget::clearScene()
{    
    // Don't clear while iterating through parameter widgets in update loop
    // Unnecessary
    //QMutexLocker lock(&m_engine->widgetManager()->updateMutex());

    //QTreeWidgetItemIterator it(this);
    //while (*it) {
    //    ComponentItem* item = static_cast<ComponentItem*>(*it);
    //    if (item->component()->getUuid() == component->getUuid()) {
    //        return item;
    //    }
    //    ++it;
    //}

    // Clear the widget
    clear();

    //clearBlueprint();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentTreeWidget::selectSceneObject(size_t sceneObjectID)
{
    // Temporarily disable updates for a performance boost
    setUpdatesEnabled(false);

    // Clear the widget
    clearScene();
    m_currentSceneObject = SceneObject::Get(sceneObjectID);

    // Return if no new scene object
    if (!currentSceneObject()) {
#ifdef DEBUG_MODE
        throw("selectSceneObject::Warning, no scene object found");
#endif
        return;
    }

    // Add render layers widget
    auto item = new View::ComponentItem(currentSceneObject().get());
    addItem(item);

    // Add transform component
    addItem((Component*)(&currentSceneObject()->transform()));

    // Populate with components of new scene object
    for (const auto& componentList : currentSceneObject()->components()) {
        for (const auto& comp : componentList) {
            addItem(comp);
        }
    }

    // Reenable updates
    setUpdatesEnabled(true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentTreeWidget::onSelectedBlueprint(QModelIndex blueprintIndex)
{
    //clearBlueprint();

    size_t idx = blueprintIndex.row();
    Blueprint& blueprint = m_engine->scenario()->blueprints()[idx];
    m_selectedBlueprint = blueprint;
    m_currentSceneObject.reset();
    m_currentScene = nullptr;

    // Temporarily disable updates for a performance boost
    setUpdatesEnabled(false);

    // Clear the widget
    clearScene();

    auto item = new View::ComponentBlueprintItem(&m_selectedBlueprint);
    addItem(item);

    // Create a new scene object just for the widgets
    //m_blueprintSceneObject = SceneObject::Create(&m_engine->scenario()->scene(), SceneObjectFlag::kIsInvisible);
    //m_blueprintSceneObject->loadFromJson(blueprint.sceneObjectJson());
    //m_blueprintSceneObject->setEnabled(false);
    //m_blueprintSceneObject->setVisible(false);

    //// Add render layers widget
    //auto item = new View::ComponentBlueprintItem(m_blueprintSceneObject.get());
    //addItem(item);


    //// Add transform component
    //addItem((Component*)(&m_blueprintSceneObject->transform()));

    //// Populate with components of new scene object
    //for (const auto& componentList : m_blueprintSceneObject->components()) {
    //    for (const auto& comp : componentList) {
    //        addItem(comp);
    //    }
    //}
    //
    //// Disable all of the newly added widgets
    //for (int i = 0; i < topLevelItemCount(); ++i)
    //{
    //    QTreeWidgetItem *item = topLevelItem(i);
    //    ComponentItem* compItem = dynamic_cast<ComponentItem*>(item);
    //    compItem->m_widget->setDisabled(true);
    //    item->setDisabled(true);
    //}

    // Reenable updates
    setUpdatesEnabled(true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentTreeWidget::clearSceneObject()
{
    // Clear the widget
    clear();

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentTreeWidget::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(item)
    Q_UNUSED(column)
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentTreeWidget::addItem(Component * component)
{
    // Create component item
    auto* componentItem = new View::ComponentItem(component);

    // Add component item
    addItem(componentItem);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentTreeWidget::addItem(View::ComponentItem * item)
{
    // Insert component item into the widget
    addTopLevelItem(item);

    // Create widget for component
    item->setWidget();

    // Resize columns to fit widget
    //resizeColumns();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentTreeWidget::addItem(View::ComponentBlueprintItem * item)
{
    // Insert component item into the widget
    addTopLevelItem(item);

    // Create widget for component
    item->setWidget();

    // Resize columns to fit widget
    //resizeColumns();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentTreeWidget::removeItem(Component * component)
{
    removeItem(getItem(component));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
View::ComponentItem * ComponentTreeWidget::getItem(Component * component)
{
    QTreeWidgetItemIterator it(this);
    while (*it) {
        ComponentItem* item = static_cast<ComponentItem*>(*it);
        if (item->component()->getUuid() == component->getUuid()) {
            return item;
        }
        ++it;
    }

    throw("Error, no item found that corresponds to the given object");

    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentTreeWidget::resizeColumns()
{
    resizeColumnToContents(0);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentTreeWidget::onItemExpanded(QTreeWidgetItem * item)
{
    Q_UNUSED(item);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void ComponentTreeWidget::clearBlueprint()
//{
    // Removed, very buggy managing components
    //if (m_blueprintSceneObject) {
    //    //m_engine->simulationLoop()->pause();

    //    // If a blueprint was being viewed, delete temporary scene object, and update blueprint JSON
    //    if (m_selectedBlueprint.sceneObjectJson().isEmpty()) {
    //        throw("Error, blueprint must have been selected to have scene object");
    //    }
    //    m_selectedBlueprint = Blueprint(*m_blueprintSceneObject);

    //    m_blueprintSceneObject->removeFromScene();
    //    m_blueprintSceneObject = nullptr;

    //    //m_engine->simulationLoop()->play();
    //}
//}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentTreeWidget::mouseReleaseEvent(QMouseEvent * event)
{
    QTreeWidget::mouseReleaseEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentTreeWidget::dropEvent(QDropEvent * event)
{
    ComponentTreeWidget* widget = static_cast<ComponentTreeWidget*>(event->source());
    if (widget == this) {
        ComponentItem* sourceItem = static_cast<ComponentItem*>(widget->currentItem());
        Q_UNUSED(sourceItem)
        QModelIndex destIndex = indexAt(event->pos());
        if (destIndex.isValid()) {
            // Dropping onto an item
            DropIndicatorPosition dropIndicator = dropIndicatorPosition();
            switch (dropIndicator) {
            case QAbstractItemView::AboveItem:
                // Dropping above an item
                // TODO: Implement reordering
                event->setDropAction(Qt::IgnoreAction);
                break;
            case QAbstractItemView::BelowItem:
                // Dropping below an item
                // TODO: Implement reordering
                event->setDropAction(Qt::IgnoreAction);
            case QAbstractItemView::OnItem:
            {
                // On an item, so ignore
                event->setDropAction(Qt::IgnoreAction);
            }
            case QAbstractItemView::OnViewport:
                // Not on the tree 
                event->setDropAction(Qt::IgnoreAction);
                break;
            }
        }
        else {
            // Dropping above an item
            event->setDropAction(Qt::IgnoreAction);
        }
    }
    else {
        // Ignore drops from another widget
        event->setDropAction(Qt::IgnoreAction);
    }
    QTreeWidget::dropEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentTreeWidget::dragEnterEvent(QDragEnterEvent * event)
{
    QTreeWidget::dragEnterEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentTreeWidget::dragLeaveEvent(QDragLeaveEvent * event)
{
    QTreeWidget::dragLeaveEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentTreeWidget::dragMoveEvent(QDragMoveEvent * event)
{
    QTreeWidget::dragMoveEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentTreeWidget::removeItem(ComponentItem * componentItem)
{
    if ((int)componentItem->componentType() - 2000 < 0) {
        throw("Error, item type is not implemented");
    }

    delete componentItem;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentTreeWidget::initializeWidget()
{
    setMinimumWidth(350);

    // Set tree widget settings
    setColumnCount(0);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setHeaderLabels(QStringList({ "" }));
    setAlternatingRowColors(true);

    // Enable drag and drop
    setDragEnabled(true);
    setDragDropMode(DragDrop);
    setDefaultDropAction(Qt::MoveAction);
    setDragDropOverwriteMode(false); // is default, but making explicit for reference

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    // Scene object components ====================================================================

    // Initialize add renderer component action
    m_addShaderComponent = new QAction(tr("&New Shader Component"), this);
    m_addShaderComponent->setStatusTip("Create a new shader component");
    connect(m_addShaderComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddSceneObjectComponent(m_engine, currentSceneObject().get(), "Create New Shader Component", (int)ComponentType::kShader));
    });

    // Initialize add camera component action
    m_addCameraComponent = new QAction(tr("&New Camera Component"), this);
    m_addCameraComponent->setStatusTip("Create a new camera component");
    connect(m_addCameraComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddSceneObjectComponent(m_engine, currentSceneObject().get(), "Create New Camera", (int)ComponentType::kCamera));
    });

    // Initialize add light component action
    m_addLightComponent = new QAction(tr("&New Light Component"), this);
    m_addLightComponent->setStatusTip("Create a new light component");
    connect(m_addLightComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddSceneObjectComponent(m_engine, currentSceneObject().get(), "Create New Light", (int)ComponentType::kLight));
    });

    // Initialize create script component action
    m_addScriptComponent = new QAction(tr("&New Script Component"), this);
    m_addScriptComponent->setStatusTip("Create a new script component");
    connect(m_addScriptComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddSceneObjectComponent(m_engine, currentSceneObject().get(), "Create New Script", (int)ComponentType::kPythonScript));
    });

    // Initialize create model component action
    m_addModelComponent = new QAction(tr("&New Model Component"), this);
    m_addModelComponent->setStatusTip("Create a new Model component");
    connect(m_addModelComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddSceneObjectComponent(m_engine, currentSceneObject().get(), "Instantiate Model", (int)ComponentType::kModel));
    });

    // Initialize create model component action
    m_addAnimationComponent = new QAction(tr("&New Animation Component"), this);
    m_addAnimationComponent->setStatusTip("Create a new Animation component");
    connect(m_addAnimationComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddSceneObjectComponent(m_engine, currentSceneObject().get(), "Instantiate Animation Component", (int)ComponentType::kBoneAnimation));
    });

    // Initialize create event listener component action
    m_addListenerComponent = new QAction(tr("&New Event Listener Component"), this);
    m_addListenerComponent->setStatusTip("Create a new Event Listener component");
    connect(m_addListenerComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddSceneObjectComponent(m_engine, currentSceneObject().get(), "Instantiate Listener", (int)ComponentType::kListener));
    });

    // Initialize create rigid body component action
    m_addRigidBodyComponent = new QAction(tr("&New Rigid Body Component"), this);
    m_addRigidBodyComponent->setStatusTip("Create a new Rigid Body component");
    connect(m_addRigidBodyComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddSceneObjectComponent(m_engine, currentSceneObject().get(), "Instantiate Rigid Body", (int)ComponentType::kRigidBody));
    });

    // Initialize create canvas component action
    m_addCanvasComponent = new QAction(tr("&New Canvas Component"), this);
    m_addCanvasComponent->setStatusTip("Create a new Canvas component");
    connect(m_addCanvasComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddSceneObjectComponent(m_engine, currentSceneObject().get(), "Instantiate Canvas", (int)ComponentType::kCanvas));
    });

    // Initialize create canvas component action
    m_addCharControllerComponent = new QAction(tr("&New Character Controller Component"), this);
    m_addCharControllerComponent->setStatusTip("Create a new Character Controller component");
    connect(m_addCharControllerComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {
        std::shared_ptr<SceneObject> so = currentSceneObject();
        m_engine->actionManager()->performAction(
            new AddSceneObjectComponent(m_engine, so.get(), "Instantiate Character Controller", (int)ComponentType::kCharacterController));
    });

    // Initialize create cubemap component action
    m_addCubeMapComponent = new QAction(tr("&New Cube Map Component"), this);
    m_addCubeMapComponent->setStatusTip("Create a new Cube Map component");
    connect(m_addCubeMapComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {
        std::shared_ptr<SceneObject> so = currentSceneObject();
        m_engine->actionManager()->performAction(
            new AddSceneObjectComponent(m_engine, so.get(), "Instantiate Cube Map", (int)ComponentType::kCubeMap));
    });

    // Initialize create audio component action
    m_addAudioSourceComponent = new QAction(tr("&New Audio Source Component"), this);
    m_addAudioSourceComponent->setStatusTip("Create a new Audio Source component");
    connect(m_addAudioSourceComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {
        std::shared_ptr<SceneObject> so = currentSceneObject();
        m_engine->actionManager()->performAction(
            new AddSceneObjectComponent(m_engine, so.get(), "Instantiate Audio Source", (int)ComponentType::kAudioSource));
    });

    // Initialize create audio listener component action
    m_addAudioListenerComponent = new QAction(tr("&New Audio Listener Component"), this);
    m_addAudioListenerComponent->setStatusTip("Create a new Audio Listener component");
    connect(m_addAudioListenerComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {
        std::shared_ptr<SceneObject> so = currentSceneObject();
        m_engine->actionManager()->performAction(
            new AddSceneObjectComponent(m_engine, so.get(), "Instantiate Audio Listener", (int)ComponentType::kAudioListener));
    });


    // Scene components ====================================================================
    // Initialize create physics scene component action
    m_addPhysicsSceneComponent = new QAction(tr("&New Physics Scene Component"), this);
    m_addPhysicsSceneComponent->setStatusTip("Create a new Scene Physics component");
    connect(m_addPhysicsSceneComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddPhysicsSceneCommand(m_engine, m_currentScene, "Instantiate Scene Physics"));
    });


    // Initialize delete component action
    m_deleteComponent = new QAction(tr("&Delete Component"), this);
    m_deleteComponent->setStatusTip("Delete the selected component");
    connect(m_deleteComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new DeleteComponentCommand(m_engine,
                currentSceneObject().get(), 
                m_currentComponentItem->component(),
                "Delete Component"));
    });

    // Connect signal for double click events
    connect(this, &ComponentTreeWidget::itemDoubleClicked,
        this, &ComponentTreeWidget::onItemDoubleClicked);

    // Connect signal for item expansion
    connect(this, &ComponentTreeWidget::itemExpanded,
        this, &ComponentTreeWidget::onItemExpanded);

    // Connect signals for scene object selection and deselection
    connect(m_engine->sceneTreeWidget(),
        &SceneTreeWidget::selectedSceneObject,
        this,
        qOverload<size_t>(&ComponentTreeWidget::selectSceneObject));

    connect(m_engine->sceneTreeWidget(),
        &SceneTreeWidget::deselectedSceneObject,
        this,
        &ComponentTreeWidget::clearSceneObject);

    // Connect signals for scene selection and deselection
    connect(m_engine->sceneTreeWidget(),
        &SceneTreeWidget::selectedScene,
        this,
        &ComponentTreeWidget::selectScene);

    connect(m_engine->sceneTreeWidget(),
        &SceneTreeWidget::deselectedScene,
        this,
        &ComponentTreeWidget::clearScene);

    connect(m_engine->eventManager(),
        &EventManager::deletedSceneObjectComponent,
        this,
        &ComponentTreeWidget::selectSceneObject);

    connect(m_engine->eventManager(),
        &EventManager::deletedSceneComponent,
        this,
        &ComponentTreeWidget::selectScene);

    // Add a component to the widget
    connect(m_engine->eventManager(),
        &EventManager::addedComponent,
        this,
        [this](Uuid compID, int typeInt, size_t soID) {
        if (!currentSceneObject()) return;

        if (soID != currentSceneObject()->id()) {
            return;
        }

        ComponentType compType = ComponentType(typeInt);
        Component* component = currentSceneObject()->getComponent(compID, compType);
        if (component) {
            m_engine->componentWidget()->addItem(component);
        }
        else {
            throw("Error, component not found on scene object");
        }
    });

    // Populate from blueprint widget as well
    connect(m_engine->widgetManager()->blueprintWidget(),
        &BlueprintListView::clicked,
        this,
        &ComponentTreeWidget::onSelectedBlueprint);

    // Connect signal for clearing on scenario load
    connect(m_engine, &CoreEngine::scenarioChanged, this, &ComponentTreeWidget::clear);
}
#ifndef QT_NO_CONTEXTMENU
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentTreeWidget::contextMenuEvent(QContextMenuEvent * event)
{
    if (currentSceneObject()) {
        // If a scene object is selected

        // Create menu
        QMenu menu(this);

        // Add actions to the menu
        if (itemAt(event->pos())) {
            auto* item = static_cast<ComponentItem*>(itemAt(event->pos()));
            m_currentComponentItem = item;

            // Add action to delete component
            menu.addAction(m_deleteComponent);
        }
        else {
            // Options to add things when there is no item selected
            if (!currentSceneObject()->hasComponent<ShaderComponent>(ComponentType::kShader)) {
                menu.addAction(m_addShaderComponent);
            }
            if (!currentSceneObject()->hasComponent<CameraComponent>(ComponentType::kCamera)) {
                menu.addAction(m_addCameraComponent);
            }
            // TODO: Set up component dependencies/constraints, and automatically add
            menu.addAction(m_addLightComponent);
            menu.addAction(m_addScriptComponent);
            menu.addAction(m_addModelComponent);
            menu.addAction(m_addAnimationComponent);
            menu.addAction(m_addListenerComponent);
            menu.addAction(m_addRigidBodyComponent);
            menu.addAction(m_addCanvasComponent);
            menu.addAction(m_addCharControllerComponent);
            menu.addAction(m_addCubeMapComponent);
            menu.addAction(m_addAudioSourceComponent);
            menu.addAction(m_addAudioListenerComponent);
        }

        // Display menu at click location
        menu.exec(event->globalPos());
    }
    else if (m_currentScene) {
        // Create menu
        QMenu menu(this);

        // Add actions to the menu
        if (itemAt(event->pos())) {
            auto* item = static_cast<ComponentItem*>(itemAt(event->pos()));
            m_currentComponentItem = item;

            // Add action to delete component
            menu.addAction(m_deleteComponent);
        }
        else {
            menu.addAction(m_addPhysicsSceneComponent);
        }

        // Display menu at click location
        menu.exec(event->globalPos());
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}// View
} // rev