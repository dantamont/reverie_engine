#ifndef GB_TIMELINE_TRACK_H
#define GB_TIMELINE_TRACK_H

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

namespace Gb {


namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class TimelineTrack
class TimelineTrack : public QGraphicsItem {
public:
    TimelineTrack();

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    void setText(const QString& text, bool isBold = false);
    void setLength(float length);
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

    /// @brief Length of the track in the scene
    size_t m_length;

    QPointF m_oldPos, m_oldMousePos;

    QGraphicsTextItem* m_label;
    size_t m_fontHeight = 0;
    size_t m_textWidth = 0;

    bool m_isRowHeader;

    /// @{

    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

};


///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // TRACK_H
