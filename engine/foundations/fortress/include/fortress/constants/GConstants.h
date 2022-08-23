#pragma once

namespace rev {

/// Classes
class Constants {
public:
    static constexpr double TwoPi = 3.14159265358979323846 * 2;
    static constexpr double Pi = 3.14159265358979323846;
    static constexpr double HalfPi = 1.57079632679489661923;
    static constexpr double QuarterPi = 0.785398163397448309616;
    static constexpr double DegToRad = Pi / 180.0;
    static constexpr double RadToDeg = 180.0 / Pi;
};

/// Macros
/* a=target variable, n=bit number to act upon 0-n */
#define BIT_SET(a,n) ((a) |= (1ULL<<(n))) // bitwise OR
#define BIT_CLEAR(a,n) ((a) &= ~(1ULL<<(n))) // bitwise AND
#define BIT_FLIP(a,n) ((a) ^= (1ULL<<(n))) // bitwise XOR
#define BIT_CHECK(a,n) (!!((a) & (1ULL<<(n)))) // Check nth bit, '!!' to make sure this returns 0 or 1

/* x=target variable, y=mask */
#define BITMASK_SET(x,y) ((x) |= (y))
#define BITMASK_CLEAR(x,y) ((x) &= (~(y)))
#define BITMASK_FLIP(x,y) ((x) ^= (y))
#define BITMASK_CHECK_ALL(x,y) (((x) & (y)) == (y))   // warning: evaluates y twice
#define BITMASK_CHECK_ANY(x,y) ((x) & (y))


// End namespaces
}
