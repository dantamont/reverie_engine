#pragma once

// Std
#include <limits>
#include <functional>

namespace rev {
namespace math {

/// @brief Get the sign of a value
template <typename T> 
inline int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

/// @brief Return true if number is near zero
template <typename T>
inline bool fuzzyIsNull(T x){
    return std::abs(x) < std::numeric_limits<T>::epsilon();
}

/// @brief Since std::abs is not defined for unsigned types
template<typename T>
bool areClose(const T& left, const T& right, double tolerance = 1e-6) {
    // This is better: compare all integral values for equality:
    if constexpr (std::is_integral<T>::value) {
        return (left == right);
    }
    else {
        return (std::abs(left - right) < tolerance);
    }
}

/// @brief Clamp a value v between lo and hi
template<class T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi)
{
    return std::max(lo, std::min(v, hi));
}


} // end math namespace
} // end rev namespace
