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
#include <stdlib.h>     /* srand, rand */

// Internal
#include "fortress/containers/math/GVector.h"

namespace rev {

/// @todo Remove this namespace, or at least make lowercase
namespace Random {

/// @brief Get a random number in the given range, inclusive
/// @see https://stackoverflow.com/questions/12657962/how-do-i-generate-a-random-number-between-two-variables-that-i-have-stored
inline int GetRandomNumber(int min, int max) {
    static std::random_device s_seeder;
    std::mt19937 rng(s_seeder());
    std::uniform_int_distribution<int> gen(min, max); // uniform, unbiased
    int r = gen(rng);
    return r;
}

/// @brief This approach is biased towards thelower end if the range isn't divisible by (max - min + 1)
/// @see https://stackoverflow.com/questions/12657962/how-do-i-generate-a-random-number-between-two-variables-that-i-have-stored
inline int GetSimpleRandomNumber(int min, int max) {
    srand(time(NULL)); // Seed the time
    int out = (rand() % (max - min + 1)) + min;
    return out;
}

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