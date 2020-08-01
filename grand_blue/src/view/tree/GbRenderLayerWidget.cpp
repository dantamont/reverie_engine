#include "GbRenderLayerWidget.h"

#include "../../core/loop/GbSimLoop.h"
#include "../../core/readers/GbJsonReader.h"
#include "../../core/containers/GbSortingLayer.h"
#include "../../core/scene/GbScenario.h"

#include "../GbWidgetManager.h"
#include "../../GbMainWindow.h"

#include "../../core/mixins/GbRenderable.h"
#include "../GL/GbGLWidget.h"
#include "../../core/rendering/renderer/GbMainRenderer.h"

namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// RenderLayerWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
RenderLayerWidget::RenderLayerWidget(CoreEngine* core, SortingLayer* sortLayer, QWidget *parent) :
    JsonWidget(core, sortLayer, parent)
{
    initializeWidgets();
    initializeConnections();
    layoutWidgets();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
RenderLayerWidget::~RenderLayerWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerWidget::wheelEvent(QWheelEvent* event) {
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
void RenderLayerWidget::initializeWidgets()
{
    JsonWidget::initializeWidgets();
    m_typeLabel = new QLabel("Render Layer");
    m_typeLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerWidget::layoutWidgets()
{
    // Format widget sizes
    m_textEdit->setMaximumHeight(0.75f * Renderable::screenDPIY());

    JsonWidget::layoutWidgets();

    // Note, cannot call again without deleting previous layout
    // https://doc.qt.io/qt-5/qwidget.html#setLayout
    setLayout(m_mainLayout);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool RenderLayerWidget::isValidObject(const QJsonObject & object)
{
    // Check that this label is not in use already
    QString label = object["label"].toString();
    std::shared_ptr<SortingLayer> layer = m_engine->scenario()->settings().renderLayer(label);
    if (!layer) return true;
    if (layer->getUuid() == sortingLayer()->getUuid())
        return true;
    else
        return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerWidget::preLoad()
{
    // Pause scenario to edit component
    pauseSimulation();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerWidget::postLoad()
{
    // Unpause scenario
    resumeSimulation();

    emit editedRenderLayers();
}




///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// RenderLayerItem
///////////////////////////////////////////////////////////////////////////////////////////////////
RenderLayerItem::RenderLayerItem(SortingLayer * layer) :
    TreeItem(layer, kSortingLayer)
{
    initializeItem();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
RenderLayerItem::~RenderLayerItem()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerItem::setWidget()
{
    // Throw error if the widget already exists
    if (m_widget) throw("Error, item already has a widget");

    // Get parent tree widget
    RenderLayerTreeWidget * parentWidget = static_cast<RenderLayerTreeWidget*>(treeWidget());

    // Set widget
    SortingLayer* sortLayer = sortingLayer();
    QJsonDocument doc(sortLayer->asJson().toObject());
    QString rep(doc.toJson(QJsonDocument::Indented));
    m_widget = new RenderLayerWidget(parentWidget->m_engine, sortLayer);

    // Connect signals and slots
    QObject::connect((RenderLayerWidget*)m_widget, &RenderLayerWidget::editedRenderLayers,
        parentWidget, &RenderLayerTreeWidget::repopulate);

    // Assign widget to item in tree widget
    parentWidget->setItemWidget(this, 0, m_widget);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerItem::removeWidget(int column)
{
    Q_UNUSED(column);
    // Only ever one column, so don't need to worry about indexing
    treeWidget()->removeItemWidget(this, 0);
    m_widget = nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
View::RenderLayerTreeWidget * RenderLayerItem::renderLayerTreeWidget() const
{
    return static_cast<View::RenderLayerTreeWidget*>(treeWidget());
}



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// RenderLayerTreeWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
RenderLayerTreeWidget::RenderLayerTreeWidget(CoreEngine* engine, QWidget* parent) : 
    TreeWidget(engine, "Global Render Layers", parent)
{
    initializeWidget();
    repopulate();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
RenderLayerTreeWidget::~RenderLayerTreeWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerTreeWidget::repopulate()
{
    // Clear the widget
    clear();

    // Set header item 
    //setHeaderItem(scenario);
    //invisibleRootItem()->setFlags(invisibleRootItem()->flags() ^ Qt::ItemIsDropEnabled);
    resizeColumns();

    // Get render layers
    SortingLayers& layers = m_engine->scenario()->settings().renderLayers();

    // Reorder map of sorting layers
    m_sortedLayers.clear();
    for (const auto& layer : layers) {
        m_sortedLayers.emplace(layer->getOrder(), layer.get());
    }

    // Add sorting layers to widget
    for (const std::pair<int, SortingLayer*>& sortPair : m_sortedLayers) {
        addItem(sortPair.second);
    }

}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerTreeWidget::addItem(SortingLayer * sortingLayer)
{
    // Create resource item
    RenderLayerItem* layerItem = new View::RenderLayerItem(sortingLayer);

    // Add resource item
    TreeWidget::addItem(layerItem);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerTreeWidget::removeItem(RenderLayerItem * RenderLayerItem)
{
    delete RenderLayerItem;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerTreeWidget::removeItem(SortingLayer* itemObject)
{
    RenderLayerItem* item = static_cast<RenderLayerItem*>(getItem(*itemObject));
    delete item;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerTreeWidget::reorder(SortingLayer* layer, 
    SortingLayer* otherLayer,
    bool before)
{
    // Get iterator for layer next to dropped layer
    std::multimap<int, SortingLayer*>::const_iterator iter =
        std::find_if(m_sortedLayers.begin(),
            m_sortedLayers.end(),
            [&](const std::pair<int, SortingLayer*>& layerPair) {
        return layerPair.second->getUuid() == otherLayer->getUuid();
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

    m_engine->scenario()->settings().sortRenderLayers();
    repopulate();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerTreeWidget::onDropAbove(QDropEvent * event, QTreeWidgetItem * source, QTreeWidgetItem * destination)
{
    // Dropping above an item
    RenderLayerItem* sourceItem = static_cast<RenderLayerItem*>(source);
    RenderLayerItem* destItem = static_cast<RenderLayerItem*>(destination);
    reorder(sourceItem->sortingLayer(),
        destItem->sortingLayer(),
        true);
    event->setDropAction(Qt::IgnoreAction);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerTreeWidget::onDropBelow(QDropEvent * event, QTreeWidgetItem * source, QTreeWidgetItem * destination)
{
    // Dropping below an item
    RenderLayerItem* sourceItem = static_cast<RenderLayerItem*>(source);
    RenderLayerItem* destItem = static_cast<RenderLayerItem*>(destination);
    reorder(sourceItem->sortingLayer(),
        destItem->sortingLayer(),
        false);
    event->setDropAction(Qt::IgnoreAction);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
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
            // TODO: Mutex lock for multiple GL Widgets
            QMutex& mutex = m_engine->widgetManager()->mainGLWidget()->renderer()->drawMutex();
            QMutexLocker lock(&mutex);

            // Add sorting layer
            m_engine->scenario()->settings().addRenderLayer();
            repopulate();
    });

    addAction(kItemSelected,
        "Remove Render Layer",
        "Remove render layer",
        [this] {
        // TODO: Mutex lock for multiple GL Widgets
        QMutex& mutex = m_engine->widgetManager()->mainGLWidget()->renderer()->drawMutex();
        QMutexLocker lock(&mutex);

        // Add sorting layer
        m_engine->scenario()->settings().removeRenderLayer(
            currentContextItem()->sortingLayer()->getName());
        repopulate();
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerTreeWidget::initializeItem(QTreeWidgetItem * item)
{
    static_cast<RenderLayerItem*>(item)->setWidget();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
}
}