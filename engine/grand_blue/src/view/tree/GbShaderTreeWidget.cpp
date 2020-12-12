#include "GbShaderTreeWidget.h"

#include "../../core/loop/GbSimLoop.h"
#include "../../core/readers/GbJsonReader.h"
#include "../../core/resource/GbResourceCache.h"

#include "../../core/scene/GbScenario.h"

#include "../GbWidgetManager.h"
#include "../../GbMainWindow.h"

namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// ShaderJsonWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
ShaderJsonWidget::ShaderJsonWidget(CoreEngine* core, ShaderPreset* preset, QWidget *parent) :
    JsonWidget(core, preset, parent)
{
    initializeWidgets();
    initializeConnections();
    layoutWidgets();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ShaderJsonWidget::~ShaderJsonWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////////////////////////
void ShaderJsonWidget::initializeWidgets()
{
    JsonWidget::initializeWidgets();
    m_typeLabel = new QLabel("ShaderPreset");
    m_typeLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ShaderJsonWidget::layoutWidgets()
{
    // Format widget sizes
    m_textEdit->setMaximumHeight(150);

    JsonWidget::layoutWidgets();

    // Note, cannot call again without deleting previous layout
    // https://doc.qt.io/qt-5/qwidget.html#setLayout
    setLayout(m_mainLayout);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool ShaderJsonWidget::isValidObject(const QJsonObject & object)
{
    Q_UNUSED(object)
    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ShaderJsonWidget::preLoad()
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
void ShaderJsonWidget::postLoad()
{
    // Unpause scenario
    SimulationLoop* simLoop = m_engine->simulationLoop();
    if (m_wasPlaying) {
        simLoop->play();
    }

    emit editedShaderPreset(shaderPreset()->getUuid());
}




///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// ShaderTreeItem
///////////////////////////////////////////////////////////////////////////////////////////////////
ShaderTreeItem::ShaderTreeItem(ShaderPreset * layer) :
    TreeItem(layer, kShaderPreset)
{
    initializeItem();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ShaderTreeItem::~ShaderTreeItem()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ShaderTreeItem::setWidget()
{
    // Throw error if the widget already exists
    if (m_widget) throw("Error, item already has a widget");

    // Get parent tree widget
    ShaderTreeWidget * parentWidget = static_cast<ShaderTreeWidget*>(treeWidget());

    // Set widget
    ShaderPreset* mtl = shaderPreset();
    QJsonDocument doc(mtl->asJson().toObject());
    QString rep(doc.toJson(QJsonDocument::Indented));
    m_widget = new ShaderJsonWidget(parentWidget->m_engine, mtl);

    // Connect signals and slots
    //QObject::connect((ShaderJsonWidget*)m_widget, &ShaderJsonWidget::editedScriptOrder,
    //    parentWidget, &ShaderTreeWidget::repopulate);

    // Assign widget to item in tree widget
    parentWidget->setItemWidget(this, 0, m_widget);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ShaderTreeItem::removeWidget(int column)
{
    // Only ever one column, so don't need to worry about indexing
    Q_UNUSED(column)
    treeWidget()->removeItemWidget(this, 0);
    m_widget = nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
View::ShaderTreeWidget * ShaderTreeItem::shaderTreeWidget() const
{
    return static_cast<View::ShaderTreeWidget*>(treeWidget());
}



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// ShaderTreeWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
ShaderTreeWidget::ShaderTreeWidget(CoreEngine* engine, QWidget* parent) :
    TreeWidget(engine, "Shader Presets", parent)
{
    initializeWidget();
    repopulate();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ShaderTreeWidget::~ShaderTreeWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ShaderTreeWidget::repopulate()
{
    // Clear the widget
    clear();

    // Add shader presets
    for (const std::shared_ptr<ShaderPreset>& shaderPreset : m_engine->scenario()->settings().shaderPresets()) {
        addItem(shaderPreset.get());
    }

    // Resize columns
    resizeColumns();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ShaderTreeWidget::addItem(ShaderPreset * ShaderPreset)
{
    // Create resource item
    ShaderTreeItem* layerItem = new View::ShaderTreeItem(ShaderPreset);

    // Add resource item
    TreeWidget::addItem(layerItem);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ShaderTreeWidget::removeItem(ShaderTreeItem * shaderMatItem)
{
    delete shaderMatItem;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ShaderTreeWidget::removeItem(ShaderPreset* itemObject)
{
    ShaderTreeItem* item = static_cast<ShaderTreeItem*>(getItem(*itemObject));
    delete item;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
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
        bool created;
        m_engine->scenario()->settings().getShaderPreset(Uuid::UniqueName("preset_"), created);
        if (!created) throw("Error, resource not created");
        repopulate();
    });

    addAction(kItemSelected,
        "Remove Shader Preset",
        "Remove shader preset from the scenario",
        [this] {
        // Add sorting layer
        m_engine->scenario()->settings().removeShaderPreset(currentContextItem()->shaderPreset()->getName());
        repopulate();
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ShaderTreeWidget::initializeItem(QTreeWidgetItem * item)
{
    static_cast<ShaderTreeItem*>(item)->setWidget();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
}
}