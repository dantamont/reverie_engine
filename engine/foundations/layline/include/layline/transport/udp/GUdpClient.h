#pragma once

#include "layline/transport/GSteadyTransport.h"
#include "layline/transport/udp/GUdpMessage.h"

namespace rev {

/// @class UdpClient
/// @brief Class representing a UDP (user datagram protocol) client
/// @detail Client is for sending UDP messages
class UdpClient: public SteadyTransport<asio::ip::udp> {
public:

    UdpClient(TransportMailboxInterfaceBase* mailbox, Uint64_t expireTimeMicroseconds);
    UdpClient(TransportMailboxInterfaceBase* mailbox, const std::shared_ptr<TransportThread>& thread, Uint64_t expireTimeMicroseconds);
    virtual ~UdpClient() = default;

    /// @brief Set the remote endpoint to send messages to
    /// @param[in] connectionIndex the index in the connections list of the connection to use for sending messages
    void setSendConnection(Uint32_t connectionIndex) {
        m_sendConnectionIndex = connectionIndex;
    }


    /// @brief Add a connection to a remote endpoint for sending/receiving messages
    /// @note Doesn't actually connect the socket to the remote endpoint
    /// @param[in] remoteEndpoint the remote endpoint to connect to
    void addRemoteConnection(const NetworkEndpoint& remoteEndpoint) {
        m_socketHolder.addConnection(std::make_shared<TransportConnection>(remoteEndpoint));
    }

protected:

    /// @brief Overrideable logic to be called before an asynchronous send
    virtual void onStartSend() override;

    /// @brief Overrideable logic for the completion handler that is called on asynchronous send
    virtual void onHandleSend(Uint64_t bytesTransferred) override;

    /// @brief Overrideable logic for sending messages
    virtual void send() override;

    /// @brief Perform an asynchronous write
    virtual void asyncWrite(HandlerFunctionType functionHandle) override;

    Uint32_t m_udpClientIndex{ 0 }; ///< The index of this client, for identification purposes
    static std::atomic<Uint32_t> s_count; ///< Number of UDP clients in global scope
    std::atomic<Uint32_t> m_sendConnectionIndex{ 0 }; ///< The index of the connection whose remote endpoint is having messages sent to it
};

} // End rev namespace
