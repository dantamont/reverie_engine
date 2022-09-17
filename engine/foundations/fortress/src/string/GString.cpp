#include "fortress/string/GString.h"
#include "fortress/encoding/binary/GSerializationProtocol.h"
#include "fortress/string/GStringView.h"
#include "fortress/json/GJson.h"
#include "fortress/GGlobal.h"

namespace rev {

GString::GString()
{
}


GString::GString(char c_str)
{
    if (m_cStr) {
        delete[] m_cStr;
    }

    m_contentLength = 1;
    m_reservedSize = 2;
    m_cStr = new char[m_reservedSize];
    m_cStr[0] = c_str;
    m_cStr[1] = 0; // Add null terminator
}

GString::GString(Uint32_t len, char chr):
    GString((char*)nullptr, len)
{
    std::fill(m_cStr, m_cStr + len, chr);
}

GString::GString(const std::string& str):
    GString(str.c_str(), str.length())
{
}

GString::GString(const char * c_str):
    GString(c_str, c_str? strlen(c_str): 0)
{
}

GString::GString(const char * c_str, Uint32_t len)
{
    initialize(c_str, len, 0);
}

GString::GString(const Char8_t* c_str)
{
    /// @todo Not called in C++17 with u8 strings. Maybe better luck in C++20
    initialize(
        c_str, 
        c_str ? strlen(reinterpret_cast<const char*>(c_str)) : 0,
        0
    );
}

GString::GString(const Char8_t* c_str, Uint32_t len)
{
    /// @todo Not called in C++17 with u8 strings. Maybe better luck in C++20
    initialize(c_str, len, 0);
}

GString::GString(const Char16_t* c_str, Uint32_t len)
{
    initialize(c_str, len, 0);
}

GString::GString(const Char32_t* c_str, Uint32_t len)
{
    initialize(c_str, len, 0);
}

GString::GString(const GString & other):
    GString(other.c_str(), other.m_contentLength)
{
}

GString::GString(GString && other) noexcept
{
    std::swap(m_cStr, other.m_cStr);
    copyMetadata(other);
}

GString::~GString()
{
    if (m_cStr) {
        delete[] m_cStr;
    }
}

const GString GString::operator+(const char other) const
{
    if (nullptr == m_cStr) {
        // This is null, so just return a GString of the character
        return GString(other);
    }

    GString str(m_cStr, m_contentLength + 1);
    str[m_contentLength] = other;
    return str;
}

GString & GString::operator+=(const char * chars)
{
    if (!chars) {
        return *this;
    }

    // Get new dimensions
    Uint32_t charLen = strlen(chars);
    Uint32_t newMaxIndex = m_contentLength + charLen;
    Uint32_t newArraySize = newMaxIndex + 1;

    if (newArraySize > m_reservedSize) {
        // Need to resize the contained array
        resize(newArraySize);
    }

    // Copy other contents into char array, and null-terminate
    memcpy(m_cStr + m_contentLength, chars, charLen * sizeof(char));
    m_cStr[newMaxIndex] = 0;

    // Set new length
    m_contentLength = newMaxIndex;

    return *this;
}

GString& GString::operator+=(const char c)
{
    // Get new dimensions
    Uint32_t newMaxIndex = m_contentLength + 1;
    Uint32_t newArraySize = newMaxIndex + 1;

    if (newArraySize > m_reservedSize) {
        // Need to resize the contained array
        resize(newArraySize);
    }

    // Copy other contents into char array, and null-terminate
    memcpy(m_cStr + m_contentLength, &c, sizeof(char));
    m_cStr[newMaxIndex] = 0;

    // Set new length
    m_contentLength = newMaxIndex;

    return *this;
}

void GString::copyInto(Uint8_t* arr, Uint32_t maxNumBytes) const
{
    if (m_cStr) {
        // Copy data
        Int32_t copyAmount = std::min(m_contentLength, maxNumBytes);
        memcpy(arr, m_cStr, std::min(m_contentLength, maxNumBytes));

        // Null terminate it
        Int32_t lastIndex = std::min(copyAmount, (Int32_t)maxNumBytes - 1);
        arr[lastIndex] = 0;
    }
}

GString GString::subStr(Int32_t pos, Int32_t len) const
{
    return GString(begin() + pos, len > 0 ? len : m_contentLength - pos);
}

bool GString::isInteger(Int32_t * outInt) const
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

GStringProtocolField GString::createProtocolField() const
{
    SerializationProtocolField<char*> charField(m_cStr, m_contentLength + 1);
    return charField;
}

void GString::resize(Uint32_t newSize)
{
    // Create an array with the new size
    char* tempStr = new char[newSize];

    // Set reserved size
    m_reservedSize = newSize;

    // Copy current contents into temp array
    Uint32_t newMaxIndex = newSize - 1;
    Uint32_t numBytesToCopy = sizeof(char) * std::min(m_contentLength, newMaxIndex/* need room for null terminator*/);
    if (m_cStr) { // This may be a null str, which won't have an allocated member
        memcpy(tempStr, m_cStr, numBytesToCopy);
        tempStr[newMaxIndex] = 0;
        delete[] m_cStr; // Can't be having a memory leak
    }
    m_cStr = tempStr;
}

GString & GString::replace(const char & find, const char & newChar)
{
    if (!m_cStr) {
        return *this;
    }
    char *currentPos = strchr(m_cStr, find);
    while (currentPos) {
        *currentPos = newChar;
        currentPos = strchr(currentPos, find);
    }
    return *this;
}

GString & GString::replace(const char * old, const char * replacement)

{
    // FIXME: Don't use fixed buffer size
    // Initialize buffer to store new string
    constexpr size_t BufferSize = 1024;
    char buffer[BufferSize] = { 0 };
    char *insert_point = &buffer[0];
    const char *tmp = m_cStr;
    GStringView tempView(tmp);

    // Get lengths of old and new substrings
    size_t old_len = strlen(old);
    size_t repl_len = strlen(replacement);
    Int32_t lengthDiff = repl_len - old_len;

    while (true) {
        // Return first occurance of old substr in tmp, or nullptr if not found
        const char *p = strstr(tmp, old);

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

        // Update string length
        m_contentLength += lengthDiff;

        // Adjust pointers, move on
        tmp = p + old_len;
        tempView = GStringView(tmp);
    }

    // Write altered string back to member
    // FIXME: Might be overflowing, I should be resizing m_cStr member to accomodate size increase
    strcpy(m_cStr, buffer);

    return *this;
}

GString & GString::replace(const GString & old, const GString & replacement)
{
    return replace((const char*)old, (const char*)replacement);
}

void GString::split(const char * delims, std::vector<GString>& outStrs) const
{
    // Iterate over string using strtok to split
    GString cpy = *this;
    char * pch;
    char* saveptr = nullptr;
    pch = Strtok_r(cpy.m_cStr, delims, saveptr); // strtok modifies original str by inserting nullptr at delimiters, so need to copy first
    while (pch != nullptr)
    {
        outStrs.emplace_back(GString(pch));
        pch = Strtok_r(nullptr, delims, saveptr); // nullptr continues scanning from last successful call
    }
}

std::vector<GString> GString::split(const char* delims) const
{
    std::vector<GString> strings;
    split(delims, strings);
    return strings;
}

std::vector<GString> GString::split(const std::regex& regex) const
{
    std::vector<GString> myStrings;
    std::regex_token_iterator<GStringConstIterator> iter(begin(),
        end(),
        regex,
        -1);
    std::regex_token_iterator<GStringConstIterator> end;
    for (; iter != end; ++iter) {
        /// Add GString from start of match sequence, with length of match
        myStrings.emplace_back(iter->first, iter->length());
    }

    return myStrings;
}

void GString::trim(Int32_t numChars) 
{
    if (0 == numChars) { return; }
    if (nullptr == m_cStr) { return; }

    // Update length
    m_contentLength -= numChars;
    m_reservedSize = m_contentLength + 1;

    // Copy trimmed part of string into buffer
    char* buffer = new char[m_contentLength + 1];
    memcpy(buffer, m_cStr, m_contentLength);
    buffer[m_contentLength] = 0;

    // Delete old string member
    delete m_cStr;

    // Assign buffer as string member
    m_cStr = buffer;
}

FileStream & operator>>(FileStream & fs, GString & str)
{
    // Read from stream
    Uint64_t count; // m_contentLength + 1
    bool success = fs.readCount(count);
    if (str.m_cStr) {
        delete[] str.m_cStr;
    }
    str.m_cStr = new char[count];
    str.m_contentLength = count - 1;
    str.m_reservedSize = count;
    success &= fs.readData(str.m_cStr, count);
    G_UNUSED(success);
    return fs;
}

GString & operator<<(GString & str, FileStream & fs)
{
    // Read from stream
    fs >> str;
    return str;
}

FileStream & operator<<(FileStream & fs, GString & str)
{
    // Write to stream
    SerializationProtocolField<char*> charField(str.m_cStr, str.m_contentLength + 1);
    GStringProtocol protocol(charField);
    protocol >> fs;

    return fs;
}

GString & operator>>(GString & str, FileStream & fs)
{
    // Write to stream
    fs << str;
    return str;
}


void to_json(json& orJson, const GString& korObject) {
    if (korObject.isEmpty()) {
        orJson = "";
    }
    else {
        orJson = korObject.m_cStr;
    }
}


void from_json(const json& korJson, GString& orObject) {
    orObject = korJson.get_ref<const std::string&>().c_str();
}


} // End namespaces
