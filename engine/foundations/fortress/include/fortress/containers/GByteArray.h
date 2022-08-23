#pragma once

// QT

// Internal
#include "fortress/layer/framework/GFlags.h"
#include "fortress/types/GSizedTypes.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

enum class ByteArrayFlag {
    kTakeOwnership = 1 << 0 // If set, byte array takes ownership over data
};
typedef Flags<ByteArrayFlag> ByteArrayFlags;

/// @brief Byte array
/// @details Takes ownership of given data by default, and will delete
template<Uint32_t FlagValue = (Uint32_t)ByteArrayFlag::kTakeOwnership>
class ByteArray{
public:
    static inline const ByteArrayFlags ByteFlags{FlagValue};

	/// @name Constructors/Destructor
	/// @{

    template<typename T>
    ByteArray(T* data):
        m_byteData(reinterpret_cast<Uint8_t*>(data))
    {
    }

    ByteArray() = default;

    ~ByteArray(){
        if (m_byteData && ByteFlags.testFlag(ByteArrayFlag::kTakeOwnership)) {
            delete[] m_byteData;
        }
    }

    /// @}

    /// @name Properties
    /// @{

    Uint8_t* data() { return m_byteData; }
    const Uint8_t* data() const { return m_byteData; }
    bool isNull() const { return nullptr == m_byteData; }

    /// @}

	/// @name Public Methods
	/// @{

    template<typename T>
    T* as() {
        return reinterpret_cast<T*>(m_byteData);
    }

    template<typename T>
    void setData(T* data) {
        m_bytedata = reinterpret_cast<Uint8_t*>(data);
    }

	/// @}

    /// @name Operators
    /// @{

    inline Uint8_t& operator[] (Size_t i) {
        return m_byteData[i];
    }
    inline const Uint8_t& operator[] (Size_t i) const {
        return m_byteData[i];
    }

    /// @}

protected:

    Uint8_t* m_byteData{nullptr}; ///< The data encapsulated by the byte array

};

typedef ByteArray<(Uint32_t)ByteArrayFlag::kTakeOwnership> ManagedByteArray;


} // End rev namespace
