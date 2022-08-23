#pragma once

// std
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdexcept> // for raising error in Format
#include <regex>

// Internal
#include "fortress/types/GSizedTypes.h"
#include "fortress/json/GJson.h"


namespace rev {

/// Forward Declarations
template<typename ...Args> class SerializationProtocol;
template<typename T> struct SerializationProtocolField;
class FileStream;

/// Type Defs
typedef SerializationProtocol<char*> GStringProtocol;
typedef SerializationProtocolField<char*> GStringProtocolField;


/// @class GStringInterface
/// @brief Base string class
/// @details The char array contained by GStringInterface is always null-terminated
template<typename CharArrayType>
class GStringInterface {
protected:

    /// @brief Destruction, protected to enforce that this is an in terface
    ~GStringInterface() {}

    /// @name Static
    /// @{

    /// @brief Join the given strings with the given separator
    template<typename StringType>
    static StringType Join_(const std::vector<StringType>& strings, const StringType& separator)
    {
        static_assert(std::is_base_of_v<GStringInterface, StringType>, "Invalid string type");

        StringType outStr;
        Uint32_t lastIndex = strings.size() - 1;
        for (Uint32_t i = 0; i < strings.size(); i++) {
            outStr += strings[i];
            if (i != lastIndex) {
                outStr += separator;
            }
        }
        return outStr;
    }

    /// @brief Obtain StringType from a number
    template<
        typename StringType,
        typename T,
        typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type> // Only allow arithmetic
    static StringType FromNumber_(const T& number) {
        static_assert(std::is_base_of_v<GStringInterface, StringType>, "Invalid string type");

        // Get required length of string
        StringType str;
        if constexpr (std::is_same<T, double>::value || std::is_same<T, float>::value) {
            std::stringstream ss;
            ss << number;
            std::string sstr = ss.str();
            str = StringType(sstr.c_str());
        }
        else {
            // Wrap std::string conversion
            str = StringType(std::to_string(number));
        }
        return str;
    }

    /// @brief Format a GStringInterface like a const char*        
    /// @details Write to a char* using std;:snprintf and then convert to a std::string
    /// @see https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
    template<typename StringType, typename ... Args>
    static StringType Format_(const StringType& format, Args ... args)
    {
        // Check formatting with a null char pointer, to avoid silent errors
        Int32_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
        if (size <= 0) { throw std::runtime_error("Error during formatting."); }

        // Allocate a buffer of the appropriate size
        StringType str(size - 1, ' '); // Don't need the null character at end, so subtract 1 from size
        snprintf(str.c_str()/*&str[0] for std::string*/, size, format.c_str(), args ...); // Need to write full size, or lose last character
        return str;
    }

public:

    typedef char* GStringIterator;
    typedef const char* GStringConstIterator;

    /// @brief Convert a hex string to an unsigned integer
    /// @param[in] endPtr the next character in str after the numerical value
    static Uint64_t HexToInt(const char* str, char** outEndPtr = nullptr) {
        return strtoull(str, outEndPtr, 16);
    }

    /// @brief Reentrant version of string tokenization
    /// @param[in] savePtr A pointer used to avoid a static char* in the implementation of strtok, must be passed as a reference to modify the original ptr. Only used internally
    static char* Strtok_r(char* str, const char* delim, char*& savePtr) {
        char* tok = nullptr;
        char* match = nullptr;

        // If the delimiter is invalid, return nullptr
        if (delim == nullptr) {
            return nullptr;
        }

        // If the input string is null, strtok resumes from the most recent location (saveptr)
        tok = (str) ? str : savePtr;
        if (tok == nullptr) {
            return nullptr;
        }

        // Find the first match of the delimiter token in the string
        match = strstr(tok, delim);

        // TODO: Create a strtok version that doesn't even modify the original str
        if (match) {
            savePtr = match + strlen(delim);
            match[0] = '\0';
        }
        else {
            savePtr = nullptr;
        }

        return tok;
    }

    /// @brief Get the length of the string with the specified format and format arguments
    /// @details Length does not include null terminator
    template<typename ...Args>
    static Int32_t GetLength(const char* format, Args ...args) {
        constexpr Uint32_t s_maxLenFormatString = 128;
        static char dummy[s_maxLenFormatString];
        return ::snprintf(dummy, s_maxLenFormatString, format, args...);
    }

    /// @}

	/// @name Constructors
	/// @{

    /// @brief Default Construction
    GStringInterface() = default;

	/// @}

    /// @name Operators
    /// @{
    /// @}

    /// @name Properties
    /// @{

    /// @}

	/// @name Public Methods
	/// @{

	/// @}

protected:

    /// @name Protected Members
    /// @{

    CharArrayType m_cStr {}; ///< The char array associated with this string

    /// @brief The size of the non-empty char array contents associated with this str (not including null terminator or reserved space)
    /// @details Since space can be reserved in a string, the contents may not take up the entire array
    /// @example A string's contents may look like "hello\0RESERVED_SPACE_HERE...." The length would be 5
    Uint32_t m_contentLength = 0;

    /// @}
};

} /// End rev namespace
