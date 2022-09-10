#pragma once

// std
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdexcept> // for raising error in Format
#include <regex>

// Internal
#include "fortress/GGlobal.h"
#include "fortress/json/GJson.h"
#include "fortress/numeric/GSizedTypes.h"
#include "fortress/string/GStringView.h"

namespace rev {

/// @class GStringFixedSize
/// @brief String class
/// @details The char array contained by GStringFixedSize is always null-terminated
/// @todo Maybe just base this off std::string? 
template<Uint32_t ReservedSizeBytes = 255>
class GStringFixedSize: public GStringInterface<std::array<char, ReservedSizeBytes>> {
private:
    template<Uint32_t FriendSizeBytes> friend class GStringFixedSize; ///< Make friends with other template instantiations
public:

    using Base = GStringInterface<std::array<char, ReservedSizeBytes>>;
    using typename Base::GStringIterator;
    using typename Base::GStringConstIterator;

    /// @name Static
    /// @{
    /// @}

	/// @name Constructors/Destructor
	/// @{

    /// @brief Default Construction
    GStringFixedSize() 
    {
    }

    /// @brief Construction from a single char
    explicit GStringFixedSize(char c_str) 
    {
        m_contentLength = 1;
        m_cStr[0] = c_str;
        m_cStr[1] = 0; // Add null terminator
    }

    /// @brief Construction from a length and a fill character
    GStringFixedSize(Uint32_t len, char chr) :
        GStringFixedSize((char*)nullptr, len)
    {
        std::fill_n(m_cStr.begin(), len, chr);
    }

    GStringFixedSize(const std::string& str) :
        GStringFixedSize(str.c_str(), str.length())
    {
    }

    /// @brief Construct with a null-terminated char array
    GStringFixedSize(const char* c_str) :
        GStringFixedSize(c_str, c_str ? strlen(c_str) : 0)
    {
    }

    /// @brief Construct with a char array and the number of characters
    /// @param[in] len The number of characters
    GStringFixedSize(const char* c_str, Uint32_t len)
    {
        initialize(c_str, len);
    }


    /// @brief Construct with a null-terminated char array. Could be UTF-8
    /// @param[in] len The number of bytes in the char array
    GStringFixedSize(const Char8_t* c_str) 
    {
        /// @todo Not called in C++17 with u8 strings. Maybe better luck in C++20
        initialize(
            c_str,
            c_str ? strlen(reinterpret_cast<const char*>(c_str)) : 0
        );
    }

    /// @brief Construct with a null-terminated char array. Could be UTF-8
    /// @param[in] len The number of bytes in the char array
    GStringFixedSize(const Char8_t* c_str, Uint32_t len) 
    {
        /// @todo Not called in C++17 with u8 strings. Maybe better luck in C++20
        initialize(c_str, len);
    }


    /// @brief Construct with a null-terminated Char16_t array. Could be UTF-16
    /// @param[in] len The number of bytes divided by 2, i.e. the number of characters and surrogate half-pairs
    /// @note Length is required as multi-byte characters may contain the null character
    GStringFixedSize(const Char16_t* c_str, Uint32_t len)
    {
        initialize(c_str, len);
    }

    /// @brief Construct with a null-terminated Char32_t array. Could be UTF-32
    /// @param[in] len The number of characters
    /// @note Length is required as multi-byte characters may contain the null character
    GStringFixedSize(const Char32_t* c_str, Uint32_t len)
    {
        initialize(c_str, len);
    }

    /// @brief Copy Construction
    GStringFixedSize(const GStringFixedSize& other) :
        GStringFixedSize(other.c_str(), other.m_contentLength)
    {
    }
    
    /// @brief Move construction
    GStringFixedSize(GStringFixedSize&& other) noexcept
    {
        std::swap(m_cStr, other.m_cStr);
        copyMetadata(other);
    }

    /// @brief Destruction
    ~GStringFixedSize() = default;

	/// @}

    /// @name Operators
    /// @{

    /// @brief Override stream insertion operator
    friend std::ostream& operator<<(std::ostream& os, const GStringFixedSize& gStr)
    {
        os << gStr.c_str();
        return os;
    }

    const GStringFixedSize operator+(const GStringFixedSize& other) const {
        GStringFixedSize str = *this;
        str += other.c_str();
        return str;
    }

    const GStringFixedSize operator+(const char* other) const {
        GStringFixedSize str = *this;
        str += other;
        return str;
    }

    const GStringFixedSize operator+(const char other) const
    {
#ifdef DEBUG_MODE
        assert((m_contentLength < ReservedSizeBytes - 1) && "Not enough space");
#endif
        GStringFixedSize str(m_cStr.data(), m_contentLength + 1);
        str[m_contentLength] = other;
        return str;
    }

    inline friend const GStringFixedSize operator+(const char* c, const GStringFixedSize &g) {
        GStringFixedSize gc(c);
        gc += g;
        return gc;
    }

    GStringFixedSize& operator+=(const GStringFixedSize& other) {
        *this += other.c_str();
        return *this;
    }

    GStringFixedSize& operator+=(const char* chars) {
        if (!chars) {
            return *this;
        }

        // Get new dimensions
        Uint32_t charLen = strlen(chars);
        Uint32_t newMaxIndex = m_contentLength + charLen;
        Uint32_t newArraySize = newMaxIndex + 1;

#ifdef DEBUG_MODE
        assert(newArraySize <= ReservedSizeBytes && "Not enough space");
#endif

        // Copy other contents into char array, and null-terminate
        memcpy(m_cStr.data() + m_contentLength, chars, charLen * sizeof(char));
        m_cStr[newMaxIndex] = 0;

        // Set new length
        m_contentLength = newMaxIndex;

        return *this;
    }

    GStringFixedSize& operator+=(const char c) {
        // Get new dimensions
        Uint32_t newMaxIndex = m_contentLength + 1;
        Uint32_t newArraySize = newMaxIndex + 1;

#ifdef DEBUG_MODE
        assert(newArraySize <= ReservedSizeBytes && "Not enough space");
#endif

        // Copy other contents into char array, and null-terminate
        memcpy(m_cStr.data() + m_contentLength, &c, sizeof(char));
        m_cStr[newMaxIndex] = 0;

        // Set new length
        m_contentLength = newMaxIndex;

        return *this;
    }

    GStringFixedSize& operator=(const GStringFixedSize& other) // copy assignment
    {
        if (this == &other) {
            return *this;
        }
        initialize(other.m_cStr.data(), other.m_contentLength);
        return *this;
    }

    GStringFixedSize& operator=(GStringFixedSize&& other) noexcept // move assignment
    {
        std::swap(m_cStr, other.m_cStr);
        copyMetadata(other);
        return *this;
    }

    bool operator==(const GStringFixedSize& other) const
    {
        return operator==(other.m_cStr.data());
    }

    bool operator==(const char& other) const
    {
        return m_contentLength == 1 && m_cStr[0] == other;
    }

    inline friend bool operator==(const char c, const GStringFixedSize& g) {
        return g == c;
    }

    bool operator==(const Char8_t* other) const
    {
        static_assert(std::is_same_v<Char8_t, char> == false, "This function is redundant");
        return operator==(reinterpret_cast<const char*>(other));
    }

    bool operator==(const Char16_t* other) const
    {
        return operator==(reinterpret_cast<const char*>(other));
    }

    bool operator==(const Char32_t* other) const
    {
        return operator==(reinterpret_cast<const char*>(other));
    }

    bool operator==(const char* other) const
    {
        return strcmp(const_cast<const char*>(m_cStr.data()), const_cast<const char*>(other)) == 0;
    }

    inline friend bool operator==(const char* c, const GStringFixedSize& g) {
        return g == c;
    }

    bool operator!=(const GStringFixedSize& other) const
    {
        return operator!=(other.m_cStr.data());
    }

    bool operator!=(const char* other) const
    {
        return strcmp(const_cast<const char*>(m_cStr.data()), other) != 0;
    }

    bool operator<(const GStringFixedSize& other) const {
        return *this < other.m_cStr.data();
    }

    bool operator<(const char* chars) const {
        return strcmp(const_cast<const char*>(m_cStr.data()), const_cast<const char*>(chars)) < 0;
    }

    bool operator>(const GStringFixedSize& other) const {
        return *this > other.m_cStr.data();
    }

    bool operator>(const char* chars) const {
        return strcmp(const_cast<const char*>(m_cStr.data()), const_cast<const char*>(chars)) > 0;
    }

    template<typename IntegerType>
    const char& operator[](IntegerType idx) const {
        static_assert(std::is_integral_v<IntegerType>, "Invalid type for GStringFixedSize::operator[]");
        return m_cStr[idx];
    }

    template<typename IntegerType>
    char& operator[](IntegerType idx) {
        static_assert(std::is_integral_v<IntegerType>, "Invalid type for GStringFixedSize::operator[]");
        return m_cStr[idx];
    }

    explicit operator char() const {
        return m_cStr[0];
    }

    explicit operator char*()
    {
        return m_cStr.data();
    }

    operator const char* () const
    {
        return m_cStr.data();
    }

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson 
    /// @param korObject
    friend void to_json(nlohmann::json& orJson, const GStringFixedSize& korObject)
    {
        if (korObject.isEmpty()) {
            orJson = "";
        }
        else {
            orJson = korObject.m_cStr.data();
        }
    }

    /// @brief Convert from JSON to the given class type
    /// @param korJson 
    /// @param orObject 
    friend void from_json(const nlohmann::json& korJson, GStringFixedSize& orObject) 
    {
        orObject = korJson.get_ref<const std::string&>().c_str();
    }

    /// @}

    /// @name Properties
    /// @{

    const char* c_str() const {
        return m_cStr.data();
    }
    char* c_str() {
        return m_cStr.data();
    }

    /// @brief The length of the contents of the char array associated with this str (not including null terminator)
    Uint32_t length() const {
        return m_contentLength;
    }

    /// @brief Set content length attribute.
    /// @note Makes no checks that the given length is accurate, so use with caution
    void setLength(Uint32_t length) {
        m_contentLength = length;
    }

    /// @brief The reserved size of the string
    constexpr Uint32_t reservedSize() const {
        return ReservedSizeBytes;
    }

    /// @}

	/// @name Public Methods
	/// @{

    /// @brief Copy into an array
    void copyInto(Uint8_t* array, Uint32_t maxReservedSizeBytes) const 
    {
        memcpy(array, m_cStr.data(), std::min(m_contentLength, maxReservedSizeBytes));
    }

    /// @brief Return a sub-string of the GStringFixedSize
    /// @param[in] pos the starting index
    /// @param[in] len the length of the sub-string. If negative, is remaining length of the string
    template<Uint32_t SubStringSize>
    GStringFixedSize<SubStringSize> subStr(Int32_t pos, Int32_t len = -1) const
    {
        return GStringFixedSize<SubStringSize>(begin() + pos, len > 0 ? len : m_contentLength - pos);
    }

    /// @brief Iterator for range-based for loop
    GStringIterator begin() {
        return &m_cStr[0];
    }
    GStringIterator end() {
        return &m_cStr[0] + m_contentLength;
    }

    /// @brief Constant iterator for range-based for loop
    GStringConstIterator begin() const {
        return &m_cStr[0];
    }
    GStringConstIterator end() const {
        return &m_cStr[0] + m_contentLength;
    }

    /// @brief Checks that the string is an integer
    bool isInteger(Int32_t* outInt = nullptr) const 
    {
        if (!m_contentLength) {
            // No length, so can't be an integer
            if (outInt) {
                outInt = 0;
            }
            return false;
        }

        // Check if is an integer using atoi
        Int32_t res = toInt();
        if (outInt) {
            *outInt = res;
        }

        // This string is an integer if either
        // 1) atoi returned a non-zero result or
        // 2) atoi returned a zero result, but the string is actually a single-digit string representation of 0
        return res > 0 || (m_contentLength == 1 && isdigit(m_cStr[0]));
    }

    /// @brief Converts the string to a number, returning 0 if there is no valid conversion
    Int32_t toInt() const {
        return atoi(m_cStr.data());
    }

    Float64_t toDouble() const {
        return atof(m_cStr.data());
    }

    /// @brief Whether or not this string contains the specified substring
    bool contains(const char* c_str) const {
        return strstr(m_cStr.data(), c_str) != nullptr;
    }
    bool contains(const GStringFixedSize& g_str) const {
        return contains(g_str.c_str());
    }

    /// @brief Return the GStringFixedSize as a differently sized string
    template<Uint32_t NewSizeBytes>
    GStringFixedSize<NewSizeBytes> resized() 
    {
        // Create a string with the new size
        GStringFixedSize<NewSizeBytes> newString;

        // Copy current contents into new string
        Uint32_t newMaxIndex = newSize - 1;
        Uint32_t numBytesToCopy = sizeof(char) * std::min(m_contentLength, newMaxIndex/* need room for null terminator*/);
        memcpy(newString.m_cStr.data(), m_cStr.data(), numBytesToCopy);
        newString.m_cStr[newMaxIndex] = 0;
        newString.m_contentLength = numBytesToCopy;

        return newString;
    }

    /// @brief Replace char in string with the specified char
    GStringFixedSize& replace(const char& find, const char& newChar)
    {
        char* currentPos = strchr(m_cStr.data(), find);
        while (currentPos) {
            *currentPos = newChar;
            currentPos = strchr(currentPos, find);
        }
        return *this;
    }

    /// @brief Replace sub-string with the specified sub-string
    GStringFixedSize& replace(const char* old, const char* replacement)
    {
        // FIXME: Don't use fixed buffer size
        // Initialize buffer to store new string
        constexpr size_t BufferSize = 1024;
        char buffer[BufferSize] = { 0 };
        char* insert_point = &buffer[0];
        const char* tmp = m_cStr.data();
        GStringView tempView(tmp);

        // Get lengths of old and new substrings
        size_t old_len = strlen(old);
        size_t repl_len = strlen(replacement);

        while (true) {
            // Return first occurance of old substr in tmp, or nullptr if not found
            const char* p = strstr(tmp, old);

            // Walked past last occurrence of old; copy remaining part
            if (!p) {
                size_t offset = insert_point - &buffer[0];
                size_t availSpace = BufferSize - offset;
#ifdef DEBUG_MODE
                if (availSpace < tempView.length()) {
                    throw("Too small, oopsie");
                }
#endif
                strcpy_s(insert_point, availSpace, tmp);
                break;
            }

            // Copy part before old
            memcpy(insert_point, tmp, p - tmp);
            insert_point += p - tmp;

            // Copy replacement string
            memcpy(insert_point, replacement, repl_len);
            insert_point += repl_len;

            // Adjust pointers, move on
            tmp = p + old_len;
            tempView = GStringView(tmp);
        }

        // Write altered string back to member
        // FIXME: Might be overflowing, I should be resizing m_cStr member to accomodate size increase
        strcpy(m_cStr.data(), buffer);

        return *this;
    }

    GStringFixedSize& replace(const GStringFixedSize& old, const GStringFixedSize& replacement)
    {
        return replace((const char*)old, (const char*)replacement);
    }

    /// @brief Split the string using any of the specified delimiters
    void split(const char* delims, std::vector<GStringFixedSize>& outStrs) const
    {
        // Iterate over string using strtok to split
        GStringFixedSize cpy = *this;
        char* pch;
        char* saveptr = nullptr;
        pch = Strtok_r(cpy.m_cStr.data(), delims, saveptr); // strtok modifies original str by inserting nullptr at delimiters, so need to copy first
        while (pch != nullptr)
        {
            outStrs.emplace_back(GStringFixedSize(pch));
            pch = Strtok_r(nullptr, delims, saveptr); // nullptr continues scanning from last successful call
        }
    }

    std::vector<GStringFixedSize> split(const char* delims) const 
    {
        std::vector<GStringFixedSize> strings;
        split(delims, strings);
        return strings;
    }

    /// @brief Split the string using the regex as a token
    /// @see https://stackoverflow.com/questions/16749069/c-split-string-by-regex/16752826
    std::vector<GStringFixedSize> split(const std::regex& regex) const
    {
        std::vector<GStringFixedSize> myStrings;
        std::regex_token_iterator<GStringConstIterator> iter(begin(),
            end(),
            regex,
            -1);
        std::regex_token_iterator<GStringConstIterator> end;
        for (; iter != end; ++iter) {
            /// Add GStringFixedSize from start of match sequence, with length of match
            myStrings.emplace_back(iter->first, iter->length());
        }

        return myStrings;
    }

    /// @brief Convert to lowercase
    void toLower() {
        for (size_t i = 0; i < m_contentLength; i++) {
            m_cStr[i] = tolower(m_cStr[i]);
        }
    }
    GStringFixedSize asLower() const {
        GStringFixedSize c(*this);
        c.toLower();
        return c;
    }

    /// @brief Convert to uppercase
    GStringFixedSize& toUpper() {
        for (size_t i = 0; i < m_contentLength; i++) {
            m_cStr[i] = toupper(m_cStr[i]);
        }
        return *this;
    }
    GStringFixedSize asUpper() const {
        GStringFixedSize c(*this);
        c.toUpper();
        return c;
    }

    bool isEmpty() const {
        return m_contentLength == 0;
    }

    /// @brief Remove the specified number of characters from the end of the string
    void trim(Int32_t numChars) 
    {
        if (0 == numChars) { return; }

        m_contentLength -= numChars;
        m_cStr[m_contentLength] = 0;
    }

	/// @}

protected:
    friend class GStringView;

    /// @brief Copy the metadata from the other GStringFixedSize
    inline void copyMetadata(const GStringFixedSize& other) {
        m_contentLength = other.m_contentLength;
    }

    /// @brief Initialize the GStringFixedSize from a char array and a length
    template<typename CharType>
    inline void initialize(const CharType* c_str, Int32_t len) {

        static_assert(std::is_integral_v<CharType>,
            "CharType must be an integral type");

        constexpr Uint32_t charByteSize = sizeof(CharType);
        m_contentLength = len * charByteSize;
        if (!c_str) {
            m_cStr[m_contentLength] = 0; // Add null terminator
        }
        else {
            memcpy(m_cStr.data(), c_str, m_contentLength); // since char is 1 byte
            m_cStr[m_contentLength] = 0; // Add null terminator
        }
    }

    /// @name Protected Members
    /// @{
    /// @}
};

} /// End rev namespace


// Std namespace for hash function
namespace std {
template <rev::Uint32_t ReservedSizeBytes> 
struct hash<rev::GStringFixedSize<ReservedSizeBytes>>
{
    // djb2 hash algorithm
    //http://www.cse.yorku.ca/~oz/hash.html
    size_t operator()(const rev::GStringFixedSize<ReservedSizeBytes>& s) const
    {
        const char* cStr = s.c_str();
        if (!cStr) {
            return 0;
        }

        size_t h = 5381;
        rev::Int32_t c;
        while ((c = *cStr++)) {
            // Older version
            h = ((h << 5) + h) + c; /* hash * 33 + c */
            //h = ((h << 5) + h) ^ c; // newer version, but slightly slower
        }
        //return XXHash32::hash(s.c_str(), s.length(), 0); // Tried alternative hash, a bit slower
        return h;
    }
};
} ///< End std namespace
