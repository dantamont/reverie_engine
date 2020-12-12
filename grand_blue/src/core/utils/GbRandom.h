/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_RANDOM_NUM_H
#define GB_RANDOM_NUM_H

// QT
#include <string>
#include <vector>
#include <array>
#include <random>

// Internal
#include "../geometry/GbVector.h"

namespace Gb {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
namespace Random {

/// @brief Obtain the specified number of random vectors
template<typename D, size_t N, typename UnaryPredicate>
void GetRandomVectors(size_t numSamples, 
    const std::array<Vector<D, 2>, N>& indexRanges,
    std::vector<Vector<D, N>>& outVec, 
    UnaryPredicate predicate) 
{
    outVec.resize(numSamples);
    GetRandomVectors(numSamples, indexRanges, outVec.data(), predicate);
}

template<typename D, size_t N, typename UnaryPredicate>
void GetRandomVectors(size_t numSamples,
    const std::array<Vector<D, 2>, N>& indexRanges,
    Vector<D, N>* outData,
    UnaryPredicate predicate) 
{
    // Random values between [0.0, 1.0]
    std::uniform_real_distribution<D> randomRange(0.0, 1.0);
    static std::default_random_engine generator;

    // Get scale and shift from desired ranges
    std::array<Vector<D, 2>, N> scaleShift;
    for (size_t i = 0; i < N; i++) {
        scaleShift[i][0] = indexRanges[i][1] - indexRanges[i][0];
        scaleShift[i][1] = indexRanges[i][0];
    }

    // Generate vectors based on number of input samples and index ranges
    for (size_t i = 0; i < numSamples; i++) {
        // Randomize direction
        for (size_t j = 0; j < N; j++) {
            D scale = scaleShift[j][0];
            D shift = scaleShift[j][1];
            outData[i][j] = randomRange(generator) * scale + shift;
        }

        // Randomize range
        outData[i].normalize();
        outData[i] *= randomRange(generator);

        // Use predicate to scale value
        outData[i] *= predicate(i, numSamples);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
}
} // End namespaces

#endif