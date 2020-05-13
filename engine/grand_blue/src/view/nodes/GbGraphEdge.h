#ifndef GB_GRAPH_EDGE_H
#define GB_GRAPH_EDGE_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// qt
#include <QGraphicsItem>

namespace Gb {
namespace View {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class GraphNode;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class GraphEdge : public QGraphicsItem
{
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructor
    /// @{
    GraphEdge(GraphNode *sourceNode, GraphNode *destNode);
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
    GraphNode *sourceNode() const { return m_source; }
    GraphNode *destNode() const { return m_dest; }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Qt Overrides
    /// @{

    enum { Type = UserType + 2 };
    int type() const override { return Type; }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    void adjust();

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected Qt overrides
    /// @{
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /// @}
private:
    //--------------------------------------------------------------------------------------------
    /// @name Private members
    /// @{
    GraphNode *m_source, *m_dest;

    QPointF m_sourcePoint;
    QPointF m_destPoint;
    qreal m_arrowSize;

    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
} // View
} // Gb

#endif // EDGE_H