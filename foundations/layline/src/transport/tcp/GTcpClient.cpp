#include "layline/transport/tcp/GTcpClient.h"

namespace rev {


TcpClient::TcpClient(TransportMailboxInterfaceBase* mailbox, Uint64_t expireTimeMicroseconds):
    SteadyTransport(mailbox, std::make_shared<TransportThread>(), expireTimeMicroseconds)
{
}

TcpClient::TcpClient(TransportMailboxInterfaceBase* mailbox, const std::shared_ptr<TransportThread>& thread, Uint64_t expireTimeMicroseconds):
    SteadyTransport(mailbox, thread, expireTimeMicroseconds)
{
}

void TcpClient::send()
{
    asyncWrite(
        [me = this] /// This function is a receive completion handler
    (std::error_code const& error, Uint64_t bytesTransferred)
        {
            /// The mailbox will keep belligerently sending messages unless the send size is dropped to zero 
            me->m_mailbox->onMessageSent();

            // Called when all data across all chains specified by completion condition is sent out
            me->handleSend(error, bytesTransferred);
        }
    );
}

void TcpClient::asyncWrite(HandlerFunctionType functionHandle)
{
    m_socketHolder.asyncWrite(
        asio::buffer(m_mailbox->sendData(), m_mailbox->getSizeOfPackedSendMessage()),
        functionHandle
    );
}

} // End rev namespace
