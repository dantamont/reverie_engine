/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_CONTAINER_EXTENSIONS_H
#define GB_CONTAINER_EXTENSIONS_H

// QT
#include <map>
#include <unordered_map>
#include <set>
#include <vector>
#include <QHash>
#include <QString>
#include <functional>
#include <list>

// Internal
#include "../encoding/GbUUID.h"

namespace std {
// Even though std::size_t is larger than unsigned int on common 64 bit 
// platforms, and thus the hash doesn't change across its full length - 
// this isn't a problem. The standard places no such requirement on a
// std::hash implementation.
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
template<>
struct hash<QString> {
    std::size_t operator()(const QString &v) const noexcept
    {
        return (size_t)qHash(v);
    }
};

template<>
struct hash<QUuid> {
    std::size_t operator()(const QUuid &u) const noexcept
    {
        return (size_t)qHash(u);
    }
};

template<>
struct hash<Gb::Uuid> {
    std::size_t operator()(const Gb::Uuid &u) const noexcept
    {
        return (size_t)qHash(u);
    }
};

#endif
}

namespace Gb {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

class Vec {
public:
    // Variadic template for vector emplace_back forwarding
    template<typename T, typename ...V>
    static void EmplaceBack(std::vector<T> & v, V && ... value)
    {
        v.emplace_back(std::forward<V>(value)...);
    }

    // Variadic template for vector emplace forwarding
    template<typename T, typename ...V>
    static void Emplace(std::vector<T> & v, 
        typename const std::vector<T>::iterator& iter,
        V && ... value)
    {
        v.emplace(iter, std::forward<V>(value)...);
    }

    // Variadic template for vector replace forwarding
    template<typename T, typename ...V>
    static void Replace(typename const std::vector<T>::iterator& iter,
        V && ... value)
    {
        *iter = { std::forward<V>(value)... };
    }

    // Variadic template for list emplace_back forwarding
    template<typename T, typename ...V>
    static void EmplaceBack(std::list<T> & v, V && ... value)
    {
        v.emplace_back(std::forward<V>(value)...);
    }
};

class Map {
public:
    // Variadic template for map forwarding
    template<typename TK, typename TV, typename ...Vals>
    static void Emplace(std::map<TK, TV>& map, 
        const TK& k, 
        Vals && ... values)
    {
        map.emplace(std::piecewise_construct,
            std::forward_as_tuple(k),
            std::forward_as_tuple(std::forward<Vals>(values)...));
    }

    template<typename TK, typename TV, typename ...Vals>
    static void Emplace(std::unordered_map<TK, TV>& map,
        const TK& k,
        Vals && ... values)
    {
        map.emplace(std::piecewise_construct,
            std::forward_as_tuple(k),
            std::forward_as_tuple(std::forward<Vals>(values)...)); // expanding parameters
    }


    template<typename TK, typename TV>
    static bool HasKey(const std::map<TK, TV>& map, const TK& k)
    {
        return (map.find(k) != map.end());
    }

    template<typename TK, typename TV>
    static bool HasKey(const std::unordered_map<TK, TV>& map, const TK& k)
    {
        return (map.find(k) != map.end());
    }

};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif