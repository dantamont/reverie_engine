#include "GScriptOrder.h"

#include "../../core/loop/GSimLoop.h"
#include "../../core/readers/GJsonReader.h"
#include "../../core/containers/GSortingLayer.h"
#include "../../core/processes/GProcessManager.h"

#include "../GWidgetManager.h"
#include "../../GMainWindow.h"

namespace rev {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// ScriptJsonWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
ScriptJsonWidget::ScriptJsonWidget(CoreEngine* core, SortingLayer* sortLayer, QWidget *parent) :
    JsonWidget(core, sortLayer, parent)
{
    initializeWidgets();
    initializeConnections();
    layoutWidgets();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ScriptJsonWidget::~ScriptJsonWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptJsonWidget::initializeWidgets()
{
    JsonWidget::initializeWidgets();
    m_typeLabel = new QLabel("SortingLayer");
    m_typeLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptJsonWidget::layoutWidgets()
{
    // Format widget sizes
    m_textEdit->setMaximumHeight(150);

    JsonWidget::layoutWidgets();

    // Note, cannot call again without deleting previous layout
    // https://doc.qt.io/qt-5/qwidget.html#setLayout
    setLayout(m_mainLayout);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool ScriptJsonWidget::isValidObject(const QJsonObject & object)
{
    // Check that this label is not in use already
    QString label = object["label"].toString();
    SortingLayer* layer = m_engine->processManager()->sortingLayers().getLayer(label);
    if (!layer) {
        return true;
    }
    else if (layer->id() == sortingLayer()->id()) {
        return true;
    }
    else {
        return false;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptJsonWidget::preLoad()
{
    // Pause scenario to edit component
    SimulationLoop* simLoop = m_engine->simulationLoop();
    m_wasPlaying = simLoop->isPlaying();
    if (m_wasPlaying) {
        simLoop->pause();
    }
    m_engine->simulationLoop()->pause();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptJsonWidget::postLoad()
{
    // Unpause scenario
    SimulationLoop* simLoop = m_engine->simulationLoop();
    if (m_wasPlaying) {
        simLoop->play();
    }

    emit editedScriptOrder();
}




///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// ScriptOrderItem
///////////////////////////////////////////////////////////////////////////////////////////////////
ScriptOrderItem::ScriptOrderItem(SortingLayer * layer) :
    TreeItem(layer, kSortingLayer)
{
    initializeItem();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ScriptOrderItem::~ScriptOrderItem()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderItem::setWidget()
{
    // Throw error if the widget already exists
    if (m_widget) throw("Error, item already has a widget");

    // Get parent tree widget
    ScriptOrderTreeWidget * parentWidget = static_cast<ScriptOrderTreeWidget*>(treeWidget());

    // Set widget
    SortingLayer* sortLayer = sortingLayer();
    QJsonDocument doc(sortLayer->asJson().toObject());
    QString rep(doc.toJson(QJsonDocument::Indented));
    m_widget = new ScriptJsonWidget(parentWidget->m_engine, sortLayer);

    // Connect signals and slots
    QObject::connect((ScriptJsonWidget*)m_widget, &ScriptJsonWidget::editedScriptOrder,
        parentWidget, &ScriptOrderTreeWidget::repopulate);

    // Assign widget to item in tree widget
    parentWidget->setItemWidget(this, 0, m_widget);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderItem::removeWidget(int column)
{
    Q_UNUSED(column);
    // Only ever one column, so don't need to worry about indexing
    treeWidget()->removeItemWidget(this, 0);
    m_widget = nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
View::ScriptOrderTreeWidget * ScriptOrderItem::scriptOrderTreeWidget() const
{
    return static_cast<View::ScriptOrderTreeWidget*>(treeWidget());
}



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// ScriptOrderTreeWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
ScriptOrderTreeWidget::ScriptOrderTreeWidget(CoreEngine* engine, QWidget* parent) : 
    TreeWidget(engine, "Script Execution Order", parent)
{
    initializeWidget();
    repopulate();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ScriptOrderTreeWidget::~ScriptOrderTreeWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderTreeWidget::repopulate()
{
    // Clear the widget
    clear();

    // Set header item 
    //setHeaderItem(scenario);
    //invisibleRootItem()->setFlags(invisibleRootItem()->flags() ^ Qt::ItemIsDropEnabled);
    resizeColumns();

    // Get process manager
    ProcessManager* pm = m_engine->processManager();

    // Reorder map of sorting layers
    //const std::map<QString, SortingLayer*>& sortingLayers = pm->sortingLayers();
    m_sortedLayers.clear();
    for (const std::unique_ptr<SortingLayer>& sortingLayer : pm->sortingLayers().m_layers) {
        m_sortedLayers.emplace(sortingLayer->getOrder(), sortingLayer.get());
    }

    // Add sorting layers to widget
    for (const std::pair<int, SortingLayer*>& sortPair : m_sortedLayers) {
        addItem(sortPair.second);
    }

}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderTreeWidget::addItem(SortingLayer * sortingLayer)
{
    // Create resource item
    ScriptOrderItem* layerItem = new View::ScriptOrderItem(sortingLayer);

    // Add resource item
    TreeWidget::addItem(layerItem);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderTreeWidget::removeItem(ScriptOrderItem * scriptOrderItem)
{
    delete scriptOrderItem;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderTreeWidget::removeItem(SortingLayer* itemObject)
{
    ScriptOrderItem* item = static_cast<ScriptOrderItem*>(getItem(*itemObject));
    delete item;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderTreeWidget::reorder(SortingLayer* layer, 
    SortingLayer* otherLayer,
    bool before)
{
    // Get iterator for layer next to dropped layer
    std::multimap<int, SortingLayer*>::const_iterator iter =
        std::find_if(m_sortedLayers.begin(),
            m_sortedLayers.end(),
            [&](const std::pair<int, SortingLayer*>& layerPair) {
        return layerPair.second->id() == otherLayer->id();
    });

    if (iter == m_sortedLayers.end()) {
        throw("Error, iterator not found");
    }

    std::multimap<int, SortingLayer*>::const_iterator neighborIter;
    if (before) {
        // Moved before iterator
        if (iter == m_sortedLayers.begin()) {
            layer->setOrder(iter->second->getOrder() - 1);
        }
        else {
            neighborIter = std::prev(iter);
            if (neighborIter->second->getOrder() == iter->second->getOrder()) {
                layer->setOrder(iter->second->getOrder());
            }
            else {
                layer->setOrder(iter->second->getOrder() - 1);
            }
        }
    }
    else {
        // Moved after iterator
        neighborIter = std::next(iter);
        if (neighborIter == m_sortedLayers.end()) {
            layer->setOrder(iter->second->getOrder() + 1);
        }
        else {
            if (neighborIter->second->getOrder() == iter->second->getOrder()) {
                layer->setOrder(iter->second->getOrder());
            }
            else {
                layer->setOrder(iter->second->getOrder() + 1);
            }
        }
    }

    m_engine->processManager()->refreshProcessOrder();
    repopulate();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderTreeWidget::onDropAbove(QDropEvent * event, QTreeWidgetItem * source, QTreeWidgetItem * destination)
{
    // Dropping above an item
    ScriptOrderItem* sourceItem = static_cast<ScriptOrderItem*>(source);
    ScriptOrderItem* destItem = static_cast<ScriptOrderItem*>(destination);
    reorder(sourceItem->sortingLayer(),
        destItem->sortingLayer(),
        true);
    event->setDropAction(Qt::IgnoreAction);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderTreeWidget::onDropBelow(QDropEvent * event, QTreeWidgetItem * source, QTreeWidgetItem * destination)
{
    // Dropping below an item
    ScriptOrderItem* sourceItem = static_cast<ScriptOrderItem*>(source);
    ScriptOrderItem* destItem = static_cast<ScriptOrderItem*>(destination);
    reorder(sourceItem->sortingLayer(),
        destItem->sortingLayer(),
        false);
    event->setDropAction(Qt::IgnoreAction);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
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
            // Add sorting layer
            m_engine->processManager()->sortingLayers().addLayer();
            repopulate();
    });

    addAction(kItemSelected,
        "Remove Sorting Layer",
        "Remove sorting layer for the script execution order",
        [this] {
        // Add sorting layer
        m_engine->processManager()->sortingLayers().removeLayer(currentContextItem()->sortingLayer()->getName(),
            [this](size_t layerId) {m_engine->processManager()->onRemoveSortingLayer(layerId); });
        repopulate();
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderTreeWidget::initializeItem(QTreeWidgetItem * item)
{
    static_cast<ScriptOrderItem*>(item)->setWidget();
}
#ifndef QT_NO_CONTEXTMENU
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderTreeWidget::contextMenuEvent(QContextMenuEvent * event)
{
    m_currentItems[kContextClick] = itemAt(event->pos());
    // Create menu
    QMenu menu(this);

    // Add actions to the menu
    ScriptOrderItem* item = static_cast<ScriptOrderItem*>(itemAt(event->pos()));
    if (item) {
        // If sorting layer is default layer, do not allow deletion
        if (item->sortingLayer()->getName() != "default") {
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

///////////////////////////////////////////////////////////////////////////////////////////////////
}
}