#include <gtest/gtest.h>

namespace rev {


TEST(SortKey, Ordering) {

    // Testing to make sure that depth is preserved in sort key
    //unsigned long long mask = 0;
    //unsigned long long one = 1;
    //for (int i = 0; i < 26; i++) {
    //    mask |= one << i;
    //}
    //QString maskStr = QString(SortKey(mask));
    //QString depthStr0 = QString(SortKey(m_sortKey.key()));
    //QString depthStr1 = QString(SortKey(m_sortKey.key() >> currentBit));
    //unsigned long long shiftedDepth = ((m_sortKey.key() >> currentBit) & mask);
    //QString depthStr = QString(SortKey(shiftedDepth));
    //float retrievedDepth = SortKey::BinaryToFloat(shiftedDepth << rightShift);
}

} /// End rev namespace