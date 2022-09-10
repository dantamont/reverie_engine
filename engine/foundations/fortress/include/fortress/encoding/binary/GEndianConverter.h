#pragma once

// Standard Includes
#include <stdexcept>
#include <array>
#include <typeinfo>

// Project
#include "fortress/system/GSystemPlatform.h"
#include "fortress/numeric/GSizedTypes.h"

namespace rev {

/// @brief Helper class for converting endianness
/// @example
/// Consider a 32 bit integer 0A0B0C0D:
/// Big Endian storage in memory:      |   0A   |   0B   |   0C   |   0D   |
/// Little Endian storage in memory:   |   0D   |   0C   |   0B   |   0A   |
/// Address:                                a      a + 1    a + 2    a + 3
class EndianConverter {
public:

    /// @brief Convert from network to host endianness
    template<typename T>
    static T NetworkToHost(T val) {
        if (SystemMonitor::GetEndianness() == SystemMonitor::SystemEndianness::kNetwork) {
            return val;
        }
        else {
            return SwapEndianness(val);
        }
    }

    /// @brief Convert from network to host endianness
    /// @param[in] arr the array to swap endianness for
    /// @return the array with swapped endianness
    template<typename T, Uint64_t N>
    static std::array<T, N> NetworkToHost(const std::array<T, N>& arr) {
        if (SystemMonitor::GetEndianness() == SystemMonitor::SystemEndianness::kNetwork) {
            return arr;
        }
        else {
            return SwapEndianness(arr);
        }
    }

    /// @brief Swap endianness of a given buffer
    /// @param[in] arr the array to swap endianness for
    /// @return the array with swapped endianness
    template<typename T, Uint64_t N>
    static std::array<T, N> SwapEndianness(const std::array<T, N>& arr) {
        std::array<T, N> outArray;
        Uint8_t* bufferDst = reinterpret_cast<Uint8_t*>(outArray.data());

        SwapEndianness(arr.data(), bufferDst, N);

        return outArray;
    }

    /// @brief Swap endianness of a given buffer
    /// @delete Must take ownership of (i.e., delete[]) the returned buffer
    /// @param[in] ptr the buffer to swap endianness for
    /// @param[in] count the number of elements T in the array ptr, of type T*
    template<typename T>
    static Uint8_t* SwapEndianness(const T* ptr, Uint64_t count) {
        static_assert(std::is_integral_v<T>,
            "Endianness can only be adjusted trivially on integral types"
            );

        constexpr Uint64_t sizeInBytes = sizeof(T);
        Uint8_t* bufferDst = new Uint8_t[sizeInBytes * count];

        SwapEndianness(ptr, bufferDst, count);

        return bufferDst;
    }

    /// @brief Swap the endianness of a given value
    template<typename T>
    static T SwapEndianness(T value) {
        T outValue;
        constexpr Uint64_t sizeInBytes = sizeof(T);
        Uint8_t* castedOut = reinterpret_cast<Uint8_t*>(&outValue);
        SwapEndianness(&value, castedOut, 1);
        return outValue;
    }

    /// @brief Swap endianness of a given buffer, into the given output buffer
    /// @param[in] ptr the buffer to swap endianness for
    /// @param[in] outputBuffer the buffer to return, with swapped endianness
    /// @param[in] count the number of elements T in the array ptr, of type T*
    /// @see https://coderedirect.com/questions/213791/how-do-i-convert-a-big-endian-struct-to-a-little-endian-struct
    template<typename T>
    static void SwapEndianness(const T* ptr, Uint8_t* outputBuffer, Uint64_t count) {
        static_assert(std::is_integral_v<T>,
            "Endianness can only be adjusted trivially on integral types"
            );
        constexpr Uint64_t sizeInBytes = sizeof(T);
        const Uint8_t* bufferSrc = reinterpret_cast<const Uint8_t*>(ptr);

        // Iterate over all members
        Uint64_t sizeTimesIterator;
        for (Uint64_t i = 0; i < count; i++)
        {
            // Reverse byte order for current member
            for (Size_t ix = 0; ix < sizeInBytes; ix++) {
                sizeTimesIterator = sizeInBytes * i;
                outputBuffer[sizeTimesIterator + (sizeInBytes - 1 - ix)] = bufferSrc[sizeTimesIterator + ix];
            }
        }
    }

};


} // end namespacing