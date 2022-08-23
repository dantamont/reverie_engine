#include "geppetto/qt/widgets/tree/GRenderLayerWidget.h"

#include <QScreen>

#include "fortress/containers/GSortingLayer.h"

#include "geppetto/qt/widgets/GWidgetManager.h"

#include "ripple/network/gateway/GMessageGateway.h"
#include "ripple/network/messages/GGetScenarioJsonMessage.h"
#include "ripple/network/messages/GScenarioJsonMessage.h"

#include "ripple/network/messages/GReorderRenderLayersMessage.h"
#include "ripple/network/messages/GAddRenderLayerMessage.h"
#include "ripple/network/messages/GRemoveRenderLayerMessage.h"

namespace rev {

RenderLayerWidget::RenderLayerWidget(WidgetManager* wm, const json& sortLayer, QWidget *parent) :
    JsonWidget(wm, sortLayer, { {"isMainRenderLayerWidget", true} }, parent)
{
    initializeWidgets();
    initializeConnections();
    layoutWidgets();
}

RenderLayerWidget::~RenderLayerWidget()
{
}

void RenderLayerWidget::initializeWidgets()
{
    JsonWidget::initializeWidgets();
    m_typeLabel = new QLabel("Render Layer");
    m_typeLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void RenderLayerWidget::layoutWidgets()
{
    // Format widget sizes
    QScreen* screen = QGuiApplication::primaryScreen();
    Float32_t dpiY = screen->logicalDotsPerInchY();
    m_textEdit->setMaximumHeight(0.75f * dpiY);

    JsonWidget::layoutWidgets();

    // Note, cannot call again without deleting previous layout
    // https://doc.qt.io/qt-5/qwidget.html#setLayout
    setLayout(m_mainLayout);
}



// RenderLayerItem

RenderLayerItem::RenderLayerItem(const json& renderLayerJson) :
    TreeItem(renderLayerJson["order"].get<Int32_t>(), kSortingLayer, renderLayerJson)
{
    initializeItem();
}

RenderLayerItem::~RenderLayerItem()
{
}

void RenderLayerItem::setWidget()
{
    // Throw error if the widget already exists
    assert (!m_widget && "Error, item already has a widget");

    // Get parent tree widget
    RenderLayerTreeWidget * parentWidget = static_cast<RenderLayerTreeWidget*>(treeWidget());

    // Set widget
    m_widget = new RenderLayerWidget(parentWidget->m_widgetManager, m_data.m_data.m_json);

    // Assign widget to item in tree widget
    parentWidget->setItemWidget(this, 0, m_widget);
}

void RenderLayerItem::removeWidget(int column)
{
    Q_UNUSED(column);
    // Only ever one column, so don't need to worry about indexing
    treeWidget()->removeItemWidget(this, 0);
    m_widget = nullptr;
}

RenderLayerTreeWidget * RenderLayerItem::renderLayerTreeWidget() const
{
    return static_cast<RenderLayerTreeWidget*>(treeWidget());
}





// RenderLayerTreeWidget

RenderLayerTreeWidget::RenderLayerTreeWidget(WidgetManager* wm, QWidget* parent) : 
    TreeWidget(wm, wm->actionManager(), "Global Render Layers", parent)
{
    initializeWidget();
    repopulate();
}

RenderLayerTreeWidget::~RenderLayerTreeWidget()
{
}

void RenderLayerTreeWidget::repopulate()
{
    // Clear the widget
    clear();

    // Set header item 
    //setHeaderItem(scenario);
    //invisibleRootItem()->setFlags(invisibleRootItem()->flags() ^ Qt::ItemIsDropEnabled);
    resizeColumns();

    // Get render layers
    const json& layers = renderLayersJson();

    // Reorder map of sorting layers
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

void RenderLayerTreeWidget::addItem(const json& sortingLayer)
{
    // Create resource item
    RenderLayerItem* layerItem = new RenderLayerItem(sortingLayer);

    // Add resource item
    TreeWidget::addItem(layerItem);
}

void RenderLayerTreeWidget::removeItem(RenderLayerItem * RenderLayerItem)
{
    delete RenderLayerItem;
}

RenderLayerItem* RenderLayerTreeWidget::getItem(const json& renderLayerJson)
{
#ifdef DEBUG_MODE
    assert(!renderLayerJson.empty() && "Null JSON provided");
#endif

    Int32_t id = renderLayerJson["id"].get<Int32_t>();
    QTreeWidgetItemIterator it(this);
    while (*it) {
        RenderLayerItem* item = static_cast<RenderLayerItem*>(*it);
        if (item->data().get<Int32_t>("id") == id) {
            return item;
        }
        ++it;
    }

    return nullptr;
}

void RenderLayerTreeWidget::removeItem(const json& renderJson)
{
    RenderLayerItem* item = static_cast<RenderLayerItem*>(getItem(renderJson));
    delete item;
}

void RenderLayerTreeWidget::reorder(Int32_t layerId, Int32_t otherLayerId, bool before)
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

    static GReorderRenderLayersMessage s_message;
    s_message.setId(layerId);
    s_message.setNewOrder(newOrder);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
}

void RenderLayerTreeWidget::onDropAbove(QDropEvent * event, QTreeWidgetItem * source, QTreeWidgetItem * destination)
{
    // Dropping above an item
    RenderLayerItem* sourceItem = static_cast<RenderLayerItem*>(source);
    RenderLayerItem* destItem = static_cast<RenderLayerItem*>(destination);
    reorder(sourceItem->m_data.get<Int32_t>("id"), destItem->m_data.get<Int32_t>("id"), true);
    event->setDropAction(Qt::IgnoreAction);
}

void RenderLayerTreeWidget::onDropBelow(QDropEvent * event, QTreeWidgetItem * source, QTreeWidgetItem * destination)
{
    // Dropping below an item
    RenderLayerItem* sourceItem = static_cast<RenderLayerItem*>(source);
    RenderLayerItem* destItem = static_cast<RenderLayerItem*>(destination);
    reorder(sourceItem->m_data.get<Int32_t>("id"), destItem->m_data.get<Int32_t>("id"), false);
    event->setDropAction(Qt::IgnoreAction);
}

void RenderLayerTreeWidget::initializeWidget()
{
    TreeWidget::initializeWidget();

    setMinimumSize(350, 350);

    initializeAsList();
    enableDragAndDrop();

    // Initialize actions
    addAction(kNoItemSelected,
        "Add Render Layer",
        "Add a render layer",
        [this] {
            static GAddRenderLayerMessage s_message;
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
        }
    );

    addAction(kItemSelected,
        "Remove Render Layer",
        "Remove render layer",
        [this] {
            static GRemoveRenderLayerMessage s_message;
            s_message.setId(currentContextItem()->data().get<Int32_t>("id"));
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
        }
    );

    // Repopulate when scenario json received
    QObject::connect(m_widgetManager,
        &WidgetManager::receivedScenarioJsonMessage,
        this,
        &RenderLayerTreeWidget::repopulate
    );

}

const json& RenderLayerTreeWidget::renderLayersJson()
{
    return m_widgetManager->scenarioJson()["settings"]["renderLayers"];
}

void RenderLayerTreeWidget::initializeItem(QTreeWidgetItem * item)
{
    static_cast<RenderLayerItem*>(item)->setWidget();
}



} // rev