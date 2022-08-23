#include "geppetto/qt/widgets/components/GAnimationComponentWidget.h"
#include "geppetto/qt/widgets/components/GComponentWidget.h"
#include "geppetto/qt/style/GFontIcon.h"

#include "geppetto/qt/widgets/nodes/animation/GAnimationNodeWidget.h"
#include "geppetto/qt/widgets/general/GComboBox.h"

namespace rev {

AnimationComponentWidget::AnimationComponentWidget(WidgetManager* wm, const json& componentJson, Uint32_t sceneObjectId, QWidget *parent) :
    ComponentWidget(wm, componentJson, sceneObjectId, parent) {
    initialize();
}

AnimationComponentWidget::~AnimationComponentWidget()
{
}

void AnimationComponentWidget::clearNodeWidget()
{
    m_animationNodeWidget = nullptr;
}

void AnimationComponentWidget::update(const GAnimationStateMachinesMessage& message)
{
    json stateMachinesJson = GJson::FromBytes(message.getJsonBytes());
    m_stateMachineSelect->populate(stateMachinesJson, "sitemap");
}

void AnimationComponentWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    // Widget for selecting state machine
    m_stateMachineSelect = new EditableComboBox(this);
    m_stateMachineSelect->comboBox()->setMaximumWidth(200);
    m_stateMachineSelect->setToolTip("Select the state machine to use for this set of animations");
    Uuid stateMachineId = m_componentJson["animationController"]["stateMachineId"];
    m_stateMachineSelect->setSelectedID(stateMachineId);

    // Request update state machines to populate widget
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_requestAnimationStateMachinesMessage);

    // Widget for instantiating animation state machine widget
    m_launchGraphButton = new QPushButton(SAIcon("project-diagram"), " Configure", this);

}

void AnimationComponentWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();

    // Rename state machine on edit
    connect(m_stateMachineSelect, &EditableComboBox::editingFinished, this,
        [this](QString text) {
            Uuid currentID = m_stateMachineSelect->comboBox()->currentData().toUuid().toString().toStdString();
        
            m_renameMachineMessage.setStateMachineId(currentID);
            m_renameMachineMessage.setNewName(text.toStdString().c_str());
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_renameMachineMessage);
        }
    );


    // Switch state machine
    connect(m_stateMachineSelect, &EditableComboBox::currentIndexChanged, this,
        [this](int idx) {
            Uuid currentID = m_stateMachineSelect->comboBox()->itemData(idx).toUuid().toString().toStdString();

            m_setComponentStateMachineMessage.setNewStateMachineId(currentID);
            m_setComponentStateMachineMessage.setSceneObjectId(m_sceneOrObjectId);
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_setComponentStateMachineMessage);
        }
    );

    // Add a new state machine
    connect(m_stateMachineSelect, &EditableComboBox::clickedAddItem, this,
        [this]() {
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_addStateMachineMessage);
        }
    );

    // TODO: Add connection to elegantly remove state machines

    // Create animation node widget on configure
    connect(m_launchGraphButton, &QPushButton::clicked, this, [this](bool checked) {
        Q_UNUSED(checked);

        m_animationNodeWidget = new AnimationNodeWidget(m_widgetManager, m_componentJson, m_sceneOrObjectId, this/*parentWidget()*/);
        
        // If no state machine, don't do anything
        Uuid stateMachineId = m_componentJson["animationController"]["stateMachineId"];
        if (stateMachineId.isNull()) {
            return;
        }

        // Create window at top left of screen
        m_animationNodeWidget->setWindowFlags(Qt::Window);
        QRect screenrect = QApplication::primaryScreen()->geometry();
        m_animationNodeWidget->move(screenrect.left(), screenrect.top());
        m_animationNodeWidget->showMaximized();
    });
}

void AnimationComponentWidget::layoutWidgets()
{
    ComponentWidget::layoutWidgets();
    m_mainLayout->addWidget(m_stateMachineSelect);
    m_mainLayout->addWidget(m_launchGraphButton);
}


} // rev
