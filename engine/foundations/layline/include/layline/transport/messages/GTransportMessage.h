#pragma once

#include "fortress/encoding/binary/GSerializationProtocol.h"
#include "layline/transport/messages/GTransportMessageInterface.h"

namespace rev {

/// @class TransportMessage
/// @brief Class representing a message to send over a transport
/// @todo Consider making this an interface type TransportMessageInterface, 
/// making it templated for the header type. Need to add a non-templated base class
/// for polymorphic usage though
template<typename HeaderType>
class TransportMessage: public TransportMessageInterface {
public:

    TransportMessage() = default;
    virtual ~TransportMessage() = default;

    const HeaderType& header() const { return  m_header; }
    HeaderType& header() { return  m_header; }

    /// @brief Get the serialized size of the message in bytes
    virtual Uint32_t getSerializedSizeInBytes() const {
        SerializationProtocol<HeaderType> headerSerializer(m_header);
        return headerSerializer.SerializedSizeInBytes();
    }

    /// @brief Pack the message to a buffer
    /// @details Returns the number of bytes written
    virtual Uint32_t pack(Uint8_t* buffer, Uint32_t index = 0) const override {
        SerializationProtocol<HeaderType> headerSerializer(m_header);
        Uint32_t sizeToWrite = getSerializedSizeInBytes();
        return headerSerializer.write(buffer, index);
    }

    /// @brief Unpack the message from a buffer
    /// @details Returns the number of bytes read
    virtual Uint32_t unpack(const Uint8_t* buffer, Uint32_t index = 0) override {
        SerializationProtocol<HeaderType> headerDeserializer(m_header);
        return headerDeserializer.read(buffer, index);
    }

protected:

    HeaderType m_header; ///< The message header
     
};

} // End rev namespace
 