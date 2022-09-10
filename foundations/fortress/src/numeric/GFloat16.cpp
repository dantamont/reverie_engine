#include "fortress/numeric/GFloat16.h"
#include <cstring>

namespace rev {

Int16_t Float16::FloatToFloat16(Float32_t value)
{
    Int16_t fltInt16;
    Int32_t fltInt32;
    memcpy(&fltInt32, &value, sizeof(Float32_t));
    fltInt16 = ((fltInt32 & 0x7fffffff) >> 13) - (0x38000000 >> 13);
    fltInt16 |= ((fltInt32 & 0x80000000) >> 16);

    return fltInt16;
}

Float32_t Float16::Float16ToFloat(Int16_t fltInt16)
{
    Int32_t fltInt32 = ((fltInt16 & 0x8000) << 16);
    fltInt32 |= ((fltInt16 & 0x7fff) << 13) + 0x38000000;

    Float32_t fRet;
    memcpy(&fRet, &fltInt32, sizeof(Float32_t));
    return fRet;
}

} // End rev namespace