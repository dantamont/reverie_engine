#include "GbTimelineIndicator.h"
#include "GbTimelineScene.h"

#include <QApplication>
#include <QPalette>

namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
// TimelineIndicator
///////////////////////////////////////////////////////////////////////////////////////////////////

TimelineIndicator::TimelineIndicator(float height):
    QGraphicsItem ()
{
    setCacheMode(DeviceCoordinateCache);

    setHeight(height);

    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsScenePositionChanges);
    setAcceptHoverEvents(true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
QSizeF TimelineIndicator::calculateSize() const
{
    float minX = s_points[0].x();
    float minY = s_points[0].y();
    float maxX = s_points[0].x();
    float maxY = s_points[0].y();
    for(const QPointF& point : s_points){
        if (point.x() < minX){
            minX = point.x();
        }
        if (point.y() < minY){
            minY = point.y();
        }
        if (point.x() > maxX){
            maxX = point.x();
        }
        if (point.y() > maxY){
            maxY = point.y();
        }
    }
    return QSizeF(maxX - minX, m_line.p2().y() - m_line.p1().y());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TimelineIndicator::setHeight(float height)
{
    m_line.setP2(QPoint(0, height));
}
///////////////////////////////////////////////////////////////////////////////////////////////////
QRectF TimelineIndicator::boundingRect() const
{
    QSizeF size = calculateSize();
    return QRectF(-10, 0, size.width(), size.height());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TimelineIndicator::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    QPalette palette = QApplication::palette();
    if (isUnderMouse()) {
        painter->setPen(QPen(palette.highlight().color(), 1.5));
        painter->setBrush(palette.light().color().lighter(120));
    }
    else {
        painter->setPen(QPen(palette.text().color(), 1.5));
        painter->setBrush(palette.light().color());
    }

    // Draw vertical line
    painter->drawLine(m_line);

    // Draw top polygon
    painter->drawPolygon(s_points);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TimelineIndicator::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    setSelected(true);
    QGraphicsItem::mousePressEvent(event);
    update();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TimelineIndicator::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    //QPointF pos = event->scenePos();
    //if(isSelected()){
    //    this->setPos(pos.x(),y());
    //}
    QGraphicsItem::mouseMoveEvent(event);
    update();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TimelineIndicator::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    setSelected(false);
    QGraphicsItem::mouseReleaseEvent(event);
    update();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TimelineIndicator::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TimelineIndicator::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TimelineIndicator::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant TimelineIndicator::itemChange(GraphicsItemChange change, const QVariant &value)
{
    TimelineScene* ts = static_cast<TimelineScene*>(scene());

    if (change == ItemPositionChange && scene()) {
        // Return value is the new position, which will move the timeline indicator
        qreal leftBound = ts->m_rowLabelWidth;
        qreal minHeight = ts->m_tickBarOffset;

        QPointF newPos = value.toPointF();
        newPos.setY(y());
        if(newPos.x() < leftBound){
            newPos.setX(leftBound);
        }

        // Just keep y-position static
        newPos.setY(minHeight - 20);
        return newPos;
    }
    return QGraphicsItem::itemChange(change, value);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
QVector<QPointF> TimelineIndicator::s_points = { 
    QPointF(-10,  0),
    QPointF(-10,  10), 
    QPointF( 0,   20),
    QPointF( 10,   10),
    QPointF( 10,  0),
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}