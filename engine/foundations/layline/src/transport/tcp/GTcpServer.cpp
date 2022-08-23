#include "layline/transport/tcp/GTcpServer.h"
#include "layline/transport/tcp/GTcpClient.h"

namespace rev {

void TcpServer::addListenerTransport(TransportMailboxInterfaceBase* mailbox, const NetworkEndpoint& endpoint)
{
    // Create new listener transport
    TcpSessionTransport::TcpSessionPointer newTransport = TcpSessionTransport::Create(mailbox, m_thread);
    m_transports.emplace_back(newTransport);

    // Set up transport to accept a connection and receive messages
    newTransport->bindForListen(endpoint, true/*unused*/);
}

void rev::TcpServer::addSendTransport(TransportMailboxInterfaceBase* mailbox, const NetworkEndpoint& endpoint, Uint64_t sendTimeMicroseconds, Int32_t clientConnectionTimeout)
{
    // Create new TcpClient transport
    std::shared_ptr<TcpClient> newTransport = std::make_shared<TcpClient>(mailbox, m_thread, sendTimeMicroseconds);
    m_transports.emplace_back(newTransport);

    // Set up transport to  send messages
    newTransport->connectRemotely(endpoint, clientConnectionTimeout);
    newTransport->startSend();
}


} // End rev namespae
