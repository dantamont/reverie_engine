#ifndef GB_PROTOCOL_H
#define GB_PROTOCOL_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// External

// Project
#include "GbProtocolField.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Template Metaprogramming
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename...>
struct is_empty : std::false_type
{ };

template <typename T, template <T...> class Z, T... Is>
struct is_empty<T, Z<Is...>> : std::true_type
{ };

template <typename T, template <T...> class Z, T First, T... Rest>
struct is_empty<T, Z<First, Rest...>> : std::false_type
{ };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @class BaseProtocol
/// @brief Abstract base class to enable storage of child Protocols
class BaseProtocol {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    BaseProtocol()
    {
    }
    virtual ~BaseProtocol() {
        for (BaseProtocol* child : m_children) {
            delete child;
        }
        m_children.clear();
    }

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief Add child protocol
    void addChild(BaseProtocol* child);

    /// @brief Write to a filestream
    virtual bool write(const FileStream& stream) {
        bool success = true;

        // Write all child protocols
        for (BaseProtocol* child : m_children) {
            success &= child->write(stream);
        }

        return success;
    }

    /// @brief Read from a filestream
    virtual bool read(const FileStream& stream) {
        bool success = true;

        // Read all child protocols
        for (BaseProtocol* child : m_children) {
            success &= child->read(stream);
        }

        return success;
    }

    /// @}

protected:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Child protocols of this protocol
    /// @details Enables protocols to be called in a tree-like manner
    std::vector<BaseProtocol*> m_children;

    /// @}

};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class Protocol
// FIXME: WARNING, cannot read in raw pointer data if pointer has not been properly resized firsr
template<typename ...FieldTypes>
class Protocol: public BaseProtocol {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{

    // See: https://www.murrayc.com/permalink/2015/12/05/modern-c-variadic-template-parameters-and-tuples/
    /// @brief Construct with indirection so Protocol Fields can be used directly in construction
    template<typename ...PFieldTypes>
    Protocol(PFieldTypes&... fields) :
        m_fields(fields...)
    {
    }

    Protocol() {

    }

    //template<typename ...PFieldTypes>
    //Protocol(PFieldTypes&&... fields) :
    //    m_fields(std::forward<PFieldTypes>(fields)...)
    //{
    //}

    //Protocol(reference_type<FieldTypes>... fields) :
    //    m_fields(fields...)
    //{
    //}

    //Protocol(FieldTypes&&... fields) :
    //    m_fields(std::forward<FieldTypes>(fields)...)
    //{
    //}

    virtual ~Protocol() {
    }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    const std::tuple<ProtocolField<FieldTypes>...>& fields() const {
        return m_fields;
    }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
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

    Protocol& operator=(const Protocol& other) {
        m_fields = other.m_fields;
        return *this;
    }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief Write to a filestream
    virtual bool write(const FileStream& stream) override {
        // Write this protocol
        bool success = write(m_fields, stream);

        // Write all child protocols
        if (success) {
            success = BaseProtocol::write(stream);
        }

        return success;
    }

    /// @brief Read from a filestream
    virtual bool read(const FileStream& stream) override {
        bool success =  read(m_fields, stream);

        // Write all child protocols
        if (success) {
            success = BaseProtocol::read(stream);
        }

        return success;
    }

    /// @}

private:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @brief Recursive routine for writing all Protocol Fields to file stream
    template<size_t I = 0, typename... T>
    bool write(const std::tuple<T...>& t, const FileStream& stream) const {
        // Write current field to stream
        const std::tuple_element_t<I, std::tuple<T...>>& field = std::get<I>(t);
        bool success = field.write(stream);
        if (!success) {
            throw("Error, failed to write to file");
            return false;
        }

        // Compile-time recursion over all fields
        if constexpr (I + 1 != sizeof...(T))
        {
            write<I + 1>(t, stream);
        }

        return true;
    }

    /// @brief Recursive routine for reading all Protocol Fields from file stream
    template<size_t I = 0, typename... T>
    bool read(std::tuple<T...>& t, const FileStream& stream) {
        // Write current field to stream
        std::tuple_element_t<I, std::tuple<T...>>& field = std::get<I>(t);
        bool success = field.read(stream);
        if (!success) {
            throw("Error, failed to read from file");
            return false;
        }

        // Compile-time recursion over all fields
        if constexpr (I + 1 != sizeof...(T))
        {
            read<I + 1>(t, stream);
        }

        return true;
    }

    /// @brief Recursive routine for obtaining Protocol as a string
    template<size_t I = 0, typename... T>
    void sprint(const std::tuple<T...>& t, GString& outStr) const {
        Q_UNUSED(t);
        if (!outStr.isEmpty()) {
            outStr += ",\n";
        }
        else {
            outStr += "Protocol " + GString("of size ") + GString::FromNumber(sizeof...(T)) +"~\n";
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

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    /// @brief See: https://stackoverflow.com/questions/27941661/generating-one-class-member-per-variadic-template-argument
    std::tuple<ProtocolField<FieldTypes>...> m_fields;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 
