#include "ripple/network/gateway/GMessageGarbageCollector.h"
#include "ripple/network/messages/GMessage.h"
#include "fortress/system/memory/GGarbageCollector.h"

namespace rev {

void MessageGarbageCollector::deleteStaleMessages()
{
    auto& collector = GarbageCollector<GMessage>::Instance();
    collector.update();
}

void MessageGarbageCollector::deferredDelete(GMessage* message)
{
    auto& collector = GarbageCollector<GMessage>::Instance();
    collector.deferredDelete(message, m_garbageCollectTimeUs);
}

} // End rev
