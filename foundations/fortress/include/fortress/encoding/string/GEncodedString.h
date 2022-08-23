#pragma once

#include "fortress/templates/GTemplates.h"
#include "fortress/types/GSizedTypes.h"

#include <array>
#include <vector>

namespace rev {

class GString;

/// @brief The type of string encoding
/// @see https://www.unicode.org/versions/Unicode14.0.0/ch03.pdf
/// @see https://unicode.org/faq/utf_bom.html
/// @see https://stackoverflow.com/questions/2241348/what-is-unicode-utf-8-utf-16
/// @see https://stackoverflow.com/questions/18534494/convert-from-utf-8-to-unicode-c
/// @see https://stackoverflow.com/questions/6240055/manually-converting-unicode-codepoints-into-utf-8-and-utf-16
/// @see https://www.herongyang.com/Unicode/UTF-8-UTF-8-Encoding-Algorithm.html
/// @see https://en.wikipedia.org/wiki/Byte_order_mark
enum class StringEncoding {
    kUnicode = -1, ///< No special encoding, /// @see https://stackoverflow.com/questions/643694/what-is-the-difference-between-utf-8-and-unicode
    kUtf8,  ///< UTF8 encoding is a variable - length 8 - bit(1 - byte) Unicode character encodings.
            ///< UTF8 is compatible with ASCII encoding.It is very efficient for Western language characters, but is not so efficient for CJK(Chinese, Japanese and Korean) language characters, which are encoded into 3 bytes per character most of the time.
            ///< The maximum number encoded bytes is 4 for characters in the latest version of Unicode character set - Unicode 5.0.
            ///< @see "UTF-8, a transformation format of ISO 10646" at https://datatracker.ietf.org/doc/html/rfc2279 gives official specifications of UTF - 8 encoding.
    kUtf8Bom, ///< UTF8 with a leading BOM 
              ///< @note Useage is discouraged. 
              ///< @see https://stackoverflow.com/questions/2223882/whats-the-difference-between-utf-8-and-utf-8-without-bom
    kUtf16, ///< UTF16, UTF16BE and UTF16LE encodings are all variable - length 16 - bit(2 - byte) Unicode character encodings.
            ///< Output byte streams of UTF16 encoding may have 3 valid formats : Big - Endian without BOM, Big - Endian with BOM, and Little - Endian with BOM.
            ///< @see "UTF-16, an encoding of ISO 10646" at https://datatracker.ietf.org/doc/html/rfc2781 gives official specifications of UTF16, UTF16BE and UTF16LE encodings.
    kUtf16Be, ///< UTF16BE encoding is identical to the Big - Endian without BOM format of UTF16 encoding.
    kUtf16Le, ///< UTF16LE encoding is identical to the Little - Endian with BOM format of UTF16 encoding without using BOM.
    kUtf32, ///< UTF32, UTF32BE and UTF32LE encodings are all fixed - length 32 - bit(4 - byte) Unicode character encodings.
            ///< Output byte streams of UTF32 encoding may have 3 valid formats : Big - Endian without BOM, Big - Endian with BOM, and Little - Endian with BOM.
            ///< @see "Unicode Standard Annex #19 - UTF-32" at https://unicode.org/reports/tr19/tr19-9.html gives quick and precise definitions of UTF32, UTF32BE and UTF32LE encodings.
    kUtf32Be, ///< UTF32BE encoding is identical to the Big - Endian without BOM format of UTF32 encoding.
    kUtf32Le, ///< UTF32LE encoding is identical to the Little - Endian with BOM format of UTF32 encoding without using BOM.
    kSIZE,
};

/// Forward declare EncodedString type for use in StringEncoder
template<typename StringType, StringEncoding EncodingType> class EncodedString;

/// @class StringEncoder
/// @brief Class for encoding a string
template<StringEncoding EncodingType>
class StringEncoder {
public:
    typedef Uint8_t  Utf8_t;
    typedef Uint16_t Utf16_t;
    typedef Uint32_t Utf32_t;

    /// @see https://en.wikipedia.org/wiki/Byte_order_mark
    static constexpr Uint32_t s_bom[(Uint32_t)StringEncoding::kSIZE] = ///< BOM (Byte-order mark). Zero-value means none is used
    {
        0, ///< UTF-8
        0xefbbbf, ///< UTF-8 BOM
        0, ///< UTF-16
        0xfeff,///< UTF-16 BE
        0xfffe,///< UTF-16 LE
        0,///< UTF-32 
        0x0000feff,///< UTF-32 BE
        0xfffe0000///< UTF-32 LE
    };


    // Unicode (non-encoded)
    static constexpr Utf32_t s_maxUnicodeValue{ 0x10FFFF }; ///< Maximum is a result of UTF-16 limitations @see https://stackoverflow.com/questions/52203351/why-unicode-is-restricted-to-0x10ffff
    static constexpr Utf32_t s_unicodeReplacementCharacter{ 0xfffd }; ///< The unicode replacement character. Represents unknown, unrecognized, or unrepresentable characters
                                                                      ///< @see https://en.wikipedia.org/wiki/Specials_(Unicode_block)

    /// Utf-8
    static constexpr Utf32_t s_singleByteCutoff   { 0x7f }; ///< Single-byte max cut-off for a unicode value as Utf-8 character
    static constexpr Utf32_t s_doubleByteCutoff   { 0x7ff }; ///< Dual-byte max cut-off for a unicode value as  Utf-8 character
    static constexpr Utf32_t s_tripleByteCutoff   { 0xffff }; ///< Triple-byte max cut-off for a unicode value as  Utf-8 character
    static constexpr Utf32_t s_quadrupleByteCutoff{ s_maxUnicodeValue }; ///< Quadruple-byte max cut-off for a unicode value as  Utf-8 character

    static constexpr Utf32_t s_singleBytePrefix   { 0b00000000 }; ///< Prefix for a single-byte character in Utf-8
    static constexpr Utf32_t s_doubleBytePrefix   { 0b11000000 }; ///< Prefix for a dual-byte character in Utf-8
    static constexpr Utf32_t s_tripleBytePrefix   { 0b11100000 }; ///< Prefix for a triple-byte character in Utf-8
    static constexpr Utf32_t s_quadrupleBytePrefix{ 0b11110000 }; ///< Prefix for a quadruple-byte character in Utf-8


    /// Utf-16
    static constexpr Utf32_t s_maxNonSurrogateCharacter{ 0xd800 - 1 }; ///< The maximum value of a non-surrogate UTF-16 character
    static constexpr Utf32_t s_leadSurrogateMin{ 0xd800 }; ///< The minimum value of a lead (high) surrogate 
    static constexpr Utf32_t s_leadSurrogateMax{ 0xdbff }; ///< The maximum value of a lead (high) surrogate
    static constexpr Utf32_t s_trailingSurrogateMin{ 0xdc00 }; ///< The minimum value of a trailing (low) surrogate
    static constexpr Utf32_t s_trailingSurrogateMax{ 0xdfff }; ///< The maximum value of a trailing (low) surrogate

    StringEncoder() = delete;
    ~StringEncoder() = delete;

    /// @brief Return a string with the specified encoding from the given unicode characters
    template<typename StringType>
    static EncodedString<StringType, EncodingType> Encode(Uint32_t unicode);
    template<typename StringType>
    static EncodedString<StringType, EncodingType> Encode(const std::vector<Uint32_t>& unicode, Int32_t numBytes = -1);

    /// @brief Return a string with the specified encoding from the given string with the encoding of this encoder
    template<typename StringType, StringEncoding OtherEncodingType>
    static EncodedString<StringType, EncodingType> Encode(const StringType& unicode);

    /// @brief Decode the given string to unicode
    /// @return A vector of the decoded code points
    template<typename StringType>
    static std::vector<Uint32_t> Decode(const StringType& str);

};

/// @class EncodedString
/// @brief Class representing a stringtype and its encoding
/// @see https://en.wikipedia.org/wiki/Escape_sequences_in_C
/// @see https://stackoverflow.com/questions/69819190/msvc-is-double-encoding-utf-8-strings-why?noredirect=1#comment123418582_69819190
template<typename StringType, StringEncoding EncodingType>
class EncodedString {
public:

    static_assert(has_member(StringType, c_str()),
        "If your string type doesn't have a c_str() routine, then this template will be broken");
    static_assert(has_member(StringType, length()),
        "If your string type doesn't have a length() routine, then this template will be broken");

    EncodedString(StringType&& string)
    {
        m_string = std::move(string);
    }

    EncodedString(const StringType& string):
        m_string(string)
    {
    }

    EncodedString(const EncodedString& string):
        m_string(string.m_string)
    {
    }

    EncodedString(EncodedString&& other) noexcept
    {
        *this = std::move(other);
    }

    /// @brief Construct encoded string's encapsulate StringType directly from arguments
    template<typename CharType>
    EncodedString(const CharType* chars) :
        m_string(chars)
    {
        static_assert(is_one_of<CharType, char, Char8_t, Char16_t, Char32_t>::value, "Type must be a valid string pointer");
    }

    template<typename CharType>
    EncodedString(const CharType* chars, Uint32_t numChars) :
        m_string(chars, numChars)
    {
        static_assert(is_one_of<CharType, char, Char8_t, Char16_t, Char32_t>::value, "Type must be a valid string pointer");
    }

    EncodedString() = default;
    ~EncodedString() = default;

    EncodedString& operator=(EncodedString&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        m_string = std::move(other.m_string);
    }

    bool operator==(const EncodedString& other) const {
        return m_string == other.m_string;
    }

    /// @brief The encoded string
    const StringType& string() const { return m_string; }

    /// @brief Return a string with the specified encoding
    template<StringEncoding OtherEncodingType>
    EncodedString<StringType, OtherEncodingType> encode() const {
        if constexpr (EncodingType == OtherEncodingType) {
            return *this;
        }
        else {
            std::vector<Uint32_t> decoded = decode();
            return StringEncoder<OtherEncodingType>::Encode<StringType>(decoded);
        }
    }

    /// @brief Return the decoded string
    std::vector<Uint32_t> decode() const {
        return StringEncoder<EncodingType>::Decode(m_string);
    }

    /// @brief Return the unicode character for the string
    /// @details Returns the first character if the string contains multiple
    Uint32_t unicode() const {
        std::vector<Uint32_t> decoded = decode();
        if (decoded.size()) {
            return decoded[0];
        }
        else {
            return StringEncoder<EncodingType>::s_unicodeReplacementCharacter;
        }
    }

    char& operator[](Uint64_t idx) {
        return m_string[idx];
    }

private:
    friend class StringEncoder<EncodingType>;

    StringType m_string; ///< The encoded string;
};



template<StringEncoding EncodingType>
template<typename StringType>
inline EncodedString<StringType, EncodingType> StringEncoder<EncodingType>::Encode(Uint32_t unicode)
{
    static_assert(has_member(StringType, c_str()),
        "If your string type doesn't have a c_str() routine, then this template will be broken");

    EncodedString<StringType, EncodingType> outStr;

    if constexpr (StringEncoding::kUtf8 == EncodingType) {
        if (unicode <= s_singleByteCutoff) {
            outStr.m_string = StringType(static_cast<Char8_t>(unicode & s_singleByteCutoff));
        }
        else if (unicode > s_maxUnicodeValue) {
            // String exceeds maximum allowed size, initialize as invalid unicode character
            outStr.m_string = StringType((Char8_t*)nullptr, 3);
            outStr[0] = 0xEF;
            outStr[1] = 0xBF;
            outStr[2] = 0xBD;
        }
        else {
            // Get number of bytes greater than 1
            Int32_t count;
            if (unicode <= s_doubleByteCutoff) {
                // one continuation byte
                count = 1;
            }
            else if (unicode <= s_tripleByteCutoff) {
                // two continuation bytes
                count = 2;
            }
            else {
                // three continuation bytes
                count = 3;
            }

            outStr.m_string = StringType((Char8_t*)nullptr, count + 1);
            for (Int32_t i = 0; i < count; ++i) {
                outStr[count - i] = 0x80 | (unicode & 0x3F);
                unicode >>= 6;
            }
            outStr[0] = (0x1E << (6 - count)) | (unicode & (0x3F >> count));
        }
    }
    else if constexpr (StringEncoding::kUtf16 == EncodingType) {
        if (((unicode >= 0x0000) && (unicode < s_leadSurrogateMin)) ||
            ((unicode > s_trailingSurrogateMax) && (unicode <= s_tripleByteCutoff)))
        {
            Char16_t unicode16 = static_cast<Char16_t>(unicode);
            outStr.m_string = StringType(&unicode16, 1);
        }
        else if ((unicode >= 0xD800) && (unicode <= 0xDFFF))
        {
            // Code point in invalid surrogate range, use replacement character
            constexpr Char16_t unicode16 = static_cast<Char16_t>(s_unicodeReplacementCharacter);
            outStr.m_string = StringType(&unicode16, 1);
        }
        else
        {
            // constants
            constexpr Utf32_t leadOffset = s_leadSurrogateMin - (0x10000 >> 10);

            // computations
            std::array<Char16_t, 2> leadTrail = { 
                leadOffset + (unicode >> 10),
                s_trailingSurrogateMin + (unicode & 0x3FF)
            };

            outStr.m_string = StringType(&leadTrail[0], 2);
        }
    }
    else if constexpr (StringEncoding::kUtf32 == EncodingType) {
        /// UTF-32 encoding is trivial
        Char32_t unicode32 = unicode;
        if (unicode > s_maxUnicodeValue) {
            // Check if unicode is invalid (too large)
            unicode32 = s_unicodeReplacementCharacter;
        }
        else if (unicode >= s_leadSurrogateMin && unicode <= s_trailingSurrogateMax) {
            // Check if unicode is within surrogate points (ill-formed)
            unicode32 = s_unicodeReplacementCharacter;
        }
        outStr.m_string = StringType(&unicode32, 1);
    }
    else {
        static_assert(false, "Unimplemented encoding type");
    }
    return outStr;
}

template<StringEncoding EncodingType>
template<typename StringType>
inline EncodedString<StringType, EncodingType> StringEncoder<EncodingType>::Encode(const std::vector<Uint32_t>& codePoints, Int32_t totalNumBytes)
{
    static_assert(has_member(StringType, c_str()),
        "If your string type doesn't have a c_str() routine, then this template will be broken");

    EncodedString<StringType, EncodingType> outStr;
    Uint64_t numCharacters = codePoints.size();
    Uint32_t numCharBytes = numCharacters * sizeof(Uint32_t);
    outStr.m_string.resize(totalNumBytes > 0? totalNumBytes + 1: numCharBytes + 1);

    /// @note Could have two for-loops. One for counting total bytes so we have the exact string size, 
    /// and one for populating, but this is more performant. So there's a memory trade-off
    if constexpr (StringEncoding::kUtf8 == EncodingType) {
        Uint64_t byteCount = 0;
        for (Uint64_t i = 0; i < numCharacters; i++) {
            Uint32_t unicode = codePoints[i];
            if (unicode <= s_singleByteCutoff) {
                outStr.m_string[byteCount] = static_cast<Char8_t>(unicode & s_singleByteCutoff);
                byteCount += 1;
            }
            else if (unicode > s_maxUnicodeValue) {
                // String exceeds maximum allowed size, initialize as invalid unicode character
                outStr[byteCount] = 0xEF;
                outStr[byteCount + 1] = 0xBF;
                outStr[byteCount + 2] = 0xBD;
                byteCount += 3;
            }
            else {
                // Get number of bytes greater than 1
                Int32_t count = 1;
                if (unicode <= s_doubleByteCutoff) {
                    // one continuation byte
                }
                else if (unicode <= s_tripleByteCutoff) {
                    // two continuation bytes
                    count = 2;
                }
                else {
                    // three continuation bytes
                    count = 3;
                }

                Uint32_t numBytes = count + 1;
                for (Int32_t j = 0; j < count; ++j) {
                    outStr[byteCount + count - j] = 0x80 | (unicode & 0x3F);
                    unicode >>= 6;
                }
                outStr[byteCount] = (0x1E << (6 - count)) | (unicode & (0x3F >> count));

                byteCount += numBytes;
            }
        }
        outStr.m_string.setLength(byteCount);
        outStr.m_string[byteCount] = 0;
    }
    else if constexpr (StringEncoding::kUtf16 == EncodingType) {
        Uint64_t twoByteCount = 0;
        Utf16_t* utf16Array = reinterpret_cast<Utf16_t*>(outStr.m_string.c_str());
        for (Uint64_t i = 0; i < numCharacters; i++) {
            Uint32_t unicode = codePoints[i];
            if (((unicode >= 0x0000) && (unicode < s_leadSurrogateMin)) ||
                ((unicode > s_trailingSurrogateMax) && (unicode <= s_tripleByteCutoff)))
            {
                utf16Array[twoByteCount] = static_cast<Char16_t>(unicode);
                twoByteCount += 1;
            }
            else if ((unicode >= 0xD800) && (unicode <= 0xDFFF))
            {
                // Code point in invalid surrogate range, use replacement character
                constexpr Char16_t unicode16 = static_cast<Char16_t>(s_unicodeReplacementCharacter);
                utf16Array[twoByteCount] = static_cast<Char16_t>(unicode);
                twoByteCount += 1;
            }
            else
            {
                // constants
                constexpr Utf32_t leadOffset = s_leadSurrogateMin/*0xd800*/ - (0x10000 >> 10);

                // computations
                Char16_t lead = leadOffset + (unicode >> 10);
                Char16_t trail = s_trailingSurrogateMin + (unicode & 0x3FF);

                utf16Array[twoByteCount] = lead;
                utf16Array[twoByteCount + 1] = trail;
                twoByteCount += 2;
            }
        }
        Uint64_t byteCount = twoByteCount * 2;
        outStr.m_string.setLength(byteCount);
        outStr.m_string[byteCount] = 0;
    }
    else if constexpr (StringEncoding::kUtf32 == EncodingType) {
        Utf32_t* utf32Array = reinterpret_cast<Utf32_t*>(outStr.m_string.c_str());
        for (Uint64_t i = 0; i < numCharacters; i++) {
            // UTF-32 encoding is trivial
            Uint32_t unicode = codePoints[i];
            Char32_t unicode32 = unicode;
            if (unicode > s_maxUnicodeValue) {
                // Check if unicode is invalid (too large)
                unicode32 = s_unicodeReplacementCharacter;
            }
            else if (unicode >= s_leadSurrogateMin && unicode <= s_trailingSurrogateMax) {
                // Check if unicode is within surrogate points (ill-formed)
                unicode32 = s_unicodeReplacementCharacter;
            }
            utf32Array[i] = unicode32;
        }
        outStr.m_string.setLength(numCharBytes);
        outStr.m_string[numCharBytes] = 0;
    }
    else {
        static_assert(false, "Unimplemented encoding type");
    }
    return outStr;
}

template<StringEncoding EncodingType>
template<typename StringType, StringEncoding OtherEncodingType>
inline EncodedString<StringType, EncodingType> StringEncoder<EncodingType>::Encode(const StringType& unicode)
{
    return EncodedString<StringType, EncodingType>();
}

template<StringEncoding EncodingType>
template<typename StringType>
inline std::vector<Uint32_t> StringEncoder<EncodingType>::Decode(const StringType& str)
{

    static_assert(has_member(StringType, c_str()),
        "If your string type doesn't have a c_str() routine, then this template will be broken");

    std::vector<Uint32_t> myCodePoints; ///< The decoded unicode
    Uint64_t stringLength = str.length();
    if constexpr (StringEncoding::kUtf8 == EncodingType) {
        /// @see https://www.unicode.org/versions/Unicode14.0.0/ch03.pdf
        const Utf8_t* utf8array = reinterpret_cast<const Utf8_t*>(str.c_str());
        myCodePoints.reserve(stringLength);
        Uint64_t inCounter = 0;

        constexpr Utf32_t leftBitMask =      0b10000000;
        constexpr Utf32_t leftThreeBitMask = 0b11100000;
        constexpr Utf32_t leftFourBitMask =  0b11110000;
        constexpr Utf32_t leftFiveBitMask =  0b11111000;

        constexpr Utf32_t rightSixBitMask =   0b00111111;
        constexpr Utf32_t rightFiveBitMask =  0b00011111;
        constexpr Utf32_t rightFourBitMask =  0b00001111;
        constexpr Utf32_t rightThreeBitMask = 0b00000111;

        for (inCounter; inCounter < stringLength;) {
            Utf8_t myChar = utf8array[inCounter];

            if ((myChar & leftBitMask) == s_singleBytePrefix) {
                myCodePoints.emplace_back(myChar);
                inCounter++;
            }
            else if ((myChar & leftThreeBitMask) == s_doubleBytePrefix) {
                Utf8_t myChar2 = utf8array[inCounter + 1];
                myCodePoints.emplace_back((Uint16_t(myChar & rightFiveBitMask) << 6) | (myChar2 & rightSixBitMask));
                inCounter += 2;
            }
            else if ((myChar & leftFourBitMask) == s_tripleBytePrefix) {
                Utf8_t myChar2 = utf8array[inCounter + 1];
                Utf8_t myChar3 = utf8array[inCounter + 2];
                myCodePoints.emplace_back((Uint32_t(myChar & rightFourBitMask) << 12) | (Uint16_t(myChar2 & rightSixBitMask) << 6) | (myChar3 & rightSixBitMask));
                inCounter += 3;
            }
            else if ((myChar & leftFiveBitMask) == s_quadrupleBytePrefix) {
                Utf8_t myChar2 = utf8array[inCounter + 1];
                Utf8_t myChar3 = utf8array[inCounter + 2];
                Utf8_t myChar4 = utf8array[inCounter + 3];
                myCodePoints.emplace_back((Uint32_t(myChar & rightThreeBitMask) << 18) | (Uint32_t(myChar2 & rightSixBitMask) << 12) | (Uint16_t(myChar3 & rightSixBitMask) << 6) | (myChar4 & rightSixBitMask));
                inCounter += 4;
            }
#ifdef DEBUG_MODE
            else {
                assert(false && "Invalid Utf-8 string");
            }
#endif
        }
    }
    else if constexpr (StringEncoding::kUtf16 == EncodingType) {
        /// @see https://unicode.org/faq/utf_bom.html
        Uint64_t inCounter = 0;
        const Utf16_t* utf16array = reinterpret_cast<const Utf16_t*>(str.c_str());
        Uint64_t halfArraySize = stringLength / 2;
        myCodePoints.reserve(halfArraySize);

        assert(stringLength % 2 == 0 && "Error, invalid length of a UTF-16 string");

        for (inCounter; inCounter < halfArraySize;) {
            Utf16_t myChar = utf16array[inCounter];

            if ((s_leadSurrogateMin/*0xd800*/ <= myChar) && (s_leadSurrogateMax/*0xdbff*/ >= myChar)) {
                /// The character is the start of a surrogate pair
                Utf16_t trail = utf16array[inCounter + 1];

                // Constants
                /// @see https://en.wikipedia.org/wiki/UTF-16
                /// @note Below are the steps to calculate the surrogate offset for this calculation
                //((lead - s_leadSurrogateMin) << 10) + (trail - s_trailingSurrogateMin) + 0x10000;
                //(lead << 10) - (s_leadSurrogateMin << 10) + trail - s_trailingSurrogateMin + 0x10000;
                //(lead << 10) + trail + 0x10000 - (s_leadSurrogateMin << 10) + trail - s_trailingSurrogateMin;
                constexpr Utf32_t surrogateOffset = 0x10000 - (s_leadSurrogateMin/*0xd800*/ << 10) - s_trailingSurrogateMin/*0xdc00*/;

                Utf32_t codePoint = (myChar << 10) + trail + surrogateOffset;
                myCodePoints.emplace_back(codePoint);

                // Move onto the next character after the surrogate pair
                inCounter += 2;
            }
            else {
                /// The character is contained within two bytes, and does not require a surrogate pair
                myCodePoints.emplace_back(myChar);

                // Move onto the next character
                inCounter++;
            }
        }
    }
    else if constexpr (StringEncoding::kUtf32 == EncodingType) {
        /// @see https://www.unicode.org/versions/Unicode14.0.0/ch03.pdf
        Uint64_t inCounter = 0;
        const Utf32_t* utf32Array = reinterpret_cast<const Utf32_t*>(str.c_str());
        Uint64_t quarterArraySize = stringLength / 4;

        assert(stringLength % 4 == 0 && "Error, invalid length of a UTF-32 string");

        myCodePoints.resize(quarterArraySize);
        memcpy(&myCodePoints[0], &utf32Array[0], sizeof(Utf32_t) * quarterArraySize);
    }
    else {
        static_assert(false, "Unimplemented encoding type");
    }

    return myCodePoints;
}

typedef StringEncoder<StringEncoding::kUtf8> StringEncoderUtf8;
typedef StringEncoder<StringEncoding::kUtf16> StringEncoderUtf16;
typedef StringEncoder<StringEncoding::kUtf32> StringEncoderUtf32;

typedef EncodedString<GString, StringEncoding::kUtf8> GStringUtf8;
typedef EncodedString<GString, StringEncoding::kUtf16> GStringUtf16;
typedef EncodedString<GString, StringEncoding::kUtf32> GStringUtf32;

} /// End rev namespace