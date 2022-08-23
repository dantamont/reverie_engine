#pragma once

#include <deque>

// Internal
#include "fortress/templates/GTemplates.h"
#include "fortress/types/GSizedTypes.h"

namespace rev {

/// @class StrictGrowContainer
/// @brief A container that only reallocates when it grows, not on delete.
template<typename ContainerType>
class StrictGrowContainer {
private:

    using ContainedType = innermost<ContainerType>;

public:

    StrictGrowContainer() = default;
    ~StrictGrowContainer() = default;

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief The encapsulated container
    ContainerType& container() { return m_container; }

    /// @brief Clear the container
    void clear() {
        m_container.clear();
        m_deletedIndices.clear();
    }

    /// @brief Ensure that container is at least the given size
    void ensureSize(Uint32_t newSize) {
        if (m_container.size() < newSize) {
            m_container.resize(newSize);
        }
    }

    /// @brief Return the size of the container
    inline Uint32_t size() const {
        return m_container.size();
    }

    /// @brief Add an entry to the container, and return the index at which it was added
    template<typename ...Args>
    inline Uint32_t push(Args&&... items) {
        Uint32_t index;
        if (m_deletedIndices.size()) {
            index = m_deletedIndices.back();
            m_deletedIndices.pop_back();
            m_container[index] = ContainedType(std::forward<Args>(items)...);
        }
        else {
            index = m_container.size();
            if constexpr (std::is_same_v<ContainerType, std::vector<ContainedType>>) {
                m_container.emplace_back(std::forward<Args>(items)...);
            }
            else {
                m_container.insert(m_container.end(), std::forward<Args>(items)...);
            }
        }
        return index;
    }

    /// @brief Add an entry to the container, and return the index at which it was added
    template<typename ...Args>
    inline Uint32_t push(const Args&... items) {
        Uint32_t index;
        if (m_deletedIndices.size()) {
            index = m_deletedIndices.back();
            m_deletedIndices.pop_back();
            m_container[index] = ContainedType(std::forward<Args>(items)...);
        }
        else {
            index = m_container.size();
            if constexpr (std::is_same_v<ContainerType, std::vector<ContainedType>>) {
                m_container.emplace_back(std::forward<Args>(items)...);
            }
            else {
                m_container.insert(m_container.end(), std::forward<Args>(items)...);
            }
        }
        return index;
    }

    template<>
    inline Uint32_t push(const ContainedType& item) {
        Uint32_t index;
        if (m_deletedIndices.size()) {
            index = m_deletedIndices.back();
            m_deletedIndices.pop_back();
            m_container[index] = item;
        }
        else {
            index = m_container.size();
            if constexpr (std::is_same_v<ContainerType, std::vector<ContainedType>>) {
                m_container.emplace_back(item);
            }
            else {
                m_container.insert(m_container.end(), item);
            }
        }
        return index;
    }

    /// @brief Add an entry to the container, and return the index at which it was added
    inline void emplace_back() {
        static_assert(std::is_same_v<ContainerType, std::vector<ContainedType>>, "Only vector supports emplace_back");
        m_container.emplace_back();
    }

    /// @brief Invalidate an index in the container
    inline void invalidate(Uint32_t index) {
        m_deletedIndices.push_back(index);
    }

    /// @brief Invalidate a range of indices in the container
    inline void invalidate(Uint32_t index, Uint32_t count) {
        for (Uint32_t i = 0; i < count; i++) {
            m_deletedIndices.push_back(index + i);
        }
    }

    /// @brief Return the buffer
    inline ContainedType* data() {
        return m_container.data();
    }

    /// @brief Return the buffer
    inline const ContainedType* data() const {
        return m_container.data();
    }

    /// @brief Return the data in the buffer at the given index
    inline ContainedType* data(Size_t index) {
        return std::begin(m_container) + index;
    }

    /// @brief Return the data in the buffer at the given index
    inline const ContainedType* data(Size_t index) const {
        return std::begin(m_container) + index;
    }

    /// @name Operators
    /// @{

    inline ContainedType& operator[] (Size_t i) {
        return m_container[i];
    }
    inline const ContainedType& operator[] (Size_t i) const {
        return m_container[i];
    }

    /// @}

protected:

    ContainerType m_container; ///< The encapsulated container
    std::deque<Uint32_t> m_deletedIndices; ///< The indices of the container that have been deleted

};

} // End rev namespace
