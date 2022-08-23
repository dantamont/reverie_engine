/// @note C++20 makes many members of this header redundant

#pragma once

/// Public
#include <type_traits>
#include <memory>
#include "fortress/types/GSizedTypes.h"

namespace rev {

/// @brief has_member implementation
/// @details Implements a "has_member" helper macro to determine if a class has the given member
/// @example static_assert(has_member(Example, Foo), "Example class must have Foo member");
///          static_assert(has_member(Example, Bar()), "Example class must have Bar() member function");
template<typename T, typename F>
constexpr auto has_member_impl(F&& f) -> decltype(f(std::declval<T>()), true) { return true; }

template<typename>
constexpr bool has_member_impl(...) { return false; }

#define has_member(T, EXPR) has_member_impl<T>( [](auto&& obj)->decltype(obj.EXPR){} )

/// @brief Ternary constexpr if for returning types based on a condition
template <bool cond_v, typename Then, typename OrElse>
decltype(auto) constexpr_if(Then&& then, OrElse&& or_else) {
    if constexpr (cond_v) {
        return std::forward<Then>(then);
    }
    else {
        return std::forward<OrElse>(or_else);
    }
}

/// @brief Check if something is a unique pointer
template <class T>
struct is_unique_ptr : std::false_type
{};

template <class T, class D>
struct is_unique_ptr<std::unique_ptr<T, D>> : std::true_type
{};

/// @brief Whether the given types are the same as one another
/// @see https://stackoverflow.com/questions/31533469/check-a-parameter-pack-for-all-of-type-t
/// @see https://stackoverflow.com/questions/3703658/specifying-one-type-for-all-arguments-passed-to-variadic-function-or-variadic-te
template <typename T, typename ...Ts>
using are_same = std::conjunction<std::is_same<T, Ts>...>;

template<typename... Ts>
inline constexpr bool are_same_v = are_same<Ts...>::value;

/// @brief is_one_of implementation
/// @details Returns whether or not the first type is any of the following types 
template <typename...>
struct is_one_of {
    static constexpr bool value = false;
};

template <typename F, typename S, typename... T>
struct is_one_of<F, S, T...> {
    static constexpr bool value =
        std::is_same<F, S>::value || is_one_of<F, T...>::value;
};

/// @brief Perform iterative function over a template parameter pack
template <typename Function>
void iteratePack(const Function&) {}

template <typename Function, typename Arg, typename ... Args>
void iteratePack(const Function& fun, Arg&& arg, Args&& ... args)
{
    fun(std::forward<Arg>(arg));
    iteratePack(fun, std::forward<Args>(args)...);
}

/// @brief Get size of template parameter pack
template<typename ...Types>
constexpr Size_t sizeOfPack() {
    constexpr Size_t sz = std::tuple_size<std::tuple<Types...>>::value;
    return sz;
}

/// @brief Get array item type from an array type, e.g. int[] -> int
template <typename T>
using stripArray = typename std::remove_all_extents<T>::type;

/// @brief Get a pointer array type from an array type, e.g. int[] -> int*
template <typename T>
using arrayToPointer = typename std::add_pointer_t<stripArray<T>>;


/// @brief Set up tag dispatching to detect if shared pointer
template <typename T> struct is_shared_ptr : std::false_type {};
template <typename T> struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};
template <typename T> struct is_shared_ptr<std::shared_ptr<T const>> : std::true_type {};


/// @brief Set up tag dispatching to detect if vector
template<typename Test, template<typename...> class Ref>
struct is_specialization : std::false_type {};

template<template<typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref> : std::true_type {};

template <typename T>
using is_std_vector = is_specialization<T, std::vector>;

/// @brief Get non-const value type if reference to a type
template <typename T>
using non_const_value_type = std::remove_pointer_t<std::decay_t<T>>;

/// @brief Check whether or not a predicate is a value type
/// @note Wrapped type in non_const_value_type to generalize check for whether or not type is a vector
template <typename T>
using is_value_type =
std::integral_constant<bool,
    (!is_specialization<non_const_value_type<T>, std::vector>::value) &&
    (!std::is_pointer<T>::value) &&
    (!std::is_array<T>::value) &&
    (!is_shared_ptr<T>::value)
>;

/// @brief Check whether or not all predicate are value types
template <typename... Ts>
using are_value_types = std::conjunction<is_value_type<Ts>...>;

/// @brief Get the innermost type of a chain, e.g. std::shared_ptr<T> -> T, std::vector<T> -> T
/// @detail Uses is_value_type so that classes other than std::vector are not stripped
namespace detail
{
    template<typename T>
    struct innermost_impl
    {
        using type = stripArray<T>;
    };

    template<template<typename...> class E, typename Head, typename... Tail>
    struct innermost_impl<E<Head, Tail...>>
    {
        using CurrentType = typename E<Head, Tail...>;
        using type = 
            std::conditional_t<is_value_type<CurrentType>::value, // Whether or not this is a value type
            CurrentType, // If value type, just return the type
            typename innermost_impl<stripArray<Head>>::type>; // Otherwise, go a level deeper
    };
}
template<typename T>
using innermost = typename detail::innermost_impl<T>::type;


/// @brief Get the pointer type member given a composite type
template <typename T>
using innermost_pointer_type = typename std::conditional_t<std::is_pointer<innermost<T>>::value,
    innermost<T>,  // Innermost type is a pointer, use directly
    typename std::add_pointer_t<innermost<T>>>; // Innermost type is a value type, return pointer to it

/// @brief Get reference to type if value, or plain old pointer if pointer
template <typename T>
using reference_type = typename std::conditional_t<std::is_pointer<std::decay_t<T>>::value,
    std::decay_t<T>,  // Innermost type is a pointer, use directly
    typename std::add_lvalue_reference_t<std::decay_t<T>>>; // Innermost type is a value type, return reference to it

/// @brief Get non-const reference type if reference to a type
template <typename T>
using non_const_reference_type = std::add_lvalue_reference_t<std::remove_const_t<std::remove_pointer_t<std::remove_reference_t<T>>>>;

/// @brief Get non-const pointer type if pointer to a type
template <typename T>
using non_const_pointer_type = std::add_pointer_t<std::remove_const_t<std::remove_pointer_t<std::remove_reference_t<T>>>>;

/// @brief Get const value type if reference to a type
template <typename T>
using const_value_type = std::add_const_t<std::decay_t<T>>;

/// @brief Get the innermost value type of a vector or array
template <typename T>
using innermost_value_type = non_const_value_type<innermost<T>> ;


} ///< End rev namespace