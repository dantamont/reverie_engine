#include "GSceneTreeWidget.h"

#include <QTreeWidgetItemIterator>

#include "../../core/GCoreEngine.h"
#include "../../model_control/commands/GActionManager.h"
#include "../../model_control/commands/commands/GSceneCommand.h"

#include "../../core/rendering/renderer/GMainRenderer.h"

#include "../../core/scene/GScenario.h"
#include "../../core/scene/GScene.h"
#include "../../core/scene/GSceneObject.h"

#include "../../core/debugging/GDebugManager.h"

//#include "../../core/canvas/GFontManager.h"
#include <core/containers/GColor.h>
#include <view/style/GFontIcon.h>

#include <view/list/GBlueprintView.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {
namespace View {

SceneRelatedItem::SceneRelatedItem(SceneObject * so):
    TreeItem(so, (int)SceneType::kSceneObject)
{
    initializeItem();
}

SceneRelatedItem::SceneRelatedItem(Scenario * scenario):
    TreeItem(scenario, (int)SceneType::kScenario)
{
    initializeItem();
}

SceneRelatedItem::SceneRelatedItem(Scene * scene):
    TreeItem(scene, (int)SceneType::kScene)
{
    initializeItem();
}

QVariant SceneRelatedItem::mimeData(int role) const
{
    QVariant outData;
    if (role == Qt::UserRole) {
        // Id
        switch (sceneType()) {
        case SceneRelatedItem::kSceneObject:
            outData = Uuid(dynamic_cast<SceneObject*>(m_object)->id());
            break;
        case SceneRelatedItem::kScene:
            outData = dynamic_cast<Scene*>(m_object)->getUuid();
            break;
        default:
            break;
        }
    }
    else if(role == Qt::UserRole + 1){
        // Type
        switch (sceneType()) {
        case SceneRelatedItem::kSceneObject:
            outData = (int)BlueprintModelDragTypes::kSceneObject;
            break;
        default:
            outData = -1;
            break;
        }
    }
    return outData;
}

void SceneRelatedItem::reparent(SceneRelatedItem * newParent, int index)
{
    // Assert type
    if (sceneType() != SceneRelatedItem::kSceneObject) {
        throw("Error, only scene objects may be reparented");
    }
    SceneObject* sceneObject = static_cast<SceneObject*>(object());

    // Set encapsulated object's parent
    switch (newParent->sceneType()) {
    case SceneRelatedItem::kScene: {
        auto* action = new ReparentSceneObjectCommand(nullptr,
            sceneTreeWidget()->m_engine,
            sceneObject,
            static_cast<Scene*>(newParent->object()),
            index);
        action->perform();
        break;
    }
    case SceneRelatedItem::kSceneObject: {
        auto* action = new ReparentSceneObjectCommand(static_cast<SceneObject*>(newParent->object()),
            sceneTreeWidget()->m_engine,
            sceneObject,
            sceneObject->scene(),
            index);
        action->perform();
        break;
    }
    default:
        throw("Error, scenario cannot be reparented");
        break;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneRelatedItem::performAction(UndoCommand * command)
{
    // Add command to action manager
    sceneTreeWidget()->m_engine->actionManager()->performAction(command);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneRelatedItem::setWidget()
{
    if (m_widget) {
        throw("Error, item already has a widget");
    }

    SceneTreeWidget * parentWidget = static_cast<SceneTreeWidget*>(treeWidget());

    // Set Widget
    QString name;
    switch (sceneType()) {
    case kScenario:
        break;
    case kScene:
    {
        // Create widget
        name = QString(nameable()->getName().c_str());
        m_widget = new QLineEdit(name);
        m_widget->setFocusPolicy(Qt::StrongFocus);
        //m_widget->show();

        // Set signal for widget value change
        QObject::connect(static_cast<QLineEdit*>(m_widget),
            &QLineEdit::editingFinished,
            static_cast<QLineEdit*>(m_widget),
            [this]() {
            if (!m_widget) {
                return;
            }
            SceneTreeWidget * parentWidget = static_cast<SceneTreeWidget*>(treeWidget());
            QString newName = static_cast<QLineEdit*>(m_widget)->text();
            if (newName.isEmpty()) {
                newName = nameable()->getName().c_str();
            }
            Scene* scene = dynamic_cast<Scene*>(object());
            performAction(new rev::ChangeNameCommand(newName, parentWidget->m_engine, scene));

        }
        );

        // Set signal for widget out of focus
        QObject::connect(treeWidget(),
            &QTreeWidget::itemSelectionChanged,
            m_widget,
            [this]() {
            if (m_widget) {
                removeWidget(1);
            }
        }
        );

        break;
    }
    case kSceneObject:
    {
        // Create widget
        name = QString(nameable()->getName().c_str());
        m_widget = new QLineEdit(name);
        m_widget->setFocusPolicy(Qt::StrongFocus);
        //m_widget->show();

        // Set signal for widget value change
        QObject::connect(static_cast<QLineEdit*>(m_widget),
            &QLineEdit::editingFinished,
            static_cast<QLineEdit*>(m_widget),
            [this]() {
            if (!m_widget) {
                return;
            }
            SceneTreeWidget * parentWidget = static_cast<SceneTreeWidget*>(treeWidget());
            QString newName = static_cast<QLineEdit*>(m_widget)->text();
            if (newName.isEmpty()) {
                newName = nameable()->getName().c_str();
            }
            SceneObject* so = dynamic_cast<SceneObject*>(object());
            performAction(new rev::ChangeNameCommand(newName, parentWidget->m_engine, so));
        }
        );

        // Set signal for widget out of focus
        QObject::connect(treeWidget(),
            &QTreeWidget::itemSelectionChanged,
            m_widget,
            [this]() {
            if (m_widget) {
                removeWidget(1);
            }
        }
        );

        break;
    }
    default:
        throw("Error, type of item is not implemented");
        break;
    }

    if (m_widget) {
        // Assign widget to item in tree widget
        parentWidget->setItemWidget(this, parentWidget->m_numColumns - 1, m_widget);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SceneTreeWidget * SceneRelatedItem::sceneTreeWidget() const
{
    return static_cast<View::SceneTreeWidget*>(treeWidget());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneRelatedItem::initializeItem()
{
    // Set column text
    refreshText();

    // Set flags
    if (sceneType() != kScenario) {
        setFlags(flags() | (Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled));
    }

    if (sceneType() == kSceneObject) {
        // Is scene object, set background color if auto-generated
        auto sceneObject = static_cast<SceneObject*>(m_object);
        if (sceneObject->isScriptGenerated()) {
            setBackground(0, QBrush(Color(205, 125, 146)));
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneRelatedItem::refreshText()
{
    auto obj = object();
    switch (sceneType()) {
    case kScenario:
        setText(0, obj->className());
        break;
    case kScene:
        setIcon(0, SAIcon("box-open"));
        break;
    case kSceneObject:
        setIcon(0, SAIcon("box"));
        break;
    default:
        throw("Error, this item type is not implemented");
        break;
    }

    setText(1, nameable()->getName().c_str());
}





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SceneTreeWidget

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SceneTreeWidget::SceneTreeWidget(CoreEngine* core, const QString & name, QWidget * parent) :
    TreeWidget(core, name, parent, 2),
    m_currentSceneItem(nullptr),
    m_currentSceneObjectItem(nullptr),
    m_lastLeftClickedItem(nullptr)
{
    initializeWidget();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SceneTreeWidget::~SceneTreeWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Scene* SceneTreeWidget::getCurrentScene()
{
    if (m_currentSceneItem) {
        return dynamic_cast<Scene*>(m_currentSceneItem->object());
    }
    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SceneObject* SceneTreeWidget::getCurrentSceneObject()
{
    if (m_currentSceneObjectItem) {
        return dynamic_cast<SceneObject*>(m_currentSceneObjectItem->object());
    }
    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::setScenarioTreeItem()
{
    Scenario* scenario = m_engine->scenario().get();
    auto* scenarioItem = new View::SceneRelatedItem(scenario);

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

    // Set debug items
    addSceneTreeItem(m_engine->debugManager()->scene().get());

    // Set scene tree items
    addSceneTreeItem(&m_engine->scenario()->scene());

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::addSceneTreeItem(Scene* scene)
{
    // Create scene item
    auto* sceneItem = new View::SceneRelatedItem(scene);

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
    //insertTopLevelItem(0, sceneItem);
    addItem(sceneItem);

    // Insert scene objects from scene into widget
    Scene* scene = static_cast<Scene*>(sceneItem->object());
    for (const std::shared_ptr<SceneObject>& sceneObject : scene->topLevelSceneObjects()) {
        if (!sceneObject->hasParents()) {
            // Add top-level scene objects
            addSceneObjectTreeItem(sceneObject.get());
        }
    }
}
////////////////////////////////////////////////////////////////////i///////////////////////////////////////////////////
void SceneTreeWidget::removeTreeItem(Scene* scene)
{
    removeItem(getItem(scene));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::addSceneObjectTreeItem(SceneObject* sceneObject, int index)
{
    // Get parent item
    SceneRelatedItem* parentItem;
    if (sceneObject->hasParents()) {
        parentItem = getItem(sceneObject->parent().get());
    }
    else {
        parentItem = getItem(sceneObject->scene());
    }

    // Create scene object item
    auto* sceneObjectItem = new View::SceneRelatedItem(sceneObject);

    // Add scene object item to parent
    if (parentItem) {
        // If a parent scene was found
        if (index < 0) {
            parentItem->addChild(sceneObjectItem);
        }
        else {
            parentItem->insertChild(index, sceneObjectItem);
        }
        parentItem->setExpanded(true);

        if (parentItem->parent()) {
            // Resize to fit additional parent levels
            resizeColumns(); 
        }
    }
    else {
        throw("Must have either scene or scene object parent");
    }

    // Add child scene objects
    if (sceneObject->hasChildren()) {
        for (const std::shared_ptr<SceneObject>& childObject : sceneObject->children()) {
            // Called recursively
            addSceneObjectTreeItem(childObject.get());
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::removeTreeItem(SceneObject* sceneObject)
{
    QTreeWidgetItemIterator it(this);
    while (*it) {
        SceneRelatedItem* item = static_cast<SceneRelatedItem*>(*it);
        
        if (item->sceneType() != SceneRelatedItem::SceneType::kSceneObject) {
            ++it;
            continue;
        }

        auto so = dynamic_cast<SceneObject*>(item->object());
        if (so->id() == sceneObject->id()) {
            removeItem(item);
            break;
        }
        ++it;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
View::SceneRelatedItem * SceneTreeWidget::getItem(SceneObject* object)
{
    QTreeWidgetItemIterator it(this);
    while (*it) {
        SceneRelatedItem* item = static_cast<SceneRelatedItem*>(*it);
        if (item->sceneType() != SceneRelatedItem::kSceneObject) {
            ++it;
            continue;
        }
        auto so = dynamic_cast<SceneObject*>(item->object());
        if (so->id() == object->id()) {
            return item;
        }
        ++it;
    }

    //throw("Error, no item found that corresponds to the given object");

    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
View::SceneRelatedItem * SceneTreeWidget::getItem(Scene* object)
{
    QTreeWidgetItemIterator it(this);
    while (*it) {
        SceneRelatedItem* item = static_cast<SceneRelatedItem*>(*it);
        if (item->sceneType() != SceneRelatedItem::kScene) {
            ++it;
            continue;
        }
        if (item->identifier()->getUuid() == dynamic_cast<Identifiable*>(object)->getUuid()) {
            return item;
        }
        ++it;
    }

    //throw("Error, no item found that corresponds to the given object");

    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
View::SceneRelatedItem * SceneTreeWidget::getItem(Scenario* object)
{
    QTreeWidgetItemIterator it(this);
    while (*it) {
        SceneRelatedItem* item = static_cast<SceneRelatedItem*>(*it);
        if (item->sceneType() != SceneRelatedItem::kScenario) {
            ++it;
            continue;
        }
        if (item->identifier()->getUuid() == dynamic_cast<Identifiable*>(object)->getUuid()) {
            return item;
        }
        ++it;
    }

    //throw("Error, no item found that corresponds to the given object");

    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::clearSelectedItems()
{
    clearSelection();
    m_currentSceneItem = nullptr;
    m_currentSceneObjectItem = nullptr;

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

    //resizeColumns();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)

    // Downcast item
    auto* sceneItem = static_cast<SceneRelatedItem*>(item);
    if (!sceneItem->m_widget) {
        // Set widget if there isn't one cached by scene tree widget
        // TODO: Investigate if TreeWidget will cause a memory leak by caching item widgets
        sceneItem->setWidget();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::onItemClicked(QTreeWidgetItem * item, int column)
{
    Q_UNUSED(column)
    
    // Downcast item
    auto* sceneItem = static_cast<SceneRelatedItem*>(item);

    if (m_lastLeftClickedItem == sceneItem) return;

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

    switch (sceneItem->sceneType()) {
    case SceneRelatedItem::kSceneObject:
    {
        m_lastLeftClickedItem = static_cast<SceneRelatedItem*>(item);
        auto sceneObject = dynamic_cast<SceneObject*>(sceneItem->object());
        emit selectedSceneObject(sceneObject->id());
        break;
    }
    case SceneRelatedItem::kScene:
    {
        m_lastLeftClickedItem = static_cast<SceneRelatedItem*>(item);
        auto scene= dynamic_cast<Scene*>(sceneItem->object());
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
QMimeData * SceneTreeWidget::mimeData(const QList<QTreeWidgetItem*> items) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedIdData;
    QByteArray encodedTypeData;

    QDataStream idStream(&encodedIdData, QIODevice::WriteOnly);
    QDataStream typeStream(&encodedTypeData, QIODevice::WriteOnly);
    for (QTreeWidgetItem* it : items) {
        SceneRelatedItem* item = dynamic_cast<SceneRelatedItem*>(it);
        //size_t idx = index.row();
        Uuid uuid = item->mimeData(Qt::UserRole).toUuid();
//        QString uuidStr = uuid.toString();
//#ifdef DEBUG_MODE
//        Logger::LogInfo(uuidStr);
//#endif
        idStream << uuid;

        int itemType = item->mimeData(Qt::UserRole + 1).toInt();
        typeStream << itemType; // Type of item
    }

    mimeData->setData("reverie/id/uuid", encodedIdData);
    mimeData->setData("reverie/id/type", encodedTypeData);

    return mimeData;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QStringList SceneTreeWidget::mimeTypes() const
{
    QStringList types;
    types << "reverie/id/uuid";
    types << "reverie/id/type";
    return types;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::initializeItem(QTreeWidgetItem * item)
{
    Q_UNUSED(item);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::onDropOn(QDropEvent * event, QTreeWidgetItem * source, QTreeWidgetItem * destination)
{
    // On an item, so add current item as child
    SceneRelatedItem* sourceItem = static_cast<SceneRelatedItem*>(source);
    SceneRelatedItem* destItem = static_cast<SceneRelatedItem*>(destination);

    if (sourceItem->sceneType() == SceneRelatedItem::kSceneObject) {
        // Item being dropped is a scene object
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

    // If not set, Qt will do its own reparenting, which DOESN'T WORK
    event->setDropAction(Qt::IgnoreAction);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::onDropBelow(QDropEvent * event, QTreeWidgetItem * source, QTreeWidgetItem * destination)
{
    // On an item, so add current item as child
    SceneRelatedItem* sourceItem = static_cast<SceneRelatedItem*>(source);
    SceneRelatedItem* destItem = static_cast<SceneRelatedItem*>(destination);
    SceneRelatedItem* parentItem = static_cast<SceneRelatedItem*>(destination->parent());

    if (sourceItem->sceneType() == SceneRelatedItem::kSceneObject) {
        // Item being dropped is a scene object
        // If no parent destination item, set to this source item's parent scene
        int index = -1;
        if (!parentItem) {
            parentItem = getItem(&m_engine->scenario()->scene());
        }
        else {
            // Get at which to insert new item
            index = parentItem->indexOfChild(destItem);
        }



        // Act based on destination item type
        switch (parentItem->sceneType()) {
        case SceneRelatedItem::kScene:
        case SceneRelatedItem::kSceneObject:
            // Fall through to default, handle drop action in reparent()
            sourceItem->reparent(parentItem, index);
        default:
            event->setDropAction(Qt::IgnoreAction);
            break;
        }
    }

    // If not set, Qt will do its own reparenting, which DOESN'T WORK
    event->setDropAction(Qt::IgnoreAction);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::onDropAbove(QDropEvent * event, QTreeWidgetItem * source, QTreeWidgetItem * destination)
{
    // On an item, so add current item as child
    SceneRelatedItem* sourceItem = static_cast<SceneRelatedItem*>(source);
    SceneRelatedItem* destItem = static_cast<SceneRelatedItem*>(destination);
    SceneRelatedItem* parentItem = static_cast<SceneRelatedItem*>(destination->parent());

    if (sourceItem->sceneType() == SceneRelatedItem::kSceneObject) {
        // Item being dropped is a scene object
        // If no destination item, set to this source item's parent scene
        int index = -1;
        if (!parentItem) {
            parentItem = getItem(&m_engine->scenario()->scene());
        }
        else {
            // Get at which to insert new item
            index = parentItem->indexOfChild(destItem);
        }

        // Act based on destination item type
        switch (parentItem->sceneType()) {
        case SceneRelatedItem::kScene:
        case SceneRelatedItem::kSceneObject:
            // Fall through to default, handle drop action in reparent()
            sourceItem->reparent(parentItem, index);
        default:
            event->setDropAction(Qt::IgnoreAction);
            break;
        }
    }

    // If not set, Qt will do its own reparenting, which DOESN'T WORK
    event->setDropAction(Qt::IgnoreAction);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::onDropViewport(QDropEvent * event, QTreeWidgetItem * source)
{
    // On viewport, so set as child of scenario's scene
    SceneRelatedItem* sourceItem = static_cast<SceneRelatedItem*>(source);
    SceneRelatedItem* sceneItem = getItem(&m_engine->scenario()->scene());
    if (sourceItem->sceneType() == SceneRelatedItem::kSceneObject) {
        sourceItem->reparent(sceneItem);
    }

    // If not set, Qt will do its own reparenting, which DOESN'T WORK
    event->setDropAction(Qt::IgnoreAction);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneTreeWidget::onDropFromOtherWidget(QDropEvent * event, QTreeWidgetItem * source, QWidget * otherWidget)
{
    //SceneRelatedItem* sourceItem = static_cast<SceneRelatedItem*>(source);
    //SceneRelatedItem* destItem = static_cast<SceneRelatedItem*>(destination);
    Logger::LogInfo("Dropped blueprint");
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
void SceneTreeWidget::initializeWidget()
{
    TreeWidget::initializeWidget();

    // Set tree widget settings
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    setHeaderLabels(QStringList({""}));
    setAlternatingRowColors(true);

    // Set first column width
    //QFontMetrics metrics(FontManager::solidFontAwesomeFamily());
    //setColumnWidth(0, metrics.width("\uf468") * 3);
    setColumnWidth(0, columnWidth(0) * 0.75);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    // Enable drag and drop
    enableDragAndDrop();

    // Initialize create scenario action
    m_addScenario = new QAction(tr("&New Scenario"), this);
    m_addScenario->setStatusTip("Create a new scenario");
    connect(m_addScenario, 
        &QAction::triggered, 
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(new AddScenarioCommand(m_engine, "Create New Scenario")); });


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
                dynamic_cast<Scene*>(m_currentSceneItem->object()),
                "Add Child Object",
                dynamic_cast<SceneObject*>(m_currentSceneObjectItem->object()));
        }
        else {
            // There is no scene object selected, so adding object without parent
            action = new AddSceneObjectCommand(m_engine,
                dynamic_cast<Scene*>(m_currentSceneItem->object()),
                "Create New Scene Object");
        }

        m_engine->actionManager()->performAction(action);

    });

    // Initialize copy scene object action
    m_copySceneObject = new QAction(tr("&Copy Scene Object"), this);
    m_copySceneObject->setStatusTip("Duplicate the selected scene object");
    connect(m_copySceneObject,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {

        size_t index = m_currentItems[kContextClick]->parent()->indexOfChild(m_currentSceneObjectItem);
        CopySceneObjectCommand* action = new CopySceneObjectCommand(m_engine,
                dynamic_cast<Scene*>(m_currentSceneItem->object()),
                dynamic_cast<SceneObject*>(m_currentSceneObjectItem->object()),
                index,
                "Copy Scene Object");

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
                dynamic_cast<SceneObject*>(m_currentSceneObjectItem->object()),
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

    // Initialize signal for debug manager selecting an object
    connect(m_engine->debugManager(), 
        &DebugManager::selectedSceneObject,
        this,
        [this](size_t sceneObjectId) {
            View::SceneRelatedItem* item = getItem(SceneObject::Get(sceneObjectId).get());
            setCurrentItem(item);
            emit selectedSceneObject(sceneObjectId); 
    });

    //connect(m_engine,
    //    &CoreEngine::scenarioLoaded,
    //    this,
    //    [&]() {
    //    m_currentSceneItem = getItem(m_engine->scenario());
    //});
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool SceneTreeWidget::dropMimeData(QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action)
{
    bool success = true;
    if (!data->hasFormat("reverie/id/uuid")) {
        success = QTreeWidget::dropMimeData(parent, index, data, action);
    }
    else {
        // Handling drops from other widgets
        QByteArray encodedIdData = data->data("reverie/id/uuid");
        QByteArray encodedTypeData = data->data("reverie/id/type");
        QDataStream stream(&encodedIdData, QIODevice::ReadOnly);
        QDataStream typeStream(&encodedTypeData, QIODevice::ReadOnly);
        Uuid itemId;
        int itemType = -1;
        while (!stream.atEnd()) {
            // Should only do this once, but using this since reinterpret_cast with encoded data can be a mess
            stream >> itemId;
        }
        while (!typeStream.atEnd()) {
            // Won't be reached if no type
            typeStream >> itemType;
        }

        // Only blueprints are valid for drop right now
        std::vector<Blueprint>& blueprints = m_engine->scenario()->blueprints();
        auto iter = std::find_if(blueprints.begin(), blueprints.end(),
            [&itemId](const Blueprint& bp) {return bp.getUuid() == itemId; });
        if (iter == blueprints.end()) {
            throw("Error, blueprint not found");
        }
        Blueprint& bp = *iter;

        SceneRelatedItem* sceneItem = dynamic_cast<SceneRelatedItem*>(parent);
        if (sceneItem) {
            switch (sceneItem->sceneType()) {
            case SceneRelatedItem::kScene:
            {
                Scene* scene = dynamic_cast<Scene*>(sceneItem->object());
                //Logger::LogInfo(GString::Format("Dropping blueprint %s onto scene %s", bp.getName().c_str(),scene->getName().c_str()).c_str());
                std::shared_ptr<SceneObject> so = SceneObject::Create(scene);
                so->loadFromJson(bp.sceneObjectJson());
                break;
            }
            case SceneRelatedItem::kSceneObject:
            {
                SceneObject* parent = dynamic_cast<SceneObject*>(sceneItem->object());
                //Logger::LogInfo(GString::Format("Dropping blueprint %s onto scene object %s", bp.getName().c_str(),so->getName().c_str()).c_str());
                std::shared_ptr<SceneObject> so = SceneObject::Create(m_engine, bp.sceneObjectJson());
                so->setParent(SceneObject::Get(parent->id()));
                break;
            }
            default:
                throw("Not recognized");
                break;
            }
            emit m_engine->scenarioChanged();
        }
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef QT_NO_CONTEXTMENU
void SceneTreeWidget::contextMenuEvent(QContextMenuEvent *event)
{
    // Create menu
    QMenu menu(this);

    // Add actions to the menu
    auto* item = static_cast<SceneRelatedItem*>(itemAt(event->pos()));
    if (item) {
        m_currentItems[kContextClick] = item;

        switch (item->sceneType()) {
        case SceneRelatedItem::kScene: {
            m_currentSceneObjectItem = nullptr;
            m_currentSceneItem = item;
            if (m_currentSceneItem->nameable()->getName() != "Debug Objects") {
                menu.addAction(m_addSceneObject);
            }
            break;
        }
        case SceneRelatedItem::kSceneObject:
        {
            m_currentSceneObjectItem = item;
            m_currentSceneItem = getItem(dynamic_cast<SceneObject*>(item->object())->scene());
            menu.addAction(m_addSceneObject);
            menu.addAction(m_copySceneObject);
            menu.addAction(m_removeSceneObject);
            if (getCurrentSceneObject()->isScriptGenerated()) {
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
        m_currentItems[kContextClick] = nullptr;

        // Options to add things when there is no item selected
        if (getCurrentScene()) {
            if (getCurrentScene() != m_engine->debugManager()->scene().get()) {
                menu.addAction(m_addSceneObject);
                menu.addSeparator();
            }
        }
        menu.addAction(m_addScenario);
    }

    // Display menu at click location
    menu.exec(event->globalPos());
}
#endif // QT_NO_CONTEXTMENU


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // rev