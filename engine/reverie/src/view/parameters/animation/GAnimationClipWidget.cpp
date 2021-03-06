#include "GAnimationClipWidget.h"
#include "../../style/GFontIcon.h"

#include "../../../core/animation/GAnimation.h"
#include "../../../core/animation/GAnimationState.h"
#include "../../../core/animation/GAnimationController.h"
#include "../../../core/animation/GAnimStateMachine.h"
#include "../../tree/animation/GAnimationTreeWidget.h"

namespace rev {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AnimClipWidget
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AnimClipWidget::AnimClipWidget(AnimationClip* clip, CoreEngine* core, QWidget* parent) :
    ParameterWidget(core, parent),
    m_clip(clip)
{
    initialize();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AnimClipWidget::~AnimClipWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AnimClipWidget::update()
{
    ParameterWidget::update();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AnimClipWidget::initializeWidgets()
{
    m_nameEdit = new QLineEdit(QString(m_clip->getName().c_str()));

    m_speedFactor = new QLineEdit(QString::number(m_clip->settings().m_speedFactor));
    m_speedFactor->setMaximumWidth(75);
    m_speedFactor->setValidator(new QDoubleValidator(0, 1e6, 3));
    m_speedFactor->setToolTip("Scaling factor for controlling playback speed");

    real_g duration = getAnimation()->getTimeDuration() / m_clip->settings().m_speedFactor;
    m_playbackDuration = new QLineEdit(QString::number(duration));
    m_playbackDuration->setMaximumWidth(75);
    m_playbackDuration->setValidator(new QDoubleValidator(0, 1e6, 3));
    m_speedFactor->setToolTip("Set the duration (in seconds) of animation playback directly");

    m_tickDelay = new QLineEdit(QString::number(m_clip->settings().m_tickOffset));
    m_tickDelay->setMaximumWidth(75);
    m_tickDelay->setValidator(new QDoubleValidator(0, 1e6, 1));
    m_tickDelay->setToolTip("Number of ticks to delay playback by");

    m_timeDelay = new QLineEdit(QString::number(m_clip->settings().m_timeOffsetSec));
    m_timeDelay->setMaximumWidth(75);
    m_timeDelay->setValidator(new QDoubleValidator(0, 1e6, 1));
    m_timeDelay->setToolTip("Number of seconds to delay playback by");
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AnimClipWidget::initializeConnections()
{
    connect(m_nameEdit,
        &QLineEdit::editingFinished,
        this,
        [this]() {
        QString newName = m_nameEdit->text();
        if (newName.isEmpty()) {
            newName = m_clip->getName().c_str();
        }
        // TODO: Maybe make this undoable, but will probably never happen (11/3/2020)
        //performAction(new rev::ChangeNameCommand(newName, parentWidget->m_engine, object()));
        // Set name in scene object and in item
        m_clip->setName(newName);
    });

    connect(m_speedFactor,
        &QLineEdit::editingFinished,
        this,
        [this]() {
        
        // Set speed factor in animation clip
        real_g speedFactor = m_speedFactor->text().toDouble();
        m_clip->settings().m_speedFactor = speedFactor;

        // Set duration in widget
        real_g duration = getAnimation()->getTimeDuration() / m_clip->settings().m_speedFactor;
        m_playbackDuration->setText(QString::number(duration));
    });

    connect(m_playbackDuration,
        &QLineEdit::editingFinished,
        this,
        [this]() {

        // Back out speed factor from playback duration
        real_g playbackDuration = m_playbackDuration->text().toDouble();
        real_g speedFactor = getAnimation()->getTimeDuration() / playbackDuration;

        // Set speed factor in animation clip and widget
        m_clip->settings().m_speedFactor = speedFactor;
        m_speedFactor->setText(QString::number(speedFactor));
    });

    connect(m_tickDelay,
        &QLineEdit::editingFinished,
        this,
        [this]() {

        // Set tick delay in animation clip
        real_g tickDelay = m_tickDelay->text().toDouble();
        m_clip->settings().m_tickOffset = tickDelay;
    });

    connect(m_timeDelay,
        &QLineEdit::editingFinished,
        this,
        [this]() {

        // Set tick delay in animation clip
        real_g timeDelay = m_timeDelay->text().toDouble();
        m_clip->settings().m_timeOffsetSec = timeDelay;
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Animation* AnimClipWidget::getAnimation() const
{
    const auto& handle = m_clip->animationHandle();
    return handle->resourceAs<Animation>();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // rev