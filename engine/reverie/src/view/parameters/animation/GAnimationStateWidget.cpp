#include "GAnimationStateWidget.h"
#include "../../style/GFontIcon.h"

#include <core/animation/GAnimationState.h>
#include <core/animation/GAnimTransition.h>
#include <core/animation/GAnimationController.h>
#include <core/animation/GAnimStateMachine.h>
#include "../../tree/animation/GAnimationTreeWidget.h"
#include <core/mixins/GRenderable.h>
#include <core/rendering/view/GViewport.h>
#include <core/scene/GSceneObject.h>
#include <core/scene/GScene.h>
namespace rev {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AnimStateWidget
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AnimStateWidget::AnimStateWidget(BaseAnimationState* state, AnimationController* controller, QWidget* parent) :
    ParameterWidget(controller->sceneObject()->scene()->engine(), parent),
    m_state(state),
    m_controller(controller)
{
    initialize();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AnimStateWidget::~AnimStateWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AnimStateWidget::update()
{
    ParameterWidget::update();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AnimStateWidget::initializeWidgets()
{
    switch (m_state->stateType()) {
    case AnimationStateType::kAnimation:
    {
        m_label = new QLabel(QStringLiteral("<b>Animation State: </b>") + m_state->getName().c_str(), this);
        m_clipWidget = new AnimationTreeWidget(m_engine, m_controller, static_cast<AnimationState*>(m_state), this);
        
        break;
    }
    case AnimationStateType::kTransition:
    {
        AnimationTransition* transition = state<AnimationTransition>();
        m_label = new QLabel(QStringLiteral("<b>Animation Transition: </b>") + m_state->getName().c_str(), this);
        m_transitionInfo = new QLabel(QStringLiteral("<b>From: </b>") + QString(transition->start()->getName())
            + "<b> To: </b>" + QString(transition->end()->getName()));
        m_transitionTypes = new QComboBox();
        m_transitionTypes->addItem("Smooth", 0);
        m_transitionTypes->addItem("First State Frozen", 1);
        m_transitionTypes->setToolTip("Set the type of transition. Smooth plays both as they blend, while First Frozen pauses the first animation for the blend");
        
        m_fadeInTimeSec = new QLineEdit(QString::number(transition->settings().m_fadeInTimeSec));
        m_fadeInTimeSec->setMaximumWidth(0.75 * Viewport::ScreenDPIX());
        m_fadeInTimeSec->setValidator(new QDoubleValidator(0, 1e20, 9));
        m_fadeInTimeSec->setToolTip("The fade in time (in seconds) for the new animation");

        m_fadeInBlendWeight = new QLineEdit(QString::number(transition->settings().m_fadeInBlendWeight));
        m_fadeInBlendWeight->setMaximumWidth(0.75 * Viewport::ScreenDPIX());
        m_fadeInBlendWeight->setValidator(new QDoubleValidator(0, 1e20, 9));
        m_fadeInBlendWeight->setToolTip("The fade in blend weight for the new animation");

        m_fadeOutTimeSec = new QLineEdit(QString::number(transition->settings().m_fadeOutTimeSec));
        m_fadeOutTimeSec->setMaximumWidth(0.75 * Viewport::ScreenDPIX());
        m_fadeOutTimeSec->setValidator(new QDoubleValidator(0, 1e20, 9));
        m_fadeOutTimeSec->setToolTip("The fade out time (in seconds) for the outgoing animation");

        m_fadeOutBlendWeight = new QLineEdit(QString::number(transition->settings().m_fadeOutBlendWeight));
        m_fadeOutBlendWeight->setMaximumWidth(0.75 * Viewport::ScreenDPIX());
        m_fadeOutBlendWeight->setValidator(new QDoubleValidator(0, 1e20, 9));
        m_fadeOutBlendWeight->setToolTip("The fade out blend weight for the outgoing animation");
        break;
    }
    case AnimationStateType::kNull:
    default:
        throw("Unimplemented");
        break;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AnimStateWidget::initializeConnections()
{
    switch (m_state->stateType()) {
    case AnimationStateType::kAnimation:
    {
        break;
    }
    case AnimationStateType::kTransition:
    {
        connect(m_transitionTypes, qOverload<int>(&QComboBox::currentIndexChanged),
            this,
            [this](int index) {
            state<AnimationTransition>()->settings().m_transitionType = AnimationTransitionType(index);
        });

        connect(m_fadeInTimeSec, &QLineEdit::editingFinished,
            this,
            [this]() {
            double fadeInTime = m_fadeInTimeSec->text().toDouble();
            state<AnimationTransition>()->settings().m_fadeInTimeSec = fadeInTime;
        });

        connect(m_fadeInBlendWeight, &QLineEdit::editingFinished,
            this,
            [this]() {
            double fadeInBlendWeight = m_fadeInBlendWeight->text().toDouble();
            state<AnimationTransition>()->settings().m_fadeInBlendWeight = fadeInBlendWeight;
        });

        connect(m_fadeOutTimeSec, &QLineEdit::editingFinished,
            this,
            [this]() {
            double fadeOutTime = m_fadeOutTimeSec->text().toDouble();
            state<AnimationTransition>()->settings().m_fadeOutTimeSec = fadeOutTime;
        });

        connect(m_fadeOutBlendWeight, &QLineEdit::editingFinished,
            this,
            [this]() {
            double fadeOutBlendWeight = m_fadeOutBlendWeight->text().toDouble();
            state<AnimationTransition>()->settings().m_fadeOutBlendWeight = fadeOutBlendWeight;
        });
        break;
    }
    case AnimationStateType::kNull:
    default:
        throw("Unimplemented");
        break;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AnimStateWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(5);

    switch (m_state->stateType()) {
    case AnimationStateType::kAnimation:
    {
        m_mainLayout->addWidget(m_label);
        m_mainLayout->addWidget(m_clipWidget);        
        break;
    }
    case AnimationStateType::kTransition:
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
    case AnimationStateType::kNull:
    default:
        throw("Unimplemented");
        break;
    }

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // rev