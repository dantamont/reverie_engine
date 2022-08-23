#include "geppetto/qt/widgets/playback/timeline/GTimelineTrack.h"
#include "geppetto/qt/widgets/playback/timeline/GTimelineScene.h"

#include "fortress/containers/GColor.h"
#include "fortress/math/GMath.h"

#include <QApplication>
#include <QPalette>
#include <QStyleOptionGraphicsItem>

namespace rev {

#define PEN_WIDTH 2
#define RECT_ROUNDING 2


// Timeline Track

TimelineTrack::TimelineTrack() :
    QGraphicsItem(),
    m_length(5),
    m_isRowHeader(false)
{
    setCacheMode(DeviceCoordinateCache);

    setFlags(ItemIsMovable | ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
    setAcceptHoverEvents(true);

    QPalette palette = QApplication::palette();
    m_label = new QGraphicsTextItem(this);
    m_label->setCacheMode(DeviceCoordinateCache);
    m_label->setDefaultTextColor(palette.text().color()); // GW-TODO: Expose to QStyle

    setColor(palette.midlight().color());
    m_oldPos = pos();
}

void TimelineTrack::setText(const QString & text, bool isBold, bool refreshTicks)
{
    assert(scene() && "Error, need to add to scene before setting text");

    QFont font = scene()->font();
    QFontMetricsF fontMetrics(font);
    m_fontHeight = fontMetrics.boundingRect(text).height();
    m_textWidth = fontMetrics.boundingRect(text).width();

    if (isBold) {
        QFont font(scene()->font());
        font.setBold(true);
        m_label->setFont(font);
    }
    m_label->setPlainText(text);
    m_label->setPos((boundingRect().width() / 2.0) - m_textWidth / 2.0, 0);

    // Resize rows if this is a header and text is too long
    if (m_isRowHeader) {
        if (m_textWidth > m_length) {
            m_length = m_textWidth + 30;
            TimelineScene* ts = static_cast<TimelineScene*>(scene());
            emit ts->resizeRowHeaders(m_length, refreshTicks);
        }
    }
}

void TimelineTrack::setLength(float length)
{
    m_length = length;
    m_label->setPos((boundingRect().width() / 2.0) - m_textWidth / 2.0, 0);
}

void TimelineTrack::setColor(const QColor & color)
{
    m_color = color;
}

int TimelineTrack::getRowIndex() const
{
    return getRowIndex(scenePos().y());
}

int TimelineTrack::getRowIndex(qreal sceneY) const
{
    TimelineScene* ts = static_cast<TimelineScene*>(scene());
    qreal top = ts->m_tickBarOffset;
    qreal totalRowHeight = ts->m_verticalTrackMargin + ts->m_verticalTrackHeight;
    return int((sceneY - top) / totalRowHeight);
}

QRectF TimelineTrack::boundingRect() const {
    TimelineScene* ts = static_cast<TimelineScene*>(scene());
    return QRectF(0, 0, m_length, ts->m_verticalTrackHeight);
}

void TimelineTrack::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    Q_UNUSED(widget);

    // Only paint dirty regions for increased performance
    painter->setClipRect(option->exposedRect);

    // Draw the track rect
    QPalette palette = QApplication::palette();

    // If is a row header, format accordingly
    if (m_isRowHeader) {
        // Paint row header
        if (isUnderMouse()) {
            painter->setBrush(QBrush(m_color.lighter(120)));
            painter->setPen(QPen(QColor(Qt::yellow)));
        }
        else {
            painter->setBrush(QBrush(m_color));
            painter->setPen(QPen(m_color.lighter(70)));
        }
        painter->drawRoundedRect(boundingRect(), 0, 0);
    }
    else {
        // Paint regular timeline track
        if (isUnderMouse()) {
            painter->setBrush(QBrush(m_color.lighter(120)));
            painter->setPen(QPen(palette.highlight().color()));
        }
        else {
            painter->setBrush(QBrush(m_color));
            painter->setPen(QPen(m_color.lighter(70)));
        }
        painter->drawRoundedRect(boundingRect(), RECT_ROUNDING, RECT_ROUNDING);
    }
}

void TimelineTrack::mousePressEvent(QGraphicsSceneMouseEvent *event){
    m_oldMousePos = event->scenePos();
    m_oldPos = scenePos();
    setSelected(true);
}

void TimelineTrack::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
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
                float delta = (int)(yDiff / (height + heightMargin)) * heightDiff;
                float newY = m_oldPos.y() + delta;

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

void TimelineTrack::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    setSelected(false);
    m_oldMousePos = event->scenePos();
    m_oldPos = scenePos();

    TimelineScene* ts = static_cast<TimelineScene*>(scene());
    emit ts->releasedTimelineItem(this, TimelineItemType::kTrack);
}

void TimelineTrack::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event){
    Q_UNUSED(event);
}


// End namespaces        
}