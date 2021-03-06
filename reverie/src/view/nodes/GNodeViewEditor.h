/*!
  @file    GbNodeViewEditor.h

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

#ifndef GB_NODE_VIEW_EDITOR_H
#define GB_NODE_VIEW_EDITOR_H

#include <QObject>

class QPointF;
class QGraphicsScene;
class QGraphicsItem;
class QGraphicsSceneMouseEvent;

namespace rev {
namespace View {

class NodeViewPort;
class NodeViewBlock;
class NodeViewConnection;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class NodeViewEditor
/// @brief Class for manipulating nodes in the node view
class NodeViewEditor : public QObject
{
    Q_OBJECT

public:
    explicit NodeViewEditor(size_t numConnectionsPerPort = 1, QObject* parent = NULL);

    /// @brief Install as an event filter of the specified scene
    void install(QGraphicsScene* scene);

    /// @brief The event filter function called when the scene receives events
    /// @returns True if the event should be filtered, i.e., stopped; otherwise must return false
    bool eventFilter(QObject* object, QEvent* event);

    /// @brief Save the entire scene to a data stream
    void save(QDataStream& stream);
    void load(QDataStream& stream);

    /// @brief Return the graphics item at the given point
    QGraphicsItem* itemAt(const QPointF& point);
    void showBlockMenu(const QPoint& point, NodeViewBlock* block);
    void showConnectionMenu(const QPoint& point, NodeViewConnection* connection);

signals:

    void mouseEventFiltered(QGraphicsSceneMouseEvent* mouseEvent);

    void connectedPorts(NodeViewConnection* connection);

private:

private:
    QGraphicsScene* m_scene;
    NodeViewConnection* m_connection;

    /// @brief Number of allowed connections per port
    size_t m_numConnectionsPerPort;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
////////////////////////////////////////////////////////////////////////////////////////////////////
}
}

#endif