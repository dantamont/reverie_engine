/*!
  @file    GbNodeViewCommon.h

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

#ifndef GB_NODE_VIEW_COMMON_H
#define GB_NODE_VIEW_COMMON_H

#include <QGraphicsItem>

namespace Gb {
namespace View {

/////////////////////////////////////////////////////////////////////////////////////////////
// Enums
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Types of graphics items in the node view
enum class NodeViewItemType
{
    kPort = QGraphicsItem::UserType + 1,
    kConnection = QGraphicsItem::UserType + 2,
    kConnectionSplit = QGraphicsItem::UserType + 3,
    kBlock = QGraphicsItem::UserType + 4
};

/// @brief Flags designating the attributes of a port label
enum NodeViewPortLabelFlags
{
    kIsName = 1 << 0,
    kIsType = 1 << 1
};


/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
/////////////////////////////////////////////////////////////////////////////////////////////
}
}

#endif