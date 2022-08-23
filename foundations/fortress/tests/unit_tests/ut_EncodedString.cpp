#include <gtest/gtest.h>

#include "fortress/types/GString.h"
#include "fortress/encoding/string/GEncodedString.h"
#include "fortress/system/GSystemPlatform.h"

namespace rev {

/// @class GEncodedStringFixture
/// @brief Contains common members used by tests
class GEncodedStringFixture : public ::testing::Test {
public:
    GEncodedStringFixture() {
        // initialization code here
    }

    void SetUp() {
        // code here will execute just before the test ensues 
    }

    void TearDown() {
        // code here will be called just after the test completes
        // ok to through exceptions from here if need be
    }

protected:

    GStringUtf8  m_testStr8{ u8"\uf5ae\uf5b6\u0061\u0161\U00013880\U00012000" };
    GStringUtf16 m_testStr16{ u"\uf5ae\uf5b6\u0061\u0161\U00013880\U00012000", 8 };
    GStringUtf32 m_testStr32{ U"\uf5ae\uf5b6\u0061\u0161\U00013880\U00012000", 6 };

    std::vector<GString> m_testStrs8 = {
        GString(u8"\uf5ae"),
        GString(u8"\uf5b6"),
        GString(u8"\u0061"),
        GString(u8"\u0161"),
        GString(u8"\U00013880"),
        GString(u8"\U00012000")
    };

    std::vector<GString> m_testStrs16 = {
        GString(u"\uf5ae", 1),
        GString(u"\uf5b6", 1),
        GString(u"\u0061", 1),
        GString(u"\u0161", 1),
        GString(u"\U00013880", 2), // Character is three bytes, but takes 4 in UTF-16
        GString(u"\U00012000", 2)  // Character is three bytes, but takes 4 in UTF-16
    };

    std::vector<GString> m_testStrs32 = {
        GString(U"\uf5ae", 1),
        GString(U"\uf5b6", 1),
        GString(U"\u0061", 1),
        GString(U"\u0161", 1),
        GString(U"\U00013880", 1),
        GString(U"\U00012000", 1)
    };

    std::vector<Uint32_t> m_decimals = {
        62894, // Two bytes, takes three in Utf-8
        62902, // Two bytes, takes three in Utf-8
        0x61,  // One byte, 97 in decimal
        353,   // Two bytes
        0x13880, // Three bytes
        73728, // Three bytes, Takes four in Utf-8
    };
};

TEST_F(GEncodedStringFixture, Utf8CharEncode) {
    // Encode value, testing every Utf-8 byte length
    for (Uint32_t i = 0; i < m_decimals.size(); i++) {
        GStringUtf8 encoded = StringEncoderUtf8::Encode<GString>(m_decimals[i]);
        EXPECT_EQ(encoded.string(), m_testStrs8[i]);
    }
}

TEST_F(GEncodedStringFixture, Utf8Encode) {
    // Encode value, testing every Utf-8 byte length
    GStringUtf8 encoded = StringEncoderUtf8::Encode<GString>(m_decimals);
    EXPECT_EQ(encoded.string(), m_testStr8.string());
}

TEST_F(GEncodedStringFixture, Utf8Decode) {
    // Setup
    /// @see https://stackoverflow.com/questions/59046071/c-u8-literal-unexpected-encoding-on-windows
    GString utf8str(u8"ï®ï¶aÅ¡ð"); // Is double-encoding,  i-umlaut (the first character in the string) is Unicode U+00EF but represented as the two bytes C3 EF in UTF-8 encoding.
    GString utf8str2(u8"\xef\x96\xae\xef\x96\xb6\x61\xc5\xa1\xf0\x92\x80\x80"); // Might be double encoding (compiler is interpreting contents as Latin-1), which is no bueno
    GString utf8str3("\xef\x96\xae\xef\x96\xb6\x61\xc5\xa1\xf0\x92\x80\x80");
    GString utf8str4("\uf5ae\uf5b6\u0061\u0161\U00012000"); // Need u for four digits (two bytes), U for 8 hexadecimal digits (four bytes)
    GString utf8str5(u8"\uf5ae\uf5b6\u0061\u0161\U00012000"); // Need u for four digits (two bytes), U for 8 hexadecimal digits (four bytes)

    // Decode value
    std::vector<Uint32_t> decoded = StringEncoderUtf8::Decode(m_testStr8.string());
    EXPECT_EQ(decoded.size(), m_decimals.size());
    for (Uint32_t i = 0; i < m_decimals.size(); i++) {
        Uint32_t codePoint = decoded[i];
        EXPECT_EQ(codePoint, m_decimals[i]);
    }
}

TEST_F(GEncodedStringFixture, Utf16Encode) {
    // Encode value, testing every Utf-16 byte length
    GStringUtf16 encoded = StringEncoderUtf16::Encode<GString>(m_decimals);
    EXPECT_EQ(encoded.string(), m_testStr16.string());
}

TEST_F(GEncodedStringFixture, Utf16CharEncode) {
    for (Uint32_t i = 0; i < m_decimals.size(); i++) {
        GStringUtf16 encoded = StringEncoderUtf16::Encode<GString>(m_decimals[i]);
        EXPECT_EQ(encoded.string(), m_testStrs16[i]);
    }
}

TEST_F(GEncodedStringFixture, Utf16Decode) {
    // Decode value
    std::vector<Uint32_t> decoded = StringEncoderUtf16::Decode(m_testStr16.string());
    EXPECT_EQ(decoded.size(), m_decimals.size());
    for (Uint32_t i = 0; i < m_decimals.size(); i++) {
        Uint32_t codePoint = decoded[i];
        EXPECT_EQ(codePoint, m_decimals[i]);
    }
}

TEST_F(GEncodedStringFixture, Utf32Encode) {
    // Encode value, testing every Utf-8 byte length
    GStringUtf32 encoded = StringEncoderUtf32::Encode<GString>(m_decimals);
    EXPECT_EQ(encoded.string(), m_testStr32.string());
}

TEST_F(GEncodedStringFixture, Utf32CharEncode) {
    for (Uint32_t i = 0; i < m_decimals.size(); i++) {
        GStringUtf32 encoded = StringEncoderUtf32::Encode<GString>(m_decimals[i]);
        EXPECT_EQ(encoded.string(), m_testStrs32[i]);
    }
}

TEST_F(GEncodedStringFixture, Utf32Decode) {
    // Decode value
    std::vector<Uint32_t> decoded = StringEncoderUtf32::Decode(m_testStr32.string());
    EXPECT_EQ(decoded.size(), m_decimals.size());
    for (Uint32_t i = 0; i < m_decimals.size(); i++) {
        Uint32_t codePoint = decoded[i];
        EXPECT_EQ(codePoint, m_decimals[i]);
    }
}

TEST_F(GEncodedStringFixture, EncodedString8) {

    // Encode value
    EXPECT_EQ(m_testStr8.encode<StringEncoding::kUtf8>(), m_testStr8);
    EXPECT_EQ(m_testStr8.encode<StringEncoding::kUtf16>(), m_testStr16);
    EXPECT_EQ(m_testStr8.encode<StringEncoding::kUtf32>(), m_testStr32);

    // Decode value
    EXPECT_EQ(m_decimals, m_testStr8.decode());
}

TEST_F(GEncodedStringFixture, EncodedString16) {
    // Encode value
    EXPECT_EQ(m_testStr16.encode<StringEncoding::kUtf8>(), m_testStr8);
    EXPECT_EQ(m_testStr16.encode<StringEncoding::kUtf16>(), m_testStr16);
    EXPECT_EQ(m_testStr16.encode<StringEncoding::kUtf32>(), m_testStr32);

    // Decode value
    EXPECT_EQ(m_decimals, m_testStr16.decode());
}

TEST_F(GEncodedStringFixture, EncodedString32) {
    // Encode value
    EXPECT_EQ(m_testStr32.encode<StringEncoding::kUtf8>(), m_testStr8);
    EXPECT_EQ(m_testStr32.encode<StringEncoding::kUtf16>(), m_testStr16);
    EXPECT_EQ(m_testStr32.encode<StringEncoding::kUtf32>(), m_testStr32);

    // Decode value
    EXPECT_EQ(m_decimals, m_testStr32.decode());
}

} // End rev namespace
