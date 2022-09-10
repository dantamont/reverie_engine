#pragma once 

/// Includes
//#include <cstdint>

namespace rev 
{ // Start namespace

typedef char Int8_t;
typedef short Int16_t;
typedef int Int32_t;
typedef long long Int64_t;

typedef unsigned char Uint8_t;
typedef unsigned short Uint16_t;
typedef unsigned int Uint32_t;
typedef unsigned long long Uint64_t;

typedef unsigned char Char8_t; ///< actual char8_t type added in C++20
typedef char16_t Char16_t;
typedef char32_t Char32_t;

typedef size_t Size_t;

/// \todo: Replace Real_t with Real_t
#ifdef GB_USE_DOUBLE
typedef double Real_t;
#else
typedef float Real_t;
#endif

typedef float Float32_t;
typedef double Float64_t;

} // End namespace
