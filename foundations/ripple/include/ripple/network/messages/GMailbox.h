#pragma once

#include "layline/transport/messages/GTransportMailboxInterface.h"
#include "ripple/network/messages/GMessage.h"

namespace rev {

/// @class Mailbox
/// @brief Class representing a mailbox for sending and receiving messages
/// @details Does not take ownership over message pointers
class Mailbox : public TransportMailboxInterface<1, 1> {
protected:

    using Base = TransportMailboxInterface;

public:
    Mailbox() = default;
    ~Mailbox() = default;


    /// @brief The size of the message headers used by this mailbox
    virtual Uint32_t messageHeaderSizeBytes() const override;

    /// @brief Return the number of bytes expected from the next message body
    virtual Uint32_t calculateMessageBodySizeBytes() override;

    /// @brief Remove a message from the mailbox for sending
    void popMessage(Uint64_t messageId);

protected:

    /// @brief Check the validity of the contents of the receive buffer
    /// @detail Called before unpacking a message from the receive buffer. Invalid contents are ignored
    virtual bool isReceiveBufferValid(Uint32_t bufferIndex) override;

    /// @brief Called when unpacking from the receive buffer into a message
    /// @detail Override this to create a message of the appropriate TransportMessageInterface subclass
    virtual TransportMessageInterface* unpackMessage(Uint32_t bufferIndex, Uint32_t& outBytesRead) const;

    /// @brief Pack a message to be sent
    /// @return the number of bytes packed
    virtual Uint32_t packMessageForSend(TransportMessageInterface* message, Uint32_t index) override;

private:

    GString m_name{"mailbox"}; ///< The string identifier for the mailbox
    GMessageHeader m_mostRecentlyReceivedHeader; ///< Last received message header cached to save deserialization calls between isReceiveBufferValid and calculateMessageBodySizeBytes
    Int64_t m_lastMessageIndex{ -1 }; ///< Index of most recently received message
};

} // End rev namespace
