#include "GTimeline.h"
#include "GTimelineIndicator.h"
#include "GTimelineTrack.h"
#include "GTimelineMarker.h"
#include "GTimelineView.h"
#include "GTimelineScene.h"

#include <QApplication>
#include <QPalette>

namespace rev {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Timeline
///////////////////////////////////////////////////////////////////////////////////////////////////
Timeline::Timeline(QObject *_parent):
    QObject(_parent)
{
    m_scene = new TimelineScene();
    m_view = new TimelineView(m_scene);

    //int xOffset =100;
    //for(int i =0; i < 10; i++){
    //    QGraphicsItem *item = m_scene->addText(QString::number(i));
    //    item->setPos(i*xOffset,-40);
    //}

    // Create indicator, default depth value is 0
    m_indicator = new TimelineIndicator(getTrackRegionHeight() + 20); // 20 for top of indicator height
    m_scene->addItem(m_indicator);
    m_indicator->setZValue(101);
    m_indicator->setPos(QPointF(m_scene->m_rowLabelWidth, m_scene->m_tickBarOffset));
    
    // Center on indicator position
    //QPointF pos = m_indicator->scenePos();
    //pos.setX(pos.x() + 500);
    //m_view->centerOn(pos);

    refreshTickLabels();

    // Set up connections
    connect(m_scene, &TimelineScene::releasedTimelineItem, this,
        [this](QGraphicsItem* item, TimelineItemType type) {
        // Refresh tick labels when items are moved beyond bounds of current labels
        Q_UNUSED(type);
        qreal rightBound = item->scenePos().x() + item->boundingRect().width();
        if (rightBound > m_majorTickLabels.back()->scenePos().x()) {
            refreshTickLabels();
        }
    });

    connect(m_scene, &TimelineScene::resizeRowHeaders, this, &Timeline::onResizeRowHeaders);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
Timeline::~Timeline()
{
    delete m_scene;
    delete m_view;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void Timeline::setTitle(const QString & title)
{
    if (m_title) {
        scene()->removeItem(m_title);
        delete m_title;
    }

    // Create row header and add to internal list/scene
    m_title = new TimelineTrack();
    m_title->setIsRowHeader(true);
    scene()->addItem(m_title);

    // Set attributes
    m_title->setLength(m_scene->m_rowLabelWidth);
    m_title->setText(title, true);

    // Set color
    QPalette palette = QApplication::palette();
    QColor color = palette.base().color();
    m_title->setColor(color);

    // Set position
    qreal verticalOffset = m_scene->m_tickBarOffset;
    qreal rowHeight = m_scene->m_verticalTrackHeight + m_scene->m_verticalTrackMargin;
    m_title->setPos(QPointF(0, -rowHeight + verticalOffset));
}
///////////////////////////////////////////////////////////////////////////////////////////////////
size_t Timeline::addRow(const QString & name)
{
    // Create row header and add to internal list/scene
    size_t rowIndex = m_rowHeaders.size();
    TimelineTrack* track = new TimelineTrack();
    track->setIsRowHeader(true);
    scene()->addItem(track);
    m_rowHeaders.push_back(track);

    // Set attributes
    track->setLength(m_scene->m_rowLabelWidth);
    track->setText(name);

    // Alternate colors
    QPalette palette = QApplication::palette();
    QColor color = rowIndex % 2 ? palette.base().color() : palette.alternateBase().color();
    track->setColor(color);

    // Set position
    qreal verticalOffset = m_scene->m_tickBarOffset;
    qreal rowHeight = m_scene->m_verticalTrackHeight + m_scene->m_verticalTrackMargin;
    track->setPos(QPointF(0, rowHeight * rowIndex + verticalOffset));

    // Update indicator size
    qreal height = getTrackRegionHeight();
    m_indicator->setHeight(height + 20); // 20 for top of indicator height
    
    return rowIndex;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void Timeline::setTickIncrement(qreal increment)
{
    m_scene->m_majorTickIncrement = increment;
    refreshTickLabels();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
TimelineTrack * Timeline::addTrack(size_t rowIndex, float tickValue, bool refreshTicks)
{
    if (rowIndex >= m_rowHeaders.size()) {
        throw("Error, timeline does not have enough rows");
    }

    TimelineTrack* track = new TimelineTrack();
    scene()->addItem(track);
    track->setLength(200);
    track->setText("Test");
    
    qreal verticalOffset = m_scene->m_tickBarOffset;
    qreal rowHeight = m_scene->m_verticalTrackHeight + m_scene->m_verticalTrackMargin;
    track->setPos(QPointF(m_scene->tickValueToX(tickValue), rowHeight * rowIndex + verticalOffset));

    if (refreshTicks) {
        refreshTickLabels();
    }
    return track;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
TimelineMarker * Timeline::addMarker(size_t rowIndex, float tickValue, bool refreshTicks)
{
    if (rowIndex >= m_rowHeaders.size()) {
        throw("Error, timeline does not have enough rows");
    }

    TimelineMarker* marker = new TimelineMarker();
    scene()->addItem(marker);

    qreal verticalOffset = m_scene->m_tickBarOffset;
    qreal rowHeight = m_scene->m_verticalTrackHeight + m_scene->m_verticalTrackMargin;
    marker->setPos(QPointF(m_scene->tickValueToX(tickValue), rowHeight * rowIndex + verticalOffset));

    if (refreshTicks) {
        refreshTickLabels();
    }
    return marker;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void Timeline::onResizeRowHeaders(qreal size)
{
    m_scene->m_rowLabelWidth = size;
    for (TimelineTrack* label : m_rowHeaders) {
        label->setLength(size);
    }
    m_title->setLength(size);

    m_indicator->setPos(std::max(m_indicator->scenePos().x(), m_scene->m_rowLabelWidth), m_indicator->scenePos().y());
    refreshTickLabels();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void Timeline::refreshTickLabels()
{
    QPalette palette = QApplication::palette();

    for (QGraphicsItem* label : m_majorTickLabels) {
        m_scene->removeItem(label);
        delete label;
    }
    m_majorTickLabels.clear();

    // Set right-bound to be at least 20 ticks in length
    const qreal left = m_scene->m_rowLabelWidth;
    const qreal tickWidth = m_scene->m_majorTickWidth;
    qreal right = m_view->sceneRect().right();
    right = std::max(right, left + tickWidth * 20);

    // Create and draw labels
    QFont font = scene()->font();
    QFontMetricsF fontMetrics(font);
    size_t count = 0;
    const qreal increment = m_scene->m_majorTickIncrement;
    const qreal top = m_scene->m_tickBarOffset;
    const qreal vertLineTop = top - m_scene->m_majorTickHeight;
    for (qreal x = left; x < right; x += tickWidth) {
        QString text = QString::number(count * increment);
        QRectF rect = fontMetrics.boundingRect(text);
        int fontHeight = rect.height();
        int fontWidth = rect.width();
        int textHeight = vertLineTop - fontHeight - 5; // Subtract 5 for some margin
        QGraphicsTextItem *item = m_scene->addText(text);
        item->setDefaultTextColor(palette.text().color());
        item->setPos(x, textHeight);

        m_majorTickLabels.push_back(item);

        count++;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
qreal Timeline::getTrackRegionHeight()
{
    return qreal(m_rowHeaders.size()) * (m_scene->m_verticalTrackHeight + m_scene->m_verticalTrackMargin);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}