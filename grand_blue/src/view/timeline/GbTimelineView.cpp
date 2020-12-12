#include "GbTimelineView.h"
#include "GbTimelineScene.h"

#include <QApplication>
#include <QPalette>

namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
// TimelineView
///////////////////////////////////////////////////////////////////////////////////////////////////
TimelineView::TimelineView(TimelineScene* scene):
    QGraphicsView(scene)
{
    setInteractive(true);
    setMouseTracking(true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
TimelineView::~TimelineView()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TimelineView::mousePressEvent(QMouseEvent *event)
{
    //if(!itemAt(event->pos())){
    //    std::cout << "Press"<<std::endl;
    //}
    //else{
    //    auto item = scene()->itemAt(event->pos(),QTransform());
    //}
    QGraphicsView::mousePressEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TimelineView::mouseMoveEvent(QMouseEvent *event)
{
    auto item = this->scene()->itemAt(event->pos(), QTransform());

    QGraphicsView::mouseMoveEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TimelineView::mouseReleaseEvent(QMouseEvent *event){
    QGraphicsView::mouseReleaseEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TimelineView::keyPressEvent(QKeyEvent *event)
{
    QGraphicsView::keyPressEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TimelineView::drawBackground(QPainter *painter, const QRectF &rect){
    TimelineScene* ts = static_cast<TimelineScene*>(scene());

    // The background color
    QPalette palette = QApplication::palette();
    painter->fillRect(rect, palette.base());

    //const qint32 gridInterval = 50;
    painter->setWorldMatrixEnabled(true);

    // Vertical line style
    QColor lineColor = palette.alternateBase().color();
    QPen linePen(lineColor, 1, Qt::DotLine, Qt::FlatCap, Qt::RoundJoin);
    linePen.setCosmetic(true); // Performance optimization
    painter->setPen(linePen);

    // Draw Vertical lines
    const qreal left = ts->m_rowLabelWidth;
    const qreal top = ts->m_tickBarOffset;
    const qreal tickWidth = ts->m_majorTickWidth;
    std::vector<QLineF> linesX;
    const qreal vertLineTop = top - ts->m_majorTickHeight;
    for (qreal x = left; x < rect.right(); x += tickWidth){
        linesX.push_back(QLineF(x, vertLineTop, x, rect.bottom()));
    }
    painter->drawLines(linesX.data() + 1, linesX.size());

    // Draw border line between row headers and track
    QLineF borderLine(left, top, left, rect.bottom());
    linePen.setStyle(Qt::SolidLine);
    linePen.setWidth(1.5);
    painter->drawLines({ borderLine });

    // Horizontal line style
    linePen.setStyle(Qt::SolidLine);
    linePen.setColor(palette.base().color().lighter(80)); // Make a bit darker
    linePen.setWidth(ts->m_verticalTrackMargin); // FIXME: Set with screen-space margin thickness
    painter->setPen(linePen);

    // Draw horizontal lines
    const qreal rowHeight = ts->m_verticalTrackHeight + ts->m_verticalTrackMargin;
    std::vector<QLineF> linesY;
    for (qreal y = top; y < rect.bottom(); y += rowHeight) {
        linesY.push_back(QLineF(rect.left(), y, rect.right(), y));
    }
    painter->drawLines(linesY.data(), linesY.size());
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void TimelineView::wheelEvent(QWheelEvent *event)
{
    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    double scaleFactor = 1.05;

    if (event->delta() > 0) {
        this->scale(scaleFactor, scaleFactor);
    }
    else {
        this->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }

    //if (event->delta() > 0) {
    //    QRectF rect = sceneRect();
    //    rect.setWidth(rect.width() * 1.1);
    //    setSceneRect(rect);
    //}
    //else {
    //    QRectF rect = sceneRect();
    //    rect.setWidth(rect.width() * 1.0 / 1.1);
    //    setSceneRect(rect);
    //}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}