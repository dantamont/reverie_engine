#include "geppetto/qt/widgets/animation/GAnimationClipWidget.h"
#include "geppetto/qt/style/GFontIcon.h"

#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/animation/GAnimationTreeWidget.h"
#include "geppetto/qt/widgets/nodes/animation/GAnimationNodeWidget.h"

#include "enums/GAnimationStateTypeEnum.h"
#include "ripple/network/gateway/GMessageGateway.h"

#include "fortress/layer/framework/GSignalSlot.h"

namespace rev {


AnimClipWidget::AnimClipWidget(WidgetManager* wm, json& animationClipJson, Uuid stateId, Uuid stateMachineId, AnimationNodeWidget* nodeWidget, QWidget* parent) :
    ParameterWidget(wm, parent),
    m_clipJson(animationClipJson),
    m_stateId(stateId),
    m_stateMachineId(stateMachineId),
    m_nodeWidget(nodeWidget)
{
    m_modifyClipMessage.setStateId(stateId);
    m_modifyClipMessage.setStateMachineId(stateMachineId);
    m_modifyClipMessage.setClipId(m_clipJson["id"].get<Uuid>());
    initialize();
}


AnimClipWidget::~AnimClipWidget()
{
}

void AnimClipWidget::update(const GAnimationDataMessage& message)
{
    m_animationDurationSec = message.getAnimationDurationSec();

    // Reset duration
    Real_t duration = getDuration();
    m_playbackDuration->setText(QString::number(m_animationDurationSec));
}

void AnimClipWidget::update()
{
    ParameterWidget::update();
}

void AnimClipWidget::initializeWidgets()
{
    QString clipName = getClipName();
    m_nameEdit = new QLineEdit(clipName);

    Float64_t speedFactor = getSpeedFactor();
    m_speedFactor = new QLineEdit(QString::number(speedFactor));
    m_speedFactor->setMaximumWidth(75);
    m_speedFactor->setValidator(new QDoubleValidator(0, 1e6, 3));
    m_speedFactor->setToolTip("Scaling factor for controlling playback speed");

    Real_t duration = getDuration();
    m_playbackDuration = new QLineEdit(QString::number(duration));
    m_playbackDuration->setMaximumWidth(75);
    m_playbackDuration->setValidator(new QDoubleValidator(0, 1e6, 3));
    m_speedFactor->setToolTip("Set the duration (in seconds) of animation playback directly");

    m_tickDelay = new QLineEdit(QString::number(getTickDelay()));
    m_tickDelay->setMaximumWidth(75);
    m_tickDelay->setValidator(new QDoubleValidator(0, 1e6, 1));
    m_tickDelay->setToolTip("Number of ticks to delay playback by");

    m_timeDelay = new QLineEdit(QString::number(getTimeDelay()));
    m_timeDelay->setMaximumWidth(75);
    m_timeDelay->setValidator(new QDoubleValidator(0, 1e6, 1));
    m_timeDelay->setToolTip("Number of seconds to delay playback by");

    requestAnimationInfo();
}

void AnimClipWidget::initializeConnections()
{
    // Connect to node tree widget
    connect(m_nodeWidget,
        &AnimationNodeWidget::receivedAnimationData,
        this,
        Signal<const GAnimationDataMessage&>::Cast(&AnimClipWidget::update));

    connect(m_nameEdit,
        &QLineEdit::editingFinished,
        this,
        [this]() {
            json j = { {"newName", m_nameEdit->text().toStdString()} };
            m_modifyClipMessage.setJsonBytes(j);
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_modifyClipMessage);
        }
    );

    connect(m_speedFactor,
        &QLineEdit::editingFinished,
        this,
        [this]() {
        
            // Set speed factor in animation clip
            Real_t speedFactor = m_speedFactor->text().toDouble();

            // Set duration in widget
            Real_t duration = m_animationDurationSec / speedFactor;
            m_playbackDuration->setText(QString::number(duration));

            json j = { {"speedFactor", m_speedFactor->text().toDouble()} };
            m_modifyClipMessage.setJsonBytes(j);
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_modifyClipMessage);
            }
    );

    connect(m_playbackDuration,
        &QLineEdit::editingFinished,
        this,
        [this]() {

            // Back out speed factor from playback duration
            Real_t playbackDuration = m_playbackDuration->text().toDouble();
            Real_t speedFactor = m_animationDurationSec / playbackDuration;

            // Set speed factor in animation clip and widget
            m_speedFactor->setText(QString::number(speedFactor));

            json j = { {"speedFactor", speedFactor} };
            m_modifyClipMessage.setJsonBytes(j);
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_modifyClipMessage);
        }
    );

    connect(m_tickDelay,
        &QLineEdit::editingFinished,
        this,
        [this]() {

            // Set tick delay in animation clip
            Real_t tickDelay = m_tickDelay->text().toDouble();

            json j = { {"tickDelay", tickDelay} };
            m_modifyClipMessage.setJsonBytes(j);
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_modifyClipMessage);
        }
    );

    connect(m_timeDelay,
        &QLineEdit::editingFinished,
        this,
        [this]() {

            // Set tick delay in animation clip
            Real_t timeDelay = m_timeDelay->text().toDouble();

            json j = { {"timeDelay", timeDelay} };
            m_modifyClipMessage.setJsonBytes(j);
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_modifyClipMessage);
        }
    );
}

void AnimClipWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(5);

    m_mainLayout->addLayout(LabeledLayout(QStringLiteral("Animation Clip: "), m_nameEdit));

    QBoxLayout* speedLayout = LabeledLayout(QStringLiteral("Speed Factor: "), m_speedFactor);
    speedLayout->addWidget(new QLabel("Duration: "));
    speedLayout->addWidget(m_playbackDuration);

    QBoxLayout* offsetLayout = LabeledLayout(QStringLiteral("Tick Delay: "), m_tickDelay);
    offsetLayout->addWidget(new QLabel("Time Delay: "));
    offsetLayout->addWidget(m_timeDelay);

    m_mainLayout->addLayout(speedLayout);
    m_mainLayout->addLayout(offsetLayout);
}

void AnimClipWidget::requestAnimationInfo()
{
    m_requestDataMessage.setClipId(m_clipJson["id"].get<Uuid>());
    m_requestDataMessage.setStateMachineId(m_stateMachineId);
    m_requestDataMessage.setStateId(m_stateId);

    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_requestDataMessage);
}

QString AnimClipWidget::getClipName() const
{
    return m_clipJson["name"].get_ref<const std::string&>().c_str();
}

GString AnimClipWidget::getAnimationName() const
{
    return m_clipJson["animation"].get_ref<const std::string&>().c_str();
}

Float64_t AnimClipWidget::getSpeedFactor() const
{
    return m_clipJson["settings"]["speedFactor"].get<Float64_t>();
}

Float64_t AnimClipWidget::getDuration() const
{
    return m_animationDurationSec / getSpeedFactor();
}

Float64_t AnimClipWidget::getTickDelay() const
{
    return m_clipJson["settings"]["tickOffset"].get<Float64_t>();
}

Float64_t AnimClipWidget::getTimeDelay() const
{
    return m_clipJson["settings"]["timeOffsetSec"].get<Float64_t>();
}

} // rev
