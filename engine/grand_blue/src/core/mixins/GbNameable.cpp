#include "GbNameable.h"

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
Nameable::Nameable(const QString & name, NameMode mode):
    m_nameMode(mode)
{
    setName(name);
}
/////////////////////////////////////////////////////////////////////////////////////////////
const QString& Nameable::getName() const
{
    return m_name;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Nameable::setName(const QString & name)
{
    switch (m_nameMode) {
    case kCaseSensitive:
        m_name = name;
        break;
    case kCaseInsensitive:
        m_name = name.toLower();
        break;
    default:
        m_name = name;
    }
}



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces