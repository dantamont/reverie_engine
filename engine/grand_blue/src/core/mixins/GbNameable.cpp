#include "GbNameable.h"

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
Nameable::Nameable(const GString & name, NameMode mode):
    m_nameMode(mode)
{
    setName(name);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Nameable::setName(const GString & name)
{
    switch (m_nameMode) {
    case kCaseSensitive:
        m_name = name;
        break;
    case kCaseInsensitive:
        m_name = name.asLower();
        break;
    default:
        m_name = name;
    }
}



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
