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

#ifndef GB_NODE_VIEW_SCENE_H
#define GB_NODE_VIEW_SCENE_H

#include <QGraphicsScene>
#include <QtWidgets>

namespace Gb {
namespace View {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class NodeViewScene
class NodeViewScene : public QGraphicsScene
{
public:
    NodeViewScene(QWidget* parent = nullptr);
    virtual ~NodeViewScene();

protected:

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
////////////////////////////////////////////////////////////////////////////////////////////////////
}
}

#endif