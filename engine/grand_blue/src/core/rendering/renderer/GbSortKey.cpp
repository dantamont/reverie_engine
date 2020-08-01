#include "GbSortKey.h"

#include <QDebug>
#include "../../GbObject.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
SortKey::SortKey()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
SortKey::SortKey(unsigned long long key):
    m_key(key)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
SortKey::~SortKey()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
unsigned long SortKey::FloatToBinary(float number)
{
    if (number == 0) {
        return 0;
    }
    
    // A reinterpret cast will convert to IEEE 754 format
    // See: http://cstl-csm.semo.edu/xzhang/Class%20Folder/CS280/Workbook_HTML/FLOATING_tut.htm
    unsigned long ieeeShort = *reinterpret_cast<unsigned long*>((&number));
    //Object().logInfo("Casted: " + bitStr(ieeeShort));
    return ieeeShort;
}
/////////////////////////////////////////////////////////////////////////////////////////////
float SortKey::BinaryToFloat(unsigned long binary)
{
    if (binary == 0) {
        return 0;
    }

    float flt = *reinterpret_cast<float*>((&binary));
    //Object().logInfo("Original: " + QString::number(flt));
    //Object().logWarning("----");

    return flt;
}
/////////////////////////////////////////////////////////////////////////////////////////////
unsigned long SortKey::FloatToSortedBinary(float number)
{
    // TODO: Remove this function, doesn't actually sort floats properly
    // Invert negative float so that more negative numbers are sorted lower
    if (number < 0) {
        number = 1.0f / number;
    }

    // Switch sign so that negative numbers are sorted first
    number *= -1;

    unsigned long out = FloatToBinary(number);
    return out;
}


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
