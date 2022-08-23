#include "layline/transport/tcp/GTcpSessionTransport.h"

namespace rev {

void TcpSessionTransport::bindForListen(const NetworkEndpoint& endpoint, bool)
{
    // Each TCP session only ever has a single socket connection, so make one
    m_socketHolder.addConnection(std::make_shared<TransportConnection>());

    // Create acceptor to accept connections
    m_acceptor = std::make_unique<asio::ip::tcp::acceptor>(
        thread().context(),
        endpoint.toAsio<typename asio::ip::tcp>());

    // Accept connection, updating remote endpoint 
    m_acceptor->async_accept(
        socketHolder().socket(),
        m_socketHolder.connections().back()->remoteEndpoint().toAsio<asio::ip::tcp>(), // WRITES the endpoint of the remote peer to this endpoint
        [=]
        (const asio::error_code& error)
        {
            if (!error) {
                handleAccept(error);
            }
            else {
                TransportInterface::CatchErrorCode(error);
            }
        }
    );
}

void TcpSessionTransport::updateConnections()
{
    /// @todo Update an internal connections list
    /// Right now, connections just stay open indefinitely

//#ifdef DEBUG_MODE
//    // Compare previous received endpoint with current, to ensure it hasn't changed
//    const NetworkEndpoint& endpoint = m_socketHolder.connections().back()->remoteEndpoint();
//    if (endpoint.isValid()) {
//        m_lastReceivedEndpoint.refresh<asio::ip::tcp>();
//        assert(m_lastReceivedEndpoint == endpoint && "Endpoint mismatch");
//    }
//#endif
//
//    // Update connection
//    const std::shared_ptr<TransportConnection>& connectionPtr = m_socketHolder.connections().back();
//    if (!connectionPtr->remoteEndpoint().isValid()) {
//        m_lastReceivedEndpoint.refresh<asio::ip::tcp>();
//        connectionPtr->remoteEndpoint() = m_lastReceivedEndpoint;
//    }

}

void TcpSessionTransport::receive()
{
    HandlerFunctionType receiveHandler = 
        [me = this] 
        (std::error_code const& error, Uint64_t bytesTransferred)
        {
            // Called when all data across all chains specified by completion condition is sent out
            me->handleReceive(error, bytesTransferred);
        };

    // Listen in the background for a new request
    asyncRead(receiveHandler);
}

void TcpSessionTransport::asyncRead(HandlerFunctionType functionHandle)
{
    // Read the bytes of the next message header
    m_socketHolder.asyncRead(
        m_mailbox->getWriteableReceiveBuffer(),
        functionHandle
    );
}

void TcpSessionTransport::handleAccept(const std::error_code& error)
{
    if (!error)
    {
        // Update remote endpoint in connection
        TransportConnection& connection = *m_socketHolder.connections().back();
        connection.remoteEndpoint().refresh<asio::ip::tcp>();

        // Update local endpoint in socket holder
        m_socketHolder.localEndpoint() = NetworkEndpoint::FromAsio(m_socketHolder.socket().local_endpoint());

        // Handling message here
        // So, what happens is the server handlesAccept (gets a connection from a client),
        // and then listens over the connection
        startReceive();
    }
    else {
        TransportInterface::CatchErrorCode(error);
    }

    /// This is where any logic could go to open other connections
}

} // End rev namespace
