#ifndef GB_GRAPH_NODE_H
#define GB_GRAPH_NODE_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Qt
#include <QGraphicsItem>
#include <QList>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QT_BEGIN_NAMESPACE
class QGraphicsSceneMouseEvent;
QT_END_NAMESPACE

namespace Gb {
namespace View {

class GraphEdge;
class GraphWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @class GraphNode
/// @brief A visual representation of a node for the GraphWidget
/// @note Would need to inherit from QGraphicsObject to use signals, slots, properties, etc.
class GraphNode : public QGraphicsItem{
public:
    //--------------------------------------------------------------------------------------------
    /// @name Statics
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructors
    /// @{
    GraphNode(GraphWidget *graphWidget);
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Qt Overrides
    /// @{
    enum { Type = UserType + 1 };
    int type() const override { return Type; }

    /// @brief Overrides shape to ensure that hit area is elliptical
    /// @details Default bounding box is a rectangle, and hit area is identical to bounding by default
    QPainterPath shape() const override;

    /// @brief Required override, defines outter bounds of item as a rectangle
    /// @details All painting must be restricted inside this items bounding rect
    /// @note QGraphicsView uses this to determine whether an item needs redrawing or not
    QRectF boundingRect() const override;

    /// @brief Renders the item in local coordinates
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    std::vector<GraphEdge *> edges() const { return m_edges; }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief calculate the forces that push and pull on this node and its neighbors
    void calculateForces();

    /// @brief Called whenver scene's state advances by one steep
    /// @details Called from GraphWidget::timerEvent()
    bool advancePosition();

    /// @brief Adds input edge to list of attached edges and adjusts end points
    void addEdge(GraphEdge *edge);

    /// @}


protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief React to state (position) changes
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Qt Overrides
    /// @{

    /// @brief Update on mouse events
    /// @note Because we have set the ItemIsMovable flag, we don't need to implement the logic 
    /// that moves the node according to mouse input; this is already provided for us. 
    /// We still need to reimplement the mouse press and release objects, though,
    /// to update the nodes' visual appearance (i.e., sunken or raised).
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    std::vector<GraphEdge *> m_edges;
    QPointF m_newPos;
    GraphWidget *m_graph;


    /// @brief Weight of force between nodes
    double m_forceStrength;

    /// @brief Weight of repulsive force between nodes
    double m_repulsiveStrength;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}
}

#endif // NODE_H