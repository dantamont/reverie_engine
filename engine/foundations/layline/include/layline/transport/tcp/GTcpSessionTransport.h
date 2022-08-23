#pragma once

#include "layline/transport/GTransportInterface.h"

namespace rev {

/// @class TcpSessionTransport
/// @brief Class representing a TCP (transmission control protocol) transport for a communication session
/// @todo Session transport only receives not, so don't subclass from steady transport
class TcpSessionTransport : public ReceiveTransportInterface<asio::ip::tcp> {
public:
    typedef std::shared_ptr<TcpSessionTransport> TcpSessionPointer;

    /// @brief Create a TCP/IP transport (session). 
    /// @detail Only will ever be instantiated as a shared pointer 
    /// so that lifetime is managed automatically
    static TcpSessionPointer Create(TransportMailboxInterfaceBase* mailbox, const std::shared_ptr<TransportThread>& thread)
    {
        return TcpSessionPointer(new TcpSessionTransport(mailbox, thread));
    }

    virtual ~TcpSessionTransport() = default;

    /// @brief Bind the transport to a local endpoint for receiving messages
    /// @param[in] endpoint the endpoint to connect to
    /// @note Unsafe to call in constructor of base classes, since it is virtual
    virtual void bindForListen(const NetworkEndpoint& endpoint, bool bind) override;

protected:

    using ReceiveTransportInterface::ReceiveTransportInterface;

    /// @brief Update connections on a message receive
    virtual void updateConnections() override;

    virtual void receive() override;

    /// @brief Perform an asynchronous read
    void asyncRead(HandlerFunctionType functionHandle) override;

private:

    /// @brief Called when asynchronous accept operation is initiated by transport
    /// @detail Services the client request
    /// @tparam TransportT the type of transport to begin communication to a network as (either a server or client)
    void handleAccept(const std::error_code& error);

    /// @detail The acceptor is sort of a passive socket, which waits for the other endpoint's socket to request a connection,
    /// and will branch off a new socket that is connected to the remote endpoint, and delegate all further I/O work to it
    std::unique_ptr<asio::ip::tcp::acceptor> m_acceptor{ nullptr }; ///< Socket acceptor, for accepting new socket connections
};

} // End rev namespace
