#include "GbSceneTreeWidget.h"

#include <QTreeWidgetItemIterator>

#include "../../core/GbCoreEngine.h"
#include "../../model_control/commands/GbActionManager.h"
#include "../../model_control/commands/commands/GbSceneCommand.h"
#include "../../model_control/models/GbSceneModels.h"

#include "../../core/rendering/renderer/GbMainRenderer.h"

#include "../../core/scene/GbScenario.h"
#include "../../core/scene/GbScene.h"
#include "../../core/scene/GbSceneObject.h"

#include "../../model_control/models/GbSceneModels.h"
#include "../../core/debugging/GbDebugManager.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SceneTreeWidget

unsigned int SceneTreeWidget::NUM_COLUMNS = 2;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SceneTreeWidget::SceneTreeWidget(CoreEngine* core, const QString & name, QWidget * parent) :
    AbstractService(name),
    QTreeWidget(parent),
    m_engine(core),
    m_currentSceneItem(nullptr),
    m_currentSceneObjectItem(nullptr),
    m_lastLeftClickedItem(nullptr),
    m_lastDoubleClickedItem(nullptr)
{
    initializeTreeWidget();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SceneTreeWidget::~SceneTreeWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Scene> SceneTreeWidget::getCurrentScene()
{
    if (m_currentSceneItem) {
        return std::static_pointer_cast<Scene>(m_currentSceneItem->object());
    }
    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SceneObject> SceneTreeWidget::getCurrentSceneObject()
{
    if (m_currentSceneObjectItem) {
        return std::static_pointer_cast<SceneObject>(m_currentSceneObjectItem->object());
    }
    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::setScenarioTreeItem()
{
    auto* scenarioItem = new View::SceneRelatedItem(m_engine->scenario(),
        nullptr,
        View::SceneRelatedItem::kScenario);

    setScenarioTreeItem(scenarioItem);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::setScenarioTreeItem(View::SceneRelatedItem * scenario)
{
    // Clear the widget
    clear();

    // Set header item and disable scene object dragging
    setHeaderItem(scenario);
    invisibleRootItem()->setFlags(invisibleRootItem()->flags() ^ Qt::ItemIsDropEnabled);
    resizeColumns();

    // Set scene tree items
    for (const auto& scenePair : m_engine->scenario()->getScenes()) {
        addSceneTreeItem(scenePair.second);
    }

    // Set debug items
    addSceneTreeItem(m_engine->debugManager()->scene());

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::addSceneTreeItem(std::shared_ptr<Scene> scene)
{
    // Create scene item
    auto* sceneItem = new View::SceneRelatedItem(scene,
        nullptr,
        View::SceneRelatedItem::kScene);

    // Add scene item to widget
    addSceneTreeItem(sceneItem);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::addSceneTreeItem(View::SceneRelatedItem * sceneItem)
{
    // Check that item type is correct
    if (sceneItem->sceneType() != SceneRelatedItem::kScene) {
        throw("Error, added item is not a scene");
    }

    // Insert scene item into widget
    insertTopLevelItem(0, sceneItem);
    resizeColumns();

    // Insert scene objects from scene into widget
    std::shared_ptr<Scene> scene = std::static_pointer_cast<Scene>(sceneItem->object());
    for (const std::shared_ptr<SceneObject>& sceneObject : scene->topLevelSceneObjects()) {
        if (!sceneObject->hasParents()) {
            // Add top-level scene objects
            addSceneObjectTreeItem(sceneObject);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::removeTreeItem(std::shared_ptr<Scene> scene)
{
    removeItem(getItem(scene));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::addSceneObjectTreeItem(const std::shared_ptr<SceneObject>& sceneObject)
{
    // Get parent item
    SceneRelatedItem* parentItem;
    if (sceneObject->hasParents()) {
        parentItem = getItem(sceneObject->parent());
    }
    else {
        parentItem = getItem(sceneObject->scene());
    }

    // Create scene object item
    auto* sceneObjectItem = new View::SceneRelatedItem(sceneObject,
        parentItem,
        View::SceneRelatedItem::kSceneObject);

    // Add scene object item to parent
    if (parentItem) {
        // If a parent scene was found
        parentItem->addChild(sceneObjectItem);
        parentItem->setExpanded(true);
        resizeColumns();
    }

    // Add child scene objects
    if (sceneObject->hasChildren()) {
        for (const std::shared_ptr<SceneObject>& childObject : sceneObject->children()) {
            // Called recursively
            addSceneObjectTreeItem(childObject);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::removeTreeItem(const std::shared_ptr<SceneObject>& sceneObject)
{
    QTreeWidgetItemIterator it(this);
    while (*it) {
        SceneRelatedItem* item = static_cast<SceneRelatedItem*>(*it);
        if (item->object()->getUuid() == sceneObject->getUuid()) {
            removeItem(item);
            break;
        }
        ++it;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
View::SceneRelatedItem * SceneTreeWidget::getItem(std::shared_ptr<Gb::Object> object)
{
    QTreeWidgetItemIterator it(this);
    while (*it) {
        SceneRelatedItem* item = static_cast<SceneRelatedItem*>(*it);
        if (item->object()->getUuid() == object->getUuid()) {
            return item;
        }
        ++it;
    }

    //throw("Error, no item found that corresponds to the given object");

    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::resizeColumns()
{
    for (unsigned int i = 0; i < NUM_COLUMNS; i++) {
        resizeColumnToContents(i);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::clearSelectedItems()
{
    clearSelection();
    m_currentSceneItem = nullptr;
    m_currentSceneObjectItem = nullptr;
    m_lastDoubleClickedItem = nullptr;

    emit deselectedSceneObject();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::removeScenarioTreeItem()
{
    // Delete scenario
    delete headerItem();
    setHeaderItem(nullptr);
    setHeaderLabels(QStringList({""}));

    // Delete scene items from scenario
    clear();

    resizeColumns();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)

    // Downcast item
    auto* sceneItem = static_cast<SceneRelatedItem*>(item);
    if (sceneItem->m_widget) {
        throw("Error, item should not have a widget");
    }

    // Set widget
    sceneItem->setWidget();

    // Set last double-clicked item
    m_lastDoubleClickedItem = sceneItem;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::onItemClicked(QTreeWidgetItem * item, int column)
{
    Q_UNUSED(column)
    
    // Emit signal that last item was deselected
    if(m_lastLeftClickedItem){
        switch (m_lastLeftClickedItem->sceneType()) {
        case SceneRelatedItem::kScene:
            emit deselectedScene();
            break;
        case SceneRelatedItem::kSceneObject:
            emit deselectedSceneObject();
            break;
        default:
            break;
        }
    }

    // Downcast item
    auto* sceneItem = static_cast<SceneRelatedItem*>(item);

    switch (sceneItem->sceneType()) {
    case SceneRelatedItem::kSceneObject:
    {
        m_lastLeftClickedItem = static_cast<SceneRelatedItem*>(item);
        auto sceneObject = std::static_pointer_cast<SceneObject>(sceneItem->object());
        emit selectedSceneObject(sceneObject->scene()->getUuid(), sceneObject->getUuid());
        break;
    }
    case SceneRelatedItem::kScene:
    {
        m_lastLeftClickedItem = static_cast<SceneRelatedItem*>(item);
        auto scene= std::static_pointer_cast<Scene>(sceneItem->object());
        emit selectedScene(scene->getUuid());
        break;
    }
    default:
        break;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::onItemExpanded(QTreeWidgetItem * item)
{
    Q_UNUSED(item)
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void SceneTreeWidget::onCurrentItemChanged(QTreeWidgetItem * item, QTreeWidgetItem * previous)
//{
//}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::mouseReleaseEvent(QMouseEvent * event)
{
    QTreeWidget::mouseReleaseEvent(event);

    if (!m_lastDoubleClickedItem) return;
    QTreeWidgetItem* item = itemAt(event->pos());
    if (!item || item != m_lastDoubleClickedItem) {
        // If item changed
        m_lastDoubleClickedItem->removeWidget();
        m_lastDoubleClickedItem = nullptr;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::dropEvent(QDropEvent * event)
{
    SceneTreeWidget* widget = static_cast<SceneTreeWidget*>(event->source());
    if (widget == this) {
        SceneRelatedItem* sourceItem = static_cast<SceneRelatedItem*>(widget->currentItem());
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
                // On an item, so add current item as child
                SceneRelatedItem* destItem = static_cast<SceneRelatedItem*>(itemFromIndex(destIndex));
                if (sourceItem->sceneType() == SceneRelatedItem::kSceneObject) {
                    switch (destItem->sceneType()) {
                    case SceneRelatedItem::kScene:
                    case SceneRelatedItem::kSceneObject:
                        // Fall through to default, handle drop action in reparent()
                        sourceItem->reparent(destItem);
                    default:
                        event->setDropAction(Qt::IgnoreAction);
                        break;
                    }
                }
                else {
                    event->setDropAction(Qt::IgnoreAction);
                }
                break;
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
void SceneTreeWidget::dragEnterEvent(QDragEnterEvent * event)
{
    QTreeWidget::dragEnterEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::dragLeaveEvent(QDragLeaveEvent * event)
{
    QTreeWidget::dragLeaveEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::dragMoveEvent(QDragMoveEvent * event)
{
    QTreeWidget::dragMoveEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::removeItem(SceneRelatedItem * sceneItem)
{
    switch (sceneItem->sceneType()) {
    case SceneRelatedItem::kScenario:
    case SceneRelatedItem::kScene:
    case SceneRelatedItem::kSceneObject:
        break;
    default:
        throw("Error, item type is not implemented");
        break;
    }

    delete sceneItem;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::initializeTreeWidget()
{
    // Set tree widget settings
    setColumnCount(NUM_COLUMNS);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    setHeaderLabels(QStringList({""}));
    setAlternatingRowColors(true);

    // Enable drag and drop
    setDragEnabled(true);
    setDragDropMode(DragDrop);
    setDefaultDropAction(Qt::MoveAction);
    setDragDropOverwriteMode(false); // is default, but making explicit for reference

    // Initialize create scenario action
    m_addScenario = new QAction(tr("&New Scenario"), this);
    m_addScenario->setStatusTip("Create a new scenario");
    connect(m_addScenario, 
        &QAction::triggered, 
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(new AddScenarioCommand(m_engine, "Create New Scenario")); });

    // Initialize create scene action
    m_addScene = new QAction(tr("&New Scene"), this);
    m_addScene->setStatusTip("Create a new scene");
    connect(m_addScene,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(new AddSceneCommand(m_engine, "Create New Scene")); });

    // Initialize rempove scene action
    m_removeScene = new QAction(tr("&Remove Scene"), this);
    m_removeScene->setStatusTip("Remove the selected scene");
    connect(m_removeScene,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(new RemoveSceneCommand(m_engine, 
            std::static_pointer_cast<Scene>(m_currentSceneItem->object()), 
            "Remove Scene")); });


    // Initialize create scene object action
    m_addSceneObject = new QAction(tr("&New Scene Object"), this);
    m_addSceneObject->setStatusTip("Create a new scene object");
    connect(m_addSceneObject,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {
        
        AddSceneObjectCommand* action;
        if (m_currentSceneObjectItem) {
            // There is a scene object selected, so add new object as a child
            action = new AddSceneObjectCommand(m_engine,
                std::static_pointer_cast<Scene>(m_currentSceneItem->object()),
                "Create New Scene Object",
                std::static_pointer_cast<SceneObject>(m_currentSceneObjectItem->object()));
        }
        else {
            // There is no scene object selected, so adding object without parent
            action = new AddSceneObjectCommand(m_engine,
                std::static_pointer_cast<Scene>(m_currentSceneItem->object()),
                "Create New Scene Object");
        }

        m_engine->actionManager()->performAction(action);

    });

    // Initialize remove scene object action
    m_removeSceneObject = new QAction(tr("&Remove Scene Object"), this);
    m_removeSceneObject->setStatusTip("Remove the selected scene object");
    connect(m_removeSceneObject,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {

        RemoveSceneObjectCommand* action = new RemoveSceneObjectCommand(m_engine,
                std::static_pointer_cast<SceneObject>(m_currentSceneObjectItem->object()),
                "Remove Scene Object");
        m_engine->actionManager()->performAction(action);

    });

    // Connect signal for double click events
    connect(this, &SceneTreeWidget::itemDoubleClicked,
        this, &SceneTreeWidget::onItemDoubleClicked);

    // Connect signal for click events
    connect(this, &SceneTreeWidget::itemClicked,
        this, &SceneTreeWidget::onItemClicked);

    // Connect signal for item expansion
    connect(this, &SceneTreeWidget::itemExpanded,
        this, &SceneTreeWidget::onItemExpanded);

    // Connect signal for change to scenario from core engine
    connect(m_engine, 
        &CoreEngine::scenarioChanged,
        this,
        static_cast<void(SceneTreeWidget::*)(void)>(&SceneTreeWidget::setScenarioTreeItem));

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef QT_NO_CONTEXTMENU
void SceneTreeWidget::contextMenuEvent(QContextMenuEvent *event)
{
    // Create menu
    QMenu menu(this);

    // Add actions to the menu
    if (itemAt(event->pos())) {
        auto* item = static_cast<SceneRelatedItem*>(itemAt(event->pos()));
        switch (item->sceneType()) {
        case SceneRelatedItem::kScene: {
            m_currentSceneObjectItem = nullptr;
            m_currentSceneItem = item;
            menu.addAction(m_addSceneObject);
            menu.addAction(m_removeScene);
            break;
        }
        case SceneRelatedItem::kSceneObject:
        {
            m_currentSceneObjectItem = item;
            m_currentSceneItem = getItem(std::static_pointer_cast<SceneObject>(item->object())->scene());
            menu.addAction(m_addSceneObject);
            menu.addAction(m_removeSceneObject);
            if (getCurrentSceneObject()->isPythonGenerated()) {
                // Disable action if scene object is python generated
                menu.actions().back()->setDisabled(true);
            }
            else {
                menu.actions().back()->setDisabled(false);
            }
            break;
        }
        default:
            break;
        }
    }
    else {
        // Options to add things when there is no item selected
        menu.addAction(m_addScenario);
        menu.addAction(m_addScene);
    }

    // Display menu at click location
    menu.exec(event->globalPos());
}
#endif // QT_NO_CONTEXTMENU


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb