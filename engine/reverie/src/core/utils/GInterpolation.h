/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_INTERPOLATION_H
#define GB_INTERPOLATION_H

// QT
#include <string>
#include <vector>

// Internal

namespace rev {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
namespace Interpolation {

/// @brief Generic linear interpolation template
template <class T>
T lerp(const T& first, const T& second, float percentFactor) {

    if (percentFactor <= 0.0f)
        return first;
    else if (percentFactor >= 1.0f)
        return second;

    return first + percentFactor * (second - first);
    //return fma(percentFactor, second, fma(-percentFactor, first, first));
}

template <> double lerp(const double& first, const double& second, float percentFactor);
template <> float lerp(const float& first, const float& second, float percentFactor);

/// @brief Generic weighted interpolation template
/// @details Weights should sum up to one for a proper weighted average
template <class T>
T lerp(const std::vector<T>& values, const std::vector<float>& weights) {
    size_t numValues = values.size();
#ifdef DEBUG_MODE
    if (numValues != weights.size()) {
        throw("Error, input size mismatch");
    }
#endif
    T result = values[0] * weights[0];
    for (size_t i = 1; i < numValues; i++) {
        result += values[i] * weights[i];
    }
    return result;
}

template <class T>
T lerp(const T* values, const std::vector<float>& weights) {
    size_t numValues = weights.size();
    T result = values[0] * weights[0];
    for (size_t i = 1; i < numValues; i++) {
        result += values[i] * weights[i];
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////
}
} // End namespaces

#endif