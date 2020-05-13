///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GbGraphWidget.h"

// Standard Includes
#include <cmath>

// Qt
#include <QKeyEvent>
#include <QRandomGenerator>

// Internal
#include "../../core/service/GbServiceManager.h"
#include "GbGraphEdge.h"
#include "GbGraphNode.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Implementations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GraphWidget::GraphWidget(const QString& name, QWidget *parent):
    QGraphicsView(parent),
    AbstractService(name),
    timerId(0)
{
    // Create and set scene
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex); // For scenes with moving objects
    setScene(scene);

    // Setting modes appropriate for this example with moving graphics
    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing); // improve rendering quality
    setTransformationAnchor(AnchorUnderMouse); // center view on mouse cursor (for zoom)
    scale(qreal(0.99), qreal(0.99));

    GraphNode *node1 = new GraphNode(this);
    GraphNode *node2 = new GraphNode(this);
    GraphNode *node3 = new GraphNode(this);
    GraphNode *node4 = new GraphNode(this);
    centerNode = new GraphNode(this);
    GraphNode *node6 = new GraphNode(this);
    GraphNode *node7 = new GraphNode(this);
    GraphNode *node8 = new GraphNode(this);
    GraphNode *node9 = new GraphNode(this);
    scene->addItem(node1);
    scene->addItem(node2);
    scene->addItem(node3);
    scene->addItem(node4);
    scene->addItem(centerNode);
    scene->addItem(node6);
    scene->addItem(node7);
    scene->addItem(node8);
    scene->addItem(node9);
    scene->addItem(new GraphEdge(node1, node2));
    scene->addItem(new GraphEdge(node2, node3));
    scene->addItem(new GraphEdge(node2, centerNode));
    scene->addItem(new GraphEdge(node3, node6));
    scene->addItem(new GraphEdge(node4, node1));
    scene->addItem(new GraphEdge(node4, centerNode));
    scene->addItem(new GraphEdge(centerNode, node6));
    scene->addItem(new GraphEdge(centerNode, node8));
    scene->addItem(new GraphEdge(node6, node9));
    scene->addItem(new GraphEdge(node7, node4));
    scene->addItem(new GraphEdge(node8, node7));
    scene->addItem(new GraphEdge(node9, node8));

    node1->setPos(0, 0);
    node2->setPos(50, 0);
    node3->setPos(100, 0);
    node4->setPos(0, 50);
    centerNode->setPos(50, 50);
    node6->setPos(100, 50);
    node7->setPos(0, 100);
    node8->setPos(50, 100);
    node9->setPos(100, 100);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GraphWidget::itemMoved()
{
    if (!timerId)
        timerId = startTimer(1000 / 25);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GraphWidget::resizeEvent(QResizeEvent *event)
{
    scene()->setSceneRect(0, 0, width(), height());
    QGraphicsView::resizeEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GraphWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
        centerNode->moveBy(0, -20);
        break;
    case Qt::Key_Down:
        centerNode->moveBy(0, 20);
        break;
    case Qt::Key_Left:
        centerNode->moveBy(-20, 0);
        break;
    case Qt::Key_Right:
        centerNode->moveBy(20, 0);
        break;
    case Qt::Key_Plus:
        zoomIn();
        break;
    case Qt::Key_Minus:
        zoomOut();
        break;
    case Qt::Key_Space:
    case Qt::Key_Enter:
        shuffle();
        break;
    default:
        // Handle all other key presses
        QGraphicsView::keyPressEvent(event);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GraphWidget::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    QList<GraphNode *> nodes;
    const QList<QGraphicsItem *> items = scene()->items();
    for (QGraphicsItem *item : items) {
        if (GraphNode *node = qgraphicsitem_cast<GraphNode *>(item))
            nodes << node;
    }

    for (GraphNode *node : qAsConst(nodes))
        node->calculateForces();

    bool itemsMoved = false;
    for (GraphNode *node : qAsConst(nodes)) {
        if (node->advancePosition())
            itemsMoved = true;
    }

    // Stop timer if items haven't moved
    if (!itemsMoved) {
        killTimer(timerId);
        timerId = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if QT_CONFIG(wheelevent)
void GraphWidget::wheelEvent(QWheelEvent *event)
{
    scaleView(pow((double)2, event->delta() / 240.0));
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GraphWidget::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);

    QRectF sceneRect = this->sceneRect();

    // Fill
    QLinearGradient gradient(sceneRect.topLeft(), sceneRect.bottomRight());
    gradient.setColorAt(0, Qt::white);
    gradient.setColorAt(1, Qt::lightGray);
    painter->fillRect(rect.intersected(sceneRect), gradient);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(sceneRect);

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GraphWidget::scaleView(qreal scaleFactor)
{
    qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.2 || factor > 10)
        return;

    scale(scaleFactor, scaleFactor);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GraphWidget::shuffle()
{
    const QList<QGraphicsItem *> items = scene()->items();
    for (QGraphicsItem *item : items) {
        if (qgraphicsitem_cast<GraphNode *>(item))
            item->setPos(-150 + QRandomGenerator::global()->bounded(300), -150 + QRandomGenerator::global()->bounded(300));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GraphWidget::zoomIn()
{
    scaleView(qreal(1.2));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GraphWidget::zoomOut()
{
    scaleView(1 / qreal(1.2));
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // view
} // Gb
