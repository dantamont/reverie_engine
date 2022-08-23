#pragma once

#include "layline/transport/tcp/GTcpServer.h"

#include "ripple/network/gateway/GMessageGarbageCollector.h"
#include "ripple/network/messages/GMailbox.h"
#include "ripple/network/messages/GMessagePort.h"

namespace rev {

class NetworkEndpoint;

/// @class GMessageGateway
/// @brief Class for handling inflow and outflow of messages
class GMessageGateway: public GMessagePort {
public:

    /// @brief Settings used to initialize a gateway server with both a sending and receiving transport
    struct ServerSettings {
        NetworkEndpoint m_listenerEndpoint; ///< The local endpoint to bind the listener transport
        NetworkEndpoint m_destinationEndpoint; ///< The remote endpoint for the send transport
        Uint64_t m_sendTimeMicroseconds{ 0 }; ///< Interval at which to send messages
        Int32_t m_clientConnectionTimeoutMicroSeconds{ -1 }; ///< If non-negative, will reattempt connection until timeout.
    };

    GMessageGateway() = default;
    virtual ~GMessageGateway();

    /// @brief Initialize a server the gateway, representing a two communication channels,
    ///    a listener to receive messages, and a sender.
    /// @note There shouldn't be a compelling reason to add transports after initialization
    /// @param[in] listenerEndpoint The endpoint for the transport to listen over
    /// @param[in] destinationEndpoint The endpoint for the transport to send to
    /// @param[in] receiveTimeMicroseconds The rate at which this transport will receive messages
    /// @param[in] sendTimeMicroseconds The rate at which this transport will send messages
    void initializeServer(const ServerSettings& serverSettings);

    /// @brief Close the gateway
    void close();

    /// @brief Clear a message from the send queue
    /// @detail For use when a message will be deleted by the time it is packed for send
    void clearMessageFromSendQueue(Uint64_t messageId);

    /// @brief Push a message to the mailbox for sending
    void queueMessageForSend(TransportMessageInterface* message);

    /// @brief Copy a message to be sent
    /// @details The created message is garbage collected 
    template<typename T>
    void copyAndQueueMessageForSend(const T& message) {
        T* copy = m_garbageCollector.createCollectedMessage(message);
        queueMessageForSend(copy);
    }

    /// @brief Retrieve the number of messages in the receive queue
    inline Uint32_t receiveQueueCount() const {
        return m_mailbox.receiveQueueCount();
    }

    /// @brief Pop a message from the mailbox that was received
    GMessage* popReceivedMessage();

    /// @brief Process the messages received by the gateway
    virtual void processMessages() = 0;

protected:

    using GMessagePort::processMessage;

    MessageGarbageCollector m_garbageCollector; ///< For (optionally) managing the lifetime of messages
    Mailbox m_mailbox; ///< The mailbox for the gateway
    TcpServer m_tcpServer; ///< The main server for the gateway
};

}