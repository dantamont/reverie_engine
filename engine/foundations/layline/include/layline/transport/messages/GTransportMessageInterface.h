#pragma once

#include "fortress/containers/GSizedBuffer.h"
#include "fortress/string/GString.h"

namespace rev {

/// @class TransportMessageInterface
/// @brief Class representing a message to send over a transport
class TransportMessageInterface {
private:

    /// @example The MTU (Maximum Transmission Unit) for Ethernet is 1500 bytes
    static constexpr Uint32_t s_maxMessageSizeBytes = 1400; ///< A good number for safety with different network interfaces and platforms

public:

    static constexpr Uint32_t GetMaxMessageSizeBytes() {
        return s_maxMessageSizeBytes;
    }

    TransportMessageInterface() = default;
    ~TransportMessageInterface() = default;

    /// @brief Get the serialized size of the message in bytes
    virtual Uint32_t getSerializedSizeInBytes() const = 0;

    /// @brief Pack the message to a buffer
    /// @details Returns the number of bytes written
    virtual Uint32_t pack(Uint8_t* buffer, Uint32_t index = 0) const = 0;

    /// @brief Unpack the message from a buffer
    /// @details Returns the number of bytes unpacked
    virtual Uint32_t unpack(const Uint8_t* buffer, Uint32_t index = 0) = 0;

    operator std::string() const {
        return GString::Format("Transport message:\n   Size: %d bytes", getSerializedSizeInBytes()).c_str();
    }

};

// Alias declarations
/// @brief A buffer for sending/receiving a given maximum number of messages simultaneously
template<Uint32_t NumAllowedMessages>
using MessageBuffer = SizedBuffer<(NumAllowedMessages* TransportMessageInterface::GetMaxMessageSizeBytes()) + 1>; ///< Add one to give space for a null terminator for end of last message in buffer

} // End rev namespace
 