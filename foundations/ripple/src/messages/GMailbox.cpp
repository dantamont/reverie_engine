#include "ripple/network/messages/GMailbox.h"

#include "logging/GLogger.h"

namespace rev {

Uint32_t Mailbox::messageHeaderSizeBytes() const
{
    static constexpr Uint32_t s_messageHeaderSize = sizeof(GMessageHeader) + sizeof(Uint64_t)/*for size of count*/;
    return s_messageHeaderSize;
}

Uint32_t Mailbox::calculateMessageBodySizeBytes()
{
    // Deserialize the most recently received header from the receive buffer
    // Assumes that message header is always at zero index
    SerializationProtocol<GMessageHeader> deserializer(m_mostRecentlyReceivedHeader);
    deserializer.read(m_receiveBuffer.data(), 0);
    return m_mostRecentlyReceivedHeader.m_serializedBodySizeBytes;
}

void Mailbox::popMessage(Uint64_t messageId)
{
    // Remove all messages with an ID matching the given ID
    std::unique_lock lock(m_messageSendMutex);
    std::deque<TransportMessageInterface*> messages;
    for (TransportMessageInterface* message : m_messageSendQueue) {
        GMessage* gMessage = static_cast<GMessage*>(message);
        if (gMessage->header().m_uniqueId != messageId) {
            messages.push_back(gMessage);
        }
    }
    m_messageSendQueue = messages;
    m_sendQueueCount = m_messageSendQueue.size();
}

bool Mailbox::isReceiveBufferValid(Uint32_t bufferIndex)
{
    // Return if exceeded maximum buffer size
    static constexpr Size_t maxSize = ReceiveMessageBuffer::GetMaxSizeBytes();
    if (bufferIndex >= maxSize) {
        return false;
    }

//#ifdef DEBUG_MODE
//    Int32_t diff = m_mostRecentlyReceivedHeader.m_index - m_lastMessageIndex;
//    bool valid = m_mostRecentlyReceivedHeader.m_index > m_lastMessageIndex;
//    if (valid) {
//        m_lastMessageIndex = m_mostRecentlyReceivedHeader.m_index;
//    }
//#endif
    //return valid;
    return true;
}

TransportMessageInterface* Mailbox::unpackMessage(Uint32_t bufferIndex, Uint32_t& outBytesRead) const
{
    GMessage* message = GMessage::Unpack(m_receiveBuffer.data(), bufferIndex);
    return message;
}

Uint32_t Mailbox::packMessageForSend(TransportMessageInterface* message, Uint32_t index)
{
    // Set the sender name for the message before packing
    GMessage* gMessage = static_cast<GMessage*>(message);
    gMessage->header().setSender(m_name);
    return Base::packMessageForSend(message, index);
}

} /// End namespace