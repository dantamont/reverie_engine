/*!
  @file    GbNodeViewCanvas.cpp

  *************************************************************************************
  FILE MODIFIED by Dante Tufano
  *************************************************************************************

  Copyright (c) 2014 Graham Wihlidal

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  @author  Graham Wihlidal
  @date    January 19, 2014
*/

#include "GbNodeViewCanvas.h"

namespace Gb {
namespace View {

////////////////////////////////////////////////////////////////////////////////////////////////////
NodeViewCanvas::NodeViewCanvas(QGraphicsScene* scene, QWidget* parent): 
    QGraphicsView(scene, parent)
{
    setRenderHint(QPainter::Antialiasing, true);
    //setDragMode(QGraphicsView::DragMode::ScrollHandDrag); // Scroll on drag
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NodeViewCanvas::~NodeViewCanvas()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//void NodeViewCanvas::contextMenuEvent(QContextMenuEvent* event)
//{
//    Q_UNUSED(event); // QGraphicsSceneContextMenuEvent
//}
////////////////////////////////////////////////////////////////////////////////////////////////////
void NodeViewCanvas::drawBackground(QPainter* painter, const QRectF& rect)
{
    // The background color
    QPalette palette = QApplication::palette();
    //QColor bgColor = palette.background().color();
    painter->fillRect(rect, palette.base());

    const qint32 gridInterval = 50;
    painter->setWorldMatrixEnabled(true);

    // GW-TODO: Expose this to QStyle
    QColor lineColor = palette.alternateBase().color();
    QPen linePen(lineColor, 1, Qt::DotLine, Qt::FlatCap, Qt::RoundJoin);
    linePen.setCosmetic(true); // Performance optimization
    painter->setPen(linePen);

    const qreal left = qint32(rect.left()) - (qint32(rect.left()) % gridInterval);
    const qreal top = qint32(rect.top()) - (qint32(rect.top()) % gridInterval);

    QVarLengthArray<QLineF, 100> linesX;
    for (qreal x = left; x < rect.right(); x += gridInterval)
        linesX.append(QLineF(x, rect.top(), x, rect.bottom()));

    QVarLengthArray<QLineF, 100> linesY;
    for (qreal y = top; y < rect.bottom(); y += gridInterval)
        linesY.append(QLineF(rect.left(), y, rect.right(), y));

    painter->drawLines(linesX.data(), linesX.size());
    painter->drawLines(linesY.data(), linesY.size());
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void NodeViewCanvas::wheelEvent(QWheelEvent *event)
{
    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    double scaleFactor = 1.15;

    if (event->delta() > 0) {
        this->scale(scaleFactor, scaleFactor);
    }
    else {
        this->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// End Namespaces
////////////////////////////////////////////////////////////////////////////////////////////////////
}
}