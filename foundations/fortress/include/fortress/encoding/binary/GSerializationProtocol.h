#pragma once

// Standard Includes
#include <stdexcept>

// External

// Project
#include "GSerializationProtocolField.h"
#include "fortress/streams/GFileStream.h"

namespace rev {

/// @class SerializationProtocolInterface
/// @brief Abstract base class to enable storage of child Protocols
class SerializationProtocolInterface {
public:
    /// @name Constructors and Destructors
    /// @{
    SerializationProtocolInterface()
    {
    }

    virtual ~SerializationProtocolInterface() {
        m_children.clear();
    }

    /// @}

    /// @name Public methods
    /// @{

    /// @brief Add child protocol
    void addChild(std::unique_ptr<SerializationProtocolInterface>&& child);

    /// @brief Write to a filestream
    virtual bool write(const FileStream& stream) const = 0;

    /// @brief Write the data represented by this protocol into a buffer
    /// @details Performs a little-endian write, regardless of system endianness
    /// @param[in] The buffer
    /// @return The number of bytes written. Zero if failed
    virtual Uint64_t write(Uint8_t* buffer, Uint64_t index = 0) const = 0;

    /// @brief Read from a filestream
    virtual bool read(const FileStream& stream) = 0;

    /// @brief Read the data represented by this protocol field from a buffer
    /// @details Performs a little-endian read, regardless of system endianness
    /// @param[in] The buffer
    /// @return The number of bytes read. Zero if failed
    virtual Uint64_t read(const Uint8_t* buffer, Uint64_t index = 0) = 0;

    /// @}

protected:
    /// @name Protected Members
    /// @{

    /// @details Enables protocols to be called in a tree-like manner
    std::vector<std::unique_ptr<SerializationProtocolInterface>> m_children; ///< Child protocols of this protocol

    /// @}

};



/// @class SerializationProtocol
/// @FIXME WARNING, cannot read in raw pointer data if pointer has not been properly resized first
template<typename ...FieldTypes>
class SerializationProtocol: public SerializationProtocolInterface {
public:

    /// @name Constructors and Destructors
    /// @{

    /// @brief Construct with indirection so SerializationProtocol Fields can be used directly in construction
    /// @see https://www.murrayc.com/permalink/2015/12/05/modern-c-variadic-template-parameters-and-tuples/
    template<typename ...PFieldTypes>
    SerializationProtocol(PFieldTypes&... fields) :
        m_fields(fields...)
    {
        static_assert(GetNumberOfFields() == sizeOfPack<PFieldTypes...>(), "Incorrect number of arguments given, expected");
        verifyProtocolTypes(m_fields, fields...);
    }

    SerializationProtocol() {

    }

    virtual ~SerializationProtocol() {
    }

    /// @}

    /// @name Properties
    /// @{

    const std::tuple<SerializationProtocolField<FieldTypes>...>& fields() const {
        return m_fields;
    }

    /// @brief Return the number of fields
    static constexpr Size_t GetNumberOfFields() {
        return sizeOfPack<FieldTypes...>();
    }

    /// @brief Return the serialized size in bytes of the serialization protocol, including children
    Uint64_t SerializedSizeInBytes() const {
        /// @note Each field adds an extra 64 bits (8 bytes) for count
        return getSerializedSizeBytes(m_fields, 0); 
    }

    /// @}

    /// @name Operators
    /// @{

    operator GString() const {
        GString outStr;
        sprint(m_fields, outStr);
        if (m_children.size()) {
            outStr += "\n Number of child protocols:" + GString::FromNumber(m_children.size());
        }
        return outStr;
    }

    SerializationProtocol& operator=(const SerializationProtocol& other) {
        m_fields = other.m_fields;
        return *this;
    }

    /// @}

    /// @name Public methods
    /// @{

    /// @brief Write to a filestream
    virtual bool write(const FileStream& stream) const override final {
        // Write this protocol
        bool success = write(m_fields, stream);

        // Write all child protocols
        if (success) {
            for (const std::unique_ptr<SerializationProtocolInterface>& child : m_children) {
                success &= child->write(stream);
            }
        }

        return success;
    }

    /// @brief Write the data represented by this protocol into a buffer
    /// @details Performs a little-endian write, regardless of system endianness
    /// @param[in] The buffer
    /// @return The number of bytes written. Zero if failed
    virtual Uint64_t write(Uint8_t* buffer, Uint64_t index = 0) const {
        // Write this protocol
        Uint64_t bytesWritten = write(m_fields, buffer, index);

        // Write all child protocols
        Uint64_t writeIndex = bytesWritten + index;
        if (0 != bytesWritten) {
            for (const std::unique_ptr<SerializationProtocolInterface>& child : m_children) {
                writeIndex += child->write(buffer, writeIndex);
            }
        }

        const Uint64_t totalBytesWritten = writeIndex - index;
        return totalBytesWritten;
    }

    /// @brief Read from a filestream
    virtual bool read(const FileStream& stream) override final {
        bool success =  read(m_fields, stream);

        // Write all child protocols
        if (success) {
            for (std::unique_ptr<SerializationProtocolInterface>& child : m_children) {
                success &= child->read(stream);
            }
        }

        return success;
    }

    /// @brief Read the data represented by this protocol field from a buffer
    /// @details Performs a little-endian read, regardless of system endianness
    /// @param[in] The buffer
    /// @return The number of bytes read. Zero if failed
    virtual Uint64_t read(const Uint8_t* buffer, Uint64_t index = 0) {
        // Read this protocol
        Uint64_t bytesRead = read(m_fields, buffer, index);

        // Read all child protocols
        Uint64_t readIndex = bytesRead + index;
        if (0 != bytesRead) {
            for (std::unique_ptr<SerializationProtocolInterface>& child : m_children) {
                readIndex += child->read(buffer, readIndex);
            }
        }

        const Uint64_t totalBytesRead = readIndex - index;
        return totalBytesRead;
    }

    /// @}

    /// @name Operators
    /// @{

    /// @brief Read from a stream
    friend FileStream& operator>>(FileStream& fs, SerializationProtocol& p) {
        p.read(fs);
        return fs;
    }
    friend SerializationProtocol& operator<<(SerializationProtocol& p, FileStream& fs) {
        p.read(fs);
        return p;
    }


    /// @brief Write to stream
    friend FileStream& operator<<(FileStream& fs, SerializationProtocol& p) {
        p.write(fs);
        return fs;
    }
    friend SerializationProtocol& operator>>(SerializationProtocol& p, FileStream& fs) {
        p.write(fs);
        return p;
    }

    /// @}

private:
    /// @name Private Methods
    /// @{

    /// @brief Recursive routine for verifying that a given type matches the expected type of an internal field
    template<size_t I = 0, typename... T, typename ArgType, typename... ArgTypes>
    constexpr void verifyProtocolTypes(const std::tuple<T...>& t, ArgType& arg, ArgTypes&... args) const {
        // Verify value of current field
        using FieldType = std::tuple_element_t<I, std::tuple<T...>>;

        // Since arguments can be passed as the field specialization type or as a field itself, need to check both
        static_assert(std::is_same_v<FieldType, std::decay_t<ArgType>> || 
            std::is_same_v<FieldType, SerializationProtocolField<std::decay_t<ArgType>>>,
            "Error, incorrect argument type for given field");

        // Compile-time recursion over all fields
        if constexpr (I + 1 != sizeof...(T))
        {
            verifyProtocolTypes<I + 1>(t, args...);
        }
    }

    /// @brief Recursive routine for getting the serialized size in bytes of the protocol
    template<size_t I = 0, typename... T>
    Uint64_t getSerializedSizeBytes(const std::tuple<T...>& t, Uint64_t totalBytesSoFar = 0) const {
        // Write current field to stream
        const std::tuple_element_t<I, std::tuple<T...>>& field = std::get<I>(t);
        totalBytesSoFar += field.SerializedSizeInBytes();

        // Compile-time recursion over all fields
        if constexpr (I + 1 != sizeof...(T))
        {
            totalBytesSoFar = getSerializedSizeBytes<I + 1>(t, totalBytesSoFar);
        }

        return totalBytesSoFar;
    }


    /// @brief Recursive routine for writing all SerializationProtocol Fields to file stream
    template<size_t I = 0, typename... T>
    bool write(const std::tuple<T...>& t, const FileStream& stream) const {
        // Write current field to stream
        const std::tuple_element_t<I, std::tuple<T...>>& field = std::get<I>(t);
        bool success = field.write(stream);
        if (!success) {
            throw std::runtime_error("Error, failed to write to file");
            return false;
        }

        // Compile-time recursion over all fields
        if constexpr (I + 1 != sizeof...(T))
        {
            write<I + 1>(t, stream);
        }

        return true;
    }

    /// @brief Recursive routine for writing all SerializationProtocol Fields to a buffer
    /// @return The number of bytes written
    template<size_t I = 0, typename... T>
    Uint64_t write(const std::tuple<T...>& t, Uint8_t* buffer, Uint64_t index) const {
        // Write current field to stream
        const std::tuple_element_t<I, std::tuple<T...>>& field = std::get<I>(t);
        Uint64_t bytesWritten = field.write(buffer, index);
        if (0 == bytesWritten) {
            throw std::runtime_error("Error, failed to write to buffer");
        }

        // Compile-time recursion over all fields
        if constexpr (I + 1 != sizeof...(T))
        {
            bytesWritten += write<I + 1>(t, buffer, bytesWritten + index);
        }

        return bytesWritten;
    }

    /// @brief Recursive routine for reading all SerializationProtocol Fields from file stream
    template<size_t I = 0, typename... T>
    bool read(std::tuple<T...>& t, const FileStream& stream) {
        // Write current field to stream
        std::tuple_element_t<I, std::tuple<T...>>& field = std::get<I>(t);
        bool success = field.read(stream);
        if (!success) {
            throw std::runtime_error("Error, failed to read from file");
            return false;
        }

        // Compile-time recursion over all fields
        if constexpr (I + 1 != sizeof...(T))
        {
            read<I + 1>(t, stream);
        }

        return true;
    }


    /// @brief Recursive routine for reading all SerializationProtocol Fields from a buffer
    /// @return The number of bytes read
    template<size_t I = 0, typename... T>
    Uint64_t read(std::tuple<T...>& t, const Uint8_t* buffer, Uint64_t index) {
        // Write current field to stream
        std::tuple_element_t<I, std::tuple<T...>>& field = std::get<I>(t);
        Uint64_t bytesRead = field.read(buffer, index);
        if (0 == bytesRead) {
            throw std::runtime_error(std::string("Invalid buffer for read: ") + typeid(field).name());
        }

        // Compile-time recursion over all fields
        if constexpr (I + 1 != sizeof...(T))
        {
            bytesRead += read<I + 1>(t, buffer, bytesRead + index);
        }

        return bytesRead;
    }

    /// @brief Recursive routine for obtaining SerializationProtocol as a string
    template<size_t I = 0, typename... T>
    void sprint(const std::tuple<T...>& t, GString& outStr) const {
        G_UNUSED(t);
        if (!outStr.isEmpty()) {
            outStr += ",\n";
        }
        else {
            outStr += "SerializationProtocol " + GString("of size ") + GString::FromNumber(sizeof...(T)) +"~\n";
        }
        // Append a string for current field
        outStr += "\tField Type: ";
        outStr += typeid(
            decltype(std::declval<
                std::tuple_element_t<I, std::tuple<T...>> // Tuple element type
                >().m_field) // Get type of tuple element's field
            ).name();
        //outStr += std::get<I>(t);

        // Compile-time recursion over all fields
        if constexpr (I + 1 != sizeof...(T)) 
        { 
            sprint<I + 1>(t, outStr);
        }
    }

    /// @}

    /// @name Private Members
    /// @{

    /// @see https://stackoverflow.com/questions/27941661/generating-one-class-member-per-variadic-template-argument
    std::tuple<SerializationProtocolField<FieldTypes>...> m_fields; ///< The fields defining this protocol

    /// @}
};

/// @brief Allow construction to define the type
/// @todo This would be neat, although not a ton of added convenience. Would also need to strip down protocol fields that are passed in, into their encapsulated type
/// @see https://devblogs.microsoft.com/oldnewthing/20220408-00/?p=106438
//template<typename ...FieldType> 
//SerializationProtocol(FieldType&&...) ->
//    SerializationProtocol<std::remove_const_t<std::remove_reference_t<FieldType...>>>;

} // end namespacing