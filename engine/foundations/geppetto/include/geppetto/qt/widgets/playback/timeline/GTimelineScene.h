#ifndef GB_TIMELINE_SCENE_H
#define GB_TIMELINE_SCENE_H

///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QWidget>

namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Types of timeline items
enum class TimelineItemType {
    kTrack,
    kMarker
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class TimelineScene: public QGraphicsScene
{
    Q_OBJECT
public:
    TimelineScene(QWidget* parent = nullptr);

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Convert tick value to x position
    qreal tickValueToX(float tickValue);

    /// @brief Vertical space between each track
    qreal verticalTrackMargin() const;

    /// @brief Set the tick increment of the timeline
    void setTickIncrement(float inc) {
        m_majorTickIncrement = inc;
    }

    /// @}

signals:
    void sendSceneMouseEventSignal(QGraphicsSceneMouseEvent *event);

    void releasedTimelineItem(QGraphicsItem* item, TimelineItemType type);
    void resizeRowHeaders(qreal size, bool refreshTicks);

protected:
    friend class Timeline;
    friend class TimelineView;
    friend class TimelineTrack;
    friend class TimelineIndicator;
    friend class TimelineMarker;

    //---------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Whether or not tracks can move rows
    bool m_canSwitchRows;

    /// @brief Vertical track margin
    qreal m_verticalTrackMargin;

    /// @brief Track height
    qreal m_verticalTrackHeight;

    /// @brief Major tick numerical increment
    qreal m_majorTickIncrement;

    /// @brief Major tick width (horizontal)
    qreal m_majorTickWidth;
    qreal m_majorTickHeight;

    /// @brief Number of minor ticks
    qreal m_minorTickHeight;
    qreal m_numMinorTicks;

    /// @brief Width of row label column
    qreal m_rowLabelWidth;

    /// @brief Offset from top of scene of tick bar
    qreal m_tickBarOffset;

    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}

#endif // GRAPHSCENE_H
