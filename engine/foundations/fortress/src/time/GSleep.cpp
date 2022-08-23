#include "fortress/time/GSleep.h"
#include <algorithm>

namespace rev {


std::chrono::nanoseconds Sleep::GetBusySleepOverhead()
{
    using namespace std::chrono;
    constexpr size_t tests = 1001;
    constexpr auto timer = 200us;

    auto init = [&timer]() {
        auto end = steady_clock::now() + timer;
        while (steady_clock::now() < end);
    };

    time_point<steady_clock> start;
    nanoseconds dur[tests];

    for (auto& d : dur) {
        start = steady_clock::now();
        init();
        d = steady_clock::now() - start - timer;
    }
    std::sort(std::begin(dur), std::end(dur));

    // Get the median value or something a little less
    return dur[tests / 3];
}

const std::chrono::nanoseconds Sleep::s_busySleepOverhead = Sleep::GetBusySleepOverhead();

} // End namespace 
