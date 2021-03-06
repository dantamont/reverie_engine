#include "GString.h"
#include "../encoding/GProtocol.h"
#include "GStringView.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
GString::GString()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
GString::GString(char c_str)
{
    if (m_cStr) {
        delete[] m_cStr;
    }

    m_len = 1;
    m_cStr = new char[2]; 
    m_cStr[0] = c_str;
    m_cStr[1] = 0; // Add null terminator
}
/////////////////////////////////////////////////////////////////////////////////////////////
GString::GString(const char * c_str):
    GString(c_str, c_str? strlen(c_str): 0)
{
}
/////////////////////////////////////////////////////////////////////////////////////////
GString::GString(const char * c_str, size_t len)
{
    if (m_cStr) {
        delete[] m_cStr;
    }

    if (!c_str) {
        m_len = 0;
        m_cStr = nullptr;
    }
    else{
        m_len = len;
        int strSize = m_len + 1; // strlen doesn't count null terminator
        m_cStr = new char[strSize];
        memcpy(m_cStr, c_str, m_len); // since char is 1 byte
        m_cStr[m_len] = 0; // Add null terminator
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
GString::GString(const QString & q_str):
    GString(q_str.toStdString().c_str(), q_str.size())
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
GString::GString(const GString & other):
    GString(other.c_str())
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
GString::GString(GString && other)
{
    std::swap(m_cStr, other.m_cStr);
    m_len = other.m_len;
}
/////////////////////////////////////////////////////////////////////////////////////////////
GString::~GString()
{
    if (m_cStr) {
        delete[] m_cStr;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
GString & GString::operator+=(const char * chars)
{
    if (!chars) {
        return *this;
    }

    // Create temp char array
    size_t charLen = strlen(chars);
    size_t newArraySize = m_len + charLen;
    size_t newLen = newArraySize + 1;
    char* tempStr = new char[newLen];

    // Copy current contents into temp array
    if (m_cStr) { // This may be a null str, which won't have an allocated member
        memcpy(tempStr, m_cStr, m_len * sizeof(char));
        delete[] m_cStr; // Can't be having a memory leak
    }
    m_cStr = tempStr;

    // Copy other contents into char array, and null-terminate
    memcpy(m_cStr + m_len, chars, charLen * sizeof(char));
    m_cStr[newArraySize] = 0;

    // Set new length
    m_len = newArraySize;

    return *this;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool GString::isInteger(int * outInt) const
{
    if (!m_len) {
        // No length, so can't be an integer
        if (outInt) {
            outInt = 0;
        }
        return false;
    }

    // Check if is an integer using atoi
    int res = toInt();
    if (outInt) {
        *outInt = res;
    }

    // This string is an integer if either
    // 1) atoi returned a non-zero result or
    // 2) atoi returned a zero result, but the string is actually a single-digit string representation of 0
    return res > 0 || (m_len == 1 && isdigit(m_cStr[0]));
}
/////////////////////////////////////////////////////////////////////////////////////////////
GStringProtocolField GString::createProtocolField() const
{
    ProtocolField<char*> charField(m_cStr, m_len + 1);
    return charField;
}
/////////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////
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

        // Adjust pointers, move on
        tmp = p + old_len;
        tempView = GStringView(tmp);
    }

    // Write altered string back to member
    // FIXME: Might be overflowing, I should be resizing m_cStr member to accomodate size increase
    strcpy(m_cStr, buffer);

    return *this;
}
/////////////////////////////////////////////////////////////////////////////////////////////
GString & GString::replace(const GString & old, const GString & replacement)
{
    return replace((const char*)old, (const char*)replacement);
}
/////////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////
void GString::trim(int numChars) 
{
    // TODO: Test me

    // Update length
    m_len -= numChars;

    // Copy trimmed part of string into buffer
    char* buffer = new char[m_len + 1];
    strcpy_s(buffer, m_len + 1, m_cStr);

    // Delete old string member
    delete m_cStr;

    // Assign buffer as string member
    m_cStr = buffer;
}
/////////////////////////////////////////////////////////////////////////////////////////////
FileStream & operator>>(FileStream & fs, GString & str)
{
    // Read from stream
    uint64 count; // m_len + 1
    bool success = fs.readCount(count);
    if (str.m_cStr) {
        delete[] str.m_cStr;
    }
    str.m_cStr = new char[count];
    str.m_len = count - 1;
    success &= fs.readData(str.m_cStr, count);
    Q_UNUSED(success);
    return fs;
}
/////////////////////////////////////////////////////////////////////////////////////////////
GString & operator<<(GString & str, FileStream & fs)
{
    // Read from stream
    fs >> str;
    return str;
}
/////////////////////////////////////////////////////////////////////////////////////////////
FileStream & operator<<(FileStream & fs, GString & str)
{
    // Write to stream
    ProtocolField<char*> charField(str.m_cStr, str.m_len + 1);
    GStringProtocol protocol(charField);
    protocol >> fs;

    return fs;
}
/////////////////////////////////////////////////////////////////////////////////////////////
GString & operator>>(GString & str, FileStream & fs)
{
    // Write to stream
    fs << str;
    return str;
}

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
