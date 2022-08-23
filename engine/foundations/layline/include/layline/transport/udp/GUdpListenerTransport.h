#pragma once

#include "layline/transport/GSteadyTransport.h"
#include "layline/transport/udp/GUdpMessage.h"

namespace rev {

/// @class UdpTransport
/// @brief Class representing a UDP (user datagram protocol) transport
/// @detail The UDP transport is an abstraction for the sending and receiving of messages
class UdpListenerTransport: public ReceiveTransportInterface<typename asio::ip::udp> {
public:

    using ReceiveTransportInterface::ReceiveTransportInterface;

    /// @copydoc TransportInterface::bindForListen
    virtual void bindForListen(const NetworkEndpoint& endpoint, bool bind) override;

private:

    /// @brief Update connections on a message receive
    void updateConnections() override;

    /// @brief Overrideable logic for receiving messages
    virtual void receive() override;

    /// @brief Launch an asynchronous receive
    virtual void asyncRead(HandlerFunctionType functionHandle) override;

    NetworkEndpoint m_lastReceivedEndpoint; ///< The endpoint from which a message was most recently received
};


} // End rev namespace
