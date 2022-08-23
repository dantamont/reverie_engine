#include "geppetto/qt/widgets/animation/GAnimationMotionWidget.h"
#include "geppetto/qt/style/GFontIcon.h"

namespace rev {

AnimMotionWidget::AnimMotionWidget(WidgetManager* wm, json& motionJson, Uint32_t sceneObjectId, QWidget* parent):
    ParameterWidget(wm, parent),
    m_motionJson(motionJson),
    m_sceneObjectId(sceneObjectId)
{
    initialize();
}


AnimMotionWidget::~AnimMotionWidget()
{
}

void AnimMotionWidget::update()
{
    ParameterWidget::update();
}

void AnimMotionWidget::initializeWidgets()
{
    m_label = new QLabel("Motion Widget", this);
    json metadata{ {"isAnimationMotionWidget", true}, {"sceneObjectId", m_sceneObjectId} };
    m_jsonWidget = new JsonWidget(m_widgetManager, m_motionJson, metadata, this);
}

void AnimMotionWidget::initializeConnections()
{

}

void AnimMotionWidget::layoutWidgets()
{
    m_mainLayout = new QHBoxLayout();
    m_mainLayout->setSpacing(0);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(m_label);
    layout->addWidget(m_jsonWidget);

    m_mainLayout->addLayout(layout);
}


} // rev