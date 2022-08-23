/*!
  @file    GbNodeViewPort.h

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

#ifndef GB_NODE_VIEWPORT_H
#define GB_NODE_VIEWPORT_H

#include <QGraphicsPathItem>
#include "GNodeViewCommon.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
////////////////////////////////////////////////////////////////////////////////////////////////////
class QPainter;

namespace rev {

class NodeViewBlock;
class NodeViewConnection;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class NodeViewPort
class NodeViewPort : public QGraphicsPathItem
{
public:
    NodeViewPort(QGraphicsItem* parent = NULL);
    virtual ~NodeViewPort();

    //---------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief Whether or not this port is connected to the other port
    bool isConnected(NodeViewPort*);

    /// @brief Set the name of this port
    void setPortName(const QString& name);

    /// @brief Set whether the port is an input or an output
    void setIsOutput(bool isOutput);

    /// @brief Set the flags for the port
    /// @details Currently, flags control whether the port is a type or a name
    virtual void setPortFlags(qint32 index);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief The number of ports connected to this port
    size_t numConnections() const { return m_connections.size(); }

    bool isInput() const { return !m_isOutput; }
    bool isOutput() const { return m_isOutput; }

    qint32 radius() const { return m_radius; }
    QVector<NodeViewConnection*>& connections() {  return m_connections; }

    NodeViewBlock* block()  const { return m_block; }
    void setBlock(NodeViewBlock* block) { m_block = block; }

    quint64 index() const { return m_index; }
    void setIndex(quint64 index) { m_index = index; }

    const QString& portName() const { return m_name; }
    int portFlags() const { return m_portFlags; }

    /// @}

public:
    // QGraphicsItem
    int type() const override { return (int)NodeViewItemType::kPort; }

protected:
    /// @brief What happens on item change
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

    /// @brief Override paint routine for handling selection and hover
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    QVector<NodeViewConnection*> m_connections;
    QString m_name;
    NodeViewBlock* m_block;
    QGraphicsTextItem* m_label;

    /// @brief Index for identifying the port on its block
    quint64 m_index;

    qint32 m_radius;
    qint32 m_margin;
    qint32 m_portFlags;

    /// @brief If true, is output, else is input
    bool m_isOutput;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
////////////////////////////////////////////////////////////////////////////////////////////////////
}


#endif