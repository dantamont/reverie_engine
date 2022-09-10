#pragma once

// Internal
#include "fortress/numeric/GSizedTypes.h"
#include <array>

namespace rev {

/// @class SizedBuffer
/// @brief An array of bytes and it's size information
template<Size_t MaxSizeBytes>
class SizedBuffer{
protected:

    using SizedByteArray = std::array<Uint8_t, MaxSizeBytes>;

public:

    SizedBuffer() = default;
    SizedBuffer(Uint8_t fillValue) {
        m_byteArray.fill(fillValue);
    }

    ~SizedBuffer() = default;

    /// @}

    /// @name Properties
    /// @{

    SizedByteArray& byteArray() { return m_byteArray; }
    const SizedByteArray& byteArray() const { return m_byteArray; }

    /// @brief Obtain max size of the underlying array in bytes
    static constexpr Size_t GetMaxSizeBytes() {
        return MaxSizeBytes;
    }

	/// @}


    /// @name Public Methods
    /// @{

    /// @brief Return the buffer
    inline Uint8_t* data() {
        return m_byteArray.data();
    }

    /// @brief Return the buffer
    inline const Uint8_t* data() const {
        return m_byteArray.data();
    }

    /// @brief Return the data in the buffer at the given index
    inline Uint8_t* data(Size_t index) {
#ifdef DEBUG_MODE
        bool validIndex = index < m_byteArray.size();
        if (!validIndex) {
            assert(validIndex && "Index is too large");
        }
#endif
        return m_byteArray.data() + index;
    }

    /// @brief Return the data in the buffer at the given index
    inline const Uint8_t* data(Size_t index) const {
#ifdef DEBUG_MODE
        bool validIndex = index < m_byteArray.size();
        if (!validIndex) {
            assert(validIndex && "Index is too large");
        }
#endif
        return m_byteArray.data() + index;
    }

    /// @brief Set bytes in the buffer
    /// @param[in] object the object being passed in
    /// @param[in] index the index in the buffer at which to insert the object
    /// @return A pointer to the next buffer index following the insertion, or -1 if no insertion occured
    template<typename T>
    Int64_t setData(const T& object, Size_t index = 0) {
#ifdef DEBUG_MODE
        assert(MaxSizeBytes >= sizeof(T) + index && "Max size in bytes exceeded");
#endif
        memcpy(&m_byteArray[index], &object, sizeof(T));
        return index + sizeof(T);
    }

    /// @}

    /// @name Operators
    /// @{

    inline Uint8_t& operator[] (Size_t i) {
        return m_byteArray[i];
    }
    inline const Uint8_t& operator[] (Size_t i) const {
        return m_byteArray[i];
    }

    /// @}

protected:

    std::array<Uint8_t, MaxSizeBytes> m_byteArray{}; ///< The byte array for the buffer
};


} // End rev namespace
