/*!
  @file    NodeViewEditor.cpp

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

#include <QEvent>
#include <QMenu>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include "GNodeViewEditor.h"
#include "GNodeViewPort.h"
#include "GNodeViewConnection.h"
#include "GNodeViewBlock.h"

namespace rev {
namespace View {
////////////////////////////////////////////////////////////////////////////////////////////////////
// NodeViewEditor
////////////////////////////////////////////////////////////////////////////////////////////////////
NodeViewEditor::NodeViewEditor(size_t numConnectionsPerPort, QObject* parent): 
    QObject(parent),
    m_connection(nullptr),
    m_scene(nullptr),
    m_numConnectionsPerPort(numConnectionsPerPort)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void NodeViewEditor::install(QGraphicsScene* scene)
{
    Q_ASSERT(scene);
    scene->installEventFilter(this);
    m_scene = scene;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool NodeViewEditor::eventFilter(QObject* object, QEvent* event)
{
    QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);

    switch (static_cast<qint32>(event->type()))
    {
    case QEvent::GraphicsSceneMousePress:
    {
        switch (static_cast<qint32>(mouseEvent->button()))
        {
        case Qt::LeftButton:
        {
            // Return if not left clicking on any item
            QGraphicsItem* item = itemAt(mouseEvent->scenePos());
            if (!item)
                break;

            if (item->type() == (int)NodeViewItemType::kPort)
            {
                // Create a new node connection if clicking on a port
                auto* port = static_cast<NodeViewPort*>(item);
                if (port->numConnections() < m_numConnectionsPerPort) {
                    // Only create connection if under accepted number of connections
                    m_connection = new NodeViewConnection(NULL);
                    m_scene->addItem(m_connection);
                    m_connection->setStartPort(port);
                    m_connection->setStartPosition(item->scenePos());
                    m_connection->setEndPosition(mouseEvent->scenePos());
                    m_connection->updatePath();
                    return true;
                }
            }
            else if (item->type() == (int)NodeViewItemType::kBlock)
            {
                // GW-TODO: Some form of property editor callback?
            }

            break;
        }

        case Qt::RightButton:
        {
            // Return if no item selected on right-click
            QGraphicsItem* item = itemAt(mouseEvent->scenePos());
            if (!item)
                break;

            if (item->type() == (int)NodeViewItemType::kConnectionSplit)
            {
                NodeViewConnectionSplit* split = static_cast<NodeViewConnectionSplit*>(item);
                NodeViewConnection* connection = split->connection();
                connection->splits().removeAll(split);
                delete split;
                connection->updatePath();
            }

            break;
        }
        }
    }

    case QEvent::GraphicsSceneMouseMove:
    {
        // If there is a connection, move end position with mouse
        if (m_connection)
        {
            m_connection->setEndPosition(mouseEvent->scenePos());
            m_connection->updatePath();
            return true;
        }

        break;
    }

    case QEvent::GraphicsSceneMouseRelease:
    {
        // Releasing left mouse button while there is an actively selected connection
        if (m_connection && mouseEvent->button() == Qt::LeftButton)
        {
            QGraphicsItem* item = itemAt(mouseEvent->scenePos());
            if (item && item->type() == (int)NodeViewItemType::kPort)
            {
                // If releasing mouse over a port
                NodeViewPort* startPort = m_connection->startPort();
                NodeViewPort* endPort = static_cast<NodeViewPort*>(item);

                // Switch ports if mouse dragged in opposite order of I/O
                if (startPort->isOutput() && endPort->isInput()) {
                    startPort = static_cast<NodeViewPort*>(item);
                    endPort = m_connection->startPort();

                    endPort->connections().removeLast(); // Need to remove connection from port
                    m_connection->setStartPort(startPort);
                    m_connection->setStartPosition(startPort->scenePos());
                }

                // If all is valid, then create connection
                if (startPort->block() != endPort->block() &&
                    startPort->isOutput() != endPort->isOutput() &&
                    !startPort->isConnected(endPort) &&
                    endPort->numConnections() < m_numConnectionsPerPort)
                {
                    // Ports are valid to connect to one another, so connect
                    m_connection->setEndPosition(endPort->scenePos());
                    m_connection->setEndPort(endPort);
                    m_connection->updatePath();
                    emit connectedPorts(m_connection);
                    m_connection = NULL;

                    // Update scene for redraw
                    m_scene->update();
                    return true;
                }
            }

            // Delete connection if never connected
            delete m_connection;
            m_connection = NULL;
            return true;
        }

        break;
    }
    }

    emit mouseEventFiltered(mouseEvent);
    return QObject::eventFilter(object, event);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void NodeViewEditor::save(QDataStream& stream)
{
    Q_FOREACH(QGraphicsItem* item, m_scene->items())
    {
        if (item->type() == (int)NodeViewItemType::kBlock)
        {
            stream << item->type();
            static_cast<NodeViewBlock*>(item)->save(stream);
        }
    }

    Q_FOREACH(QGraphicsItem* item, m_scene->items())
    {
        if (item->type() == (int)NodeViewItemType::kConnection)
        {
            stream << item->type();
            static_cast<NodeViewConnection*>(item)->save(stream);
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void NodeViewEditor::load(QDataStream& stream)
{
    QMap<quint64, NodeViewPort*> portMap;

    Q_ASSERT(m_scene);
    m_scene->clear();

    while (!stream.atEnd())
    {
        qint32 type;
        stream >> type;

        if (type == (int)NodeViewItemType::kBlock)
        {
            NodeViewBlock* block = new NodeViewBlock(NULL);
            m_scene->addItem(block);
            block->load(stream, portMap);
        }
        else if (type == (int)NodeViewItemType::kConnection)
        {
            NodeViewConnection* connection = new NodeViewConnection(NULL);
            m_scene->addItem(connection);
            connection->load(stream, portMap);
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QGraphicsItem* NodeViewEditor::itemAt(const QPointF& point)
{
    Q_ASSERT(m_scene);

    // Get all items within a small rectangle located at the scene point
    QList<QGraphicsItem*> items = m_scene->items(QRectF(point - QPointF(1, 1), QSize(3, 3)));

    // Return the first item that is a user type
    Q_FOREACH(QGraphicsItem* item, items)
    {
        // Filter out non-user scene items
        if (item->type() > QGraphicsItem::UserType) {
            return item;
        }
    }

    // No user scene items found at point
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
////////////////////////////////////////////////////////////////////////////////////////////////////
}
}