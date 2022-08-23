#pragma once

#include <stdio.h>

/// @brief Determination a platform of an operation system
/// @note Fully supported supported only GNU GCC/G++, partially on Clang/LLVM
/// @see https://web.archive.org/web/20191012035921/http://nadeausoftware.com/articles/2012/01/c_c_tip_how_use_compiler_predefined_macros_detect_operating_system#BSD
/// @see https://stackoverflow.com/questions/142508/how-do-i-check-os-with-a-preprocessor-directive
/// @see https://newbedev.com/how-to-detect-reliably-mac-os-x-ios-linux-windows-in-c-preprocessor

#if defined(_WIN32)
    #define G_PLATFORM_NAME "windows" // Windows
    #define G_SYSTEM_PLATFORM_WINDOWS

#elif defined(_WIN64)
    #define G_PLATFORM_NAME "windows" // Windows
    #define G_SYSTEM_PLATFORM_WINDOWS

#elif defined(__CYGWIN__) && !defined(_WIN32)
    #define G_PLATFORM_NAME "windows" // Windows (Cygwin POSIX under Microsoft Window)
    #define G_SYSTEM_PLATFORM_WINDOWS_CYGWIN

#elif defined(__ANDROID__)
    #define G_PLATFORM_NAME "android" // Android (implies Linux, so it must come first)
    #define G_SYSTEM_PLATFORM_ANDROID

#elif defined(__linux__)
    #define G_PLATFORM_NAME "linux" // Debian, Ubuntu, Gentoo, Fedora, openSUSE, RedHat, Centos and other
    #define G_SYSTEM_PLATFORM_UNIX
    #define G_SYSTEM_PLATFORM_LINUX

#elif defined(__APPLE__) && defined(__MACH__) // Apple OSX and iOS (Darwin)
    #define G_SYSTEM_PLATFORM_APPLE
    #include <TargetConditionals.h>

    #if TARGET_IPHONE_SIMULATOR && TARGET_OS_IPHONE
        #define G_PLATFORM_NAME "ios" // Apple iOS in Xcode simulator
        #define G_SYSTEM_PLATFORM_IOS

    #elif TARGET_OS_IPHONE
        #define G_PLATFORM_NAME "ios" // Apple iOS
        #define G_SYSTEM_PLATFORM_IOS

    #elif TARGET_OS_MAC
        /// @note This doesn't need to be as explicit, as TARGET_OS_IPHONE  is a subset of TARGET_OS_MAC
        #include <sys/param.h>
        #define G_PLATFORM_NAME "osx" // Apple OSX
        #define G_SYSTEM_PLATFORM_OSX
    #endif

#elif defined(__unix__) || defined(__unix) || defined(unix)
    #include <sys/param.h>
    #define G_SYSTEM_PLATFORM_UNIX
    #if defined(BSD)
        #define G_PLATFORM_NAME "bsd" // FreeBSD, NetBSD, OpenBSD, DragonFly BSD
        #define G_SYSTEM_PLATFORM_BSD
    #endif

#elif defined(__hpux)
    #define G_PLATFORM_NAME "hp-ux" // HP-UX
    #define G_SYSTEM_PLATFORM_UNIX
    #define G_SYSTEM_PLATFORM_HPUX_UNIX

#elif defined(_AIX)
    #define G_PLATFORM_NAME "aix" // IBM AIX
    #define G_SYSTEM_PLATFORM_UNIX
    #define G_SYSTEM_PLATFORM_AIX_UNIX

#elif defined(__sun) && defined(__SVR4)
    #define G_PLATFORM_NAME "solaris" // Oracle Solaris, Open Indiana
    #define G_SYSTEM_PLATFORM_UNIX
    #define G_SYSTEM_PLATFORM_SUN_UNIX

#else
    #define G_PLATFORM_NAME NULL
    #define G_SYSTEM_PLATFORM_NULL

#endif

namespace rev {

/// @class SystemMonitor
/// @brief Class for monitoring the system and retrieving system info
class SystemMonitor {
public:

    /// @brief The platform of the system
    enum class SystemPlatform {
        kNull = -1, // No platform, no bueno
        kWindows,
        kAndroid, // Android (implies Linux)
        kLinux, // Debian, Ubuntu, Gentoo, Fedora, openSUSE, RedHat, Centos and other
        kBsd, // FreeBSD, NetBSD, OpenBSD, DragonFly BSD
        kHpux, // HP-UX
        kAix, // IBM AIX
        kIos, // Apple iOS
        kOsx, // Apple OSX
        kSolaris // Oracle Solaris, Open Indiana
    };

    /// @brief The endianness of the system
    /// @example
    /// Consider a 32 bit integer 0A0B0C0D:
    /// Big Endian storage in memory:      |   0A   |   0B   |   0C   |   0D   |
    /// Little Endian storage in memory:   |   0D   |   0C   |   0B   |   0A   |
    /// Address:                                a      a + 1    a + 2    a + 3
    enum class SystemEndianness {
        kUnhandled = -1,
        kLittle, ///< LSB (Least-significant byte) is stored at the smallest memory address. Way more common (all intel processors)
        kBig, ///< LSB (Least-significant byte) is stored at the largest memory address
        kNetwork = kBig ///< Network endianness
    };

    /// @brief Determine the platform of an operation system
    /// @note Fully supported supported only GNU GCC/G++, partially on Clang/LLVM
    static SystemPlatform GetPlatform() {
        return s_platform;
    }

    /// @brief Return a name of platform, if determined, otherwise - an empty string
    static const char* GetPlatformName() {
        return s_platformName;
    }

    /// @brief Return the endianness of the platform
    static SystemEndianness GetEndianness() {
        constexpr int value{ 0x01 };
        const void* address = static_cast<const void*>(&value);
        const unsigned char* leastSignificantAddress = static_cast<const unsigned char*>(address);
        return (*leastSignificantAddress == 0x01) ? SystemEndianness::kLittle : SystemEndianness::kBig;
    }

private:

    /// Determine platform based on preprocessor defines
#if defined(G_SYSTEM_PLATFORM_WINDOWS)
    static constexpr SystemPlatform s_platform = SystemPlatform::kWindows;
#elif defined(G_SYSTEM_PLATFORM_ANDROID)
    static constexpr SystemPlatform s_platform = SystemPlatform::kAndroid;
#elif defined(G_SYSTEM_PLATFORM_LINUX)
    static constexpr SystemPlatform s_platform = SystemPlatform::kLinux;
#elif defined(G_SYSTEM_PLATFORM_BSD)
    static constexpr SystemPlatform s_platform = SystemPlatform::kBsd;
#elif defined(G_SYSTEM_PLATFORM_HPUX_UNIX)
    static constexpr SystemPlatform s_platform = SystemPlatform::kHpux;
#elif defined(G_SYSTEM_PLATFORM_AIX_UNIX)
    static constexpr SystemPlatform s_platform = SystemPlatform::kAix;
#elif defined(G_SYSTEM_PLATFORM_APPLE)
    #if defined(G_SYSTEM_PLATFORM_IOS)
        static constexpr SystemPlatform s_platform = SystemPlatform::kIos;
    #else
        static constexpr SystemPlatform s_platform = SystemPlatform::kOsx;
    #endif
#elif defined(G_SYSTEM_PLATFORM_SUN_UNIX)
    static constexpr SystemPlatform s_platform = SystemPlatform::kSolaris;
#endif

#ifndef G_SYSTEM_PLATFORM_NULL
    static constexpr char* s_platformName = G_PLATFORM_NAME; ///< Platform name
#endif

};




} /// End namespace rev
