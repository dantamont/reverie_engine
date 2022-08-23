#include "core/GManager.h"
#include "core/GCoreEngine.h"

namespace rev{


Manager::Manager(CoreEngine* core, const GString& name):
    ManagerInterface(name),
    m_engine(core)
{
}

Manager::~Manager()
{
}


// End namespaces        
}