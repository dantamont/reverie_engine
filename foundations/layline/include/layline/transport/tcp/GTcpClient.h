#pragma once

#include <iostream>
#include <array>
#include <asio.hpp>
#include "layline/transport/GSteadyTransport.h"

namespace rev {

/// @class TcpClient
/// @brief Class representing a TCP (transmission control protocol) client
/// @detail A client is for sending TCP/IP (internet protocol) messages to a server
/// @see https://stackoverflow.com/questions/7039057/boost-asio-for-sync-server-keeping-tcp-session-open-with-google-proto-buffers
class TcpClient: public SteadyTransport<asio::ip::tcp> {
public:
    TcpClient(TransportMailboxInterfaceBase* mailbox, Uint64_t expireTimeMicroseconds);
    TcpClient(TransportMailboxInterfaceBase* mailbox, const std::shared_ptr<TransportThread>& thread, Uint64_t expireTimeMicroseconds);
    ~TcpClient() = default;

    /// @brief Connect the transport to a remote endpoint for sending/receiving messages
    /// @param[in] remoteEndpoint the remote endpoint to connect to
    /// @param[in] clientConnectionTimeout how long to wait until giving up connection attempt. Negative values mean no reattempts will be made
    void connectRemotely(const NetworkEndpoint& remoteEndpoint, Int32_t clientConnectionTimeout = -1) {
        m_socketHolder.connect(remoteEndpoint, clientConnectionTimeout);
    }

protected:

    /// @brief Overrideable logic to send a message
    virtual void send() override;

    /// @brief Perform an asynchronous write
    virtual void asyncWrite(HandlerFunctionType functionHandle) override;
};

} // End rev namespace
