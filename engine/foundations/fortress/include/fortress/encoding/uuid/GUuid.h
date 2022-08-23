#pragma once

/// Public includes
#include <array>
#include <sstream>
#include <iomanip>
#include <mutex>
#include "fortress/types/GSizedTypes.h"
#include "fortress/types/GString.h"
#include "fortress/layer/framework/GFlags.h"
#include "fortress/encoding/extern/tinysha1/TinySHA1.hpp"

namespace rev {

/// Forward declarations
class GString;

/** @class Uuid
    @brief  A type representing a Uuid
    @details A Universally Unique Identifier (Uuid) is a standard way to uniquely identify entities
        in a distributed computing environment
    @see https://docs.microsoft.com/en-us/windows/win32/api/guiddef/ns-guiddef-guid
    @see https://en.wikipedia.org/wiki/Universally_unique_identifier
*/
class Uuid {
public:

    enum class StringFormatFlag {
        kWithoutBraces = 1 << 0
    };
    MAKE_FLAGS(StringFormatFlag, StringFormatFlags)

    /// @name Static
    /// @{
    
    /// @brief Generate a unique name
    static GString UniqueName(const GString& prefix) {
        return prefix + UniqueName();
    }

    static GString UniqueName() {
        return Uuid(true).createUniqueName();
    }

    static Uuid NullID() {
        return Uuid(false);
    }

    /// @}

    /// @name Constructors and Destructors
    /// @{
    Uuid(const GString& str)
    {
        /// @todo, use hexToInt
        const char* start;
        if (str[0] == '{') {
            start = &str[1];
        }
        else {
            start = &str[0];
        }
        
        /// Read first block
        // First four bytes
        char* nextBlock;
        m_data1 = GString::HexToInt(start, &nextBlock);

        /// Read second block
        // Next two bytes
        nextBlock += 1;
        m_data2 = GString::HexToInt(nextBlock, &nextBlock);

        /// Read third block
        // Next two bytes
        nextBlock += 1;
        m_data3 = GString::HexToInt(nextBlock, &nextBlock);

        /// Read fourth block
        // Next two bytes
        nextBlock += 1;
        Uint16_t nextTwoBytes = GString::HexToInt(nextBlock, &nextBlock);
        m_data4[0] = static_cast<Uint8_t>((nextTwoBytes & 0xff00) >> 8);
        m_data4[1] = static_cast<Uint8_t>(nextTwoBytes & 0x00ff);

        // Next six bytes
        nextBlock += 1;
        Uint64_t nextSixBytes = GString::HexToInt(nextBlock, &nextBlock);
        m_data4[2] = static_cast<Uint8_t>((nextSixBytes & 0xff0000000000) >> 40);
        m_data4[3] = static_cast<Uint8_t>((nextSixBytes & 0x00ff00000000) >> 32);
        m_data4[4] = static_cast<Uint8_t>((nextSixBytes & 0x0000ff000000) >> 24);
        m_data4[5] = static_cast<Uint8_t>((nextSixBytes & 0x000000ff0000) >> 16);
        m_data4[6] = static_cast<Uint8_t>((nextSixBytes & 0x00000000ff00) >> 8);
        m_data4[7] = static_cast<Uint8_t>(nextSixBytes & 0x0000000000ff);
    }
    explicit Uuid(Uint32_t id) :
        m_data1(id)
    {
    }
    explicit Uuid(Uint64_t id) :
        m_data1((id & 0xffffffff00000000) >> 32), ///< Take left-most four bytes
        m_data2((id & 0x00000000ffff0000) >> 16 ), ///< Take next two bytes to the right
        m_data3(id & 0x000000000000ffff) ///< Take the final, right-most two bytes
    {
    }

    explicit Uuid(bool generate = true)
    {
        if (generate) {
            makeUuid(*this);
        }
    }

    ~Uuid() 
    {
    }

    /// @}

    /// @name Operators
    /// @{

    /// @note Will only preserve ID for manually-specified Uuids that are smaller than sizeof(unsigned int)
    explicit operator Uint64_t() const {
        return reinterpret_cast<const Uint64_t&>(*this);
    }
    explicit operator Uint32_t() const {
        return m_data1;
    }

    explicit operator GString() const {
        return asString();
    }

    bool operator==(const Uuid& other) const {
        return m_data1 == other.m_data1 &&
            m_data2 == other.m_data2 &&
            m_data3 == other.m_data3 &&
            m_data4 == other.m_data4;
    }

    bool operator!=(const Uuid& other) const {
        if (m_data1 != other.m_data1) {
            return true;
        }
        else if(m_data2 != other.m_data2){
            return true;
        }
        else if (m_data3 != other.m_data3) {
            return true;
        }
        else if (m_data4 != other.m_data4) {
            return true;
        }
        else {
            return false;
        }
    }

    /// @}

    /// @name Public methods
    /// @{

    /// @brief Get data1 field
    Uint32_t data1() const { return m_data1; }

    /// @brief Get data2 field
    Uint16_t data2() const { return m_data2; }

    /// @brief Get data3 field
    Uint16_t data3() const { return m_data3; }

    /// @brief Get data4 field
    const std::array<Uint8_t, 8>& data4() const { return m_data4; }

    /// @brief Return whether or not the UUID is null
    bool isNull() const {
        return m_data4[0] == 0 && m_data4[1] == 0 && m_data4[2] == 0 && m_data4[3] == 0 &&
            m_data4[4] == 0 && m_data4[5] == 0 && m_data4[6] == 0 && m_data4[7] == 0 &&
            m_data1 == 0 && m_data2 == 0 && m_data3 == 0;
    }

    /// @brief Obtain string representation of the UUID
    /// @details Format is {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}
    /// @see https://stackoverflow.com/questions/5100718/integer-to-hex-string-in-c
    GString asString(Uuid::StringFormatFlags format = 0) const {
        std::stringstream stream;

        if (!format.testFlag(Uuid::StringFormatFlag::kWithoutBraces)) {
            stream << '{';
        }

        stream << std::setfill('0') << std::setw(sizeof(Uint32_t) * 2) << std::hex << m_data1
            << '-'
            << std::setfill('0') << std::setw(sizeof(Uint16_t) * 2) << std::hex << m_data2
            << '-'
            << std::setfill('0') << std::setw(sizeof(Uint16_t) * 2) << std::hex << m_data3
            << '-'
            << std::setfill('0') << std::setw(sizeof(Uint16_t) * 2) << std::hex << ((static_cast<Uint16_t>(m_data4[0]) << 8) | static_cast<Uint16_t>(m_data4[1]))
            << '-';

        /// @todo Break this out into a function
        Uint64_t lastBlock = static_cast<Uint64_t>(m_data4[7]) & 0xff;
        lastBlock |= (static_cast<Uint64_t>(m_data4[6]) << 8)  & 0xff00;
        lastBlock |= (static_cast<Uint64_t>(m_data4[5]) << 16) & 0xff0000;
        lastBlock |= (static_cast<Uint64_t>(m_data4[4]) << 24) & 0xff000000;
        lastBlock |= (static_cast<Uint64_t>(m_data4[3]) << 32) & 0xff00000000;
        lastBlock |= (static_cast<Uint64_t>(m_data4[2]) << 40) & 0xff0000000000;

        stream << std::setfill('0') << std::setw(12) << std::hex << lastBlock;

        if (!format.testFlag(Uuid::StringFormatFlag::kWithoutBraces)) {
            stream << '}';
        }

        return stream.str().c_str();
    }

    /// @brief Generate a unique name from the UUID
    GString createUniqueName() const {
        return asString(Uuid::StringFormatFlag::kWithoutBraces);
    }

    GString createUniqueName(const GString& prefix) const {
        return prefix + asString(Uuid::StringFormatFlag::kWithoutBraces);
    }

    /// @}

    /// @name Friend methods
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson 
    /// @param korObject
    friend void to_json(nlohmann::json& orJson, const Uuid& korObject) {
        orJson = korObject.asString();
    }

    /// @brief Convert from JSON to the given class type
    /// @param korJson 
    /// @param orObject 
    friend void from_json(const nlohmann::json& korJson, Uuid& orObject) {
        orObject = Uuid(korJson.get<GString>());
    }

    /// @}

private:
    friend std::hash<Uuid>; ///< So hash can access private members

    /// @brief  Constuct Uuid from raw byte data
    template<typename ValueType>
    explicit Uuid(ValueType* data)
    {
        // Copy data into Uuid. Doesn't check size so unsafe for use outside class
        memcpy(this, data, sizeof(Uuid));
    }

    /// @brief  Generate a UUID using SHA1
    /// @param outID The output UUID
    /// @see https://github.com/mariusbancila/stduuid
    static void makeUuid(Uuid& outId) {
        static sha1::SHA1 hasher; 
        static std::mutex makeMutex;

        sha1::SHA1::digest8_t digest;

        // Need to lock in case called from multiple threads
        makeMutex.lock();
        hasher.getDigestBytes(digest);
        makeMutex.unlock();

        // variant must be 0b10xxxxxx
        digest[8] &= 0xBF;
        digest[8] |= 0x80;

        // version must be 0b0101xxxx
        digest[6] &= 0x5F;
        digest[6] |= 0x50;

        outId = Uuid{ digest };
    }

    Uint32_t m_data1{ 0 }; ///< Specifies the first 8 hexadecimal digits of the UUID.
    Uint16_t m_data2{ 0 }; ///< Specifies first group of 4 hexadecimal digits of the UUID.
    Uint16_t m_data3{ 0 }; ///< Specifies second group of 4 hexadecimal digits of the UUID.
    std::array<Uint8_t, 8> m_data4{ 0 }; ///< Array of 8 bytes. The first 2 bytes contain the third group of 4 hexadecimal digits. The remaining 6 bytes contain the final 12 hexadecimal digits.
};

} // End rev namespace


///@ brief Define std::hash overload for rev::Uuid
namespace std {

template <> struct std::hash<rev::Uuid>
{
    size_t operator()(const rev::Uuid& uuid) const noexcept
    {
        return uuid.m_data1 ^ uuid.m_data2 ^ (uuid.m_data3 << 16)
            ^ ((uuid.m_data4[0] << 24) | (uuid.m_data4[1] << 16) | (uuid.m_data4[2] << 8) | uuid.m_data4[3])
            ^ ((uuid.m_data4[4] << 24) | (uuid.m_data4[5] << 16) | (uuid.m_data4[6] << 8) | uuid.m_data4[7])
            ^ 7; // Seed, could be anything
    }
};

} // End std namespace
