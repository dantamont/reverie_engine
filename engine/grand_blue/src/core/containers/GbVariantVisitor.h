/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Based on variant/recursive_wrapper.hpp from boost.
//
// Original license:
//
// Copyright (c) 2002-2003
// Eric Friedman, Itay Maman
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GB_VARIANT_VISITOR_H
#define GB_VARIANT_VISITOR_H

#include <type_traits>
#include <utility>

namespace Gb {
namespace variant {

template <typename... Fns>
struct visitor;

template <typename Fn>
struct visitor<Fn> : Fn
{
    using Fn::operator();

    template<typename T>
    visitor(T&& fn) : Fn(std::forward<T>(fn)) {}
};

template <typename Fn, typename... Fns>
struct visitor<Fn, Fns...> : Fn, visitor<Fns...>
{
    using Fn::operator();
    using visitor<Fns...>::operator();

    template<typename T, typename... Ts>
    visitor(T&& fn, Ts&&... fns)
        : Fn(std::forward<T>(fn))
        , visitor<Fns...>(std::forward<Ts>(fns)...) {}
};

// Decay is useful for handling array types, it applies pass by value transformations
// See: https://stackoverflow.com/questions/25732386/what-is-stddecay-and-when-it-should-be-used
template <typename... Fns>
visitor<typename std::decay<Fns>::type...> make_visitor(Fns&&... fns)
{
    return visitor<typename std::decay<Fns>::type...>
        (std::forward<Fns>(fns)...);
}


///////////////////////////////////////////////////////////////////////////////
// (detail) class reflect
//
// Generic static visitor that performs a typeid on the value it visits.
//

struct ReflectVisitor
{
    template <typename T>
    const std::type_info& operator()(const T&) const 
    {
        return typeid(T);
    }
};


} // namespace variant
} // namespace Gb

#endif // GB_VARIANT_VISITOR_H