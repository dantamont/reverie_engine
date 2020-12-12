/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_STRING_VIEW_H
#define GB_STRING_VIEW_H


// Internal
#include "GbString.h"
#include "../encoding/xxhash/xxhash32.h"

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
// Defines
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Type Defs
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////


/// @brief String viewer class
/// @details Encapsulates a string without making a copy
class GStringView {
public:

    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{

    /// @brief Default construction
    GStringView();

    /// @brief Construct with a null-terminated char array
    GStringView(const char* c_str);

    /// @brief Construct with a char array and the number of characters
    GStringView(const char* c_str, size_t len);

    /// @brief View for an already constructed GString
    GStringView(const GString& other);

    /// @brief Destruction
    ~GStringView();

	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    operator const char* ()
    {
        return m_cStr;
    }

    /// @brief Override stream insertion operator
    friend std::ostream& operator<<(std::ostream& os, const GStringView& gStr)
    {
        os << gStr.m_cStr;
        return os;
    }

    const GString operator+(const GString& other) const {
        GString str = GString(m_cStr);
        str += other.m_cStr;
        return str;
    }

    const GString operator+(const char* other) const {
        GString str = GString(m_cStr);
        str += other;
        return str;
    }

    inline friend const GString operator+(const char* c, const GStringView &g) {
        GString gc(c);
        gc += g.m_cStr;
        return gc;
    }

    GStringView& operator=(const GString& other) // copy assignment
    {
        return *this = GStringView(other);
    }
    GStringView& operator=(const GStringView& other) // copy assignment
    {
        m_cStr = other.m_cStr;
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
    bool operator==(const GStringView& other) const
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
    bool operator>(const GStringView& other) const {
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

    /// @brief The size of the char array associated with this str (not including null terminator)
    const size_t& length() const {
        return m_len;
    }

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

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

 
    GString asLower() const {
        GString c(*this);
        c.toLower();
        return c;
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

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief The dynamically managed char array associated with this string
    const char* m_cStr = nullptr;

    /// @brief The size of the char array associated with this str (not including null terminator)
    size_t m_len = 0;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End Gb namespace


// Std namespace for hash function
namespace std {
template <> struct hash<Gb::GStringView>
{
    // djb2 hash algorithm
    //http://www.cse.yorku.ca/~oz/hash.html
    size_t operator()(const Gb::GStringView & s) const
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