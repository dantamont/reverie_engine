#pragma once

#include "fortress/numeric/GSizedTypes.h"
#include "layline/transport/GServerInterface.h"

namespace rev {

class TransportMailboxInterfaceBase;
class UdpListenerTransport;

/// @class UdpServer
/// @brief Class representing a UDP (user datagram protocol) server
/// @detail Server is for receiving UDP messages
/// @note Multiple UDP servers can subscribe to the same listening port
class UdpServer: public ServerInterface<asio::ip::udp> {
public:
    UdpServer();
    ~UdpServer() = default;

    /// @brief Add a transport, representing a communication channel through a local endpoint
    /// @note There shouldn't be a compelling reason to add transports after initialization
    /// @param[in] mailbox The mailbox that will receive messages through this transport
    /// @param[in] endpoint The endpoint for the transport to listen over
    /// @param[in] sendTimeMicroseconds The rate at which this transport will send messages
    void addTransport(TransportMailboxInterfaceBase* mailbox, const NetworkEndpoint& endpoint, Uint64_t sendTimeMicroseconds = s_defaultSendTimeMicroseconds);

    /// @brief The UDP transports for this server
    const std::vector<std::shared_ptr<UdpListenerTransport>>& transports() const { return m_transports; }

private:

    std::vector<std::shared_ptr<UdpListenerTransport>> m_transports; ///< The transports managed by the server
};

} // End rev namespace
