#include "GbClock.h"
#include "../../core/GbCoreEngine.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
Clock::Clock(CoreEngine* coreEngine):
    Manager(coreEngine, "Clock")
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Clock::~Clock()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}