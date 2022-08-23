#pragma once

// QT
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <functional>
#include <list>

// Internal
#include "fortress/json/GJson.h"
#include "fortress/encoding/uuid/GUuid.h"
#include "fortress/containers/extern/tsl/robin_map.h"

namespace rev {

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
    static auto EmplaceBack(std::vector<T> & v, V && ... value)
    {
        return v.emplace_back(std::forward<V>(value)...);
    }

    // Variadic template for vector emplace forwarding
    template<typename T, typename ...V>
    static auto Emplace(std::vector<T> & v,
        typename const std::vector<T>::iterator& iter,
        V && ... value)
    {
        return v.emplace(iter, std::forward<V>(value)...);
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
    static auto EmplaceBack(std::list<T> & v, V && ... value)
    {
        return v.emplace_back(std::forward<V>(value)...);
    }

    /// @brief Obtain an ordering permuation from the first vector and comparator, and apply it to all additional vectors given
    template<typename T, typename Compare, typename... SS>
    static void SortVectors(
        const std::vector<T>& t,
        Compare comp,
        std::vector<SS>&... ss)
    {
        std::vector<unsigned> order;
        GetSortPermutation(order, t, comp);
        ApplyPermutation(order, ss...);
    }

private:

    /// @brief Retrieve a permutation (order of elements) for a vector to be applied to another vector
    template <typename T, typename Compare>
    static void GetSortPermutation(
        std::vector<unsigned>& out,
        const std::vector<T>& v,
        Compare compare = std::less<T>())
    {
        out.resize(v.size());
        std::iota(out.begin(), out.end(), 0);

        std::sort(out.begin(), out.end(),
            [&](unsigned i, unsigned j) { return compare(v[i], v[j]); });
    }

    /// @brief Apply a permutation (sort order) to the given vector
    template <typename T>
    static void ApplyPermutation(
        const std::vector<unsigned>& order,
        std::vector<T>& t)
    {
        assert(order.size() == t.size());
        std::vector<T> st(t.size());
        for (unsigned i = 0; i < t.size(); i++)
        {
            st[i] = t[order[i]];
        }
        t = st;
    }

    /// @brief Sort the given vectors by the specified order
    template <typename T, typename... S>
    static void ApplyPermutation(
        const std::vector<unsigned>& order,
        std::vector<T>& t,
        std::vector<S>&... s)
    {
        ApplyPermutation(order, t);
        ApplyPermutation(order, s...);
    }

};

class Map {
public:
    // Variadic template for map forwarding
    template<typename TK, typename TV, typename ...Vals>
    static auto Emplace(std::map<TK, TV>& map, 
        const TK& k, 
        Vals && ... values)
    {
        return map.emplace(std::piecewise_construct,
            std::forward_as_tuple(k),
            std::forward_as_tuple(std::forward<Vals>(values)...));
    }

    template<typename TK, typename TV, typename ...Vals>
    static auto Emplace(std::unordered_map<TK, TV>& map,
        const TK& k,
        Vals && ... values)
    {
        return map.emplace(std::piecewise_construct,
            std::forward_as_tuple(k),
            std::forward_as_tuple(std::forward<Vals>(values)...)); // expanding parameters
    }

    template<typename TK, typename TV, typename ...Vals>
    static auto Emplace(tsl::robin_map<TK, TV>& map,
        const TK& k,
        Vals && ... values)
    {
        return map.emplace(std::piecewise_construct,
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

    template<typename TK, typename TV>
    static bool HasKey(const tsl::robin_map<TK, TV>& map, const TK& k)
    {
        return (map.find(k) != map.end());
    }

};


} // End namespaces
