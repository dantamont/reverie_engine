#include "GbScriptComponentWidget.h"

#include "../components/GbPhysicsWidgets.h"
#include "../../core/physics/GbPhysicsShape.h"
#include "../../core/physics/GbPhysicsGeometry.h"
#include "../../core/physics/GbPhysicsManager.h"

#include "../../core/GbCoreEngine.h"
#include "../../core/loop/GbSimLoop.h"
#include "../tree/GbComponentWidget.h"
#include "../../core/resource/GbResourceCache.h"

#include "../../core/scene/GbSceneObject.h"
#include "../../core/readers/GbJsonReader.h"
#include "../../core/components/GbComponent.h"
#include "../../core/components/GbScriptComponent.h"
#include "../../core/components/GbPhysicsComponents.h"
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
// ScriptWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
ScriptWidget::ScriptWidget(CoreEngine* core, Component* component, QWidget *parent) :
    ComponentWidget(core, component, parent){
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ScriptComponent* ScriptWidget::scriptComponent() const {
    return static_cast<ScriptComponent*>(m_component);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ScriptWidget::~ScriptWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    QString path = "";
    if (scriptComponent()->script()) {
        path = scriptComponent()->script()->getPath();
    }
    m_fileWidget = new FileLoadWidget(m_engine, path, "Open Python Behavior", "Python Scripts (*.py)");
    m_fileWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    // Add refresh script button
    m_confirmButton = new QPushButton();
    m_confirmButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();

    // Make connection to load python script
    connect(m_fileWidget->lineEdit(), &QLineEdit::textChanged, this, [this]() {

        const QString& text = m_fileWidget->lineEdit()->text();
        scriptComponent()->initializeBehavior(text);
    });

    // Make connection to resize script 
    connect(m_confirmButton,
        &QPushButton::clicked, 
        this, [this](bool checked) {
        Q_UNUSED(checked);

        // Pause scenario to edit component
        SimulationLoop* simLoop = m_engine->simulationLoop();
        bool wasPlaying = simLoop->isPlaying();
        if (wasPlaying) {
            simLoop->pause();
        }
        m_engine->simulationLoop()->pause();

        // Refresh script 
        auto scriptComponent = (ScriptComponent*)(m_component);
        scriptComponent->reset();

        // Unpause scenario
        if (wasPlaying) {
            simLoop->play();
        }
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptWidget::layoutWidgets()
{
    // Create base widget layout
    ComponentWidget::layoutWidgets();

    // Create new widgets
    QBoxLayout* layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addWidget(new QLabel("Python Script Path:"));
    layout->addWidget(m_fileWidget);
    layout->addWidget(m_confirmButton);

    // Add widgets to main layout
    m_mainLayout->addLayout(layout);

}



///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
} // View
} // Gb