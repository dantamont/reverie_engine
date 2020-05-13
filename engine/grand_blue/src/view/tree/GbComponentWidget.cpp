#include "GbComponentWidget.h"
#include "GbSceneTreeWidget.h"

#include "../../core/GbCoreEngine.h"
#include "../../core/events/GbEventManager.h"
#include "../../model_control/commands/GbActionManager.h"
#include "../../model_control/commands/commands/GbComponentCommand.h"
#include "../parameters/GbComponentWidgets.h"

#include "../../core/geometry/GbTransform.h"
#include "../../core/scene/GbScenario.h"
#include "../../core/scene/GbScene.h"
#include "../../core/scene/GbSceneObject.h"
#include "../../core/components/GbComponent.h"
#include "../../model_control/models/GbComponentModels.h"
#include "../../core/debugging/GbDebugManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {
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
void ComponentTreeWidget::selectObject(const Uuid& id, int parentType) {
    Component::ComponentType type = Component::ComponentType(parentType);
    switch (type) {
    case Component::kScene:
        selectScene(id);
        break;
    case Component::kSceneObject:
        selectSceneObject(id);
        break;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentTreeWidget::selectScene(const Uuid& sceneID) {
    clearSceneObject();

    // Set new scene object
    m_currentScene = m_engine->scenario()->getScene(sceneID);

    // Return if no scene
    if (!currentScene()) {
#ifdef DEBUG_MODE
        logWarning("selectSceneObject::Warning, no scene found");
#endif
        return;
    }

    // Populate with components of new scene
    for (const auto& componentListPair : currentScene()->components()) {
        for (const auto& comp : componentListPair.second) {
            addItem(comp);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentTreeWidget::clearScene()
{    // Clear the widget
    clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Deprecate this function
void ComponentTreeWidget::selectSceneObject(const Uuid& sceneID, const Uuid& sceneObjectID) {
    // Clear the widget
    clearScene();

    // Set new scene object
    std::shared_ptr<Scene> sc = m_engine->scenario()->getScene(sceneID);
    if (!sc) {
        // Scene belongs to debug manager if not in scenario
        sc = m_engine->debugManager()->scene();
    }
    m_currentSceneObject = sc->getSceneObject(sceneObjectID);

    // Return if no new scene object
    if (!currentSceneObject()) {
#ifdef DEBUG_MODE
        logWarning("selectSceneObject::Warning, no scene object found");
#endif
        return;
    }

    // Add transform component
    addItem((Component*)currentSceneObject()->transform().get());

    // Populate with components of new scene object
    for (const auto& componentListPair : currentSceneObject()->components()) {
        for (const auto& comp : componentListPair.second) {
            addItem(comp);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentTreeWidget::selectSceneObject(const Uuid & sceneObjectID)
{
    // Clear the widget
    clearScene();
    m_currentSceneObject = SceneObject::get(sceneObjectID);

    // Return if no new scene object
    if (!currentSceneObject()) {
#ifdef DEBUG_MODE
        logWarning("selectSceneObject::Warning, no scene object found");
#endif
        return;
    }

    // Add transform component
    addItem((Component*)currentSceneObject()->transform().get());

    // Populate with components of new scene object
    for (const auto& componentListPair : currentSceneObject()->components()) {
        for (const auto& comp : componentListPair.second) {
            addItem(comp);
        }
    }
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
    resizeColumns();
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

    // Scene object components ====================================================================

    // Initialize add renderer component action
    m_addRendererComponent = new QAction(tr("&New Renderer Component"), this);
    m_addRendererComponent->setStatusTip("Create a new renderer component");
    connect(m_addRendererComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddSceneObjectComponent(m_engine, currentSceneObject(), "Create New Renderer", Component::kRenderer));
    });

    // Initialize add camera component action
    m_addCameraComponent = new QAction(tr("&New Camera Component"), this);
    m_addCameraComponent->setStatusTip("Create a new camera component");
    connect(m_addCameraComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddSceneObjectComponent(m_engine, currentSceneObject(), "Create New Camera", Component::kCamera));
    });

    // Initialize add light component action
    m_addLightComponent = new QAction(tr("&New Light Component"), this);
    m_addLightComponent->setStatusTip("Create a new light component");
    connect(m_addLightComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddSceneObjectComponent(m_engine, currentSceneObject(), "Create New Light", Component::kLight));
    });

    // Initialize create script component action
    m_addScriptComponent = new QAction(tr("&New Script Component"), this);
    m_addScriptComponent->setStatusTip("Create a new script component");
    connect(m_addScriptComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddSceneObjectComponent(m_engine, currentSceneObject(), "Create New Script", Component::kPythonScript));
    });

    // Initialize create model component action
    m_addModelComponent = new QAction(tr("&New Model Component"), this);
    m_addModelComponent->setStatusTip("Create a new Model component");
    connect(m_addModelComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddSceneObjectComponent(m_engine, currentSceneObject(), "Instantiate Model", Component::kModel));
    });

    // Initialize create listener component action
    m_addListenerComponent = new QAction(tr("&New Listener Component"), this);
    m_addListenerComponent->setStatusTip("Create a new Listener component");
    connect(m_addListenerComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddSceneObjectComponent(m_engine, currentSceneObject(), "Instantiate Listener", Component::kListener));
    });

    // Initialize create rigid body component action
    m_addRigidBodyComponent = new QAction(tr("&New Rigid Body Component"), this);
    m_addRigidBodyComponent->setStatusTip("Create a new Rigid Body component");
    connect(m_addRigidBodyComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddSceneObjectComponent(m_engine, currentSceneObject(), "Instantiate Rigid Body", Component::kRigidBody));
    });

    // Initialize create canvas component action
    m_addCanvasComponent = new QAction(tr("&New Canvas Component"), this);
    m_addCanvasComponent->setStatusTip("Create a new Canvas component");
    connect(m_addCanvasComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddSceneObjectComponent(m_engine, currentSceneObject(), "Instantiate Canvas", Component::kCanvas));
    });

    // Initialize create canvas component action
    m_addCharControllerComponent = new QAction(tr("&New Character Controller Component"), this);
    m_addCharControllerComponent->setStatusTip("Create a new Character Controller component");
    connect(m_addCharControllerComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddSceneObjectComponent(m_engine, currentSceneObject(), "Instantiate Character Controller", Component::kCharacterController));
    });

    // Scene components ====================================================================
    // Initialize create physics scene component action
    m_addPhysicsSceneComponent = new QAction(tr("&New Physics Scene Component"), this);
    m_addPhysicsSceneComponent->setStatusTip("Create a new Scene Physics component");
    connect(m_addPhysicsSceneComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddPhysicsSceneCommand(m_engine, currentScene(), "Instantiate Scene Physics"));
    });


    // Initialize delete component action
    m_deleteComponent = new QAction(tr("&Delete Component"), this);
    m_deleteComponent->setStatusTip("Delete the selected component");
    connect(m_deleteComponent,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new DeleteComponentCommand(m_engine,
                currentSceneObject(), 
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
        qOverload<const Uuid&, const Uuid&>(&ComponentTreeWidget::selectSceneObject));

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
        &EventManager::deletedComponent,
        this,
        &ComponentTreeWidget::selectObject);

    // Connect signal for clearing on scenario load
    connect(m_engine, &CoreEngine::scenarioChanged, this, &ComponentTreeWidget::clear);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentTreeWidget::contextMenuEvent(QContextMenuEvent * event)
{
    // If a scene object is selected
    if (currentSceneObject()) {

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
            if (!currentSceneObject()->hasRenderer()) {
                menu.addAction(m_addRendererComponent);
            }
            if (!currentSceneObject()->hasCamera()) {
                menu.addAction(m_addCameraComponent);
            }
            // TODO: Set up component dependencies/constraints, and automatically add
            menu.addAction(m_addLightComponent);
            menu.addAction(m_addScriptComponent);
            menu.addAction(m_addModelComponent);
            menu.addAction(m_addListenerComponent);
            menu.addAction(m_addRigidBodyComponent);
            menu.addAction(m_addCanvasComponent);
            menu.addAction(m_addCharControllerComponent);
        }

        // Display menu at click location
        menu.exec(event->globalPos());
    }
    else if (currentScene()) {
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


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}// View
} // Gb