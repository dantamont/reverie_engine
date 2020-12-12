#ifndef GB_TIMELINE_H
#define GB_TIMELINE_H

///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTransform>
#include <QDebug>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QPen>
#include <QBrush>
#include <QGraphicsItem>
#include <QPoint>

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {
namespace View {

class TimelineTrack;
class TimelineIndicator;
class TimelineScene;
class TimelineView;
class TimelineMarker;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

class Timeline : public QObject {
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructor/Destructor
    /// @{

    Timeline(QObject *parent = nullptr);
    ~Timeline();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    TimelineScene* scene() const { return m_scene; }
    TimelineView* view() const { return m_view; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    void refreshTickLabels();

    /// @brief Set title for the timeline
    void setTitle(const QString& title);

    /// @brief Add a row to the timeline with the specified name
    size_t addRow(const QString& name);

    /// @brief Set major tick increment
    void setTickIncrement(qreal increment);

    /// @brief Add track in the given row
    TimelineTrack* addTrack(size_t rowIndex, float tickValue, bool refreshTickLabels = true);
    TimelineMarker* addMarker(size_t rowIndex, float tickValue, bool refreshTickLabels = true);

    /// @}

private:
    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    void onResizeRowHeaders(qreal size);

    /// @brief Get height of track region
    qreal getTrackRegionHeight();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    TimelineTrack* m_title = nullptr;

    /// @brief Row headers (special case of timeline track)
    std::vector<TimelineTrack*> m_rowHeaders;

    /// @brief Major tick labels
    std::vector<QGraphicsTextItem*> m_majorTickLabels;

    TimelineScene* m_scene;
    TimelineView* m_view;
    TimelineIndicator* m_indicator;

    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}


#endif // TIMELINE_H
