#include "GbGraphEdge.h"
#include "GbGraphNode.h"

#include <qmath.h>
#include <QPainter>

namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GraphEdge::GraphEdge(GraphNode *sourceNode, GraphNode *destNode): 
    m_arrowSize(10)
{
    // Edge items are not considered for mouse input at all
    setAcceptedMouseButtons(0);
    m_source = sourceNode;
    m_dest = destNode;
    m_source->addEdge(this);
    m_dest->addEdge(this);
    adjust();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GraphEdge::adjust()
{
    if (!m_source || !m_dest)
        return;

    QLineF line(mapFromItem(m_source, 0, 0), mapFromItem(m_dest, 0, 0));
    qreal length = line.length();

    // Important to call this before modifying m_sourcePoint and m_destPoint, 
    // as these are used in the boundingRect() reimplementation
    prepareGeometryChange();

    if (length > qreal(20.)) {
        // Radius of node used to point to edges
        QPointF edgeOffset((line.dx() * 10) / length, (line.dy() * 10) / length);
        m_sourcePoint = line.p1() + edgeOffset;
        m_destPoint = line.p2() - edgeOffset;
    }
    else {
        // Align all points on top of each other if overlapping (rarely happens)
        m_sourcePoint = m_destPoint = line.p1();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QRectF GraphEdge::boundingRect() const
{
    if (!m_source || !m_dest)
        return QRectF();

    // Since an arrow is drawn on each side, half the arrow size and half the pen width are used
    // in all directions
    qreal penWidth = 1;
    qreal extra = (penWidth + m_arrowSize) / 2.0;

    return QRectF(m_sourcePoint, QSizeF(m_destPoint.x() - m_sourcePoint.x(),
        m_destPoint.y() - m_sourcePoint.y()))
        .normalized() // returns rect with non-negative width and height (doesn't affect actual size)
        .adjusted(-extra, -extra, extra, extra);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GraphEdge::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (!m_source || !m_dest)
        // Nothing to draw
        return;

    QLineF line(m_sourcePoint, m_destPoint);
    if (qFuzzyCompare(line.length(), qreal(0.)))
        // Return if length of edge is approximately zero
        return;

    // Draw the line itself (round pen, no joins or caps)
    painter->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawLine(line);

    // Draw the arrows as polygons with black fills
    double angle = std::atan2(-line.dy(), line.dx());

    QPointF sourceArrowP1 = m_sourcePoint + QPointF(sin(angle + M_PI / 3) * m_arrowSize,
        cos(angle + M_PI / 3) * m_arrowSize);
    QPointF sourceArrowP2 = m_sourcePoint + QPointF(sin(angle + M_PI - M_PI / 3) * m_arrowSize,
        cos(angle + M_PI - M_PI / 3) * m_arrowSize);
    QPointF destArrowP1 = m_destPoint + QPointF(sin(angle - M_PI / 3) * m_arrowSize,
        cos(angle - M_PI / 3) * m_arrowSize);
    QPointF destArrowP2 = m_destPoint + QPointF(sin(angle - M_PI + M_PI / 3) * m_arrowSize,
        cos(angle - M_PI + M_PI / 3) * m_arrowSize);

    painter->setBrush(Qt::black);
    painter->drawPolygon(QPolygonF() << line.p1() << sourceArrowP1 << sourceArrowP2);
    painter->drawPolygon(QPolygonF() << line.p2() << destArrowP1 << destArrowP2);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
} // View
} // Gb