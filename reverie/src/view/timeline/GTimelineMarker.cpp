#include "GTimelineMarker.h"
#include "GTimelineScene.h"

#include "../../core/containers/GColor.h"
#include "../../core/utils/GMath.h"

#include <QApplication>
#include <QPalette>
#include <QStyleOptionGraphicsItem>

namespace rev {
namespace View {

#define PEN_WIDTH 2
#define RECT_ROUNDING 1
#define MARKER_WIDTH 25

///////////////////////////////////////////////////////////////////////////////////////////////////
// Timeline Marker
///////////////////////////////////////////////////////////////////////////////////////////////////
TimelineMarker::TimelineMarker() :
    QGraphicsItem(),
    m_isRowHeader(false)
{
    setCacheMode(DeviceCoordinateCache);

    setFlags(ItemIsMovable | ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
    setAcceptHoverEvents(true);

    QPalette palette = QApplication::palette();
    setColor(palette.midlight().color());
    m_oldPos = pos();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TimelineMarker::setColor(const QColor & color)
{
    m_color = color;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
int TimelineMarker::getRowIndex() const
{
    return getRowIndex(scenePos().y());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
int TimelineMarker::getRowIndex(qreal sceneY) const
{
    TimelineScene* ts = static_cast<TimelineScene*>(scene());
    qreal top = ts->m_tickBarOffset;
    qreal totalRowHeight = ts->m_verticalTrackMargin + ts->m_verticalTrackHeight;
    return int((sceneY - top) / totalRowHeight);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
QRectF TimelineMarker::boundingRect() const {
    TimelineScene* ts = static_cast<TimelineScene*>(scene());
    return QRectF(-MARKER_WIDTH / 2.0, 0, MARKER_WIDTH, ts->m_verticalTrackHeight);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TimelineMarker::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    Q_UNUSED(widget);

    // Only paint dirty regions for increased performance
    painter->setClipRect(option->exposedRect);

    // Draw the marker rect
    QPalette palette = QApplication::palette();

    if (isUnderMouse()) {
        painter->setBrush(QBrush(m_color.lighter(120)));
        painter->setPen(QPen(palette.highlight().color()));
    }
    else {
        painter->setBrush(QBrush(m_color));
        painter->setPen(QPen(m_color.lighter(70)));
    }
    //painter->rotate(45); // Rotate coordinate system 45 degrees clockwise
    painter->drawPolygon(s_polygon);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TimelineMarker::mousePressEvent(QGraphicsSceneMouseEvent *event){
    m_oldMousePos = event->scenePos();
    m_oldPos = scenePos();
    setSelected(true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TimelineMarker::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    if (m_isRowHeader) {
        return;
    }

    if (isSelected()){
        TimelineScene* ts = static_cast<TimelineScene*>(scene());

        // Check if should be moving vertically
        qreal height = ts->m_verticalTrackHeight;
        qreal heightMargin = ts->m_verticalTrackMargin;
        qreal heightDiff = height + heightMargin;

        qreal maxVertPosition = ts->m_tickBarOffset;
        qreal minHorPosition = ts->m_rowLabelWidth;
        QPointF newPos = event->scenePos();

        if (ts->m_canSwitchRows) {
            // If tracks can switch rows, see if it did
            qreal yDiff = newPos.y() - m_oldPos.y();
            int moveCutoff = height / 2.0;
            if (abs(yDiff) > moveCutoff) {
                // If moved far enough vertically, move down a row
                float newY = m_oldPos.y() + (int)(yDiff / (height + heightMargin)) * heightDiff;

                // Verify that track hasn't moved below valid rows
                if (newY < ts->sceneRect().bottom()) {
                    setY(newY);
                }

                // Verify that track hasn't moved above valid rows
                float getY = y();
                if (getY < ts->m_tickBarOffset) {
                    setY(maxVertPosition);
                }
            }
        }

        // Move horizontally
        qreal dx = (newPos - m_oldMousePos).x();
        setX(std::max(m_oldPos.x() + dx, minHorPosition));
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TimelineMarker::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    setSelected(false);
    m_oldMousePos = event->scenePos();
    m_oldPos = scenePos();

    TimelineScene* ts = static_cast<TimelineScene*>(scene());
    emit ts->releasedTimelineItem(this, TimelineItemType::kTrack);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TimelineMarker::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event){
    Q_UNUSED(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
QVector<QPointF> TimelineMarker::s_polygon = {
    QPointF(-MARKER_WIDTH / 2.0, MARKER_WIDTH / 2.0),
    QPointF(0, MARKER_WIDTH),
    QPointF(MARKER_WIDTH / 2.0,  MARKER_WIDTH / 2.0),
    QPointF(0, 0)
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}
