/*!
  @file    GbNodeViewBlock.h

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

#ifndef GB_NODE_VIEW_CANVAS_H
#define GB_NODE_VIEW_CANVAS_H

#include <QGraphicsView>
#include <QtWidgets>

namespace Gb {
namespace View {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class NodeViewCanvas
/// @brief View for visualizing contents of NodeViewScene
class NodeViewCanvas : public QGraphicsView
{
public:
    NodeViewCanvas(QGraphicsScene* scene, QWidget* parent = NULL);
    virtual ~NodeViewCanvas();

    /// @brief UNUSED
    //void contextMenuEvent(QContextMenuEvent* event);

    /// @brief Draw background grid
    void drawBackground(QPainter* painter, const QRectF& rect);

protected:

    /// @brief Control zoom level
    virtual void wheelEvent(QWheelEvent* event) override;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
////////////////////////////////////////////////////////////////////////////////////////////////////
}
}

#endif