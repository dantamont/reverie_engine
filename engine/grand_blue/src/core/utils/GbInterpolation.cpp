/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
#include "GbInterpolation.h"

namespace Gb {
namespace Interpolation {
/////////////////////////////////////////////////////////////////////////////////////////////

template <>
double lerp(const double& first, const double& second, float percentFactor) {
    // Optimized lerp for slightly better accuracy and performance
    double percent = (double)percentFactor;
    return fma(percent, second, fma(-percent, first, first));
}
template <>
float lerp(const float& first, const float& second, float percentFactor) {
    // Optimized lerp for slightly better accuracy and performance
    return fma(percentFactor, second, fma(-percentFactor, first, first));
}



/////////////////////////////////////////////////////////////////////////////////////////////
}
} // End namespaces
