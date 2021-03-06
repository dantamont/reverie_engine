#include "GRenderLayerWidgets.h"
#include "../style/GFontIcon.h"
#include "../../core/readers/GJsonReader.h"
#include "../../core/components/GModelComponent.h"
#include "../../core/utils/GMemoryManager.h"
#include "../../core/scene/GScenario.h"
#include "../tree/GRenderLayerWidget.h"

namespace rev {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// RenderLayerInstanceWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
RenderLayerInstanceWidget::RenderLayerInstanceWidget(CoreEngine* core,
    SortingLayer* sortLayer, 
    std::vector<size_t>& renderLayers,
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
std::vector<SortingLayer*> RenderLayerInstanceWidget::renderLayers()
{
    // Get render layers
    std::vector<SortingLayer*> layers;
    for (size_t layerId : m_renderLayers) {
        SortingLayer* layer = m_engine->scenario()->settings().renderLayers().getLayerFromId(layerId);
        layers.push_back(layer);
    }

    return layers;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool RenderLayerInstanceWidget::addRenderLayer(SortingLayer* layer)
{
    std::vector<SortingLayer*> layers = renderLayers();
    auto iter = std::find_if(layers.begin(), layers.end(),
        [&](SortingLayer* l) {
        return l->id() == layer->id();
    });
    if (iter != layers.end()) {
#ifdef DEBUG_MODE
        throw("Error, layer already found");
#endif
        return false;
    }
    m_renderLayers.push_back(layer->id());
    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool RenderLayerInstanceWidget::removeRenderLayer(const GString & label)
{
    // Get ID of the layer to remove
    std::vector<SortingLayer*> layers = renderLayers();
    auto liter = std::find_if(layers.begin(), layers.end(),
        [&](SortingLayer* l) {
        return l->getName() == label;
    });
    if (liter == layers.end()) {
#ifdef DEBUG_MODE
        throw("Error, layer not found");
#endif
        return false;
    }

    // Use the ID to remove layer from widget
    size_t layerId = (*liter)->id();
    auto iter = std::find_if(m_renderLayers.begin(), m_renderLayers.end(),
        [layerId](size_t lid) {
        return layerId == lid;
    });
    m_renderLayers.erase(iter);
    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderLayerInstanceWidget::initializeWidgets()
{
    ParameterWidget::initializeWidgets();

    m_checkBox = new QCheckBox(m_sortingLayer->getName().c_str());
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
            auto layerIter = std::find_if(layers.m_layers.begin(), layers.m_layers.end(),
                [this](const auto& layer) {
                return layer->id() == m_sortingLayer->id();
            });
            if (layerIter == layers.m_layers.end()) {
                throw("Error, layer not found");
            }
            addRenderLayer((*layerIter).get());
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
    m_mainLayout->setMargin(1);
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
    Q_UNUSED(column)
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
    std::vector<size_t>& renderLayerIds,
    QWidget* parent) :
    TreeWidget(core, "Render Layers", parent),
    m_renderLayers(renderLayerIds)
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

    // Found the magic ticket for adjusting the size of a tree widget! The scroll area
    // apparently does not adjust to contents by default
    setMinimumSize(0, 0);
    setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);

    // Get render layers
    //SortingLayers& layers = m_renderable.renderLayers();
    SortingLayers& layers = m_engine->scenario()->settings().renderLayers();

    // Reorder map of sorting layers
    m_sortedLayers.clear();
    for (const auto& layer : layers.m_layers) {
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
    //setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

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
} // rev