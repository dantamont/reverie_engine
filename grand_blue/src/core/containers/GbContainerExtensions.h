/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_CONTAINER_EXTENSIONS_H
#define GB_CONTAINER_EXTENSIONS_H

// QT
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <QHash>
#include <QString>
#include <functional>
#include <list>
#include <QJsonValue>

// Internal
#include "../encoding/GbUUID.h"

/////////////////////////////////////////////////////////////////////////////////////////////
// Global
/////////////////////////////////////////////////////////////////////////////////////////////

/// @brief QJsonArray lacks a swap routine, which is needed to call std::sort
inline void swap(QJsonValueRef v1, QJsonValueRef v2)
{
    QJsonValue temp(v1);
    v1 = QJsonValue(v2);
    v2 = temp;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// std
/////////////////////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////////////////////
// Gb
/////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

class Vec {
public:

    // Whether or not the two vectors intersect
    template<typename T>
    static bool Intersects(const std::vector<T> & v1, const std::vector<T> & v2)
    {
        static std::unordered_set<T> inSet;
        inSet.clear();
        inSet.reserve(v1.size());
        inSet.insert(v1.begin(), v1.end());

        for (const T& val : v2) {
            if (inSet.find(val) != inSet.end()) {
                return true;
            }
        }
        return false;
    }
    // The intersection between two vectors
    template<typename T>
    static void Intersection(const std::vector<T> & v1, const std::vector<T> & v2, std::vector<T>& intersection)
    {
        static std::unordered_set<T> inSet;
        inSet.clear();
        inSet.reserve(v1.size());
        inSet.insert(v1.begin(), v1.end());

        intersection.reserve(min(v1.size(), v2.size()));
        for (const T& val : v2) {
            if (inSet.find(val) != inSet.end()) {
                // Assume we only have hundreds in output set and therefore
                // favour a linear search here for cache-friendliness:
                if (std::find(begin(intersection), std::end(intersection), val) == std::end(intersection))
                    intersection.emplace_back(val);
            }
        }
    }

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