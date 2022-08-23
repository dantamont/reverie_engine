#pragma once

#include <atomic>
#include <deque>
#include <mutex>

#include <asio.hpp>
#include "layline/transport/messages/GTransportMessage.h"

namespace rev {

enum class TransportCommunicationType : Int32_t;

template<typename AsioProtocolType>
class TransportInterface;

/// @class TransportMailboxInterfaceBase
/// @brief Class representing a mailbox for sending and receiving messages
/// @details Does not take ownership over message pointers
class TransportMailboxInterfaceBase {
public:

    TransportMailboxInterfaceBase() = default;
    virtual ~TransportMailboxInterfaceBase() = default;

    /// @brief The size of the message headers used by this mailbox
    virtual Uint32_t messageHeaderSizeBytes() const = 0;

    /// @brief Return the number of bytes expected from the next message body
    virtual Uint32_t calculateMessageBodySizeBytes() = 0;

    /// @brief Get data from send buffer
    virtual Uint8_t* sendData() = 0;

    /// @brief Get data from receive buffer
    virtual Uint8_t* receiveData() = 0;

    /// @brief Get the asio buffer to be used to write received message data
    /// @details Returns a buffer either sized to receive a message header, or a message body if a header has already been received
    virtual asio::mutable_buffer getWriteableReceiveBuffer() {
        Uint32_t bufferSize = 0 == m_expectedMessageLength ? messageHeaderSizeBytes() : m_expectedMessageLength;
        return asio::buffer(
            receiveData() + m_receiveBufferWriteOffsetBytes,
            bufferSize
        );
    }

    /// @brief Get size of send queue
    Uint32_t sendQueueCount() const {
        return m_sendQueueCount;
    }

    /// @brief Get size of receive queue
    Uint32_t receiveQueueCount() const {
        return m_receiveQueueCount;
    }

    /// @brief Reset packed message settings once message is sent
    /// @details The mailbox will keep belligerently sending messages unless the send size is dropped to zero 
    void onMessageSent() {
        m_packedSendMessageSize = 0;
    }

    /// @brief Pack the message from the top of the send message queue for sending
    /// @return The size of the packed message
    Uint32_t packSend() {
        // Pack the first message in the send queue to the send buffer
        std::unique_lock lock(m_messageSendMutex);

        if (m_messageSendQueue.size()) {
            TransportMessageInterface* message = m_messageSendQueue.front();
            m_packedSendMessageSize = packMessageForSend(message, 0);
            m_messageSendQueue.pop_front();

            if (m_sendBufferEmpty) {
                m_sendBufferEmpty = false;
            }
            m_sendQueueCount--;
        }

        return m_packedSendMessageSize;
    }

    /// @brief Pack multiple messages from the send message queue to send. 
    /// @param[in] count the number of messages to send. -1 will send as many as allowed by send buffer
    /// @return The size of the packed messages
    virtual Uint32_t packGroupSend(Int32_t count = -1) = 0;

    /// @brief Unpack the message from the receive buffer and add to the received message queue
    /// @detail This can be overridden for different processing of messages
    ///   For example, if a mailbox subclass wants to offload message processing 
    ///   to a separate thread, or read multiple messages at a time, that could
    ///   be done by overriding this routine
    /// @note Override with caution:
    ///    - The receive queue mutex MUST be locked
    ///    - getWriteableReceiveBuffer() must be overridden appropriately
    virtual void unpackReceive(Uint64_t bytesTransferred) {
        G_UNUSED(bytesTransferred);

        // Unpack the message from the receive buffer and add to the receive queue
        std::unique_lock lock(m_messageReceivedMutex);

        /// @todo Optimize so header isn't deserialized multiple times between isReceiveBufferValid and calculateMessageBodySizeBytes calls
        auto verifyAndUnpackMessage = [this]() {
            // Unpack message, and reset message length
            Uint32_t unusedBytesRead;
            if (isReceiveBufferValid(0)) {
                // Incoming messages are ignored if invalid
                TransportMessageInterface* message = unpackMessage(0, unusedBytesRead);
                m_messageReceivedQueue.push_back(message);
                m_receiveQueueCount++;
            }
            m_expectingHeader = true;
        };

        if (m_expectingHeader) {
            // Received a header
            m_expectedMessageLength = calculateMessageBodySizeBytes();

            if (0 == m_expectedMessageLength) {
                // No expected message length, so will receive another header after this
                verifyAndUnpackMessage();
                m_receiveBufferWriteOffsetBytes = 0;
                m_expectingHeader = true;
            }
            else {
                m_receiveBufferWriteOffsetBytes = messageHeaderSizeBytes();
                m_expectingHeader = false;
            }
        }
        else {
            // Received the body of a message
            verifyAndUnpackMessage();
            m_expectingHeader = true;
            m_expectedMessageLength = 0;
            m_receiveBufferWriteOffsetBytes = 0;
        }

    }

    /// @brief Add a message to the mailbox for sending
    void push(TransportMessageInterface* message) {
        std::unique_lock lock(m_messageSendMutex);
        m_messageSendQueue.push_back(message);
        m_sendQueueCount++;
    }

    /// @brief Retrieve a received message from the mailbox
    TransportMessageInterface* retrieve() {
        /// Unpack the first message from the receive queue
        std::unique_lock lock(m_messageReceivedMutex);
        if (!m_messageReceivedQueue.size()) { return nullptr; }
        TransportMessageInterface* message = m_messageReceivedQueue.front();
        m_messageReceivedQueue.pop_front();
        m_receiveQueueCount--;
        return message;
    }

    /// @brief Retrieve all received messages from the mailbox
    template<typename Container, typename CastType = TransportMessageInterface>
    void retrieveAll(Container& c) {
        /// Unpack all messages from the receive queue
        std::unique_lock lock(m_messageReceivedMutex);
        while (m_messageReceivedQueue.size()) 
        { 
            CastType* message = static_cast<CastType*>(m_messageReceivedQueue.front());
            c.push_back(message);
            m_messageReceivedQueue.pop_front();
            m_receiveQueueCount--;
        }
    }


    /// @brief The size of the next message to send
    /// @return The size of the next message to be sent, or zero if there is none
    virtual Uint32_t getSizeOfPackedSendMessage() = 0;

protected:

    /// @brief Check the validity of the contents of the receive buffer
    /// @detail Called before unpacking a message from the receive buffer. Invalid contents are ignored
    /// @param[in] bufferIndex The index at which to check the buffer validity
    virtual bool isReceiveBufferValid(Uint32_t bufferIndex) = 0;

    /// @brief Called when packing a message into the send buffer from the send message queue
    /// @detail Override this to pack a message for sending
    virtual Uint32_t packMessageForSend(TransportMessageInterface* message, Uint32_t index) = 0;

    /// @brief Called when unpacking from the receive buffer into a message
    /// @detail Override this to create a message of the appropriate TransportMessageInterface subclass
    /// @param[in] bufferIndex the index at which to unpack the message
    /// @param[out] outBytesRead the number of bytes read from the buffer
    virtual TransportMessageInterface* unpackMessage(Uint32_t bufferIndex, Uint32_t& outBytesRead) const = 0;

    std::atomic<bool> m_expectingHeader{ true }; ///< Whether or not a header is expected to next be received
    std::atomic<bool> m_sendBufferEmpty{ true }; ///< Whether or not any messages have been packed into the send buffer
    std::atomic<Uint32_t> m_receiveBufferWriteOffsetBytes{ 0 }; ///< The offset at which to write to the receive buffer
    std::atomic<Uint32_t> m_expectedMessageLength{ 0 }; ///< The length of the next expected message
    Uint32_t m_packedSendMessageSize{ 0 }; ///< The size of the next message to send

    std::atomic<Uint32_t> m_sendQueueCount{ 0 }; ///< The number of messages in the send queue
    std::atomic<Uint32_t> m_receiveQueueCount{ 0 }; ///< The number of messages in the receive queue

    std::deque<TransportMessageInterface*> m_messageSendQueue; ///< The messages to be sent by the mailbox
    std::deque<TransportMessageInterface*> m_messageReceivedQueue; ///< The messages to be received by the mailbox
    mutable std::mutex m_messageSendMutex; ///< Mutex for the send message queue
    mutable std::mutex m_messageReceivedMutex; ///< Mutex for the received message queue

};

/// @class TransportMailboxInterface
/// @copydoc TransportMailboxInterfaceBase
/// @tparam NumAllowedSimultaneousSends Controls the size of the send message buffer
/// @tparam NumAllowedSimultaneousReceives Controls the size of the receive message buffer
template<Uint32_t NumAllowedSimultaneousSends, Uint32_t NumAllowedSimultaneousReceives>
class TransportMailboxInterface: public TransportMailboxInterfaceBase {
protected:

    using SendMessageBuffer = MessageBuffer<1>; ///< Only ever sends one message at a time
    using ReceiveMessageBuffer = MessageBuffer<NumAllowedSimultaneousReceives>;

public:

    TransportMailboxInterface() = default;
    virtual ~TransportMailboxInterface() = default;

    /// @brief Get data from send buffer
    virtual Uint8_t* sendData() {
        return m_sendBuffer.data();
    }

    /// @brief Get data from receive buffer
    virtual Uint8_t* receiveData() {
        return m_receiveBuffer.data();
    }

    /// @brief The size of the next message to send
    /// @return The size of the next message to be sent, or zero if there is none
    Uint32_t getSizeOfPackedSendMessage() override {
        return m_packedSendMessageSize;
    }

    /// @brief Pack multiple messages from the send message queue to send. 
    /// @param[in] count the number of messages to send. -1 will send as many as allowed by send buffer
    /// @return The size of the packed messages
    /// @todo Test this
    Uint32_t packGroupSend(Int32_t count = -1) override {
        std::unique_lock lock(m_messageSendMutex);

        // Pack all messages for send
        m_packedSendMessageSize = 0;
        Uint32_t sendQueueSize = m_messageSendQueue.size();
        Uint32_t trueCount = count < 0 ? sendQueueSize : std::min(sendQueueSize, static_cast<Uint32_t>(count));
        trueCount = std::min(trueCount, NumAllowedSimultaneousSends);
        for (Uint32_t i = 0; i < trueCount; i++) {
            TransportMessageInterface* message = m_messageSendQueue.front();
            m_packedSendMessageSize += packMessageForSend(message, m_packedSendMessageSize);
            m_messageSendQueue.pop_front();

            m_sendQueueCount--;
        }

        // Denote that send buffer is not empty
        if (trueCount > 0) {
            m_sendBufferEmpty = false;
        }

        // The total packed message size cannot exceed what is allowed by the protocol
        /// \note Since simultaneous receives do not come from a single sent message, this limitation
        /// does not apply for receives
        assert(m_packedSendMessageSize <= TransportMessageInterface::GetMaxMessageSizeBytes());

        return m_packedSendMessageSize;
    }

protected:

    /// @brief Null-terminate at end of last received message
    void nullTerminateReceiveBuffer(Uint64_t validBytesTransferred) {
        m_receiveBuffer[validBytesTransferred] = 0;
    }

    /// @brief Pack a message to be sent
    /// @return the number of bytes packed
    virtual Uint32_t packMessageForSend(TransportMessageInterface* message, Uint32_t index) override {
        const Uint64_t sizeToWrite = message->getSerializedSizeInBytes();
        assert(SendMessageBuffer::GetMaxSizeBytes() >= static_cast<Uint64_t>(sizeToWrite + index) && "Max size in bytes exceeded");
        Uint32_t bytesPacked = message->pack(m_sendBuffer.data(), index);
        return bytesPacked;
    }

    SendMessageBuffer m_sendBuffer{ 0 }; ///< Buffer for sending a message. Can store multiple messages, but their aggregated size cannot exceed max allowed size for a single message
    ReceiveMessageBuffer m_receiveBuffer{ 0 }; ///< Buffer for receiving a message. Stores one message  as allowed by NumAllowedSimultaneousReceives

};

} // End rev namespace
