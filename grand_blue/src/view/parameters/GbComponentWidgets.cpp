#include "GbComponentWidgets.h"

#include "../../core/GbCoreEngine.h"
#include "../../core/loop/GbSimLoop.h"
#include "../tree/GbComponentWidget.h"

#include "../../core/scene/GbSceneObject.h"
#include "../../core/readers/GbJsonReader.h"
#include "../../core/components/GbComponent.h"
#include "../../core/components/GbScriptComponent.h"
#include "../../core/scripting/GbPythonScript.h"
#include "../../core/components/GbLight.h"

#include "../../core/rendering/renderer/GbRenderers.h"
#include "../../core/components/GbRendererComponent.h"

namespace Gb {
namespace View {
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// Component Widget
///////////////////////////////////////////////////////////////////////////////////////////////////
ComponentWidget::ComponentWidget(CoreEngine* core, Component* component, QWidget *parent) :
    ParameterWidget(core, parent),
    m_component(component)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentWidget::initializeWidgets()
{
    ParameterWidget::initializeWidgets();
    m_toggle = new QCheckBox(nullptr);
    m_toggle->setChecked(m_component->isEnabled());
    m_typeLabel = new QLabel(m_component->className());
    m_typeLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ComponentWidget::~ComponentWidget()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentWidget::initializeConnections()
{
    ParameterWidget::initializeConnections();
    connect(m_toggle, &QCheckBox::stateChanged, this, [this](int state) {
        m_component->toggle(state);
        emit toggled(state);
    });

}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout;

    QBoxLayout* layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addWidget(m_toggle);
    layout->addWidget(m_typeLabel);
    layout->setAlignment(Qt::AlignCenter);

    m_mainLayout->addLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// Component JSON Widget
///////////////////////////////////////////////////////////////////////////////////////////////////
ComponentJsonWidget::ComponentJsonWidget(CoreEngine* core, Component* component, QWidget *parent):
    JsonWidget(core, component, parent)
{
    initialize();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
ComponentJsonWidget::~ComponentJsonWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
Component* ComponentJsonWidget::component() { 
    return static_cast<Component*>(m_serializable);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentJsonWidget::preLoad()
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
void ComponentJsonWidget::postLoad()
{
    // Unpause scenario
    SimulationLoop* simLoop = m_engine->simulationLoop();
    if (m_wasPlaying) {
        simLoop->play();
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// GenericComponentWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
GenericComponentWidget::GenericComponentWidget(CoreEngine* core, Component* component, QWidget *parent) :
    ComponentWidget(core, component, parent) {
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
GenericComponentWidget::~GenericComponentWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void GenericComponentWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    m_jsonWidget = new ComponentJsonWidget(m_engine, m_component, nullptr);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void GenericComponentWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();

    // Repopulate text on toggle
    connect(this, &ComponentWidget::toggled, this, [this]() {
        m_jsonWidget->updateText(true);
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void GenericComponentWidget::layoutWidgets()
{
    ComponentWidget::layoutWidgets();
    m_mainLayout->addWidget(m_jsonWidget);
}


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
}
}