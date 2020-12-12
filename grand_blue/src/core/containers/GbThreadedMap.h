/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_THREADED_MAP
#define GB_THREADED_MAP

// standard library
#include <shared_mutex>
#include <mutex>

// QT
#include <QColor>

// Internal
#include "../geometry/GbVector.h"
#include "../../third_party/tsl/robin_map.h"
#include "../containers/GbContainerExtensions.h"

namespace Gb {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////


/// @brief ThreadedMap class
template<typename K, typename V>
class ThreadedMap {
public:

    typedef tsl::robin_map<K, V> MapType;

    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    std::shared_mutex& mutex() const { return m_mutex; }

    V& operator[](const K& key) {
        // Non-const, so needs a unique lock
        std::unique_lock lock(m_mutex);
        return m_map[key];
    }

    V& operator[](K&& key) {
        // Non-const, so needs a unique lock
        std::unique_lock lock(m_mutex);
        return m_map[std::forward<K>(key)];
    }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    size_t size() const {
        std::shared_lock lock(m_mutex);
        return m_map.size();
    }

    size_t erase(const K& key) {
        std::unique_lock lock(m_mutex);
        return m_map.erase(key);
    }

    typename MapType::const_iterator begin() const {
        std::shared_lock lock(m_mutex);
        return m_map.begin();
    }

    typename MapType::const_iterator end() const {
        std::shared_lock lock(m_mutex);
        return m_map.end();
    }

    /// @brief Perform an element-wise operation using the map
    template<class UnaryPredicate>
    void forEach(UnaryPredicate pred) const {
        std::shared_lock lock(m_mutex);
        for (const std::pair<K, V>& mapPair : m_map) {
            pred(mapPair);
        }
    }

    /// @brief Perform an element-wise operation using the map
    template<class UnaryPredicate>
    void forEach(UnaryPredicate pred) {
        std::unique_lock lock(m_mutex);
        for (std::pair<K, V>& mapPair : m_map) {
            pred(mapPair);
        }
    }

    /// @brief Find an element given a function
    template<class UnaryPredicate>
    inline auto any_of(UnaryPredicate pred) const // Const is important
    {
        std::shared_lock lock(m_mutex);
        return ThreadedMap::any_of(m_map.begin(), m_map.end(), pred);
    }

    /// @brief Find an element given a function
    template<class UnaryPredicate>
    inline auto find_if(UnaryPredicate pred) const // Const is important
    {
        std::shared_lock lock(m_mutex);
        return ThreadedMap::find_if(m_map.begin(), m_map.end(), pred);
    }

    bool hasKey(const K& key) const {
        // find() and end() have const variants which are being used, so read-safe
        std::shared_lock lock(m_mutex);
        return Map::HasKey(m_map, key);
    }

    const V& at(const K& k) const {
        // Can call const methods from any number of theads concurrently
        // Still need lock so that a write doesn't happen while accessing
        std::shared_lock lock(m_mutex);
        return m_map.at(k);
    }

    /// @brief Emplace into the map, with thread-safety
    template<typename ...Vals>
    auto emplace(const K& key, Vals && ... values) {
        std::unique_lock lock(m_mutex);
        return Map::Emplace(m_map, key, std::forward<Vals>(values)...);
    }

	/// @}

protected:

    //--------------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @brief Used for thread-safe any_of
    template< class InputIt, class UnaryPredicate >
    inline static constexpr bool any_of(InputIt first, InputIt last, UnaryPredicate p)
    {
        return ThreadedMap::find_if(first, last, p) != last;
    }

    /// @brief Used for thread-safe find_if
    template<class InputIterator, class UnaryPredicate>
    inline static InputIterator find_if(InputIterator first, InputIterator last, UnaryPredicate pred)
    {
        while (first != last) {
            if (pred(*first)) return first;
            ++first;
        }
        return last;
    }

    /// @}


    //--------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    /// @brief The mutex for controlling access to this map
    mutable std::shared_mutex m_mutex;

    /// @brief The encapsulated map
    tsl::robin_map<K, V> m_map;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif