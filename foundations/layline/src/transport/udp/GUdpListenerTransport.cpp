#include "layline/transport/udp/GUdpListenerTransport.h"

namespace rev {

void UdpListenerTransport::bindForListen(const NetworkEndpoint& endpoint, bool bind)
{
    m_socketHolder.bindForListen(endpoint, bind);
}

void UdpListenerTransport::updateConnections()
{
    // Refresh last received endpoint so that it's value matches ASIO value
    bool refreshed = m_lastReceivedEndpoint.refresh<typename asio::ip::udp>();

    if (refreshed) {
        m_socketHolder.addConnection(m_lastReceivedEndpoint);
    }
}

void UdpListenerTransport::receive()
{
    // Listen in the background for a new request
    asyncRead(
        [me = this] /// This function is a receive completion handler
    (std::error_code const& error, Uint64_t bytesTransferred)
        {
            // Called when all data across all chains specified by completion condition is sent out
            me->handleReceive(error, bytesTransferred);
        }
    );
}

void UdpListenerTransport::asyncRead(HandlerFunctionType functionHandle)
{
    m_socketHolder.asyncReceive(
        m_mailbox->getWriteableReceiveBuffer(),
        m_lastReceivedEndpoint.toAsio<typename asio::ip::udp>(),
        functionHandle
    );
}


} // End rev namespace
