#ifndef GB_TIMELINE_INDICATOR_H
#define GB_TIMELINE_INDICATOR_H

///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
#include <QGraphicsItem>
#include <QBrush>
#include <QPen>
#include <QPainter>
#include <QLine>
#include <QPolygonF>
#include <QVector>
#include <QPointF>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QObject>

namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

class TimelineIndicator: public QGraphicsItem {
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    TimelineIndicator(float height);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    QSizeF calculateSize() const;
    void setHeight(float height);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name QGraphicsItem overrides
    /// @{

    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /// @}

protected:
    //---------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    /// @}
    
    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Indicator line
    QLine m_line;   

    /// @brief Indicator head polygon
    QPolygonF m_headPolygon;

    static QVector<QPointF> s_points;

    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}


#endif // INDICATOR_H
