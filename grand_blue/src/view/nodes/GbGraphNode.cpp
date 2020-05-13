#include "GbGraphNode.h"
#include "GbGraphEdge.h"
#include "GbGraphWidget.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>

namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GraphNode::GraphNode(GraphWidget *graphWidget) : 
    m_graph(graphWidget),
    m_forceStrength(0.05),
    m_repulsiveStrength(10.0)
{
    // Allow item to move in response to mouse dragging
    setFlag(ItemIsMovable);

    // Enable itemChange() notifications for position and transformation changes
    setFlag(ItemSendsGeometryChanges);

    // Speed up rendering performance
    setCacheMode(DeviceCoordinateCache);

    // Ensure that nodes are always on top of edges
    setZValue(-1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GraphNode::addEdge(GraphEdge *edge)
{
    m_edges.push_back(edge);
    edge->adjust();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GraphNode::calculateForces()
{
    // Return if not in the scene
    if (!scene()) return;

    // Return if item is selected by the mouse
    if (scene()->mouseGrabberItem() == this) {
        m_newPos = pos();
        return;
    }

    // Sum up all forces pushing this item away
    qreal xvel = 0;
    qreal yvel = 0;
    const QList<QGraphicsItem *> items = scene()->items();
    for (QGraphicsItem *item : items) {
        // Iterate through all items in the scene
        GraphNode *node = qgraphicsitem_cast<GraphNode*>(item);
        if (!node)
            // Skip if item is not a node
            continue;

        // Create a temporary vector from this node to "node" in local coordinates
        QPointF vec = mapToItem(node, 0, 0);
        qreal dx = vec.x();
        qreal dy = vec.y();
        double l = 2.0 * (dx * dx + dy * dy);
        if (l > 0) {
            // Sum up such that force degrades with greater distance
            xvel += (dx * 150.0) / l;
            yvel += (dy * 150.0) / l;
        }
    }

    // Now subtract all forces pulling items together
    double weight = (m_edges.size() + 1) * m_repulsiveStrength;
    for (const GraphEdge *edge : m_edges) {
        QPointF vec;
        if (edge->sourceNode() == this)
            vec = mapToItem(edge->destNode(), 0, 0);
        else
            vec = mapToItem(edge->sourceNode(), 0, 0);
        xvel -= vec.x() / weight;
        yvel -= vec.y() / weight;
    }

    // Scale strength of forces
    xvel *= m_forceStrength;
    yvel *= m_forceStrength;

    // Force sum of forces to be zero when they are less than 0.1 (to stabilize)
    if (abs(xvel) < 0.1 && abs(yvel) < 0.1)
        xvel = yvel = 0;

    // Determine nodes new position by adding force to current position
    QRectF sceneRect = scene()->sceneRect();
    m_newPos = pos() + QPointF(xvel, yvel);

    // Ensure that new position is within scene boundaries
    m_newPos.setX(qMin(qMax(m_newPos.x(), sceneRect.left() + 10), sceneRect.right() - 10));
    m_newPos.setY(qMin(qMax(m_newPos.y(), sceneRect.top() + 10), sceneRect.bottom() - 10));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool GraphNode::advancePosition()
{
    if (m_newPos == pos())
        // Position unchanged
        return false;

    setPos(m_newPos);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QRectF GraphNode::boundingRect() const
{
    // 20 x 20 rectangle

    // Adjust for outline stroke
    qreal adjust = 2;

    // Adjust 3 units down and right for drop shadow
    return QRectF(-10 - adjust, -10 - adjust, 23 + adjust, 23 + adjust);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QPainterPath GraphNode::shape() const
{
    QPainterPath path;
    path.addEllipse(-10, -10, 20, 20);
    return path;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GraphNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    // Draw shadow
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::darkGray);
    painter->drawEllipse(-7, -7, 20, 20);

    // Create gradient for rendering ellipse
    QRadialGradient gradient(-3, -3, 10);
    if (option->state & QStyle::State_Sunken) {
        // Check if state is sunken using bitwise "AND"
        gradient.setCenter(3, 3);
        gradient.setFocalPoint(3, 3);
        gradient.setColorAt(1, QColor(Qt::yellow).lighter(120));
        gradient.setColorAt(0, QColor(Qt::darkYellow).lighter(120));
    }
    else {
        gradient.setColorAt(0, Qt::yellow);
        gradient.setColorAt(1, Qt::darkYellow);
    }

    // Set painter settings and draw ellipse
    painter->setBrush(gradient);
    painter->setPen(QPen(Qt::black, 0));
    painter->drawEllipse(-10, -10, 20, 20);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QVariant GraphNode::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemPositionHasChanged:
        for (GraphEdge *edge : m_edges)
            edge->adjust();
        m_graph->itemMoved();
        break;
    default:
        break;
    };

    return QGraphicsItem::itemChange(change, value);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GraphNode::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    QGraphicsItem::mousePressEvent(event);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GraphNode::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    QGraphicsItem::mouseReleaseEvent(event);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}
}