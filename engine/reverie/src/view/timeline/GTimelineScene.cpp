#include "GTimelineScene.h"

namespace rev {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Timeline Scene
///////////////////////////////////////////////////////////////////////////////////////////////////
TimelineScene::TimelineScene(QWidget* parent) :
    QGraphicsScene(nullptr),
    m_verticalTrackMargin(3),
    m_verticalTrackHeight(25),
    m_rowLabelWidth(150),
    m_majorTickIncrement(5),
    m_majorTickWidth(50),
    m_majorTickHeight(20),
    m_minorTickHeight(5),
    m_numMinorTicks(4),
    m_tickBarOffset(50),
    m_canSwitchRows(false)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
qreal TimelineScene::tickValueToX(float tickValue)
{
    float tickNumber = tickValue / m_majorTickIncrement;
    float tickDistance = m_majorTickWidth * tickNumber;
    return m_rowLabelWidth + tickDistance;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
qreal TimelineScene::verticalTrackMargin() const
{
    return m_verticalTrackMargin;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TimelineScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit sendSceneMouseEventSignal(event);
    QGraphicsScene::mousePressEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TimelineScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    emit sendSceneMouseEventSignal(event);
    QGraphicsScene::mouseReleaseEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TimelineScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    emit sendSceneMouseEventSignal(event);
    QGraphicsScene::mouseMoveEvent(event);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}