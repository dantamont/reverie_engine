#pragma once

#include "fortress/types/GStringInterface.h"

namespace rev {

/// @class GString
/// @brief String class
/// @details The char array contained by GString is always null-terminated
class GString: public GStringInterface<char*> {
public:

    using Base = GStringInterface<char*>;
    using typename Base::GStringIterator;
    using typename Base::GStringConstIterator;

    /// @name Static
    /// @{

    /// @brief Join the given strings with the given separator
    static GString Join(const std::vector<GString>& strings, const GString& separator)
    {
        return Base::Join_(strings, separator);
    }

    /// @brief Obtain GString from number
    template<
        typename T, 
        typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type> // Only allow arithmetic
    static GString FromNumber(const T& number)
    {
        return Base::FromNumber_<GString>(number);
    }

    /// @brief Format a GString like a const char*        
    /// @details Write to a char* using std;:snprintf and then convert to a std::string
    /// @see https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
    template<typename ... Args>
    static GString Format(const GString& format, Args ... args)
    {
        return GStringInterface::Format_(format, args...);
    }

    /// @}

	/// @name Constructors/Destructor
	/// @{

    /// @brief Default Construction
    GString();

    /// @brief Construction from a single char
    explicit GString(char c_str);

    /// @brief Construction from a length and a fill character
    GString(Uint32_t len, char chr);

    GString(const std::string& str);

    /// @brief Construct with a null-terminated char array
    GString(const char* c_str);

    /// @brief Construct with a char array and the number of characters
    /// @param[in] len The number of characters
    GString(const char* c_str, Uint32_t len);

    /// @brief Construct with a null-terminated char array. Could be UTF-8
    /// @param[in] len The number of bytes in the char array
    GString(const Char8_t* c_str);

    /// @brief Construct with a null-terminated char array. Could be UTF-8
    /// @param[in] len The number of bytes in the char array
    GString(const Char8_t* c_str, Uint32_t len);

    /// @brief Construct with a null-terminated Char16_t array. Could be UTF-16
    /// @param[in] len The number of bytes divided by 2, i.e. the number of characters and surrogate half-pairs
    /// @note Length is required as multi-byte characters may contain the null character
    GString(const Char16_t* c_str, Uint32_t len);

    /// @brief Construct with a null-terminated Char32_t array. Could be UTF-32
    /// @param[in] len The number of characters
    /// @note Length is required as multi-byte characters may contain the null character
    GString(const Char32_t* c_str, Uint32_t len);

    /// @brief Copy Construction
    GString(const GString& other);
    
    /// @brief Move construction
    GString(GString&& other) noexcept;

    /// @brief Destruction
    ~GString();

	/// @}

    /// @name Operators
    /// @{

    /// @brief Override stream insertion operator
    friend std::ostream& operator<<(std::ostream& os, const GString& gStr)
    {
        os << gStr.c_str();
        return os;
    }

    const GString operator+(const GString& other) const {
        GString str = *this;
        str += other.c_str();
        return str;
    }

    const GString operator+(const char* other) const {
        GString str = *this;
        str += other;
        return str;
    }

    const GString operator+(const char other) const;

    inline friend const GString operator+(const char* c, const GString &g) {
        GString gc(c);
        gc += g;
        return gc;
    }

    GString& operator+=(const GString& other) {
        *this += other.c_str();
        return *this;
    }

    GString& operator+=(const char* chars);
    GString& operator+=(const char c);

    GString& operator=(const GString& other) // copy assignment
    {
        if (this == &other) {
            return *this;
        }
        initialize(other.m_cStr, other.m_contentLength, other.m_reservedSize);
        return *this;
    }

    GString& operator=(GString&& other) noexcept // move assignment
    {
        std::swap(m_cStr, other.m_cStr);
        copyMetadata(other);
        return *this;
    }

    bool operator==(const GString& other) const
    {
        return operator==(other.m_cStr);
    }

    bool operator==(const char& other) const
    {
        return m_contentLength == 1 && m_cStr[0] == other;
    }

    inline friend bool operator==(const char c, const GString& g) {
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
        if (nullptr == m_cStr) {
            if (nullptr == other) {
                return true;
            }
            else {
                return false;
            }
        }
        else if(nullptr == other){
            return false;
        }
        return strcmp(const_cast<const char*>(m_cStr), const_cast<const char*>(other)) == 0;
    }

    inline friend bool operator==(const char* c, const GString& g) {
        return g == c;
    }

    bool operator!=(const GString& other) const
    {
        return operator!=(other.m_cStr);
    }

    bool operator!=(const char* other) const
    {
        if (nullptr == m_cStr) {
            if (nullptr == other) {
                return false;
            }
            else {
                return true;
            }
        }
        else if (nullptr == other) {
            return true;
        }
        return strcmp(const_cast<const char*>(m_cStr), other) != 0;
    }

    bool operator<(const GString& other) const {
        return *this < other.m_cStr;
    }

    bool operator<(const char* chars) const {
        if (!m_cStr) {
            return true;
        }
        return strcmp(const_cast<const char*>(m_cStr), const_cast<const char*>(chars)) < 0;
    }

    bool operator>(const GString& other) const {
        return *this > other.m_cStr;
    }

    bool operator>(const char* chars) const {
        if (!m_cStr) {
            return false;
        }
        return strcmp(const_cast<const char*>(m_cStr), const_cast<const char*>(chars)) > 0;
    }

    template<typename IntegerType>
    const char& operator[](IntegerType idx) const {
        static_assert(std::is_integral_v<IntegerType>, "Invalid type for GString::operator[]");
        return m_cStr[idx];
    }

    template<typename IntegerType>
    char& operator[](IntegerType idx) {
        static_assert(std::is_integral_v<IntegerType>, "Invalid type for GString::operator[]");
        return m_cStr[idx];
    }

    explicit operator char() const {
        return m_cStr[0];
    }

    explicit operator char*()
    {
        return m_cStr;
    }

    operator const char* () const
    {
        return m_cStr;
    }

    /// @brief Read from a file stream
    friend FileStream& operator>>(FileStream& stream, GString& str);
    friend GString& operator<<(GString& str, FileStream& fs);

    /// @brief Write to a file streeam
    friend FileStream& operator<<(FileStream& stream, GString& str);
    friend GString& operator>>(GString& str, FileStream& fs);


    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson 
    /// @param korObject
    friend void to_json(nlohmann::json& orJson, const GString& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson 
    /// @param orObject 
    friend void from_json(const nlohmann::json& korJson, GString& orObject);


    /// @}

    /// @name Properties
    /// @{

    const char* c_str() const {
        return m_cStr;
    }
    char* c_str() {
        return m_cStr;
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
    Uint32_t reservedSize() const {
        return m_reservedSize;
    }

    /// @}

	/// @name Public Methods
	/// @{

    /// @brief Copy into an array
    void copyInto(Uint8_t* array, Uint32_t maxNumBytes) const;

    /// @brief Return a sub-string of the GString
    /// @param[in] pos the starting index
    /// @param[in] len the length of the sub-string. If negative, is remaining length of the string
    GString subStr(Int32_t pos, Int32_t len = -1) const;

    /// @brief Iterator for range-based for loop
    GStringIterator begin() {
        if (m_cStr) {
            return &m_cStr[0];
        }
        else {
            return nullptr;
        }
    }
    GStringIterator end() {
        if (m_cStr) {
            return &m_cStr[0] + m_contentLength;
        }
        else {
            return nullptr;
        }
    }

    /// @brief Constant iterator for range-based for loop
    GStringConstIterator begin() const {
        if (m_cStr) {
            return &m_cStr[0];
        }
        else {
            return nullptr;
        }
    }
    GStringConstIterator end() const {
        if (m_cStr) {
            return &m_cStr[0] + m_contentLength;
        }
        else {
            return nullptr;
        }
    }

    /// @brief Checks that the string is an integer
    bool isInteger(Int32_t* outInt = nullptr) const;

    /// @brief Converts the string to a number, returning 0 if there is no valid conversion
    Int32_t toInt() const {
        return atoi(m_cStr);
    }

    Float64_t toDouble() const {
        return atof(m_cStr);
    }

    /// @brief Create a protocol field from the GString
    GStringProtocolField createProtocolField() const;

    /// @brief Whether or not this string contains the specified substring
    bool contains(const char* c_str) const {
        if (!m_cStr) {
            return false;
        }
        else {
            return strstr(m_cStr, c_str) != nullptr;
        }
    }
    bool contains(const GString& g_str) const {
        return contains(g_str.c_str());
    }

    /// @brief Resize the GString's internal array
    void resize(Uint32_t newSize);

    /// @brief Replace char in string with the specified char
    GString& replace(const char& find, const char& newChar);

    /// @brief Replace sub-string with the specified sub-string
    GString& replace(const char* old, const char* replacement);
    GString& replace(const GString& old, const GString& replacement);

    /// @brief Split the string using any of the specified delimiters
    void split(const char* delims, std::vector<GString>& outStrs) const;
    std::vector<GString> split(const char* delims) const;

    /// @brief Split the string using the regex as a token
    /// @see https://stackoverflow.com/questions/16749069/c-split-string-by-regex/16752826
    std::vector<GString> split(const std::regex& regex) const;

    /// @brief Convert to lowercase
    void toLower() {
        for (size_t i = 0; i < m_contentLength; i++) {
            m_cStr[i] = tolower(m_cStr[i]);
        }
    }
    GString asLower() const {
        GString c(*this);
        c.toLower();
        return c;
    }

    /// @brief Convert to uppercase
    GString& toUpper() {
        for (size_t i = 0; i < m_contentLength; i++) {
            m_cStr[i] = toupper(m_cStr[i]);
        }
        return *this;
    }
    GString asUpper() const {
        GString c(*this);
        c.toUpper();
        return c;
    }

    GString capitalized() const {
        GString c(*this);
        c.m_cStr[0] = toupper(c.m_cStr[0]);
        return c;
    }

    GString lowerCasedFirstLetter() const {
        GString c(*this);
        c.m_cStr[0] = tolower(c.m_cStr[0]);
        return c;
    }

    bool isNull() const {
        return m_cStr == nullptr;
    }

    bool isEmpty() const {
        return m_contentLength == 0;
    }

    /// @brief Remove the specified number of characters from the end of the string
    void trim(Int32_t numChars);

	/// @}

protected:
    friend class GStringView;

    /// @brief Copy the metadata from the other GString
    inline void copyMetadata(const GString& other) {
        m_contentLength = other.m_contentLength;
        m_reservedSize = other.m_reservedSize;
    }

    /// @brief Initialize the GString from a char array and a length
    template<typename CharType>
    inline void initialize(const CharType* c_str, Int32_t len, Int32_t reservedSize) {

        static_assert(std::is_integral_v<CharType>,
            "CharType must be an integral type");

        if (m_cStr) {
            delete[] m_cStr;
        }

        constexpr Uint32_t charByteSize = sizeof(CharType);
        m_contentLength = len * charByteSize;
        m_reservedSize = m_contentLength + 1; // Plus one for null terminator
        if (!c_str) {
            if (0 == m_contentLength) {
                m_cStr = nullptr;
            }
            else {
                m_reservedSize = m_reservedSize > reservedSize ? m_reservedSize: reservedSize;
                m_cStr = new char[m_reservedSize];
                m_cStr[m_contentLength] = 0; // Add null terminator
            }
        }
        else {
            m_reservedSize = m_reservedSize > reservedSize ? m_reservedSize : reservedSize;
            m_cStr = new char[m_reservedSize];
            memcpy(m_cStr, c_str, m_contentLength); // since char is 1 byte
            m_cStr[m_contentLength] = 0; // Add null terminator
        }
    }

    /// @name Protected Members
    /// @{

    Uint32_t m_reservedSize = 0; ///< The size of the char array associated with this str

    /// @}
};

} /// End rev namespace



// Std namespace for hash function
namespace std {
template <> struct hash<rev::GString>
{
    // djb2 hash algorithm
    //http://www.cse.yorku.ca/~oz/hash.html
    size_t operator()(const rev::GString& s) const
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
