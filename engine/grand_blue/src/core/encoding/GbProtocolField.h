#ifndef GB_PROTOCOL_FIELD_H
#define GB_PROTOCOL_FIELD_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes
#include <type_traits>
#include <vector>
#include <array>
#include <memory>
#include <utility> // declval, used in templates where acceptable parameters may have no construcotr in common

// External

// Project
#include "../GbObject.h"
#include "../containers/GbString.h"
#include "../readers/GbFileStream.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Get array item type from an array type, e.g. int[] -> int
template <typename T>
using stripArray = typename std::remove_all_extents<T>::type;

// Get a pointer array type from an array type, e.g. int[] -> int*
template <typename T>
using arrayToPointer = typename std::add_pointer_t<stripArray<T>>;


// Get the innermost type of a chain, e.g. std::shared_ptr<T> -> T, std::vector<T> -> T
template<typename T>
struct innermost_impl
{
    using type = stripArray<T>;
};

template<template<typename...> class E, typename Head, typename... Tail>
struct innermost_impl<E<Head, Tail...>>
{
    using type = typename innermost_impl<stripArray<Head>>::type;
};

template<typename T>
using innermost = typename innermost_impl<T>::type;


// Set up tag dispatching to detect if vector
template<typename T> struct is_vector : public std::false_type {};
template<typename T, typename A> // Allocator, which is almost always left to default
struct is_vector<std::vector<T, A>> : public std::true_type {};

// Set up tag dispatching to detect if shared pointer
template <typename T> struct is_shared_ptr : std::false_type {};
template <typename T> struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};
template <typename T> struct is_shared_ptr<std::shared_ptr<T const>> : std::true_type {};


// Utility to test whether every predicate in a pack is true
// See: https://stackoverflow.com/questions/29603364/type-trait-to-check-that-all-types-in-a-parameter-pack-are-copy-constructible/29603857#29603857
// See: https://stackoverflow.com/questions/32267516/creating-variadic-accepting-only-reference-or-pointer
template<typename... Conds>
struct and_
    : std::true_type
{ };

template<typename Cond, typename... Conds>
struct and_<Cond, Conds...>
    : std::conditional<Cond::value, and_<Conds...>, std::false_type>::type
{ };

// Check whether or not a predicate is a value type
template <typename T>
using is_value_type =
std::integral_constant<bool, 
    !is_vector<T>::value &&
    !std::is_pointer<T>::value &&
    !std::is_array<T>::value &&
    !is_shared_ptr<T>::value
>;

// Check whether or not all predicate are value types
template <typename... Ts>
using are_value_types = and_<is_value_type<Ts>...>;

// Get the pointer type member given a composite type
template <typename T>
using innermost_pointer_type = typename std::conditional_t<std::is_pointer<innermost<T>>::value,
    innermost<T>,  // Innermost type is a pointer, use directly
    typename std::add_pointer_t<innermost<T>>>; // Innermost type is a value type, return pointer to it

// Get reference to type if value, or plain old pointer if pointer
template <typename T>
using reference_type = typename std::conditional_t<std::is_pointer<T>::value,
    T,  // Innermost type is a pointer, use directly
    typename std::add_lvalue_reference_t<T>>; // Innermost type is a value type, return reference to it


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @struct ProtocolFieldInfo
/// @brief Protocol for a field, representing the format of a set of data to be packetized
/// @note Enable if can be given as a template argument, function argument, 
/// or as in this case, the return type.
/// The compiled return type is specified in the second argument of enable_if
template<typename T>
struct ProtocolFieldInfo {
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Arrays
    /// @{

    /// @brief Specialize for an array of bounded size
    template<typename U = T>
    ProtocolFieldInfo(U& field, typename std::enable_if_t<std::is_array<U>::value>* = nullptr) :
        ProtocolFieldInfo(field[0], std::size(field))
    {
        Object().logInfo("Array construction");
    }

    /// @brief Specialize for a pointer array
    template<typename U = T>
    ProtocolFieldInfo(U& field, llong count, 
        typename std::enable_if_t<std::is_pointer<U>::value>* = nullptr) :
        ProtocolFieldInfo(field[0], count)
    {
        Object().logInfo("Pointer array construction");
    }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Vectors
    /// @{

    /// @brief Specialize for a vector with no count specified
    template<typename U = T>
    ProtocolFieldInfo(U& field, typename std::enable_if_t<is_vector<U>::value>* = nullptr) :
        ProtocolFieldInfo(*field.data(), field.size()) // Call constructor using vector data and size
    {
    }

    /// @brief Specialize for a vector with a count specified
    template<typename U = T>
    ProtocolFieldInfo(U& field, llong count, typename std::enable_if_t<is_vector<U>::value>* = nullptr) :
        ProtocolFieldInfo(*field.data(), count) // Call constructor using vector data and count
    {
    }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Shared pointers
    /// @{

    /// @brief Specialize for a shared pointer as an argument
    template<typename U = T>
    ProtocolFieldInfo(U& field, typename std::enable_if_t<is_shared_ptr<U>::value>* = nullptr):
        ProtocolFieldInfo((decltype(*std::declval<U>()))*field)
    {
    }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Innermost value construction
    /// @{

    /// @brief Specialize for construction via the innermost value
    /// @details Checks that argument is not a vector, array, pointer, or shared pointer
    template<typename U = T> // SFINAE only works for deduced template argument, i.e. for function templates
    ProtocolFieldInfo(U& field, llong count,
        typename std::enable_if_t<is_value_type<U>::value>* = nullptr) :
        m_fieldData(&field), // Store a pointer to the value
        m_count(count)
    {
    }

    /// @brief Specialize for innermost value as a temporary pointer
    /// @details Checks that argument is not a vector, array, pointer, or shared pointer
    template<typename U = T> // SFINAE only works for deduced template argument, i.e. for function templates
    ProtocolFieldInfo(U field, llong count, bool dummyPtr,
        typename std::enable_if_t<std::is_pointer_v<U>>* = nullptr) :
        m_fieldData(field), // Store a pointer to the value
        m_count(count)
    {
        Q_UNUSED(dummyPtr)
    }

    // TODO: Const argument
    //template<typename U = T> // SFINAE only works for deduced template argument, i.e. for function templates
    //ProtocolFieldInfo(const U& field, llong count,
    //    typename std::enable_if_t<is_value_type<U>::value>* = nullptr) :
    //    m_fieldData(&field), // Store a pointer to the value
    //    m_count(count)
    //{
    //}


    /// @brief Countless value construction
    /// @details SFINAE conditions ensure that copy construction is actually called when appropriate
    template<typename U = T> // SFINAE only works for deduced template argument, i.e. for function templates
    ProtocolFieldInfo(U& field,
        typename std::enable_if_t<is_value_type<U>::value && 
        !std::is_same<U, ProtocolFieldInfo>::value>* = nullptr) :
        m_fieldData(&field), // Store a pointer to the value
        m_count(1)
    {
    }

    // TODO: Const argument
    //template<typename U = T> // SFINAE only works for deduced template argument, i.e. for function templates
    //ProtocolFieldInfo(const U& field,
    //    typename std::enable_if_t<is_value_type<U>::value &&
    //    !std::is_same<U, ProtocolFieldInfo>::value>* = nullptr) :
    //    m_fieldData(&field), // Store a pointer to the value
    //    m_count(1)
    //{
    //}

    /// @}

    /// @brief Copy construction
    ProtocolFieldInfo(const ProtocolFieldInfo& other):
        m_fieldData(other.m_fieldData),
        m_count(other.m_count)
    {
    }

    // May work, untested
    ///// @brief Move construction
    //ProtocolFieldInfo(ProtocolFieldInfo&& other) :
    //    m_fieldData(std::forward<ProtocolFieldInfo>(other).m_fieldData),
    //    m_count(std::forward<ProtocolFieldInfo>(other).m_count)
    //{
    //}

    /// @brief Destructor
    ~ProtocolFieldInfo() {

    }


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    ProtocolFieldInfo<T>& operator=(const ProtocolFieldInfo<T>& info) {
        m_fieldData = info.m_fieldData;
        m_count = info.m_count;
        return *this;
    }

    /// @}


    /// @brief Return type size
    /// @details Since the innermost_pointer_type will never be a pointer (thanks to recursive construction down to a value type),
    /// this should always return the correct size
    static const size_t& TypeSize() 
    {
        return s_typeSize;
    }

    /// @brief A clever way of storing the innermost value's pointer found via recursive conditional construction
    innermost_pointer_type<T> m_fieldData;

    llong m_count;
    static const size_t s_typeSize;
};

template<typename T>
const size_t ProtocolFieldInfo<T>::s_typeSize = sizeof(decltype(*std::declval<innermost_pointer_type<T>>()));


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
struct ProtocolField {
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors/Destructors
    /// @{

    // Indirection was causing compile-time sadness when caching m_field for arrays fixed-length arrays
    //template<typename U = T> // Indirection allows to pass fixed length arrays without exact length, e.g. int[8] as int[]
    ProtocolField(T& field):
        m_info(field),
        m_field(field)
    {
        //static_assert(std::is_same<U, T>(), "Types must be absolutely identical");
    }

    ProtocolField(ProtocolField& pField) :
        m_info(pField.m_info),
        m_field(pField.m_field)
    {
    }

    // Move constructor, untested
    ProtocolField(ProtocolField&& pField) :
        m_info(std::forward<ProtocolField<T>>(pField).m_info),
        m_field(std::forward<ProtocolField<T>>(pField).m_field)
    {
    }

    //template<typename U = T> // Indirection allows to pass fixed length arrays without exact length, e.g. int[8] as int[]
    ProtocolField(T& field, const long& count) :
        m_info(field, count),
        m_field(field)
    {
    }

    /// @brief Special pass by value construction for pointer arguments
    /// @details Temporary pointers were not being treated kindly, 
    /// such as those returned from functions, e.g., ProtocolField(foo.c_str())
    template<typename U = T>
    ProtocolField(U field, const long& count, 
        typename std::enable_if_t<std::is_pointer<U>::value>* = nullptr) :
        m_info(field, count, true), // Ambiguous call without third argument
        m_field(field)
    {
    }

    ~ProtocolField() {
    }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Write the data represented by this protocol field (vector version)
    /// @param[in] stream the input stream
    // C++17, constexpr does a compile-time check for the condition
    // if constexpr(std::is_same<T, std::vector<std::remove_pointer_t<decltype(m_info.m_fieldData)>>>::value) {
    inline bool write(const FileStream& stream) const
    {
        // TODO: Figure out a way to do this while still enabling recursive construction
        // Write for a vector field
        if constexpr (std::is_same<T, std::vector<std::remove_pointer_t<innermost_pointer_type<T>>>>::value) {
            // Field is a vector
            return stream.write(m_field);
        }
        else {
            // Field is not a vector
            return stream.write(m_info.m_fieldData, m_info.m_count);
        }
    }

    /// @brief Write the data represented by this protocol field
    // SFINAE solution, C++17 one is cleaner
    //template<typename U = T>
    //inline std::enable_if_t<!std::is_same<U, std::vector<std::remove_pointer_t<innermost_pointer_type<U>>>>::value, bool>
    //    write(const FileStream& stream)
    //{
    //    return stream.write(m_info.m_fieldData, m_info.m_count);
    //}

    /// @brief Read the data represented by this protocol field 
    /// @param[in] stream the input stream
    inline bool read(const FileStream& stream)
    {
        if constexpr (std::is_same_v<T, 
            std::vector<std::remove_pointer_t<innermost_pointer_type<T>>>>) {
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

    // SFINAE solution, C++17 one is cleaner
    //template<typename U = T>
    //inline std::enable_if_t<std::is_same<U, std::vector<std::remove_pointer_t<innermost_pointer_type<U>>>>::value, bool> read(const FileStream& stream)
    //{
    //    return stream.read(m_field);
    //}
    //template<typename U = T>
    //inline std::enable_if_t<!std::is_same<U, std::vector<std::remove_pointer_t<innermost_pointer_type<U>>>>::value, bool> read(const FileStream& stream)
    //{
    //    return stream.read(m_info.m_fieldData, m_info.m_count);
    //}

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    ProtocolField<T>& operator=(const ProtocolField<T>& field) {
        m_field = field.m_field;
        m_info = field.m_info;
        return *this;
    }

    /// @}


    /// @brief Metadata relating to the field encapsulated by this Protocol Field
    ProtocolFieldInfo<T> m_info;

    /// @brief The field represented by the ProtocolField
    //T& m_field;
    reference_type<T> m_field;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 
