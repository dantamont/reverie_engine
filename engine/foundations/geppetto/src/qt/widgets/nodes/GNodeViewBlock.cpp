/*!
  @file    GbNodeViewBlock.cpp

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

#include <QPen>
#include <QGraphicsScene>
#include <QFontMetrics>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QApplication>
#include <QGraphicsSceneEvent>

#include "geppetto/qt/widgets/nodes/GNodeViewBlock.h"
#include "geppetto/qt/widgets/nodes/GNodeViewPort.h"

namespace rev {

NodeViewBlock::NodeViewBlock(QGraphicsItem* parent): 
    QGraphicsPathItem(parent),
    m_width(100),
    m_height(5), 
    m_horizontalMargin(20), 
    m_verticalMargin(5),
    m_leftHoverRect(nullptr),
    m_rightHoverRect(nullptr),
    m_indexedPortCount(0),
    m_numInputPorts(0),
    m_numOutputPorts(0)
{
    setCacheMode(DeviceCoordinateCache);

    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsFocusable);

    // By default, items do not accept hover events
    setAcceptHoverEvents(true);

    // Set to draw a rectangle
    QPalette palette = QApplication::palette();
    QPainterPath path;
    path.addRoundedRect(-50, -15, 100, 30, 5, 5);
    setPath(path);
    setPen(QPen(palette.midlight().color()));
    setBrush(palette.dark().color()); 

    // Add drop shadow
    //m_dropShadow.setColor(palette.shadow().color());
    //m_dropShadow.setBlurRadius(16);
    //m_dropShadow.setXOffset(0.0);
    //m_dropShadow.setYOffset(5.0);
    //setGraphicsEffect(&m_dropShadow);

    // Create hover rects
    createHoverRects();
}

NodeViewBlock::~NodeViewBlock()
{
    clearHoverRects();
}

bool NodeViewBlock::inputPortsHoveredOver() const
{
    return m_leftHoverRect->isVisible();
}

bool NodeViewBlock::outputPortsHoveredOver() const
{
    return m_rightHoverRect->isVisible();
}

NodeViewPort* NodeViewBlock::addPort(const QString& name, bool isOutput, qint32 flags, qint32 index)
{
    // Create port
    NodeViewPort* port = new NodeViewPort(this);
    port->setPortName(name);
    port->setIsOutput(isOutput);
    port->setBlock(this);
    port->setPortFlags(flags);
    port->setIndex(index < 0? m_indexedPortCount++: index); // Auto-set index if negative

    // Get dimensions of port based on name
    QFontMetrics fontMetrics(scene()->font());
    const qint32 width = fontMetrics.width(name);
    const qint32 height = fontMetrics.height();

    // Update block dimensions
    if (width > m_width - m_horizontalMargin) {
        m_width = width + 2.0 * m_horizontalMargin; // Buffer to grow enough that text won't go beyond bounds
    }

    // Expand block height if necessary to fit port
    bool canFitPort = (isOutput && m_numOutputPorts < m_numInputPorts) ||
        (!isOutput && m_numInputPorts < m_numOutputPorts);
    if (flags != 0 || !canFitPort) {
        m_height += height;
    }

    // Resize the block by setting path
    QPainterPath path;
    path.addRoundedRect(-(m_width >> 1), -(m_height >> 1), m_width, m_height, 5, 5);
    setPath(path);

    // Increment port count
    if (isOutput) {
        m_numOutputPorts++;
    }
    else if(flags == 0){
        m_numInputPorts++;
    }

    // Iterate through ports to calculate new size
    qint32 inputY = -(m_height >> 1) + m_verticalMargin + port->radius();
    qint32 specialPortY = inputY; // Track height separately for "special" "ports", like Name, type, etc.
    qint32 outputY = inputY;
    Q_FOREACH(QGraphicsItem* childItem, childItems())
    {
        // Skip any children that aren't ports
        if (childItem->type() != (int)NodeViewItemType::kPort) {
            continue;
        }

        NodeViewPort* childPort = static_cast<NodeViewPort*>(childItem);

        if (childPort->isOutput()) {
            childPort->setPos((m_width >> 1) + childPort->radius()*0.75, outputY);
            outputY += height;
        }
        else {
            // If there are any port flags, then the port is a name or label, so scooch input/output height
            if (childPort->portFlags() != 0) {
                childPort->setPos(-(m_width >> 1) - childPort->radius()*0.75, specialPortY);
                specialPortY += height;
                inputY += height;
                outputY += height;
            }
            else {
                childPort->setPos(-(m_width >> 1) - childPort->radius()*0.75, inputY);
                inputY += height;
            }
        }

    }

    // Create hover rects (resizes them)
    createHoverRects();

    return port;
}


void NodeViewBlock::save(QDataStream& stream)
{
    stream << pos();

    qint32 count = 0;

    Q_FOREACH(QGraphicsItem* childItem, childItems())
    {
        if (childItem->type() != (int)NodeViewItemType::kPort)
            continue;

        ++count;
    }

    stream << count;

    Q_FOREACH(QGraphicsItem* childItem, childItems())
    {
        if (childItem->type() != (int)NodeViewItemType::kPort)
            continue;

        NodeViewPort* port = static_cast<NodeViewPort*>(childItem);
        stream << reinterpret_cast<quint64>(port);
        stream << port->portName();
        stream << port->isOutput();
        stream << port->portFlags();
    }
}

void NodeViewBlock::load(QDataStream& stream, QMap<quint64, NodeViewPort*>& portMap)
{
    QPointF position;
    stream >> position;
    setPos(position);

    qint32 count;
    stream >> count;

    for (qint32 iter = 0; iter < count; iter++)
    {
        quint64 index;
        stream >> index;

        QString name;
        stream >> name;

        bool output;
        stream >> output;

        qint32 flags;
        stream >> flags;

        portMap[index] = addPort(name, output, flags, index);
    }
}

void rev::NodeViewBlock::onSceneMouseMove(QGraphicsSceneMouseEvent* event)
{
    // Display only visible hover rect
    QPointF scenePos = event->scenePos();
    //QPointF parentPos = mapFromScene(scenePos);
    //Object().logInfo(QString::number(parentPos.x()) + ", " + QString::number(parentPos.y()));
    QPointF leftPos = m_leftHoverRect->mapFromScene(scenePos);
    QPointF rightPos = m_rightHoverRect->mapFromScene(scenePos);
    bool leftVisible = m_leftHoverRect->contains(leftPos);
    bool rightVisible = m_rightHoverRect->contains(rightPos);
    m_leftHoverRect->setVisible(leftVisible);
    m_rightHoverRect->setVisible(rightVisible);
}

void NodeViewBlock::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);

    // Only paint dirty regions for increased performance
    painter->setClipRect(option->exposedRect);

    QPalette palette = QApplication::palette();
    if (isSelected()) {
        // Lighten box if selected
        painter->setBrush(palette.light());
    }
    else {
        painter->setBrush(palette.midlight());
    }

    // Highlight border if under mouse
    if (isUnderMouse()) {
        painter->setPen(QPen(palette.highlight().color(), 1.5));
    }
    else {
        painter->setPen(QPen(palette.dark().color()));
    }

    painter->drawPath(path());
}

NodeViewBlock* NodeViewBlock::clone()
{
    NodeViewBlock* block = new NodeViewBlock(NULL);
    this->scene()->addItem(block);

    Q_FOREACH(QGraphicsItem* childPort, childItems())
    {
        if (childPort->type() == (int)NodeViewItemType::kPort)
        {
            NodeViewPort* clonePort = static_cast<NodeViewPort*>(childPort);
            block->addPort(
                clonePort->portName(),
                clonePort->isOutput(),
                clonePort->portFlags(),
                clonePort->index());
        }
    }

    return block;
}

QVector<NodeViewPort*> NodeViewBlock::ports()
{
    QVector<NodeViewPort*> result;

    Q_FOREACH(QGraphicsItem* childItem, childItems())
    {
        if (childItem->type() == (int)NodeViewItemType::kPort) {
            result.append(static_cast<NodeViewPort*>(childItem));
        }
    }

    return result;
}

QVariant NodeViewBlock::itemChange(GraphicsItemChange change, const QVariant& value)
{
    Q_UNUSED(change);
    return value;
}

void rev::NodeViewBlock::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
    // Display only visible hover rect
    m_leftHoverRect->setVisible(false);
    m_rightHoverRect->setVisible(false);

    // Apparently called by default implementation, but adding to help debug
    //bool v = isVisible();
    //QRectF rect = boundingRect();
    update();
    scene()->update(); // This is necessary for hover to work correctly
}

void rev::NodeViewBlock::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
    // Display only visible hover rect
    m_leftHoverRect->setVisible(false);
    m_rightHoverRect->setVisible(false);

    // Apparently called by default implementation, but adding to help debug
    update();
    scene()->update(); // This is necessary for hover to work correctly
}

void rev::NodeViewBlock::hoverMoveEvent(QGraphicsSceneHoverEvent * event)
{
    Q_UNUSED(event);
    // Hover rects occlude the block, so using onSceneMouseMove to drive at scene level instead
    //m_leftHoverRect->setVisible(m_leftHoverRect->isUnderMouse());
    //m_rightHoverRect->setVisible(m_rightHoverRect->isUnderMouse());
}

void rev::NodeViewBlock::createHoverRects()
{
    clearHoverRects();

    QPalette palette = QApplication::palette();

    // Create left hover rect, height is always inverted, so annoying.
    size_t rectWidth = 20;
    m_leftHoverRect = new QGraphicsPathItem(this);
    QPainterPath lpath;
    lpath.addRoundedRect(-m_width / 2.0, -m_height / 2.0, rectWidth, m_height, 5, 5); // anchor is top left corner
    m_leftHoverRect->setPath(lpath);
    m_leftHoverRect->setPen(QPen(palette.highlight().color()));
    m_leftHoverRect->setBrush(palette.highlight().color());
    m_leftHoverRect->setOpacity(0.5);
    m_leftHoverRect->setVisible(false);

    // Create right hover rect
    m_rightHoverRect = new QGraphicsPathItem(this);
    QPainterPath rpath;
    rpath.addRoundedRect((m_width / 2.0) - rectWidth, -m_height / 2.0, rectWidth, m_height, 5, 5); // anchor is top left corner
    m_rightHoverRect->setPath(rpath);
    m_rightHoverRect->setPen(QPen(palette.highlight().color()));
    m_rightHoverRect->setBrush(palette.highlight().color());
    m_rightHoverRect->setOpacity(0.5);
    m_rightHoverRect->setVisible(false);
}

void rev::NodeViewBlock::clearHoverRects()
{
    if (m_leftHoverRect) {
        delete m_leftHoverRect;
        m_leftHoverRect = nullptr;
    }
    if (m_rightHoverRect) {
        delete m_rightHoverRect;
        m_rightHoverRect = nullptr;
    }
}


}