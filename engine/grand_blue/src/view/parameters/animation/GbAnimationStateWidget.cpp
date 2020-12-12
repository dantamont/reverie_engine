#include "GbAnimationStateWidget.h"
#include "../../style/GbFontIcon.h"

#include "../../../core/animation/GbAnimationState.h"
#include "../../../core/animation/GbAnimationController.h"
#include "../../../core/animation/GbAnimStateMachine.h"
#include "../../tree/animation/GbAnimationTreeWidget.h"

namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AnimStateWidget
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AnimStateWidget::AnimStateWidget(BaseAnimationState* state, AnimationController* controller, QWidget* parent) :
    ParameterWidget(controller->engine(), parent),
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
    m_label = new QLabel(QStringLiteral("<b>Animation State: </b>") + m_state->getName().c_str() , this);

    switch (m_state->stateType()) {
    case AnimationStateType::kAnimation:
    {
        m_clipWidget = new AnimationTreeWidget(m_engine, m_controller, static_cast<AnimationState*>(m_state), this);
        
        break;
    }
    case AnimationStateType::kTransition:
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
    case AnimationStateType::kNull:
    default:
        throw("Unimplemented");
        break;
    }

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb