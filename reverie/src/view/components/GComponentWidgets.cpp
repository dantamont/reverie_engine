#include "GComponentWidgets.h"

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
void ComponentJsonWidget::wheelEvent(QWheelEvent* event) {
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
void ComponentJsonWidget::layoutWidgets()
{
    // Format widget sizes
    m_textEdit->setMaximumHeight(350);

    JsonWidget::layoutWidgets();

    // Note, cannot call again without deleting previous layout
    // https://doc.qt.io/qt-5/qwidget.html#setLayout
    setLayout(m_mainLayout);
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
    m_jsonWidget->setReloadJson(true);

    // Repopulate text on toggle
    connect(this, &ComponentWidget::toggled, this, [this]() {
        m_jsonWidget->updateText();
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void GenericComponentWidget::layoutWidgets()
{
    ComponentWidget::layoutWidgets();
    m_mainLayout->addWidget(m_jsonWidget);
}





///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
} // View
} // rev