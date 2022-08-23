#pragma once

#include "layline/transport/GServerInterface.h"
#include "layline/transport/tcp/GTcpSessionTransport.h"

namespace rev {

/// @class TcpServer
/// @brief Class representing a TCP/IP (transmission control protocol) server (listener)
/// @note Only one TCP/IP server can listen per IP/port, due to how TCP/IP connections work
/// @note On terminology:
/// An endpoint is a ip address and a port.
/// A socket is an abstraction on the concept of a channel in the engineering sense of the word. 
///   It's like a wire between two telephones, or the air between two antenna. 
///   It's defined by two endpoints the sender and the receiver.
/// An acceptor sets up sockets. Can be thought of as a listenter, or receiver.
/// @see https://stackoverflow.com/questions/7039057/boost-asio-for-sync-server-keeping-tcp-session-open-with-google-proto-buffers
class TcpServer: public ServerInterface<asio::ip::tcp> {
public:

    TcpServer() = default;

    /// @brief The TCP/IP transports being used by this server
    const std::vector<std::shared_ptr<TransportInterface<asio::ip::tcp>>>& transports() const {
        return m_transports;
    }

    /// @brief Add a listener transport, representing a communication channel through a local endpoint
    /// @note There shouldn't be a compelling reason to add transports after initialization
    /// @param[in] mailbox The mailbox that will receive messages through this transport
    /// @param[in] endpoint The endpoint for the transport to listen over
    void addListenerTransport(TransportMailboxInterfaceBase* mailbox, const NetworkEndpoint& endpoint);

    /// @brief Add a send transport, representing a communication channel through a local endpoint
    /// @note There shouldn't be a compelling reason to add transports after initialization
    /// @param[in] mailbox The mailbox that will receive messages through this transport
    /// @param[in] endpoint The endpoint for the transport to send over
    /// @param[in] sendTimeMicroseconds The rate at which this transport will send messages
    /// @param[in] clientConnectionTimeout how long to wait until giving up connection attempt. Negative values mean no reattempts will be made
    void addSendTransport(TransportMailboxInterfaceBase* mailbox, const NetworkEndpoint& endpoint, Uint64_t sendTimeMicroseconds = s_defaultSendTimeMicroseconds, Int32_t clientConnectionTimeout = -1);

private:

    std::vector<std::shared_ptr<TransportInterface<asio::ip::tcp>>> m_transports; ///< The TCP/IP transports being used by this server
};

} // End rev namespace
