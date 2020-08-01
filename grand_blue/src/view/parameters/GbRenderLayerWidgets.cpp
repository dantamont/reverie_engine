#include "GbRenderLayerWidgets.h"
#include "../style/GbFontIcon.h"
#include "../../core/readers/GbJsonReader.h"
#include "../../core/components/GbModelComponent.h"
#include "../../core/utils/GbMemoryManager.h"
#include "../../core/scene/GbScenario.h"
#include "../tree/GbRenderLayerWidget.h"

namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// RenderLayerInstanceWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
RenderLayerInstanceWidget::RenderLayerInstanceWidget(CoreEngine* core,
    SortingLayer* sortLayer, 
    std::vector<std::weak_ptr<SortingLayer>>& renderLayers,
    QWidget *parent) :
    ParameterWidget(core, parent),
    m_sortingLayer(sortLayer),
    m_renderLayers(renderLayers)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
RenderLayerInstanceWidget::~RenderLayerInstanceWidget()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<std::shared_ptr<SortingLayer>> RenderLayerInstanceWidget::renderLayers()
{
    // Get render layers
    std::vector<std::shared_ptr<SortingLayer>> layers;
    for (std::weak_ptr<SortingLayer> weakPtr : m_renderLayers) {
        if (std::shared_ptr<SortingLayer> ptr = weakPtr.lock()) {
            layers.push_back(ptr);
        }
    }

    // Clear any layers that have gone out of scope
    if (layers.size() < m_renderLayers.size()) {
        m_renderLayers.clear();
        for (const std::shared_ptr<SortingLayer>& ptr : layers) {
            m_renderLayers.push_back(ptr);
        }
    }

    return layers;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool RenderLayerInstanceWidget::addRenderLayer(const std::shared_ptr<SortingLayer>& layer)
{
    std::vector<std::shared_ptr<SortingLayer>> layers = renderLayers();
    auto iter = std::find_if(layers.begin(), layers.end(),
        [&](const auto& l) {
        return l->getUuid() == layer->getUuid();
    });
    if (iter != layers.end()) {
#ifdef DEBUG_MODE
        throw("Error, layer already found");
#endif
        return false;
    }
    m_renderLayers.push_back(layer);
    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool RenderLayerInstanceWidget::removeRenderLayer(const QString & label)
{
    auto iter = std::find_if(m_renderLayers.begin(), m_renderLayers.end(),
        [&](const auto& layer) {
        std::shared_ptr<SortingLayer> layerPtr = layer.lock();
        return layerPtr->getName() == label;
    });
    if (iter == m_renderLayers.end()) {
#ifdef DEBUG_MODE
        throw("Error, layer not found");
#endif
        return false;
    }

    m_renderLayers.erase(iter);
    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerInstanceWidget::initializeWidgets()
{
    ParameterWidget::initializeWidgets();

    m_checkBox = new QCheckBox(m_sortingLayer->getName());
    bool hasRenderLayer;
    auto layers = renderLayers();
    auto iter = std::find_if(layers.begin(), layers.end(),
        [&](const auto& layer) {
        return layer->getName() == m_sortingLayer->getName();
    });
    if (iter == layers.end()) {
        hasRenderLayer = false;
    }
    else {
        hasRenderLayer = true;
    }
    
    m_checkBox->setChecked(hasRenderLayer);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerInstanceWidget::initializeConnections()
{
    ParameterWidget::initializeWidgets();
    
    // Toggle render layer for renderable
    connect(m_checkBox,
        &QCheckBox::stateChanged,
        this,
        [this](int state) {
        bool toggled = state == 0 ? false : true;
        if (toggled) {
            // Find sorting layer in scenario settings and add to renderable
            SortingLayers& layers = m_engine->scenario()->settings().renderLayers();
            auto layerIter = std::find_if(layers.begin(), layers.end(),
                [this](const std::shared_ptr<SortingLayer>& layer) {
                return layer->getUuid() == m_sortingLayer->getUuid();
            });
            if (layerIter == layers.end()) {
                throw("Error, layer not found");
            }
            addRenderLayer(*layerIter);
        }
        else {
            // Remove render layer from renderable
            removeRenderLayer(m_sortingLayer->getName());
        }
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerInstanceWidget::layoutWidgets()
{
    ParameterWidget::layoutWidgets();

    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_checkBox);
}




///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// RenderLayerSelectItem
///////////////////////////////////////////////////////////////////////////////////////////////////
RenderLayerSelectItem::RenderLayerSelectItem(SortingLayer * layer) :
    TreeItem<SortingLayer>(layer, kSortingLayer)
{
    initializeItem();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
RenderLayerSelectItem::~RenderLayerSelectItem()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerSelectItem::setWidget()
{
    // Throw error if the widget already exists
    if (m_widget) throw("Error, item already has a widget");

    // Get parent tree widget
    RenderLayerSelectWidget * parentWidget = static_cast<RenderLayerSelectWidget*>(treeWidget());

    // Set widget
    SortingLayer* sortLayer = sortingLayer();
    QJsonDocument doc(sortLayer->asJson().toObject());
    QString rep(doc.toJson(QJsonDocument::Indented));
    m_widget = new RenderLayerInstanceWidget(parentWidget->m_engine,
        sortLayer,
        parentWidget->m_renderLayers);

    // Assign widget to item in tree widget
    parentWidget->setItemWidget(this, 0, m_widget);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerSelectItem::removeWidget(int column)
{
    // Only ever one column, so don't need to worry about indexing
    treeWidget()->removeItemWidget(this, 0);
    m_widget = nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
View::RenderLayerSelectWidget * RenderLayerSelectItem::renderLayerTreeWidget() const
{
    return static_cast<View::RenderLayerSelectWidget*>(treeWidget());
}




///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// RenderLayerSelectWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
RenderLayerSelectWidget::RenderLayerSelectWidget(CoreEngine* core,
    std::vector<std::weak_ptr<SortingLayer>>& renderLayers,
    QWidget* parent) :
    TreeWidget(core, "Render Layers", parent),
    m_renderLayers(renderLayers)
{
    initializeWidget();
    repopulate();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
RenderLayerSelectWidget::~RenderLayerSelectWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerSelectWidget::repopulate()
{
    // Clear the widget
    clear();

    // Set header item 
    QTreeWidgetItem* headerItem = new QTreeWidgetItem(0);
    headerItem->setText(0, "Render Layers");
    setHeaderItem(headerItem);
    resizeColumns();

    // Get render layers
    //SortingLayers& layers = m_renderable.renderLayers();
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
RenderLayerSelectItem * RenderLayerSelectWidget::currentContextItem() const
{
    return static_cast<RenderLayerSelectItem*>(m_currentItems[kContextClick]);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerSelectWidget::initializeItem(QTreeWidgetItem * item)
{
    static_cast<RenderLayerSelectItem*>(item)->setWidget();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerSelectWidget::addItem(SortingLayer * sortingLayer)
{
    // Create resource item
    RenderLayerSelectItem* layerItem = new View::RenderLayerSelectItem(sortingLayer);

    // Add resource item
    TreeWidget::addItem(layerItem);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerSelectWidget::removeItem(RenderLayerSelectItem * renderLayerItem)
{
    delete renderLayerItem;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerSelectWidget::removeItem(SortingLayer* itemObject)
{
    RenderLayerSelectItem* item = static_cast<RenderLayerSelectItem*>(getItem(*itemObject));
    delete item;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerSelectWidget::initializeWidget()
{
    TreeWidget::initializeWidget();

    //setMinimumSize(350, 350);

    initializeAsList();
    //enableDragAndDrop();

    // Connect signal for double click events
    connect(this, &RenderLayerSelectWidget::itemDoubleClicked,
        this, &RenderLayerSelectWidget::onItemDoubleClicked);

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerSelectWidget::onItemDoubleClicked(QTreeWidgetItem * item, int column)
{
    TreeWidget::onItemDoubleClicked(item, column);

    // Downcast item
    RenderLayerSelectItem* layerItem = static_cast<RenderLayerSelectItem*>(item);
    if (!layerItem->m_widget) {
        throw("Error, item should have a widget");
    }

    // Toggle render layer for renderable 
    //auto* layerWidget = static_cast<RenderLayerInstanceWidget*>(layerItem->m_widget);
    //layerWidget->m_checkBox->toggle();

}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb