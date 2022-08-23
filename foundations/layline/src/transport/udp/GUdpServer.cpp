#include "layline/transport/udp/GUdpServer.h"
#include "layline/transport/udp/GUdpListenerTransport.h"

namespace rev {

UdpServer::UdpServer() :
    ServerInterface()
{
}

void UdpServer::addTransport(TransportMailboxInterfaceBase* mailbox, const NetworkEndpoint& endpoint, Uint64_t sendTimeMicroseconds)
{
    /// Create transport and add to internal list
    auto udpTransport = std::make_shared<UdpListenerTransport>(mailbox, m_thread);
    m_transports.emplace_back(udpTransport);

    /// Set up transport to receive messages
    udpTransport->bindForListen(endpoint, true);
    udpTransport->startReceive();
}


} // End rev namespace

