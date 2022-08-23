/*!
  @file    GbNodeViewConnection.cpp

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

#include <QBrush>
#include <QPen>
#include <QGraphicsScene>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QApplication>

#include "geppetto/qt/widgets/nodes/GNodeViewConnection.h"
#include "geppetto/qt/widgets/nodes/GNodeViewPort.h"

namespace rev {


NodeViewConnectionSplit::NodeViewConnectionSplit(NodeViewConnection* connection): 
    QGraphicsPathItem(nullptr), // No parent
    m_connection(connection),
    m_radius(5)
{
    setCacheMode(DeviceCoordinateCache);

    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

NodeViewConnectionSplit::~NodeViewConnectionSplit()
{

}

void NodeViewConnectionSplit::setSplitPosition(const QPointF& position)
{
    m_splitPosition = position;

    if (pos() != position) {
        // Set the position of the connection item. Position is in parent coordinates if there
        // is a parent, and scene coordinates if there is not
        setPos(m_splitPosition);
    }
}

void NodeViewConnectionSplit::updatePath()
{
    // Add ellipse to the path
    QPainterPath path;
    path.moveTo(m_splitPosition); // Moves the current point to the given point, implicitly starting a new subpath and closing the previous one.
    path.addEllipse(-m_radius, -m_radius, m_radius * 2, m_radius * 2);
    setPath(path); // QGraphicsPathItem will use this to implement BoundingRect(), shape(), contains(), and for painting
}

void NodeViewConnectionSplit::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);

    // Only paint dirty regions for increased performance
    painter->setClipRect(option->exposedRect);

    if (isSelected())
    {
        painter->setBrush(QColor(180, 180, 180)); // GW-TODO: Expose to QStyle
    }
    else
    {
        painter->setBrush(QColor(155, 155, 155)); // GW-TODO: Expose to QStyle
    }

    painter->drawPath(path());
}

QVariant NodeViewConnectionSplit::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemScenePositionHasChanged)
    {
        const QPointF newPosition = value.toPointF();
        setSplitPosition(newPosition);
        m_connection->updatePath();
    }

    return value;
}





// NodeViewConnection

NodeViewConnection::NodeViewConnection(QGraphicsItem* parent): 
    QGraphicsPathItem(parent), 
    m_startPort(nullptr),
    m_endPort(nullptr)
{
    // Setting cache mode, since by default, item caching is disabled and paint() is called every 
    // time the item needs redrawing.
    // For items that can move, but are not rotated, scaled or sheared.
    // If item is transformed (directly or indirectly), the cache will be regenerated automatically
    // Always renders at maximum quality
    setCacheMode(DeviceCoordinateCache);
    setBrush(Qt::NoBrush);
    setZValue(-1);

    // By default, items do not accept hover events
    setAcceptHoverEvents(true);
}

NodeViewConnection::~NodeViewConnection()
{
    if (m_startPort)
        m_startPort->connections().remove(m_startPort->connections().indexOf(this));

    if (m_endPort)
        m_endPort->connections().remove(m_endPort->connections().indexOf(this));

    Q_FOREACH(NodeViewConnectionSplit* split, m_splits)
        delete split;
}

void NodeViewConnection::setStartPosition(const QPointF& position)
{
    m_startPosition = position;
}

void NodeViewConnection::setEndPosition(const QPointF& position)
{
    m_endPosition = position;
}

void NodeViewConnection::setStartPort(NodeViewPort* port)
{
    m_startPort = port;
    m_startPort->connections().append(this);
}

void NodeViewConnection::setEndPort(NodeViewPort* port)
{
    m_endPort = port;
    m_endPort->connections().append(this);
}

void NodeViewConnection::updatePosition()
{
    m_startPosition = m_startPort->scenePos();
    m_endPosition = m_endPort->scenePos();
}

void NodeViewConnection::updatePath()
{
    QPainterPath path;

    // Generate curve points, which include start, end, and all split positions
    QVector<QPointF> curvePoints;
    curvePoints.append(m_startPosition);
    Q_FOREACH(NodeViewConnectionSplit* split, m_splits)
        curvePoints.append(split->splitPosition());
    curvePoints.append(m_endPosition);

    // Iterate over curve points to generate a painter path
    for (qint32 index = 0; index < curvePoints.size() - 1; ++index)
    {
        const QPointF& startPosition = curvePoints[index + 0];
        const QPointF& endPosition = curvePoints[index + 1];

        const qreal deltaX = endPosition.x() - startPosition.x();
        const qreal deltaY = endPosition.y() - startPosition.y();

        QPointF anchor1(startPosition.x() + deltaX * 0.25, startPosition.y() + deltaY * 0.1);
        QPointF anchor2(startPosition.x() + deltaX * 0.75, startPosition.y() + deltaY * 0.9);

        // Use a cubic with specified anchor points to connect start and end positions
        path.moveTo(startPosition);
        path.cubicTo(anchor1, anchor2, endPosition);
    }

    // Set the  path to the generated path
    setPath(path);
}

void NodeViewConnection::updateSplits()
{
    Q_FOREACH(NodeViewConnectionSplit* split, m_splits)
    {
        split->updatePath();
    }
}

NodeViewPort* NodeViewConnection::startPort() const
{
    return m_startPort;
}

NodeViewPort* NodeViewConnection::endPort() const
{
    return m_endPort;
}

void NodeViewConnection::save(QDataStream& stream)
{
    stream << reinterpret_cast<quint64>(m_startPort);
    stream << reinterpret_cast<quint64>(m_endPort);

    const qint32 splitCount = m_splits.size();
    stream << splitCount;

    Q_FOREACH(NodeViewConnectionSplit* split, m_splits)
    {
        const QPointF splitPosition = split->splitPosition();
        stream << splitPosition;
    }
}

void NodeViewConnection::load(QDataStream& stream, const QMap<quint64, NodeViewPort*>& portMap)
{
    quint64 startPortIndex;
    quint64 endPortIndex;
    stream >> startPortIndex;
    stream >> endPortIndex;

    setStartPort(portMap[startPortIndex]);
    setEndPort(portMap[endPortIndex]);

    qint32 splitCount;
    stream >> splitCount;

    for (qint32 splitIndex = 0; splitIndex < splitCount; splitIndex++)
    {
        QPointF splitPosition;
        stream >> splitPosition;
        NodeViewConnectionSplit* split = new NodeViewConnectionSplit(this);
        scene()->addItem(split);
        split->setSplitPosition(splitPosition);
        m_splits.append(split);
    }

    updatePosition();
    updatePath();
    updateSplits();
}

void NodeViewConnection::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    Q_UNUSED(widget);

    // Only paint dirty regions for increased performance
    painter->setClipRect(option->exposedRect);

    QPalette palette = QApplication::palette();
    float penWidth = 2;

    if (m_startPort != nullptr && m_endPort != nullptr) {
        painter->setPen(QPen(palette.highlight().color().lighter(150), penWidth)); // Brighten by 50%
    }
    else {
        painter->setPen(QPen(palette.light().color().lighter(150), penWidth)); // Brighten by 50%
    }

    if (isUnderMouse()) {
        // Highlight if under mouse
        painter->setPen(QPen(palette.text().color(), penWidth));
    }

    painter->drawPath(path());
}


}