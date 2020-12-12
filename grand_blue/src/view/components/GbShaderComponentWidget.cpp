#include "GbShaderComponentWidget.h"

#include "../../core/scene/GbScenario.h"

#include "../../core/GbCoreEngine.h"
#include "../../core/loop/GbSimLoop.h"
#include "../tree/GbComponentWidget.h"
#include "../../core/resource/GbResourceCache.h"

#include "../../core/scene/GbSceneObject.h"
#include "../../core/readers/GbJsonReader.h"
#include "../../core/components/GbScriptComponent.h"
#include "../../core/components/GbShaderComponent.h"
#include "../../core/scripting/GbPythonScript.h"
#include "../../core/components/GbLightComponent.h"
#include "../../core/components/GbCameraComponent.h"

#include "../../core/components/GbShaderComponent.h"
#include "../../core/components/GbTransformComponent.h"
#include "../style/GbFontIcon.h"
#include "../../core/geometry/GbEulerAngles.h"

#include "../../core/physics/GbPhysicsActor.h"

namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// ShaderComponentWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
ShaderComponentWidget::ShaderComponentWidget(CoreEngine* core,
    Component* component, 
    QWidget *parent) :
    ComponentWidget(core, component, parent) {
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ShaderComponentWidget::~ShaderComponentWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ShaderComponent* ShaderComponentWidget::shaderComponent() const {
    return static_cast<ShaderComponent*>(m_component);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ShaderComponentWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    // Presets
    m_presetSelectWidget = new QComboBox;
    m_presetSelectWidget->setMaximumWidth(3000);
    std::vector<std::shared_ptr<ShaderPreset>>& presets = m_engine->scenario()->settings().shaderPresets();
    for (const auto& preset : presets) {
        m_presetSelectWidget->addItem(SAIcon("list-alt"), preset->getName());
    }

    std::shared_ptr<ShaderPreset> preset = shaderComponent()->shaderPreset();
    
    if (preset) {
        const QString& name = preset->getName();
        m_presetSelectWidget->setCurrentText(name);
    }
    else {
        // Set preset if there are any in the list
        if (presets.size()) {
            shaderComponent()->setShaderPreset(presets[0]);
        }
;    }

}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ShaderComponentWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();

    connect(m_presetSelectWidget,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int index) {
        Q_UNUSED(index)

        pauseSimulation();

        const QString& presetName = m_presetSelectWidget->currentText();
        bool wasCreated;
        auto shaderPreset = m_engine->scenario()->settings().getShaderPreset(presetName, wasCreated);
        if (wasCreated) throw("Error, shader preset must exist");
        if (!shaderPreset) throw("Error, preset does not exist");
        shaderComponent()->setShaderPreset(shaderPreset);

        resumeSimulation();
    }
    );

}
///////////////////////////////////////////////////////////////////////////////////////////////////
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



///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
} // View
} // Gb