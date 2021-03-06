#include "GShaderComponentWidget.h"

#include "../../core/scene/GScenario.h"

#include "../../core/GCoreEngine.h"
#include "../../core/loop/GSimLoop.h"
#include "../tree/GComponentWidget.h"
#include "../../core/resource/GResourceCache.h"

#include "../../core/scene/GSceneObject.h"
#include "../../core/readers/GJsonReader.h"
#include "../../core/components/GScriptComponent.h"
#include "../../core/components/GShaderComponent.h"
#include "../../core/scripting/GPythonScript.h"
#include "../../core/components/GLightComponent.h"
#include "../../core/components/GCameraComponent.h"

#include "../../core/components/GShaderComponent.h"
#include "../../core/components/GTransformComponent.h"
#include "../style/GFontIcon.h"
#include "../../core/geometry/GEulerAngles.h"

#include "../../core/physics/GPhysicsActor.h"

namespace rev {
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
        m_presetSelectWidget->addItem(SAIcon("list-alt"), (QString)preset->getName());
    }

    std::shared_ptr<ShaderPreset> preset = shaderComponent()->shaderPreset();
    
    if (preset) {
        QString name = preset->getName().c_str();
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
} // rev