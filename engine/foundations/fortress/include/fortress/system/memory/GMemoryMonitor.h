#pragma once
/*
@author  David Robert Nadeau
@see http://NadeauSoftware.com/
@see https://stackoverflow.com/questions/142508/how-do-i-check-os-with-a-preprocessor-directive
License: Creative Commons Attribution 3.0 Unported License
http://creativecommons.org/licenses/by/3.0/deed.en_US
 */

#include "fortress/system/GSystemPlatform.h"
#include "fortress/numeric/GSizedTypes.h"

#if defined(G_SYSTEM_PLATFORM_WINDOWS)
/// Windows 32-bit and 64-bit
#define NOMINMAX // Prevent max from being declared a macro
#include <windows.h>
#include <psapi.h>

#elif defined(G_SYSTEM_PLATFORM_UNIX) || defined(G_SYSTEM_PLATFORM_APPLE)
/// Unix (Linux, *BSD, but Mac OS X)
#include <unistd.h>
#include <sys/resource.h>

#if defined(G_SYSTEM_PLATFORM_APPLE)
    /// Mac OS X
    #include <mach/mach.h>

#elif defined(G_SYSTEM_PLATFORM_AIX_UNIX) || defined(G_SYSTEM_PLATFORM_SUN_UNIX)
    /// IBM proprietary Unix
    #include <fcntl.h>
    #include <procfs.h>

#elif defined(G_SYSTEM_PLATFORM_LINUX)
    /// Linux
    #include <stdio.h>

#endif

#else
    #error "Cannot define GetPeakRSS( ) or GetCurrentRSS( ) for an unknown OS."
#endif

namespace rev {

/// @class MemoryMonitor
class MemoryMonitor {
public:

    /// @note Most operating systems report RAM using mebibytes (1MB = 1024 *1024 bytes)
    /// @see https://wiki.ubuntu.com/UnitsPolicy
    static constexpr Uint64_t s_numberOfBytesInKb{ 1024 };///< Number of bytes in a kilobyte
    static constexpr Uint64_t s_numberOfBytesInMb{ 1024 * 1024 };///< Number of kilobytes in a megabyte


    /// @brief Returns the peak (maximum so far) resident set size (physical memory use) measured in bytes, 
    /// or zero if the value cannot be determined on this O
     /// @note Usage: Uint64_t currentSize = GetPeakRSS()
    static Uint64_t GetPeakRSS()
    {
#if defined(G_SYSTEM_PLATFORM_WINDOWS)
        /* Windows -------------------------------------------------- */
        PROCESS_MEMORY_COUNTERS info;
        GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
        return (Uint64_t)info.PeakWorkingSetSize;

#elif defined(G_SYSTEM_PLATFORM_SUN_UNIX) || defined(G_SYSTEM_PLATFORM_AIX_UNIX)
        /* AIX and Solaris ------------------------------------------ */
        struct psinfo psinfo;
        int fd = -1;
        if ((fd = open("/proc/self/psinfo", O_RDONLY)) == -1)
            return (Uint64_t)0L;      /* Can't open? */
        if (read(fd, &psinfo, sizeof(psinfo)) != sizeof(psinfo))
        {
            close(fd);
            return (Uint64_t)0L;      /* Can't read? */
        }
        close(fd);
        return (Uint64_t)(psinfo.pr_rssize * 1024L);

#elif defined(G_SYSTEM_PLATFORM_UNIX) || defined(G_SYSTEM_PLATFORM_APPLE)
        /* BSD, Linux, and OSX -------------------------------------- */
        struct rusage rusage;
        getrusage(RUSAGE_SELF, &rusage);
#if defined(__APPLE__) && defined(__MACH__)
        return (Uint64_t)rusage.ru_maxrss;
#else
        return (Uint64_t)(rusage.ru_maxrss * 1024L);
#endif

#else
        /* Unknown OS ----------------------------------------------- */
        return (Uint64_t)0L;          /* Unsupported. */
#endif
    }

    
    /// @brief  Returns the current resident set size (physical memory use) measured in bytes, 
    /// or zero if the value cannot be determined on this OS.
    static Uint64_t GetCurrentRSS()
    {
#if defined(G_SYSTEM_PLATFORM_WINDOWS)
        /* Windows -------------------------------------------------- */
        PROCESS_MEMORY_COUNTERS info;
        GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
        return (Uint64_t)info.WorkingSetSize;

#elif defined(G_SYSTEM_PLATFORM_APPLE)
        /* OSX ------------------------------------------------------ */
        struct mach_task_basic_info info;
        mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
        if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO,
            (task_info_t)&info, &infoCount) != KERN_SUCCESS)
            return (Uint64_t)0L;      /* Can't access? */
        return (Uint64_t)info.resident_size;

#elif defined(G_SYSTEM_PLATFORM_LINUX)
        /* Linux ---------------------------------------------------- */
        long rss = 0L;
        FILE* fp = NULL;
        if ((fp = fopen("/proc/self/statm", "r")) == NULL)
            return (Uint64_t)0L;      /* Can't open? */
        if (fscanf(fp, "%*s%ld", &rss) != 1)
        {
            fclose(fp);
            return (Uint64_t)0L;      /* Can't read? */
        }
        fclose(fp);
        return (Uint64_t)rss * (Uint64_t)sysconf(_SC_PAGESIZE);

#else
        /* AIX, BSD, Solaris, and Unknown OS ------------------------ */
        return (Uint64_t)0L;          /* Unsupported. */
#endif
    }

    /// @brief Return the total amount of physical memory, in bytes
    static Uint64_t GetMaxMemory()
    {
#if defined(G_SYSTEM_PLATFORM_WINDOWS)
        /* Windows -------------------------------------------------- */
        MEMORYSTATUSEX status;
        status.dwLength = sizeof(status);
        GlobalMemoryStatusEx(&status);
        return status.ullTotalPhys;

#elif defined(G_SYSTEM_PLATFORM_APPLE)
        /* OSX ------------------------------------------------------ */
        struct mach_task_basic_info info;
        mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
        if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO,
            (task_info_t)&info, &infoCount) != KERN_SUCCESS)
            return (Uint64_t)0L;      /* Can't access? */
        return (Uint64_t)info.resident_size;

#elif defined(G_SYSTEM_PLATFORM_LINUX)
        /* Linux ---------------------------------------------------- */
        long pages = sysconf(_SC_PHYS_PAGES);
        long page_size = sysconf(_SC_PAGE_SIZE);
        return pages * page_size;

#else
        /* AIX, BSD, Solaris, and Unknown OS ------------------------ */
        return (Uint64_t)0L;          /* Unsupported. */
#endif
    }

    /// @brief  Return the maximum memory allowed by the system in bytes
    /// @param[in] fraction Multiplicative fraction to scale return value for max memory
    static Uint64_t GetMaxMemory(double fraction)
    {
        return GetMaxMemory() * fraction;
    }

    static Uint64_t GetMaxMemoryMb(double fraction = 1.0)
    {
        return Uint64_t(GetMaxMemory(fraction) / s_numberOfBytesInMb);
    }
};

} // End rev namespace
