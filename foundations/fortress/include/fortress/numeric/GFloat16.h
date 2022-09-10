#pragma once

#include "fortress/numeric/GSizedTypes.h"

namespace rev {

/// @brief A 16-bit float type
/// @see https://stackoverflow.com/questions/22210684/16-bit-floats-and-gl-half-float
/// @see https://www.khronos.org/opengl/wiki/Small_Float_Formats
class Float16
{
public:
    static Int16_t FloatToFloat16(Float32_t value);
    static Float32_t Float16ToFloat(Int16_t value);

    Float16();
    Float16(Float32_t value);
    Float16(const Float16& value);

    operator Float32_t();
    operator Float32_t() const;

    friend Float16 operator + (const Float16& val1, const Float16& val2);
    friend Float16 operator - (const Float16& val1, const Float16& val2);
    friend Float16 operator * (const Float16& val1, const Float16& val2);
    friend Float16 operator / (const Float16& val1, const Float16& val2);

    Float16& operator =(const Float16& val);
    Float16& operator +=(const Float16& val);
    Float16& operator -=(const Float16& val);
    Float16& operator *=(const Float16& val);
    Float16& operator /=(const Float16& val);
    Float16& operator -();

protected:
    Int16_t m_value; ///< Storage for the value of the float
};

inline Float16::Float16()
{
}

inline Float16::Float16(Float32_t value)
{
    m_value = FloatToFloat16(value);
}

inline Float16::Float16(const Float16& value)
{
    m_value = value.m_value;
}

inline Float16::operator Float32_t()
{
    return Float16ToFloat(m_value);
}

inline Float16::operator Float32_t() const
{
    return Float16ToFloat(m_value);
}

inline Float16& Float16::operator =(const Float16& val)
{
    m_value = val.m_value;
}

inline Float16& Float16::operator +=(const Float16& val)
{
    *this = *this + val;
    return *this;
}

inline Float16& Float16::operator -=(const Float16& val)
{
    *this = *this - val;
    return *this;

}

inline Float16& Float16::operator *=(const Float16& val)
{
    *this = *this * val;
    return *this;
}

inline Float16& Float16::operator /=(const Float16& val)
{
    *this = *this / val;
    return *this;
}

inline Float16& Float16::operator -()
{
    *this = Float16(-(Float32_t)*this);
    return *this;
}

inline Float16 operator + (const Float16& val1, const Float16& val2)
{
    return Float16((Float32_t)val1 + (Float32_t)val2);
}

inline Float16 operator - (const Float16& val1, const Float16& val2)
{
    return Float16((Float32_t)val1 - (Float32_t)val2);
}

inline Float16 operator * (const Float16& val1, const Float16& val2)
{
    return Float16((Float32_t)val1 * (Float32_t)val2);
}

inline Float16 operator / (const Float16& val1, const Float16& val2)
{
    return Float16((Float32_t)val1 / (Float32_t)val2);
}

typedef Float16 Float16_t;

} // End namespace rev