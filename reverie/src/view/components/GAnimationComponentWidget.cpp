#include "GAnimationComponentWidget.h"

#include "../../core/GCoreEngine.h"
#include "../../core/loop/GSimLoop.h"
#include "../tree/GComponentWidget.h"
#include "../../core/resource/GResourceCache.h"

#include "../../core/scene/GSceneObject.h"
#include "../../core/components/GAnimationComponent.h"
#include "../../core/components/GTransformComponent.h"
#include "../style/GFontIcon.h"

#include "../../GMainWindow.h"
#include "../nodes/animation/GAnimationNodeWidget.h"
#include "../wrappers/GComboBox.h"
#include "../../core/animation/GAnimStateMachine.h"
#include "../../core/animation/GAnimationManager.h"

namespace rev {
namespace View {
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// AnimationComponentWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
AnimationComponentWidget::AnimationComponentWidget(CoreEngine* core, Component* component, QWidget *parent) :
    ComponentWidget(core, component, parent) {
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
AnimationComponentWidget::~AnimationComponentWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
BoneAnimationComponent * AnimationComponentWidget::animationComponent() const
{
    return static_cast<BoneAnimationComponent*>(m_component);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationComponentWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    // Widget for selecting state machine
    m_stateMachineSelect = new EditableComboBox(this);
    m_stateMachineSelect->comboBox()->setMaximumWidth(200);
    m_stateMachineSelect->setToolTip("Select the state machine to use for this set of animations");
    AnimationStateMachine* stateMachine = animationComponent()->animationController().stateMachine();
    if (stateMachine) {
        m_stateMachineSelect->setSelectedID(stateMachine->getUuid());
    }
    repopulateStateMachineList();

    // Widget for instantiating animation state machine widget
    m_launchGraphButton = new QPushButton(SAIcon("project-diagram"), " Configure", this);

    //m_jsonWidget = new ComponentJsonWidget(m_engine, m_component, nullptr);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationComponentWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();
    //m_jsonWidget->setReloadJson(true);

    //// Repopulate text on toggle
    //connect(this, &ComponentWidget::toggled, this, [this]() {
    //    m_jsonWidget->updateText();
    //});

    // Rename state machine on edit
    connect(m_stateMachineSelect, &EditableComboBox::editingFinished, this,
        [this](QString text) {
        Uuid currentID = m_stateMachineSelect->comboBox()->currentData().toUuid();
        AnimationStateMachine* selected = m_engine->animationManager()->getStateMachine(currentID);
        if (selected) {
            selected->setName(text);
            repopulateStateMachineList();
        }
    });


    // Switch state machine
    connect(m_stateMachineSelect, &EditableComboBox::currentIndexChanged, this,
        [this](int idx) {
        Uuid currentID = m_stateMachineSelect->comboBox()->itemData(idx).toUuid();
        AnimationStateMachine* selected = m_engine->animationManager()->getStateMachine(currentID);
        animationComponent()->animationController().setStateMachine(selected);
    });

    // Add a new state machine
    connect(m_stateMachineSelect, &EditableComboBox::clickedAddItem, this,
        [this]() {
        m_engine->animationManager()->addStateMachine();
        repopulateStateMachineList();
    });

    // TODO: Add connection to elegantly remove state machines

    // Create animation node widget on configure
    connect(m_launchGraphButton, &QPushButton::clicked, this, [this](bool checked) {
        Q_UNUSED(checked);

        AnimationNodeWidget* animNodeWidget = new AnimationNodeWidget(m_engine, animationComponent(), m_engine->mainWindow());
        
        // If no state machine, don't do anything
        if (!animationComponent()->animationController().stateMachine()) {
            return;
        }

        // Create window at top left of screen
        animNodeWidget->setWindowFlags(Qt::Window);
        QRect screenrect = QApplication::primaryScreen()->geometry();
        animNodeWidget->move(screenrect.left(), screenrect.top());
        //animNodeWidget->show();
        animNodeWidget->showMaximized();
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationComponentWidget::layoutWidgets()
{
    ComponentWidget::layoutWidgets();
    m_mainLayout->addWidget(m_stateMachineSelect);
    m_mainLayout->addWidget(m_launchGraphButton);
    //m_mainLayout->addWidget(m_jsonWidget);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationComponentWidget::repopulateStateMachineList()
{
    m_stateMachineSelect->populate(m_engine->animationManager()->stateMachines(), "sitemap");
}





///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
} // View
} // rev