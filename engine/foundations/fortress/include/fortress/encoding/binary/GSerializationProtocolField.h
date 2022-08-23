#pragma once

// Standard Includes
#include <vector>
#include <array>
#include <utility> // declval, used in templates where acceptable parameters may have no construcotr in common
#include <exception>

// External

// Project
#include "fortress/encoding/binary/GEndianConverter.h"
#include "fortress/templates/GTemplates.h"
#include "fortress/types/GString.h"
#include "fortress/streams/GFileStream.h"

namespace rev {

/// @struct SerializationProtocolFieldInfo
/// @brief SerializationProtocol for a field, representing the format of a set of data to be packetized
/// @note enable_if can be given as a template argument, function argument, or as in this case, the return type.
/// The compiled return type is specified in the second argument of enable_if
template<typename T>
struct SerializationProtocolFieldInfo {
    /// @name Arrays
    /// @{

    /// @brief Specialize for an array of bounded size
    template<typename U = T>
    SerializationProtocolFieldInfo(U& field, 
        typename std::enable_if_t<std::is_array<U>::value>* = nullptr) :
        SerializationProtocolFieldInfo(field[0], std::size(field))
    {
    }

    /// @brief Specialize for a pointer array
    template<typename U = T>
    SerializationProtocolFieldInfo(U& field, Uint64_t count, 
        typename std::enable_if_t<std::is_pointer<U>::value>* = nullptr) :
        SerializationProtocolFieldInfo(field[0], count)
    {
    }

    /// @}

    /// @name Vectors
    /// @{

    /// @brief Specialize for a vector with no count specified
    template<typename U = T>
    SerializationProtocolFieldInfo(U& field, 
        typename std::enable_if_t<is_std_vector<non_const_value_type<U>>::value>* = nullptr) :
        SerializationProtocolFieldInfo(*field.data(), field.size()) // Call constructor using vector data and size
    {
    }

    /// @brief Specialize for a vector with a count specified
    template<typename U = T>
    SerializationProtocolFieldInfo(U& field, Uint64_t count, 
        typename std::enable_if_t<is_std_vector<non_const_value_type<U>>::value>* = nullptr) :
        SerializationProtocolFieldInfo(*field.data(), count) // Call constructor using vector data and count
    {
    }

    /// @}

    /// @name Shared pointers
    /// @{

    /// @brief Specialize for a shared pointer as an argument
    template<typename U = T>
    SerializationProtocolFieldInfo(U& field, typename std::enable_if_t<is_shared_ptr<U>::value>* = nullptr):
        SerializationProtocolFieldInfo((decltype(*std::declval<U>()))*field)
    {
    }

    /// @}

    /// @name Innermost value construction
    /// @{

    /// @brief Specialize for construction via the innermost value
    /// @details Checks that argument is not a vector, array, pointer, or shared pointer
    template<typename U = T> // SFINAE only works for deduced template argument, i.e. for function templates
    SerializationProtocolFieldInfo(U& field, Uint64_t count,
        typename std::enable_if_t<is_value_type<U>::value>* = nullptr) :
        m_fieldData(const_cast<non_const_pointer_type<U>>(&field)), // Store a pointer to the value, handling const types
        m_count(count)
    {
    }

    /// @brief Specialize for innermost value as a temporary pointer
    template<typename U = T>
    SerializationProtocolFieldInfo(const U& field, Uint64_t count, bool dummyPtr,
        typename std::enable_if_t<std::is_pointer_v<U>>* = nullptr) :
        m_fieldData(const_cast<non_const_pointer_type<U>>(field)), // Store a pointer to the value
        m_count(count)
    {
        G_UNUSED(dummyPtr);
    }


    /// @brief Construction for a single innermost value, i.e., m_count = 1
    /// @details SFINAE conditions ensure that copy construction is actually called when appropriate
    template<typename U = T> 
    SerializationProtocolFieldInfo(U& field,
        typename std::enable_if_t<is_value_type<U>::value && 
        !std::is_same<U, SerializationProtocolFieldInfo>::value>* = nullptr) :
        m_fieldData(&field), // Store a pointer to the value
        m_count(1)
    {
    }

    /// @}

    /// @brief Copy construction
    SerializationProtocolFieldInfo(const SerializationProtocolFieldInfo& other):
        m_fieldData(other.m_fieldData),
        m_count(other.m_count)
    {
    }

    // Copy is so trivial that this isn't worth the testing hassle
    ///// @brief Move construction
    ///// @todo Test
    //SerializationProtocolFieldInfo(SerializationProtocolFieldInfo&& other) :
    //    m_fieldData(std::forward<innermost_pointer_type<T>>(other.m_fieldData)),
    //    m_count(std::forward<Uint64_t>(other.m_count))
    //{
    //}

    /// @brief Destructor
    ~SerializationProtocolFieldInfo() = default;

    /// @name Operators
    /// @{

    SerializationProtocolFieldInfo<T>& operator=(const SerializationProtocolFieldInfo<T>& info) {
        m_fieldData = info.m_fieldData;
        m_count = info.m_count;
        return *this;
    }

    /// @}

    innermost_pointer_type<T> m_fieldData; ///< A clever way of storing the innermost value's pointer found via recursive conditional construction
    Uint64_t m_count; ///< The number of stored values

    static constexpr Size_t s_typeSize = sizeof(decltype(*std::declval<innermost_pointer_type<T>>())); ///< The size of the innermost value type
};


template<typename T>
struct SerializationProtocolField {

    /// @name Constructors/Destructors
    /// @{

    // Indirection was causing compile-time sadness when caching m_field for arrays fixed-length arrays
    //template<typename U = T> // Indirection allows to pass fixed length arrays without exact length, e.g. int[8] as int[]
    SerializationProtocolField(const T& field):
        m_info(const_cast<non_const_reference_type<T>>(field)),
        m_field(const_cast<non_const_reference_type<T>>(field))
    {
        //static_assert(std::is_same<U, T>(), "Types must be absolutely identical");
    }

    SerializationProtocolField(SerializationProtocolField& pField) :
        m_info(pField.m_info),
        m_field(pField.m_field)
    {
    }

    // Move constructor, untested
    SerializationProtocolField(SerializationProtocolField&& pField) :
        m_info(std::forward<SerializationProtocolField<T>>(pField).m_info),
        m_field(std::forward<SerializationProtocolField<T>>(pField).m_field)
    {
    }

    //template<typename U = T> // Indirection allows to pass fixed length arrays without exact length, e.g. int[8] as int[]
    SerializationProtocolField(const T& field, const long& count) :
        m_info(field, count),
        m_field(const_cast<T&>(field))
    {
    }

    /// @brief Special pass by value construction for pointer arguments
    /// @details Temporary pointers were not being treated kindly, 
    ///    such as those returned from functions, e.g., SerializationProtocolField(foo.c_str())
    template<typename U = T>
    SerializationProtocolField(U field, const long& count, 
        typename std::enable_if_t<std::is_pointer<U>::value>* = nullptr) :
        m_info(field, count, true), // Ambiguous call without third argument
        m_field(const_cast<non_const_pointer_type<U>>(field)) // Handle const types
    {
    }

    ~SerializationProtocolField() = default;

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Return the serialized size in bytes of the field
    Uint64_t SerializedSizeInBytes() const {
        return (m_info.m_count * sizeof(innermost_value_type<T>)) + sizeof(Uint64_t); // Extra 64 bits (8 bytes) for count
    }

    /// @brief Write the data represented by this protocol field (vector version)
    /// @param[in] stream the input stream
    inline bool write(const FileStream& stream) const
    {
        // TODO: Figure out a way to do this while still enabling recursive construction
        // Write for a vector field
        if constexpr (is_std_vector<T>::value) {
            // Field is a vector
            return stream.write(m_field);
        }
        else {
            // Field is not a vector
            return stream.write(m_info.m_fieldData, m_info.m_count);
        }
    }

    /// @brief Write the data represented by this protocol field into a buffer
    /// @details Performs a little-endian write, regardless of system endianness
    /// @param[in] buffer The buffer to write to
    /// @param[in] index The index of the buffer to write to
    /// @return The number of bytes written. Zero if failed
    inline Uint64_t write(Uint8_t* buffer, Uint64_t index = 0) const
    {
        static const bool isLittleEndian = SystemMonitor::GetEndianness() == SystemMonitor::SystemEndianness::kLittle;
        const Uint64_t numDataBytesWritten = sizeof(innermost_value_type<T>) * m_info.m_count;
        const Uint64_t numBytesWritten = numDataBytesWritten + sizeof(Uint64_t); // Need extra bytes for the count!
        if (isLittleEndian)
        {
            // Little endian machine, use type directly 
            
            // Write count
            memcpy(&buffer[index], &m_info.m_count, sizeof(Uint64_t));

            // Write data
            memcpy(&buffer[index + sizeof(Uint64_t)], &m_info.m_fieldData[0], numDataBytesWritten);
        }
        else
        {
            /// @todo Implement this. Is very non-trivial, since you need to know the layout of integral 
            ///   members of type T, as well as the padding of the structs, which may differ between
            ///   systems
            // Big endian machine, so flip bytes while copying
            // @see https://coderedirect.com/questions/213791/how-do-i-convert-a-big-endian-struct-to-a-little-endian-struct
            assert(false && "Unimplemented, non-trivial to swap endian bytes");
            //EndianConverter::SwapEndianness(&m_info.m_fieldData[0], &buffer[index], m_info.m_count);
        }
        return numBytesWritten;
    }

    /// @brief Read the data represented by this protocol field 
    /// @detail Read assumes little-endaian
    /// @param[in] stream the input stream
    inline bool read(const FileStream& stream)
    {
        if constexpr (is_std_vector<T>::value) {
            // Field is a vector, so resize vector and read in directly
            bool success = stream.read(m_field);
            if (success) {
                m_info.m_count = m_field.size();
                m_info.m_fieldData = m_field.data();
            }
            return success;
        }
        else {
            // Field is not a vector, so read in using data pointer
            return stream.read(m_info.m_fieldData, m_info.m_count);
        }
    }

    /// @brief Read the data represented by this protocol field from a buffer
    /// @details Performs a little-endian read, regardless of system endianness
    /// @param[in] buffer The buffer to read from
    /// @param[in] index The index of the buffer to read from
    /// @return The number of bytes read. Zero if failed
    inline Uint64_t read(const Uint8_t* buffer, Uint64_t index = 0)
    {
        static const bool isLittleEndian = SystemMonitor::GetEndianness() == SystemMonitor::SystemEndianness::kLittle;
        Uint64_t numBytesRead = 0;
        if (isLittleEndian)
        {
            /// Little endian machine, use type directly

            // Read in count
            memcpy(&m_info.m_count, &buffer[index], sizeof(Uint64_t));
            numBytesRead += sizeof(Uint64_t);

            // Check that the count is not absurdly large
            static constexpr Uint32_t s_maxByteCount = 65535; /// @todo Pass this limit in. Used for TCP messaging
            if (m_info.m_count > s_maxByteCount) {
#ifdef DEBUG_MODE
                static constexpr Uint64_t previewSize = 100;
                std::vector<Uint64_t> bufferVec = std::vector(reinterpret_cast<const Uint64_t*>(&buffer[0]), reinterpret_cast<const Uint64_t*>(&buffer[0]) + previewSize);
#endif
                return 0;
            }

            if constexpr (is_std_vector<T>::value) {
                // If internal storage is a vector (or really any dynamically allocated type),
                // must ensure it has enough space
                if (m_field.size() < m_info.m_count) {
                    m_field.resize(m_info.m_count);
                }

                // Also need to make sure that the field info points to the vector data
                m_info.m_fieldData = m_field.data();
            }

            // Read in data
            constexpr Uint64_t sizeOfInnermostValueType = sizeof(innermost_value_type<T>);
            Uint64_t numDataBytesRead = m_info.m_count * sizeOfInnermostValueType;
            memcpy(&m_info.m_fieldData[0], &buffer[index + numBytesRead], numDataBytesRead);
            numBytesRead += numDataBytesRead;
        }
        else
        {
            /// @todo Implement this. Is very non-trivial, since you need to know the layout of integral 
            ///   members of type T, as well as the padding of the structs, which may differ between
            ///   systems
            // Big endian machine, so flip bytes while copying
            // @see https://coderedirect.com/questions/213791/how-do-i-convert-a-big-endian-struct-to-a-little-endian-struct
            assert(false && "Unimplemented, non-trivial to swap endian bytes");
        }
        return numBytesRead;
    }


    /// @}

    /// @name Operators
    /// @{

    SerializationProtocolField<T>& operator=(const SerializationProtocolField<T>& field) {
        m_field = field.m_field;
        m_info = field.m_info;
        return *this;
    }

    /// @}

    SerializationProtocolFieldInfo<T> m_info; ///< Metadata relating to the field encapsulated by this SerializationProtocol Field
    reference_type<T> m_field; ///< The field represented by the SerializationProtocolField
};



} // end namespacing
