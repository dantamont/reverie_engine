
// Includes

#include "fortress/encoding/binary/GSerializationProtocol.h"

// Standard Includes

// External

// Project


// Namespace Definitions

namespace rev {


// Class Implementations


void SerializationProtocolInterface::addChild(std::unique_ptr<SerializationProtocolInterface>&& child)
{
    m_children.emplace_back(std::move(child));
}


} // end namespacing
