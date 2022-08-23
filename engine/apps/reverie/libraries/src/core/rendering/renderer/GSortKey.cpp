#include "core/rendering/renderer/GSortKey.h"

#include <QDebug>

namespace rev {


SortKey::SortKey()
{
}

SortKey::SortKey(Uint64_t key):
    m_key(key)
{
}

SortKey::~SortKey()
{
}

Uint32_t SortKey::FloatToBinary(float number)
{
    if (number == 0) {
        return 0;
    }
    
    // A reinterpret cast will convert to IEEE 754 format
    /// \see http://cstl-csm.semo.edu/xzhang/Class%20Folder/CS280/Workbook_HTML/FLOATING_tut.htm
    Uint32_t ieeeShort = *reinterpret_cast<Uint32_t*>((&number));
    //Object().logInfo("Casted: " + bitStr(ieeeShort));
    return ieeeShort;
}

float SortKey::BinaryToFloat(Uint32_t binary)
{
    if (binary == 0) {
        return 0;
    }

    float flt = *reinterpret_cast<float*>((&binary));
    //Object().logInfo("Original: " + QString::number(flt));
    //Object().Logger::LogWarning("----");

    return flt;
}

Uint32_t SortKey::FloatToSortedBinary(float number)
{
    // TODO: Remove this function, doesn't actually sort floats properly
    // Invert negative float so that more negative numbers are sorted lower
    if (number < 0) {
        number = 1.0f / number;
    }

    // Switch sign so that negative numbers are sorted first
    number *= -1;

    Uint32_t out = FloatToBinary(number);
    return out;
}



} // End namespaces
