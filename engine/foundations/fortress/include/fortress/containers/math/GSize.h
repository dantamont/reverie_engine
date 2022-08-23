#pragma once

#include "fortress/types/GSizedTypes.h"

namespace rev {

//// Forward declarations
template<typename D, size_t N> class Vector;
typedef Vector<Int32_t, 2> Vector2i;


/// @class GSize
/// @brief Class representing the size of a two-dimensional object
class GSize {
public:
    GSize() {}
    GSize(Int32_t width, Int32_t height):
        m_width(width),
        m_height(height)
    {

    }

    /// @brief Return true if both width and height are uninitialized
    bool isEmpty() const { return m_width <= 0 && m_height <= 0; }

    /// @brief Return true if both width and height are zero
    bool isNull() const { return m_width == 0 && m_height == 0; }

    /// @brief Get the width of the object
    Int32_t width() const { return m_width; }

    /// @brief Set the width of the object
    void setWidth(Int32_t width) { m_width = width; }

    /// @brief Get the height of the object
    Int32_t height() const { return m_height; }

    /// @brief Set the height of the object
    void setHeight(Int32_t height) { m_height = height; }

    /// @brief Convert to a vector2
    Vector2i toVector2();

private:

    Int32_t m_width{ -1 }; ///< Width of the object
    Int32_t m_height{ -1 }; ///< Height of the object
};

}