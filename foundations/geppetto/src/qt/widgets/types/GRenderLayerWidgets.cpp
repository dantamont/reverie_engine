#include "geppetto/qt/widgets/types/GRenderLayerWidgets.h"
#include "geppetto/qt/style/GFontIcon.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/tree/GRenderLayerWidget.h"
#include "geppetto/qt/widgets/tree/GSceneTreeWidget.h"

#include "fortress/json/GJson.h"
#include "fortress/system/memory/GPointerTypes.h"

#include "ripple/network/gateway/GMessageGateway.h"

namespace rev {

RenderLayerInstanceWidget::RenderLayerInstanceWidget(WidgetManager* wm, const json& sortingLayerJson, Int32_t sceneObjectId, ERenderLayerWidgetMode mode, QWidget *parent) :
    ParameterWidget(wm, parent),
    m_sceneObjectId(sceneObjectId),
    m_sortingLayerJson(sortingLayerJson),
    m_widgetMode(mode)
{
    m_addSceneObjectRenderLayerMessage.setSceneObjectId(sceneObjectId);
    m_removeSceneObjectRenderLayerMessage.setSceneObjectId(sceneObjectId);
    m_addCameraRenderLayerMessage.setSceneObjectId(sceneObjectId);
    m_removeCameraRenderLayerMessage.setSceneObjectId(sceneObjectId);
    initialize();
}

RenderLayerInstanceWidget::~RenderLayerInstanceWidget()
{ 
}

const json& RenderLayerInstanceWidget::renderLayersJson()
{
    return m_widgetManager->scenarioJson()["settings"]["renderLayers"];
}

void RenderLayerInstanceWidget::addRenderLayer(Uint32_t layerId)
{
    const json& layers = renderLayersJson();
    auto iter = std::find_if(layers.begin(), layers.end(),
        [&](const auto& l) {
            return l["id"].get<Uint32_t>() == layerId;
        }
    );
#ifdef DEBUG_MODE
    assert(iter == layers.end() && "Error, layer already exists");
#endif
    switch (m_widgetMode) {
    case ERenderLayerWidgetMode::eSceneObject:
        m_addSceneObjectRenderLayerMessage.setRenderLayerId(layerId);
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_addSceneObjectRenderLayerMessage);
        break;
    case ERenderLayerWidgetMode::eCamera:
        m_addCameraRenderLayerMessage.setRenderLayerId(layerId);
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_addCameraRenderLayerMessage);
        break;
    default:
        assert(false && "unimplemented");
    }
}

void RenderLayerInstanceWidget::removeRenderLayer(Uint32_t layerId)
{
    // Get ID of the layer to remove
    const json& layers = renderLayersJson();
    auto liter = std::find_if(layers.begin(), layers.end(),
        [&](const auto& l) {
            return l["id"].get<Uint32_t>() == layerId;
        }
    );

#ifdef DEBUG_MODE
    assert(liter != layers.end() && "Error, layer not found");
#endif

    switch (m_widgetMode) {
    case ERenderLayerWidgetMode::eSceneObject:
        m_removeSceneObjectRenderLayerMessage.setRenderLayerId(layerId);
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_removeSceneObjectRenderLayerMessage);
        break;
    case ERenderLayerWidgetMode::eCamera:
        m_removeCameraRenderLayerMessage.setRenderLayerId(layerId);
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_removeCameraRenderLayerMessage);
        break;
    default:
        assert(false && "unimplemented");
    }
}

void RenderLayerInstanceWidget::initializeWidgets()
{
    m_checkBox = new QCheckBox(m_sortingLayerJson["label"].get_ref<const std::string&>().c_str());
    bool hasRenderLayer;
    auto layers = renderLayersJson();
    auto iter = std::find_if(layers.begin(), layers.end(),
        [&](const auto& layer) {
        return layer["id"].get<Uint32_t>() == m_sortingLayerJson["id"].get<Uint32_t>();
    });
    if (iter == layers.end()) {
        hasRenderLayer = false;
    }
    else {
        hasRenderLayer = true;
    }
    
    m_checkBox->setChecked(hasRenderLayer);
}

void RenderLayerInstanceWidget::initializeConnections()
{    
    // Toggle render layer for renderable
    connect(m_checkBox,
        &QCheckBox::stateChanged,
        this,
        [this](int state) {
        bool toggled = state == 0 ? false : true;
        Uint32_t sortingLayerId = m_sortingLayerJson["id"].get<Uint32_t>();
        if (toggled) {
            // Add sorting layer to renderable
            addRenderLayer(sortingLayerId);
        }
        else {
            // Remove render layer from renderable
            removeRenderLayer(sortingLayerId);
        }
    });
}

void RenderLayerInstanceWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(0);
    m_mainLayout->setMargin(1);
    m_mainLayout->addWidget(m_checkBox);
}




RenderLayerSelectItem::RenderLayerSelectItem(const json& sortingLayerJson) :
    TreeItem(sortingLayerJson["id"].get<Uint32_t>(), kSortingLayer, sortingLayerJson)
{
    initializeItem();
}

RenderLayerSelectItem::~RenderLayerSelectItem()
{
}

void RenderLayerSelectItem::setWidget()
{
    assert(!m_widget && "Error, item already has a widget");

    // Get parent tree widget
    RenderLayerSelectWidget * parentWidget = static_cast<RenderLayerSelectWidget*>(treeWidget());

    // Set widget
    m_widget = new RenderLayerInstanceWidget(parentWidget->m_widgetManager,
        m_data.m_data.m_json,
        parentWidget->m_sceneObjectId,
        parentWidget->m_widgetMode);

    // Assign widget to item in tree widget
    parentWidget->setItemWidget(this, 0, m_widget);
}

void RenderLayerSelectItem::removeWidget(int column)
{
    Q_UNUSED(column)
    // Only ever one column, so don't need to worry about indexing
    treeWidget()->removeItemWidget(this, 0);
    m_widget = nullptr;
}

RenderLayerSelectWidget * RenderLayerSelectItem::renderLayerTreeWidget() const
{
    return static_cast<RenderLayerSelectWidget*>(treeWidget());
}




RenderLayerSelectWidget::RenderLayerSelectWidget(WidgetManager* wm, Int32_t sceneObjectId, ERenderLayerWidgetMode mode, QWidget* parent) :
    TreeWidget(wm, wm->actionManager(), "Render Layers", parent),
    m_sceneObjectId(sceneObjectId),
    m_widgetMode(mode)
{
    initializeWidget();
    repopulate();
}

RenderLayerSelectWidget::~RenderLayerSelectWidget()
{
}

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
    const json& sortingLayersJson = renderLayersJson();

    // Order a map of sorting layers
    std::multimap<int, json> sortedJsonLayers;
    for (const auto& layerJson : sortingLayersJson) {
        sortedJsonLayers.emplace(layerJson["order"].get<Int32_t>(), layerJson);
    }

    // Add sorting layers to widget
    for (const std::pair<Int32_t, json>& sortPair : sortedJsonLayers) {
        addItem(sortPair.second);
    }
}

const json& RenderLayerSelectWidget::renderLayersJson()
{
    return m_widgetManager->scenarioJson()["settings"]["renderLayers"];
}

RenderLayerSelectItem * RenderLayerSelectWidget::currentContextItem() const
{
    return static_cast<RenderLayerSelectItem*>(m_currentItems[kContextClick]);
}

void RenderLayerSelectWidget::initializeItem(QTreeWidgetItem * item)
{
    static_cast<RenderLayerSelectItem*>(item)->setWidget();
}

void RenderLayerSelectWidget::addItem(const json& sortingLayerJson)
{
    // Create resource item
    RenderLayerSelectItem* layerItem = new RenderLayerSelectItem(sortingLayerJson);

    // Add resource item
    TreeWidget::addItem(layerItem);
}

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

void RenderLayerSelectWidget::onItemDoubleClicked(QTreeWidgetItem * item, int column)
{
    TreeWidget::onItemDoubleClicked(item, column);

    // Downcast item
    RenderLayerSelectItem* layerItem = static_cast<RenderLayerSelectItem*>(item);
    assert(layerItem->m_widget && "Error, item should have a widget");
}

} // rev