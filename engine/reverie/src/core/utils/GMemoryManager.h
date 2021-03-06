#ifndef GB_MEMORY_MANAGER_H
#define GB_MEMORY_MANAGER_H

/////////////////////////////////////////////////////////////////////////////////////////////
/*
 * Author:  David Robert Nadeau
 * Site:    http://NadeauSoftware.com/
 * License: Creative Commons Attribution 3.0 Unported License
 *          http://creativecommons.org/licenses/by/3.0/deed.en_US
 */
 /////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
#include <QtGlobal>
#include <QString>
#if defined(_WIN32)
#define NOMINMAX // Prevent max from being declared a macro
#include <windows.h>
#include <psapi.h>

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#include <sys/resource.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <mach/mach.h>

#elif (defined(_AIX) || defined(__TOS__AIX__)) || (defined(__sun__) || defined(__sun) || defined(sun) && (defined(__SVR4) || defined(__svr4__)))
#include <fcntl.h>
#include <procfs.h>

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
#include <stdio.h>

#endif

#else
#error "Cannot define getPeakRSS( ) or getCurrentRSS( ) for an unknown OS."
#endif

namespace rev {
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Casting shortcuts
#define S_CAST std::static_pointer_cast
#define D_CAST std::dynamic_pointer_cast

/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Template for making a shared pointer of a class with only protected constructors
template<typename Obj, typename... Args>
inline std::shared_ptr<Obj> prot_make_shared(Args&&... args)
{
    struct helper : public Obj
    {
        helper(Args&&... args)
            : Obj{ std::forward< Args >(args)... }
        {}
    };

    return std::make_shared<helper>(std::forward< Args >(args)...);
}

/// @brief Template for making a unique pointer of a class with only protected constructors
template<typename Obj, typename... Args>
inline std::unique_ptr<Obj> prot_make_unique(Args&&... args)
{
    struct helper : public Obj
    {
        helper(Args&&... args)
            : Obj{ std::forward< Args >(args)... }
        {}
    };

    return std::make_unique<helper>(std::forward< Args >(args)...);
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class MemoryManager
class MemoryManager {
public:
    /**
     * Returns the peak (maximum so far) resident set size (physical
     * memory use) measured in bytes, or zero if the value cannot be
     * determined on this OS.
     */
     /// @note Usage: size_t currentSize = GET_PEAK_RSS()
    static size_t GET_PEAK_RSS()
    {
#if defined(_WIN32)
        /* Windows -------------------------------------------------- */
        PROCESS_MEMORY_COUNTERS info;
        GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
        return (size_t)info.PeakWorkingSetSize;

#elif (defined(_AIX) || defined(__TOS__AIX__)) || (defined(__sun__) || defined(__sun) || defined(sun) && (defined(__SVR4) || defined(__svr4__)))
        /* AIX and Solaris ------------------------------------------ */
        struct psinfo psinfo;
        int fd = -1;
        if ((fd = open("/proc/self/psinfo", O_RDONLY)) == -1)
            return (size_t)0L;      /* Can't open? */
        if (read(fd, &psinfo, sizeof(psinfo)) != sizeof(psinfo))
        {
            close(fd);
            return (size_t)0L;      /* Can't read? */
        }
        close(fd);
        return (size_t)(psinfo.pr_rssize * 1024L);

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
        /* BSD, Linux, and OSX -------------------------------------- */
        struct rusage rusage;
        getrusage(RUSAGE_SELF, &rusage);
#if defined(__APPLE__) && defined(__MACH__)
        return (size_t)rusage.ru_maxrss;
#else
        return (size_t)(rusage.ru_maxrss * 1024L);
#endif

#else
        /* Unknown OS ----------------------------------------------- */
        return (size_t)0L;          /* Unsupported. */
#endif
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    /**
     * Returns the current resident set size (physical memory use) measured
     * in bytes, or zero if the value cannot be determined on this OS.
     */
    static size_t GET_CURRENT_RSS()
    {
#if defined(_WIN32)
        /* Windows -------------------------------------------------- */
        PROCESS_MEMORY_COUNTERS info;
        GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
        return (size_t)info.WorkingSetSize;

#elif defined(__APPLE__) && defined(__MACH__)
        /* OSX ------------------------------------------------------ */
        struct mach_task_basic_info info;
        mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
        if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO,
            (task_info_t)&info, &infoCount) != KERN_SUCCESS)
            return (size_t)0L;      /* Can't access? */
        return (size_t)info.resident_size;

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
        /* Linux ---------------------------------------------------- */
        long rss = 0L;
        FILE* fp = NULL;
        if ((fp = fopen("/proc/self/statm", "r")) == NULL)
            return (size_t)0L;      /* Can't open? */
        if (fscanf(fp, "%*s%ld", &rss) != 1)
        {
            fclose(fp);
            return (size_t)0L;      /* Can't read? */
        }
        fclose(fp);
        return (size_t)rss * (size_t)sysconf(_SC_PAGESIZE);

#else
        /* AIX, BSD, Solaris, and Unknown OS ------------------------ */
        return (size_t)0L;          /* Unsupported. */
#endif
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    static qint64 GET_MAX_MEMORY()
    {

#if defined(Q_OS_ANDROID)
        return 2.0 * 1024 * 1024 * 1024;
#elif defined(Q_OS_BLACKBERRY)
        return 2.0 * 1024 * 1024 * 1024;
#elif defined(Q_OS_IOS)
        return 2.0 * 1024 * 1024 * 1024;
#elif defined(Q_OS_MACOS)
        int mib[2] = { CTL_HW, HW_MEMSIZE };
        u_int namelen = sizeof(mib) / sizeof(mib[0]);
        uint64_t size;
        size_t len = sizeof(size);
#elif defined(Q_OS_WIN)
        // Return available memory
        MEMORYSTATUSEX status;
        status.dwLength = sizeof(status);
        GlobalMemoryStatusEx(&status);
        return status.ullTotalPhys;
#elif defined(Q_OS_LINUX)
        try {
            // Return available memory
            long pages = sysconf(_SC_AVPHYS_PAGES);
            long page_size = sysconf(_SC_PAGE_SIZE);
            return pages * page_size;
        }
        catch {
            // Return total memory
            long pages = sysconf(_SC_PHYS_PAGES);
            long page_size = sysconf(_SC_PAGE_SIZE);
            return pages * page_size;
        }
#elif defined(Q_OS_TVOS)
        return 2.0 * 1024 * 1024 * 1024;
#elif defined(Q_OS_WATCHOS)
        return 2.0 * 1024 * 1024 * 1024;
#elif defined(Q_OS_WINCE)
        return 2.0 * 1024 * 1024 * 1024;
#elif defined(Q_OS_UNIX)
        try {
            // Return available memory
            long pages = sysconf(_SC_AVPHYS_PAGES);
            long page_size = sysconf(_SC_PAGE_SIZE);
            return pages * page_size;
        }
        catch {
            // Return total memory
            long pages = sysconf(_SC_PHYS_PAGES);
            long page_size = sysconf(_SC_PAGE_SIZE);
            return pages * page_size;
        }
#else
        return 2.0 * 1024 * 1024 * 1024;
#endif
    }
    /////////////////////////////////////////////////////////////////////////////////////////////
    static qint64 GET_MAX_MEMORY(double fraction)
    {
        return GET_MAX_MEMORY() * fraction;
    }
    /////////////////////////////////////////////////////////////////////////////////////////////
    static qint64 GET_MAX_MEMORY_MB(double fraction = 1.0)
    {
        return qint64(GET_MAX_MEMORY(fraction) / (1024.0 * 1024));
    }
    /////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Return the maximum available memory of the system in MB
    /// @details Optional argument can be used to obtain fraction of available memory
    static QString GET_MAX_MEMORY_GB_QSTR(double fraction = 1) {
        return QString::number(GET_MAX_MEMORY_MB(fraction) / (1024.0));
    }
    /////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Return the maximum available memory of the system in MB
    /// @details Optional argument can be used to obtain fraction of available memory
    static QString GET_MAX_MEMORY_MB_QSTR(double fraction = 1) {
        return QString::number(GET_MAX_MEMORY_MB(fraction));
    }



};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespace

#endif