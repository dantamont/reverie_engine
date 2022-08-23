#pragma once

#include "fortress/time/GExpireTimer.h"
#include "layline/transport/GTransportInterface.h"

namespace rev {


/// @class SteadyTransport
/// @brief Transport that continually sends messages from mailbox, checking for duplicates
/// @tparam AsioProtocolType the class used to represent a ASIO networking protocol type
/// @detail Since the logic to continually send messages and check for messages in buffer establishes
/// a fundamental mailbox behavior that may not be ideal in all Transport use cases, it has been 
/// relegated to this child class of TransportInterface
template<typename AsioProtocolType>
class SteadyTransport: public SendTransportInterface<AsioProtocolType> {
public:

    SteadyTransport(TransportMailboxInterfaceBase* mailbox, const std::shared_ptr<TransportThread>& thread, Uint64_t expireTimeMicroseconds) :
        SendTransportInterface(mailbox, thread),
        m_sendTimer(expireTimeMicroseconds)
    {
    }
    SteadyTransport(TransportMailboxInterfaceBase* mailbox, Uint64_t expireTimeMicroseconds) :
        SendTransportInterface(mailbox, std::make_shared<TransportThread>()),
        m_sendTimer(expireTimeMicroseconds)
    {
    }

    virtual ~SteadyTransport() = default;

protected:

    /// @brief Overrideable logic to be called before an asynchronous send
    virtual void onStartSend() override {
        m_sendTimer.restart();

        // Pack message for send
        m_mailbox->packSend();
    }

    /// @brief Overrideable logic for the completion handler that is called on asynchronous send
    virtual void onHandleSend(Uint64_t bytesTransferred) override {
        // Continue sending
        m_sendTimer.waitUntilExpired();
        startSend();
    }

private:

    ExpireTimer m_sendTimer{ 0 }; ///< The timer for setting message send rate. Defaults to no timeout. Asio timers require boost, which is sad
};


} // End rev namespace
 