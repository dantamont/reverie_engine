/*!
  @file    GbNodeViewScene.h

  *************************************************************************************
  FILE MODIFIED by Dante Tufano
  *************************************************************************************

  Copyright (c) 2020 Dante Tufano

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  @author  Dante Tufano
  @date    November 4, 2020
*/

#include "geppetto/qt/widgets/nodes/GNodeViewScene.h"
#include <QGraphicsSceneEvent>

#include "geppetto/qt/widgets/nodes/GNodeViewCommon.h"
#include "geppetto/qt/widgets/nodes/GNodeViewBlock.h"

namespace rev {

NodeViewScene::NodeViewScene(QWidget* parent):
    QGraphicsScene(parent)
{
}

NodeViewScene::~NodeViewScene()
{
}

void NodeViewScene::mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent)
{
    // TODO: Alternatively, could have put this in the NodeViewEditor, which is an event filter
    // Perform move event for each occluded block item
    QList<QGraphicsItem*> itemsAt = items(mouseEvent->scenePos());
    size_t numItems = itemsAt.size();
    for (uint32_t i = 0; i < numItems; i++) {
        QGraphicsItem* item = itemsAt[i];
        if (item->type() == (int)NodeViewItemType::kBlock) {
            NodeViewBlock* blockItem = static_cast<NodeViewBlock*>(item);
            blockItem->onSceneMouseMove(mouseEvent);
        }
    }

    QGraphicsScene::mouseMoveEvent(mouseEvent);
}



} //rev