#pragma once

#include <iostream>
#include <mutex>
#include <sstream>
#include "fortress/types/GSizedTypes.h"

namespace rev {


/// @brief Thread safe cout class
/// @example Example of use:
/// ThreadSafeCout{} << "Hello world!" << std::endl;
class ThreadSafeCout : public std::ostringstream
{
public:
    ThreadSafeCout() = default;

    ~ThreadSafeCout()
    {
        std::lock_guard<std::mutex> guard(m_printMutex);
        std::cout << this->str();
    }

private:
    static std::mutex m_printMutex;
};

} // End namespaces

