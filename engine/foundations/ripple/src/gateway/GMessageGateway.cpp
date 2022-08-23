#include "ripple/network/gateway/GMessageGateway.h"

namespace rev {

GMessageGateway::~GMessageGateway()
{
    close();
}

void GMessageGateway::close()
{
    m_tcpServer.shutdown();
}

void GMessageGateway::initializeServer(const ServerSettings& serverSettings)
{
    m_tcpServer.addListenerTransport(&m_mailbox, serverSettings.m_listenerEndpoint);
    m_tcpServer.addSendTransport(&m_mailbox, 
        serverSettings.m_destinationEndpoint, 
        serverSettings.m_sendTimeMicroseconds,
        serverSettings.m_clientConnectionTimeoutMicroSeconds);
    m_tcpServer.run();
}

void GMessageGateway::clearMessageFromSendQueue(Uint64_t messageId)
{
    m_mailbox.popMessage(messageId);
}

void rev::GMessageGateway::queueMessageForSend(TransportMessageInterface* message)
{
    // Allow queueing from multiple threads
    static std::mutex s_reentrantMutex;
    std::unique_lock lock(s_reentrantMutex);

    // Confirm that message header is constructed, and construct if not
    GMessage* msg = static_cast<GMessage*>(message);
    if (GMessage::MessageType::eInvalid == static_cast<GMessage::MessageType>(msg->header().m_messageType)) {
        msg->postConstruction();
    }
    m_mailbox.push(message);
}

GMessage* GMessageGateway::popReceivedMessage()
{
    return static_cast<GMessage*>(m_mailbox.retrieve());
}

} // End rev
