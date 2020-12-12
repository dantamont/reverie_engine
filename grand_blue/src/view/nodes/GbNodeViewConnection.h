/*!
  @file    GbNodeViewConnection.h

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

#ifndef GB_NODE_VIEW_CONNECTION_H
#define GB_NODE_VIEW_CONNECTION_H

#include <QGraphicsPathItem>
#include "GbNodeViewCommon.h"

namespace Gb {
namespace View {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
////////////////////////////////////////////////////////////////////////////////////////////////////
class NodeViewPort;
class NodeViewConnection;


////////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class NodeViewConnectionSplit
class NodeViewConnectionSplit : public QGraphicsPathItem
{
public:
    NodeViewConnectionSplit(NodeViewConnection* connection);
    virtual ~NodeViewConnectionSplit();

    void setSplitPosition(const QPointF& position);

    void updatePath();

    QPointF splitPosition() const { return m_splitPosition; }
    NodeViewConnection* connection() const { return m_connection; }

public:
    // QGraphicsItem
    int type() const override { return (int)NodeViewItemType::kConnectionSplit; }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

protected:

    /// @brief Notifies custom items that some part of the item's state has changed
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    QPointF m_splitPosition;
    NodeViewConnection* m_connection;
    qint32 m_radius;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class NodeViewConnection
class NodeViewConnection : public QGraphicsPathItem
{
public:
    NodeViewConnection(QGraphicsItem* parent = NULL);
    virtual ~NodeViewConnection();

    void setStartPosition(const QPointF& position);
    void setEndPosition(const QPointF& position);

    void setStartPort(NodeViewPort* port);
    void setEndPort(NodeViewPort* port);

    void updatePosition();
    void updatePath();
    void updateSplits();

    QList<NodeViewConnectionSplit*>& splits() { return m_splits; }

    QPointF startPosition() const { return m_startPosition; }
    QPointF endPosition() const { return m_endPosition; }

    NodeViewPort* startPort() const;
    NodeViewPort* endPort() const;

public:
    void save(QDataStream& stream);
    void load(QDataStream&, const QMap<quint64, NodeViewPort*>& portMap);

public:
    //---------------------------------------------------------------------------------------
    /// @name QGraphicsItem Overrides
    /// @{

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    int type() const override { return (int)NodeViewItemType::kConnection; }

    /// @}

private:
    /// @brief Start position in parent coordinates
    QPointF m_startPosition;
    QPointF m_endPosition;

    QList<NodeViewConnectionSplit*> m_splits;

    NodeViewPort* m_startPort;
    NodeViewPort* m_endPort;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
////////////////////////////////////////////////////////////////////////////////////////////////////
}
}

#endif