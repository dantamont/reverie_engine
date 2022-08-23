#ifndef GB_TIMELINE_MARKER_H
#define GB_TIMELINE_MARKER_H

///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
#include <QGraphicsItem>
#include <QBrush>
#include <QPen>
#include <QPainter>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>

namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class TimelineMarker
class TimelineMarker : public QGraphicsItem {
public:
    TimelineMarker();

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    void setColor(const QColor& color);

    void setIsRowHeader(bool isHeader) {
        m_isRowHeader = isHeader;
    }

    /// @brief Get the row index of the track, based on it's vertical position
    int getRowIndex() const;
    int getRowIndex(qreal sceneY) const;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name QGraphicsItem Overrides
    /// @{

    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /// @}

protected:
    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private Members

    /// @brief Base color of the track
    QColor m_color;

    QPointF m_oldPos, m_oldMousePos;

    bool m_isRowHeader;

    /// @{

    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    static QVector<QPointF> s_polygon;

    /// @}

};



// End namespaces        
}

#endif // TRACK_H
