#include "time/GTime.h"
#include "units/GUnits.h"

namespace rev {


Time::Time()
{
}

Time::~Time()
{
}

Float64_t Time::getFractionalSeconds() const
{
    return Float64_t(m_second) + MicroSecToSec(m_microSeconds);
}




// End namespaces        
}