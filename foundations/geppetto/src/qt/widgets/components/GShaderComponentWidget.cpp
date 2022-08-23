#include "geppetto/qt/widgets/components/GShaderComponentWidget.h"
#include "geppetto/qt/widgets/components/GComponentWidget.h"
#include "geppetto/qt/style/GFontIcon.h"

#include "fortress/containers/math/GEulerAngles.h"
#include "fortress/json/GJson.h"

namespace rev {

ShaderComponentWidget::ShaderComponentWidget(WidgetManager* wm, const json& componentJson, Int32_t sceneObjectId, QWidget* parent) :
    SceneObjectComponentWidget(wm, componentJson, sceneObjectId, parent) {
    m_selectedPresetMessage.setSceneObjectId(sceneObjectId);
    initialize();
}

ShaderComponentWidget::~ShaderComponentWidget()
{
}

void ShaderComponentWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    // Populate presets
    m_presetSelectWidget = new QComboBox;
    m_presetSelectWidget->setMaximumWidth(3000);
    const json& presets = m_widgetManager->scenarioJson()["settings"]["shaderPresets"];
    for (const json& preset : presets) {
        m_presetSelectWidget->addItem(SAIcon("list-alt"), preset["name"].get_ref<const std::string&>().c_str());
    }

    // Select the current preset
    assert((m_componentJson.contains("shaderPreset") || !presets.size()) && "Component has no preset selected");
    
    if (m_componentJson.contains("shaderPreset")) {
        // If a preset is already selected, select that one
        QString name = m_componentJson["shaderPreset"].get_ref<const std::string&>().c_str();
        m_presetSelectWidget->setCurrentText(name);
    }

}

void ShaderComponentWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();

    connect(m_presetSelectWidget,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int index) {
        G_UNUSED(index);

        GStringFixedSize presetName = m_presetSelectWidget->currentText().toStdString();
        m_selectedPresetMessage.setShaderPresetName(presetName);
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_selectedPresetMessage);

    }
    );

}

void ShaderComponentWidget::layoutWidgets()
{
    // Create base widget layout
    ComponentWidget::layoutWidgets();

    // Create new widgets
    QBoxLayout* layout = new QVBoxLayout;
    //layout->setMargin(0);
    layout->setSpacing(0);

    layout->addWidget(new QLabel("Preset:"));
    layout->addWidget(m_presetSelectWidget);

    // Add widgets to main layout
    m_mainLayout->addLayout(layout);
}


} // rev