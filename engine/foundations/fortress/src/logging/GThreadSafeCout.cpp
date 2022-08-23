#include "fortress/logging/GThreadSafeCout.h"

namespace rev {

std::mutex ThreadSafeCout::m_printMutex{}; ///< Mutex for thread-guarding std::cout calls


} /// End namespaces