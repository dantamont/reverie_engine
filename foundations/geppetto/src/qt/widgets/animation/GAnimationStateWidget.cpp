#include "geppetto/qt/widgets/animation/GAnimationStateWidget.h"
#include "geppetto/qt/style/GFontIcon.h"
#include "geppetto/qt/widgets/nodes/animation/GAnimationNodeWidget.h"
#include "geppetto/qt/widgets/animation/GAnimationTreeWidget.h"
#include "geppetto/qt/widgets/GWidgetManager.h"

#include "enums/GAnimationStateTypeEnum.h"
#include "ripple/network/gateway/GMessageGateway.h"

namespace rev {


AnimStateWidget::AnimStateWidget(WidgetManager* wm, json& stateJson, json& animationComponentState, Uint32_t sceneObjectId, AnimationNodeWidget* nodeWidget, QWidget* parent) :
    ParameterWidget(wm, parent),
    m_stateJson(stateJson),
    m_animationComponentJson(animationComponentState),
    m_nodeWidget(nodeWidget),
    m_sceneObjectId(sceneObjectId)
{
    initialize();
}


AnimStateWidget::~AnimStateWidget()
{
}

void AnimStateWidget::update()
{
    ParameterWidget::update();
}

void AnimStateWidget::initializeWidgets()
{
    const std::string& stateName = m_stateJson["name"].get_ref<const std::string&>();
    GAnimationStateType stateType = (GAnimationStateType)m_stateJson["stateType"].get<Int32_t>();
    switch ((EAnimationStateType)stateType) {
    case EAnimationStateType::eAnimation:
    {
        m_label = new QLabel(QStringLiteral("<b>Animation State: </b>") + stateName.c_str(), this);
        m_clipWidget = new AnimationClipTreeWidget(m_widgetManager, m_animationComponentJson, m_stateJson, m_sceneObjectId, m_nodeWidget, this);
        
        break;
    }
    case EAnimationStateType::eTransition:
    {
        /// @todo: Replace with viewport function once pulled in 
        QScreen* screen = QGuiApplication::primaryScreen();
        Float32_t dpi = screen->logicalDotsPerInchX();

        const std::string& startName = m_stateJson["start"].get_ref<const std::string&>();
        const std::string& endName = m_stateJson["end"].get_ref<const std::string&>();
        m_label = new QLabel(QStringLiteral("<b>Animation Transition: </b>") + stateName.c_str(), this);
        m_transitionInfo = new QLabel(QStringLiteral("<b>From: </b>") + QString(startName.c_str())
            + "<b> To: </b>" + QString(endName.c_str()));
        m_transitionTypes = new QComboBox();
        m_transitionTypes->addItem("Smooth", 0);
        m_transitionTypes->addItem("First State Frozen", 1);
        m_transitionTypes->setToolTip("Set the type of transition. Smooth plays both as they blend, while First Frozen pauses the first animation for the blend");
        
        const json& transitionSettings = m_stateJson["settings"];
        Float32_t fadeInTimeSec = transitionSettings["fadeInTime"];
        Float32_t fadeInWeight = transitionSettings["fadeInWeight"];
        Float32_t fadeOutTimeSec = transitionSettings["fadeOutTime"];
        Float32_t fadeOutWeight = transitionSettings["fadeOutWeight"];

        m_fadeInTimeSec = new QLineEdit(QString::number(fadeInTimeSec));
        m_fadeInTimeSec->setMaximumWidth(0.75 * dpi);
        m_fadeInTimeSec->setValidator(new QDoubleValidator(0, 1e20, 9));
        m_fadeInTimeSec->setToolTip("The fade in time (in seconds) for the new animation");

        m_fadeInBlendWeight = new QLineEdit(QString::number(fadeInWeight));
        m_fadeInBlendWeight->setMaximumWidth(0.75 * dpi);
        m_fadeInBlendWeight->setValidator(new QDoubleValidator(0, 1e20, 9));
        m_fadeInBlendWeight->setToolTip("The fade in blend weight for the new animation");

        m_fadeOutTimeSec = new QLineEdit(QString::number(fadeOutTimeSec));
        m_fadeOutTimeSec->setMaximumWidth(0.75 * dpi);
        m_fadeOutTimeSec->setValidator(new QDoubleValidator(0, 1e20, 9));
        m_fadeOutTimeSec->setToolTip("The fade out time (in seconds) for the outgoing animation");

        m_fadeOutBlendWeight = new QLineEdit(QString::number(fadeOutWeight));
        m_fadeOutBlendWeight->setMaximumWidth(0.75 * dpi);
        m_fadeOutBlendWeight->setValidator(new QDoubleValidator(0, 1e20, 9));
        m_fadeOutBlendWeight->setToolTip("The fade out blend weight for the outgoing animation");
        break;
    }
    default:
        assert(false && "Unimplemented");
        break;
    }
}

void AnimStateWidget::initializeConnections()
{
    GAnimationStateType stateType = (GAnimationStateType)m_stateJson["stateType"].get<Int32_t>();
    switch ((EAnimationStateType)stateType) {
    case EAnimationStateType::eAnimation:
    {
        break;
    }
    case EAnimationStateType::eTransition:
    {
        connect(m_transitionTypes, qOverload<int>(&QComboBox::currentIndexChanged),
            this,
            [this](int index) {
                m_stateJson["settings"]["transitionType"] = index;
                refreshMessageFromJson();
                m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_modifyTransitionMessage);
            }
        );

        connect(m_fadeInTimeSec, &QLineEdit::editingFinished,
            this,
            [this]() {
                double fadeInTime = m_fadeInTimeSec->text().toDouble();
                m_stateJson["settings"]["fadeInTime"] = fadeInTime;
                refreshMessageFromJson();
                m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_modifyTransitionMessage);
            }
        );

        connect(m_fadeInBlendWeight, &QLineEdit::editingFinished,
            this,
            [this]() {
                double fadeInBlendWeight = m_fadeInBlendWeight->text().toDouble();
                m_stateJson["settings"]["fadeInWeight"] = fadeInBlendWeight;
                refreshMessageFromJson();
                m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_modifyTransitionMessage);
            }
        );

        connect(m_fadeOutTimeSec, &QLineEdit::editingFinished,
            this,
            [this]() {
                double fadeOutTime = m_fadeOutTimeSec->text().toDouble();
                m_stateJson["settings"]["fadeOutTime"] = fadeOutTime;
                refreshMessageFromJson();
                m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_modifyTransitionMessage);
            }
        );

        connect(m_fadeOutBlendWeight, &QLineEdit::editingFinished,
            this,
            [this]() {
                double fadeOutBlendWeight = m_fadeOutBlendWeight->text().toDouble();
                m_stateJson["settings"]["fadeOutWeight"] = fadeOutBlendWeight;
                refreshMessageFromJson();
                m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_modifyTransitionMessage);
            }
        );
        break;
    }
    default:
        assert(false && "Unimplemented");
        break;
    }
}

void AnimStateWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(5);

    GAnimationStateType stateType = (GAnimationStateType)m_stateJson["stateType"].get<Int32_t>();
    switch ((EAnimationStateType)stateType) {
    case EAnimationStateType::eAnimation:
    {
        m_mainLayout->addWidget(m_label);
        m_mainLayout->addWidget(m_clipWidget);        
        break;
    }
    case EAnimationStateType::eTransition:
    {
        m_mainLayout->addWidget(m_label);
        m_mainLayout->addWidget(m_transitionInfo);

        QBoxLayout* typeLayout = LabeledLayout("Transition Type:", m_transitionTypes);

        QBoxLayout* fadeInLayout = LabeledLayout("Fade-In Time:", m_fadeInTimeSec);
        fadeInLayout->addWidget(new QLabel("Fade-In Blend Weight"));
        fadeInLayout->addWidget(m_fadeInBlendWeight);

        QBoxLayout* fadeOutLayout = LabeledLayout("Fade-Out Time:", m_fadeOutTimeSec);
        fadeOutLayout->addWidget(new QLabel("Fade-Out Blend Weight"));
        fadeOutLayout->addWidget(m_fadeOutBlendWeight);

        m_mainLayout->addLayout(typeLayout);
        m_mainLayout->addLayout(fadeInLayout);
        m_mainLayout->addLayout(fadeOutLayout);
        break;
    }
    default:
        assert(false && "Unimplemented");
        break;
    }

}

void AnimStateWidget::refreshMessageFromJson()
{
    // Update modify-transition message
    Uuid stateMachineId = m_animationComponentJson["animationController"]["stateMachineId"];
    Uuid transitionId = m_stateJson["id"];
    m_modifyTransitionMessage.setStateMachineId(stateMachineId);
    m_modifyTransitionMessage.setTransitionId(transitionId);
    m_modifyTransitionMessage.setSettingsJsonBytes(m_stateJson["settings"]);
}


} // rev