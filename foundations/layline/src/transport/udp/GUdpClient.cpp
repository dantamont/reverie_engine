#include "layline/transport/udp/GUdpClient.h"

namespace rev {

UdpClient::UdpClient(TransportMailboxInterfaceBase* mailbox, Uint64_t expireTimeMicroseconds):
    SteadyTransport(mailbox, expireTimeMicroseconds),
    m_udpClientIndex(s_count++)
{
}

UdpClient::UdpClient(TransportMailboxInterfaceBase* mailbox, const std::shared_ptr<TransportThread>& thread, Uint64_t expireTimeMicroseconds) :
    SteadyTransport(mailbox, thread, expireTimeMicroseconds),
    m_udpClientIndex(s_count++)
{
}


void UdpClient::onStartSend()
{
    SteadyTransport::onStartSend();
}

void UdpClient::onHandleSend(Uint64_t bytesTransferred)
{
    SteadyTransport::onHandleSend(bytesTransferred);
    //ThreadSafeCout{} << "UDP client " << m_udpClientIndex << " sent message\n"
    //    << bytesTransferred << "bytes" << "\n";
}

void UdpClient::send()
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

void UdpClient::asyncWrite(HandlerFunctionType functionHandle)
{
    m_socketHolder.asyncSend(
        asio::buffer(m_mailbox->sendData(), m_mailbox->getSizeOfPackedSendMessage()),
        m_socketHolder.connections()[m_sendConnectionIndex]->remoteEndpoint().toAsio<typename asio::ip::udp>(),
        functionHandle
    );
}

std::atomic<Uint32_t> UdpClient::s_count = 0;

} // End rev namespace
