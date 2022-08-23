#include "geppetto/qt/widgets/tree/GShaderTreeWidget.h"

#include "fortress/json/GJson.h"

#include "geppetto/qt/widgets/GWidgetManager.h"

#include "ripple/network/gateway/GMessageGateway.h"
#include "ripple/network/messages/GGetScenarioJsonMessage.h"
#include "ripple/network/messages/GScenarioJsonMessage.h"

#include "ripple/network/messages/GAddShaderPresetMessage.h"
#include "ripple/network/messages/GRemoveShaderPresetMessage.h"

namespace rev {

ShaderJsonWidget::ShaderJsonWidget(WidgetManager* wm, const json& presetJson, QWidget *parent) :
    JsonWidget(wm, presetJson, { {"isShaderJsonWidget", true} }, parent)
{
    initializeWidgets();
    initializeConnections();
    layoutWidgets();
}

ShaderJsonWidget::~ShaderJsonWidget()
{
}

void ShaderJsonWidget::wheelEvent(QWheelEvent* event) {
    if (!event->pixelDelta().isNull()) {
        ParameterWidget::wheelEvent(event);
    }
    else {
        // If scrolling has reached top or bottom
        // Accept event and stop propagation if at bottom of scroll area
        event->accept();
    }
}

void ShaderJsonWidget::initializeWidgets()
{
    JsonWidget::initializeWidgets();
    m_typeLabel = new QLabel("ShaderPreset");
    m_typeLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void ShaderJsonWidget::layoutWidgets()
{
    // Format widget sizes
    m_textEdit->setMaximumHeight(150);

    JsonWidget::layoutWidgets();

    // Note, cannot call again without deleting previous layout
    // https://doc.qt.io/qt-5/qwidget.html#setLayout
    setLayout(m_mainLayout);
}





ShaderTreeItem::ShaderTreeItem(const json& presetJson) :
    TreeItem(0, kShaderPreset, presetJson)
{
    initializeItem();
}

ShaderTreeItem::~ShaderTreeItem()
{
}

void ShaderTreeItem::setWidget()
{
    // Throw error if the widget already exists
    assert (!m_widget && "Error, item already has a widget");

    // Get parent tree widget
    ShaderTreeWidget * parentWidget = static_cast<ShaderTreeWidget*>(treeWidget());

    // Set widget
    m_widget = new ShaderJsonWidget(parentWidget->m_widgetManager, m_data.m_data.m_json);

    // Assign widget to item in tree widget
    parentWidget->setItemWidget(this, 0, m_widget);
}

void ShaderTreeItem::removeWidget(int column)
{
    // Only ever one column, so don't need to worry about indexing
    Q_UNUSED(column)
    treeWidget()->removeItemWidget(this, 0);
    m_widget = nullptr;
}

ShaderTreeWidget * ShaderTreeItem::shaderTreeWidget() const
{
    return static_cast<ShaderTreeWidget*>(treeWidget());
}




ShaderTreeWidget::ShaderTreeWidget(WidgetManager* wm, QWidget* parent) :
    TreeWidget(wm, wm->actionManager(), "Shader Presets", parent)
{
    initializeWidget();
    repopulate();
}

ShaderTreeItem* ShaderTreeWidget::getItem(const json& presetJson)
{
#ifdef DEBUG_MODE
    assert(!presetJson.empty() && "Null JSON provided");
#endif

    const std::string& presetName = presetJson["name"].get_ref<const std::string&>();
    QTreeWidgetItemIterator it(this);
    while (*it) {
        ShaderTreeItem* item = static_cast<ShaderTreeItem*>(*it);
        if (item->data().getRef<const std::string&>("name") == presetName) {
            return item;
        }
        ++it;
    }

    return nullptr;
}

ShaderTreeWidget::~ShaderTreeWidget()
{
}

void ShaderTreeWidget::repopulate()
{
    // Clear the widget
    clear();

    // Add shader presets
    const json& presets = m_widgetManager->scenarioJson()["settings"]["shaderPresets"];
    for (const json& shaderPreset : presets) {
        addItem(shaderPreset);
    }

    // Resize columns
    resizeColumns();
}

void ShaderTreeWidget::addItem(const json& presetJson)
{
    // Create resource item
    ShaderTreeItem* layerItem = new ShaderTreeItem(presetJson);

    // Add resource item
    TreeWidget::addItem(layerItem);
}

void ShaderTreeWidget::removeItem(ShaderTreeItem * shaderMatItem)
{
    delete shaderMatItem;
}

void ShaderTreeWidget::removeItem(const json& presetJson)
{
    ShaderTreeItem* item = static_cast<ShaderTreeItem*>(getItem(presetJson));
    delete item;
}

void ShaderTreeWidget::initializeWidget()
{
    TreeWidget::initializeWidget();

    setMinimumSize(350, 350);

    initializeAsList();
    enableDragAndDrop();

    // Initialize actions
    addAction(kNoItemSelected,
        "Add Shader Preset",
        "Add a shader preset to the scenario",
        [this] {
            static GAddShaderPresetMessage s_message;
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
            requestRepopulate();
        }
    );

    addAction(kItemSelected,
        "Remove Shader Preset",
        "Remove shader preset from the scenario",
        [this] {
            static GRemoveShaderPresetMessage s_message;
            GString name = currentContextItem()->data().getRef<const std::string&>("name");
            s_message.setName(name.c_str());
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
            requestRepopulate();
        }
    );

    // Repopulate on scenario JSON receive
    connect(m_widgetManager, &WidgetManager::receivedScenarioJsonMessage, this, &ShaderTreeWidget::repopulate);
}

void ShaderTreeWidget::initializeItem(QTreeWidgetItem * item)
{
    static_cast<ShaderTreeItem*>(item)->setWidget();
}

void ShaderTreeWidget::requestRepopulate() const {
    static GGetScenarioJsonMessage s_message;
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
}


} // rev