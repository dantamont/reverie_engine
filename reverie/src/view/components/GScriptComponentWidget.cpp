#include "GScriptComponentWidget.h"

#include "../components/GPhysicsWidgets.h"
#include "../../core/physics/GPhysicsShape.h"
#include "../../core/physics/GPhysicsGeometry.h"
#include "../../core/physics/GPhysicsManager.h"

#include "../../core/GCoreEngine.h"
#include "../../core/loop/GSimLoop.h"
#include "../tree/GComponentWidget.h"
#include "../../core/resource/GResourceCache.h"

#include "../../core/scene/GSceneObject.h"
#include "../../core/readers/GJsonReader.h"
#include "../../core/components/GComponent.h"
#include "../../core/components/GScriptComponent.h"
#include <core/components/GCharControlComponent.h>
#include <core/components/GRigidBodyComponent.h>
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
        path = (const char*)scriptComponent()->script()->getPath();
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
} // rev