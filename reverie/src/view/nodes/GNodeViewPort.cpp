/*!
  @file    GbNodeViewPort.cpp

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

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QFontMetrics>
#include <QPen>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QApplication>
#include <QPalette>

#include "GNodeViewPort.h"
#include "GNodeViewConnection.h"

namespace rev {
namespace View {
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// NodeViewPort
////////////////////////////////////////////////////////////////////////////////////////////////////
NodeViewPort::NodeViewPort(QGraphicsItem* parent): 
    QGraphicsPathItem(parent),
    m_radius(5), 
    m_margin(2),
    m_portFlags(0x0)
{
    setCacheMode(DeviceCoordinateCache);

    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
    setFlag(QGraphicsItem::ItemIsSelectable);

    // By default, items do not accept hover events
    setAcceptHoverEvents(true);

    m_label = new QGraphicsTextItem(this);
    m_label->setCacheMode(DeviceCoordinateCache);

    QPainterPath path;
    path.addEllipse(-m_radius, -m_radius, m_radius * 2, m_radius * 2);
    setPath(path);

    setPen(QPen(QColor(100, 100, 100))); // GW-TODO: Expose to QStyle
    setBrush(QColor(155, 155, 155)); // GW-TODO: Expose to QStyle
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NodeViewPort::~NodeViewPort()
{
    // Delete all connections associated with this port
    Q_FOREACH(NodeViewConnection* connection, m_connections)
        delete connection;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void NodeViewPort::setPortName(const QString& name)
{
    m_name = name;
    m_label->setPlainText(name);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void NodeViewPort::setIsOutput(bool isOutput)
{
    m_isOutput = isOutput;

    const qreal boundingWidth = m_label->boundingRect().width();
    const qreal boundingHalfHeight = m_label->boundingRect().height() / 2;

    if (m_isOutput) {
        m_label->setPos(-m_radius - m_margin - boundingWidth, -boundingHalfHeight);
    }
    else {
        m_label->setPos(m_radius + m_margin, -boundingHalfHeight);
    }

    QPalette palette = QApplication::palette();
    m_label->setDefaultTextColor(palette.text().color());
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void NodeViewPort::setPortFlags(qint32 flags)
{
    m_portFlags = flags;

    if (m_portFlags & NodeViewPortLabelFlags::kIsType)
    {
        // If port is a type, don't visualize circle, and format text accordingly
        QFont font(scene()->font());
        font.setItalic(true);
        m_label->setFont(font);
        setPath(QPainterPath());
    }
    else if (m_portFlags & NodeViewPortLabelFlags::kIsName)
    {
        // If port is a name, don't visualize circle, and format text accordingly
        QFont font(scene()->font());
        font.setBold(true);
        m_label->setFont(font);
        setPath(QPainterPath());
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool NodeViewPort::isConnected(NodeViewPort* other)
{
    Q_FOREACH(NodeViewConnection* connection, m_connections)
    {
        if (connection->startPort() == other || connection->endPort() == other) {
            return true;
        }
    }

    return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QVariant NodeViewPort::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemScenePositionHasChanged)
    {
        // Update positions of all connections when port moves
        Q_FOREACH(NodeViewConnection* connection, m_connections)
        {
            connection->updatePosition();
            connection->updatePath();
            connection->updateSplits();
        }
    }

    return value;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void NodeViewPort::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);

    // Only paint dirty regions for increased performance
    painter->setClipRect(option->exposedRect);

    QPalette palette = QApplication::palette();
    if (isSelected()) {
        // Lighten box if selected
        painter->setBrush(QColor(255, 255, 0)); // yellow
    }
    else {
        painter->setBrush(palette.light());
    }

    // Highlight border if under mouse
    //QPoint mousePos = QCursor::pos();
    //QPoint viewportPos = scene()->views().first()->viewport()->mapFromGlobal(mousePos);
    //QPointF scenePos = scene()->views().first()->mapToScene(viewportPos);

    if (isUnderMouse()) {
        painter->setPen(QPen(QColor(255, 255, 0), 1.5));// yellow
    }
    else {
        if (m_connections.size()) {
            painter->setPen(QPen(palette.light().color().lighter(250), 1.5)); // Brighten by 50%
        }
        else {
            //if (!contains(scenePos)) {
            painter->setPen(QPen(palette.light().color().lighter(150))); // Brighten by 50%
            //}
            //else {
            //    painter->setPen(QPen(QColor(255, 255, 0), 1.5));// yellow
            //}
        }
    }

    painter->drawPath(path());
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
////////////////////////////////////////////////////////////////////////////////////////////////////
}
}