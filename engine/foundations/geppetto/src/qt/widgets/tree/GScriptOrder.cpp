#include "geppetto/qt/widgets/tree/GScriptOrder.h"

#include "fortress/json/GJson.h"
#include "fortress/containers/GSortingLayer.h"

#include "geppetto/qt/widgets/GWidgetManager.h"

#include "ripple/network/gateway/GMessageGateway.h"
#include "ripple/network/messages/GGetScenarioJsonMessage.h"
#include "ripple/network/messages/GScenarioJsonMessage.h"

#include "ripple/network/messages/GReorderScriptProcessesMessage.h"
#include "ripple/network/messages/GAddScriptProcessLayerMessage.h"
#include "ripple/network/messages/GRemoveScriptProcessLayerMessage.h"

namespace rev {

ScriptJsonWidget::ScriptJsonWidget(WidgetManager* wm, const json& sortingLayerJson, QWidget *parent) :
    JsonWidget(wm, sortingLayerJson, { {"isScriptOrderWidget", true} }, parent)
{
    initializeWidgets();
    initializeConnections();
    layoutWidgets();
}

ScriptJsonWidget::~ScriptJsonWidget()
{
}

void ScriptJsonWidget::wheelEvent(QWheelEvent* event) {
    if (!event->pixelDelta().isNull()) {
        ParameterWidget::wheelEvent(event);
    }
    else {
        // If scrolling has reached top or bottom
        // Accept event and stop propagation if at bottom of scroll area
        event->accept();
    }
}

void ScriptJsonWidget::initializeWidgets()
{
    JsonWidget::initializeWidgets();
    m_typeLabel = new QLabel("SortingLayer");
    m_typeLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void ScriptJsonWidget::layoutWidgets()
{
    // Format widget sizes
    m_textEdit->setMaximumHeight(150);

    JsonWidget::layoutWidgets();

    // Note, cannot call again without deleting previous layout
    // https://doc.qt.io/qt-5/qwidget.html#setLayout
    setLayout(m_mainLayout);
}




// ScriptOrderItem

ScriptOrderItem::ScriptOrderItem(const json& layerJson) :
    TreeItem(layerJson["order"].get<Int32_t>(), kSortingLayer, layerJson)
{
    initializeItem();
}

ScriptOrderItem::~ScriptOrderItem()
{
}

void ScriptOrderItem::setWidget()
{
    // Throw error if the widget already exists
    assert(!m_widget && "Error, item already has a widget");

    // Get parent tree widget
    ScriptOrderTreeWidget * parentWidget = static_cast<ScriptOrderTreeWidget*>(treeWidget());

    // Set widget
    m_widget = new ScriptJsonWidget(parentWidget->m_widgetManager, m_data.m_data.m_json);

    // Assign widget to item in tree widget
    parentWidget->setItemWidget(this, 0, m_widget);
}

void ScriptOrderItem::removeWidget(int column)
{
    Q_UNUSED(column);
    // Only ever one column, so don't need to worry about indexing
    treeWidget()->removeItemWidget(this, 0);
    m_widget = nullptr;
}

ScriptOrderTreeWidget * ScriptOrderItem::scriptOrderTreeWidget() const
{
    return static_cast<ScriptOrderTreeWidget*>(treeWidget());
}





// ScriptOrderTreeWidget

ScriptOrderTreeWidget::ScriptOrderTreeWidget(WidgetManager* wm, QWidget* parent) : 
    TreeWidget(wm, wm->actionManager(), "Script Execution Order", parent)
{
    initializeWidget();
    repopulate();
}

ScriptOrderTreeWidget::~ScriptOrderTreeWidget()
{
}

void ScriptOrderTreeWidget::repopulate()
{
    // Clear the widget
    clear();

    // Set header item 
    //setHeaderItem(scenario);
    //invisibleRootItem()->setFlags(invisibleRootItem()->flags() ^ Qt::ItemIsDropEnabled);
    resizeColumns();

    // Reorder map of sorting layers
    //const std::map<QString, SortingLayer*>& sortingLayers = pm->sortingLayers();
    const json& layers = m_widgetManager->scenarioJson()["processManager"]["sortingLayers"];
    m_sortedLayers.clear();
    for (const json& layerJson : layers) {
        Int32_t order = layerJson["order"].get<Int32_t>();
        m_sortedLayers.emplace(order, layerJson);
    }

    // Add sorting layers to widget
    for (const std::pair<int, json>& sortPair : m_sortedLayers) {
        addItem(sortPair.second);
    }
}

void ScriptOrderTreeWidget::addItem(const json& sortingLayer)
{
    // Create resource item
    ScriptOrderItem* layerItem = new ScriptOrderItem(sortingLayer);

    // Add resource item
    TreeWidget::addItem(layerItem);
}

void ScriptOrderTreeWidget::removeItem(ScriptOrderItem * scriptOrderItem)
{
    delete scriptOrderItem;
}

ScriptOrderItem* ScriptOrderTreeWidget::getItem(const json& itemObject)
{
#ifdef DEBUG_MODE
    assert(!itemObject.empty() && "Null JSON provided");
#endif

    Int32_t id = itemObject["id"].get<Int32_t>();
    QTreeWidgetItemIterator it(this);
    while (*it) {
        ScriptOrderItem* item = static_cast<ScriptOrderItem*>(*it);
        if (item->data().get<Int32_t>("id") == id) {
            return item;
        }
        ++it;
    }

    return nullptr;
}

void ScriptOrderTreeWidget::removeItem(const json& itemObject)
{
    ScriptOrderItem* item = static_cast<ScriptOrderItem*>(getItem(itemObject));
    delete item;
}

void ScriptOrderTreeWidget::reorder(Int32_t layerId, Int32_t otherLayerId, bool before)
{
    // Get iterator for layer next to dropped layer
    std::multimap<int, json>::const_iterator iter = std::find_if(
        m_sortedLayers.begin(), m_sortedLayers.end(),
        [&](const std::pair<int, json>& layerPair) {
            return layerPair.second["id"].get<Uint32_t>() == otherLayerId;
        }
    );

    assert(iter != m_sortedLayers.end() && "Error, iterator not found");

    std::multimap<int, json>::const_iterator neighborIter;
    Int32_t newOrder;
    Int32_t order = iter->second["order"].get<Int32_t>();
    if (before) {
        // Moved before iterator
        if (iter == m_sortedLayers.begin()) {
            newOrder = order - 1;
        }
        else {
            neighborIter = std::prev(iter);
            if (neighborIter->second["order"].get<Int32_t>() == order) {
                newOrder = order;
            }
            else {
                newOrder = order - 1;
            }
        }
    }
    else {
        // Moved after iterator
        neighborIter = std::next(iter);
        if (neighborIter == m_sortedLayers.end()) {
            newOrder = order + 1;
        }
        else {
            if (neighborIter->second["order"].get<Int32_t>() == order) {
                newOrder = order;
            }
            else {
                newOrder = order + 1;
            }
        }
    }

    static GReorderScriptProcessesMessage s_message;
    s_message.setId(layerId);
    s_message.setNewOrder(newOrder);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
}

void ScriptOrderTreeWidget::onDropAbove(QDropEvent * event, QTreeWidgetItem * source, QTreeWidgetItem * destination)
{
    // Dropping above an item
    ScriptOrderItem* sourceItem = static_cast<ScriptOrderItem*>(source);
    ScriptOrderItem* destItem = static_cast<ScriptOrderItem*>(destination);
    reorder(sourceItem->m_data.get<Int32_t>("id"), destItem->m_data.get<Int32_t>("id"), true);
    event->setDropAction(Qt::IgnoreAction);
}

void ScriptOrderTreeWidget::onDropBelow(QDropEvent * event, QTreeWidgetItem * source, QTreeWidgetItem * destination)
{
    // Dropping below an item
    ScriptOrderItem* sourceItem = static_cast<ScriptOrderItem*>(source);
    ScriptOrderItem* destItem = static_cast<ScriptOrderItem*>(destination);
    reorder(sourceItem->m_data.get<Int32_t>("id"), destItem->m_data.get<Int32_t>("id"), false);
    event->setDropAction(Qt::IgnoreAction);
}

void ScriptOrderTreeWidget::initializeWidget()
{
    TreeWidget::initializeWidget();

    setMinimumSize(350, 350);

    initializeAsList();
    enableDragAndDrop();

    // Initialize actions
    addAction(kNoItemSelected,
        "Add Sorting Layer",
        "Add a sorting layer for the script execution order",
        [this] {
            static GAddScriptProcessLayerMessage s_message;
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
        }
    );

    addAction(kItemSelected,
        "Remove Sorting Layer",
        "Remove sorting layer for the script execution order",
        [this] {
            static GRemoveScriptProcessLayerMessage s_message;
            s_message.setId(currentContextItem()->data().get<Int32_t>("id"));
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
        }
    );

    // Repopulate when scenario json received
    QObject::connect(m_widgetManager,
        &WidgetManager::receivedScenarioJsonMessage,
        this,
       &ScriptOrderTreeWidget::repopulate
    );
}

void ScriptOrderTreeWidget::initializeItem(QTreeWidgetItem * item)
{
    static_cast<ScriptOrderItem*>(item)->setWidget();
}
#ifndef QT_NO_CONTEXTMENU

void ScriptOrderTreeWidget::contextMenuEvent(QContextMenuEvent * event)
{
    m_currentItems[kContextClick] = itemAt(event->pos());
    // Create menu
    QMenu menu(this);

    // Add actions to the menu
    ScriptOrderItem* item = static_cast<ScriptOrderItem*>(itemAt(event->pos()));
    if (item) {
        // If sorting layer is default layer, do not allow deletion
        if (item->data().getRef<const std::string&>("name") != "default") {
            m_currentItems[kContextClick] = item;
            for (QAction* action : m_actions[kItemSelected]) {
                menu.addAction(action);
            }
        }
    }
    else {
        m_currentItems[kContextClick] = nullptr;
        for (QAction* action : m_actions[kNoItemSelected]) {
            menu.addAction(action);
        }
    }

    // Display menu at click location
    menu.exec(event->globalPos());
}

#endif


} // rev