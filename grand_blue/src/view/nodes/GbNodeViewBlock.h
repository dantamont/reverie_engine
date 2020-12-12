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


#ifndef GB_NODE_VIEW_BLOCK_H
#define GB_NODE_VIEW_BLOCK_H

#include <QGraphicsPathItem>
#include <QGraphicsDropShadowEffect>
#include "GbNodeViewCommon.h"


namespace Gb {
namespace View {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
////////////////////////////////////////////////////////////////////////////////////////////////////
class NodeViewPort;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class NodeViewBlock
class NodeViewBlock : public QGraphicsPathItem
{
public:
    NodeViewBlock(QGraphicsItem* parent = NULL);
    virtual ~NodeViewBlock();

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    qint32 width() const { return m_width; }
    qint32 height() const { return m_height; }

    qint32 horizontalMargin() const { return m_horizontalMargin; }
    qint32 verticalMargin() const { return m_verticalMargin; }

    /// @brief Whether or not  a hover rect is highlighted
    bool inputPortsHoveredOver() const;
    bool outputPortsHoveredOver() const;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    NodeViewPort* addPort(const QString& name, bool isOutput, qint32 flags = 0, qint32 index = -1);

    /// @brief Create a copy of this block and add it to the scene
    NodeViewBlock* clone();

    /// @brief Return 
    QVector<NodeViewPort*> ports();

    /// @brief What to perform on mouse move at the scene level
    void onSceneMouseMove(QGraphicsSceneMouseEvent* event);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name QGraphicsItem Overrides
    /// @{

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    int type() const override { return (int)NodeViewItemType::kBlock; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Serialization
    /// @{

    /// @brief Save the viewport to a data stream
    void save(QDataStream& stream);

    /// @brief Load the viewport from a data stream
    void load(QDataStream&, QMap<quint64, NodeViewPort*>& portMap);

    /// @}

protected:
    /// @brief What to do on item changes
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;

    void createHoverRects();
    void clearHoverRects();

private:
    //QGraphicsDropShadowEffect m_dropShadow;
    qint32 m_width;
    qint32 m_height;
    qint32 m_horizontalMargin;
    qint32 m_verticalMargin;

    /// @brief Rects to appear on hover event
    QGraphicsPathItem* m_leftHoverRect = nullptr;
    QGraphicsPathItem* m_rightHoverRect = nullptr;

    /// @brief Count of indexed ports
    /// @details Ports with index < 0 are not considered indexed
    size_t m_indexedPortCount;

    /// @brief Number of input ports (Ports with flags don't count as input ports, need to add a categorization)
    size_t m_numInputPorts;

    /// @brief Number of output ports
    size_t m_numOutputPorts;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
////////////////////////////////////////////////////////////////////////////////////////////////////
}
}

#endif