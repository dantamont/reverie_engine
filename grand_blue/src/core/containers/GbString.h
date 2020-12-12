/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_STRING_H
#define GB_STRING_H

// std
#include <string>
#include <iostream>
#include <sstream>

// QT
#include <QString>

// Internal
#include "../encoding/xxhash/xxhash32.h"

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
// Defines
/////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_LEN_FMT_STR 128 // Used when converting format/args to string

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
template<typename ...Args> class Protocol;
template<typename T> struct ProtocolField;
class FileStream;

/////////////////////////////////////////////////////////////////////////////////////////////
// Type Defs
/////////////////////////////////////////////////////////////////////////////////////////////
typedef Protocol<char*> GStringProtocol;
typedef ProtocolField<char*> GStringProtocolField;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class GString
/// @brief String class
/// @details The char array contained by GString is always null-terminated
class GString {
public:

    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Obtain GString from number
    template<typename T, 
        typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type> // Only allow arithmetic
    static GString FromNumber(const T& number) {
        // Get required length of string
        GString str;
        if constexpr (std::is_same<T, double>::value || std::is_same<T, float>::value) {
            std::stringstream ss;
            ss << number;
            std::string sstr = ss.str();
            str = GString(sstr.c_str());
        }
        else {
            // Get format of number
            const char* fmt;
            if constexpr (std::is_same<T, size_t>::value) {
                fmt = "%zu";
            }
            else if constexpr (std::is_same<T, int>::value) {
                fmt = "%d";
            }
            else if constexpr (std::is_same<T, unsigned long>::value) {
                fmt = "%lu";
            }
            else if constexpr (std::is_same<T, unsigned long long>::value) {
                fmt = "%llu";
            }
            else if constexpr (true) {
                static_assert(false, "This type is invalid");
            }

            int len = GetLength(fmt, number);
            if (len < 0) {
                throw("Encoding error!");
            }

            // Set length of string, and create
            str.m_len = len; // Add space for null terminator
            str.m_cStr = new char[str.m_len + 1];

            // Set string contents
            sprintf(str.m_cStr, fmt, number);
        }
        return str;
    }

    /// @brief Get the length of the string with the specified format and format arguments
    template<typename ...Args>
    static int GetLength(const char* format, Args ...args) {
        static char dummy[MAX_LEN_FMT_STR];
        return ::snprintf(dummy, MAX_LEN_FMT_STR, format, args...) + 1; // +1 for null terminator
    }

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{

    /// @brief Default Construction
    GString();

    /// @brief Swap construction with char
    //GString(char* c_str);

    /// @brief Construct with a null-terminated char array
    GString(const char* c_str);

    /// @brief Construct with a char array and the number of characters
    GString(const char* c_str, size_t len);

    /// @brief QString compatibility
    GString(const QString& q_str);

    /// @brief Copy Construction
    GString(const GString& other);
    
    /// @brief Move construction
    GString(GString&& other);

    /// @brief Destruction
    ~GString();

	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    operator char*()
    {
        return m_cStr;
    }
    operator const char* ()
    {
        return m_cStr;
    }

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

    GString& operator=(const GString& other) // copy assignment
    {
        return *this = GString(other);
    }

    GString& operator=(GString&& other) noexcept // move assignment
    {
        std::swap(m_cStr, other.m_cStr);
        m_len = other.m_len;
        return *this;
    }

    bool operator==(const GString& other) const
    {
        if (!m_cStr) {
            if (!other.m_cStr) {
                return true;
            }
            else {
                return false;
            }
        }
        return strcmp(const_cast<const char*>(m_cStr), const_cast<const char*>(other.m_cStr)) == 0;
    }

    bool operator==(const char* other) const
    {
        if (!m_cStr) {
            if (!other) {
                return true;
            }
            else {
                return false;
            }
        }
        return strcmp(const_cast<const char*>(m_cStr), const_cast<const char*>(other)) == 0;
    }

    bool operator!=(const GString& other) const
    {
        if (!m_cStr) {
            if (!other.m_cStr) {
                return true;
            }
            else {
                return false;
            }
        }
        return strcmp(const_cast<const char*>(m_cStr), const_cast<const char*>(other.m_cStr)) != 0;
    }

    bool operator!=(const char* other) const
    {
        if (!m_cStr) {
            if (!other) {
                return true;
            }
            else {
                return false;
            }
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

    /// @brief Implicit conversion to QString for use with qDebug
    operator QString() const {
        if (m_cStr) {
            return QString(m_cStr);
        }
        else {
            return QString();
        }
    }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    const char* c_str() const {
        return m_cStr;
    }
    char* c_str() {
        return m_cStr;
    }

    /// @brief The size of the char array associated with this str (not including null terminator)
    const size_t& length() const {
        return m_len;
    }
    size_t& length() {
        return m_len;
    }

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    std::string toStdString() const {
        return std::string(m_cStr);
    }

    /// @brief Read from a file stream m);
    bool read(const FileStream& stream);

    /// @brief Create a protocol from the GString
    /// @note String does not claim ownership of the created protocol, make sure to delete
    GStringProtocol* createProtocol();
    GStringProtocolField createProtocolField();

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

    /// @brief Replace char in string with the specified char
    GString& replace(const char& find, const char& newChar);

    /// @brief Replace sub-string with the specified sub-string
    GString& replace(const GString& old, const GString& replacement);
    GString& replace(const char* old, const char* replacement);

    /// @brief Convert to lowercase
    void toLower() {
        for (size_t i = 0; i < m_len; i++) {
            m_cStr[i] = tolower(m_cStr[i]);
        }
    }
    GString asLower() const {
        GString c(*this);
        c.toLower();
        return c;
    }

    /// @brief Convert to uppercase
    void toUpper() {
        for (size_t i = 0; i < m_len; i++) {
            m_cStr[i] = toupper(m_cStr[i]);
        }
    }
    GString asUpper() const {
        GString c(*this);
        c.toUpper();
        return c;
    }

    bool isNull() const {
        return m_cStr == nullptr;
    }

    bool isEmpty() const {
        return m_len == 0;
    }

	/// @}

protected:
    //friend class ProtocolField<char*>;
    //friend class ModelReader;
    friend class GStringView;

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief The dynamically managed char array associated with this string
    char* m_cStr = nullptr;

    /// @brief The size of the char array associated with this str (not including null terminator)
    size_t m_len = 0;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End Gb namespace


// Std namespace for hash function
namespace std {
template <> struct hash<Gb::GString>
{
    // djb2 hash algorithm
    //http://www.cse.yorku.ca/~oz/hash.html
    size_t operator()(const Gb::GString & s) const
    {
        const char* cStr = s.c_str();
        if (!cStr) {
            return 0;
        }

        size_t h = 5381;
        int c;
        while ((c = *cStr++)) {
            // Older version
            h = ((h << 5) + h) + c; /* hash * 33 + c */
            //h = ((h << 5) + h) ^ c; // newer version, but slightly slower
        }
        //return XXHash32::hash(s.c_str(), s.length(), 0); // Tried alternative hash, a bit slower
        return h;
    }
};
}


/////////////////////////////////////////////////////////////////////////////////////////////
#endif