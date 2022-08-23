#pragma once

// Public
#include <vector>
#include "fortress/types/GSizedTypes.h"
#include "fortress/math/GMath.h"
#include "fortress/json/GJson.h"
#include "fortress/containers/math/GVector.h"

namespace rev {

/// @class Color
/// @brief Class representing a color
class Color {
public:
    /// @name Static methods
    /// @{

    static const Color& White();

    /// @}

	/// @name Constructors/Destructor
	/// @{

    Color();
    Color(const std::vector<Real_t>& vec);
    Color(const std::vector<int>& vec);
    Color(const Vector3& vec);
    Color(const Vector4& vec);
    explicit Color(int r, int g, int b, int a = 255);
    explicit Color(Real_t r, Real_t g, Real_t b, Real_t a = 1.0);
	~Color();

	/// @}

	/// @name Public Methods
	/// @{

    Int32_t red() const { return ToIntColor(m_r); }
    Int32_t green() const { return ToIntColor(m_g); }
    Int32_t blue() const { return ToIntColor(m_b); }
    Int32_t alpha() const { return ToIntColor(m_a); }

    Float32_t redF() const { return m_r; }
    Float32_t greenF() const { return m_g; }
    Float32_t blueF() const { return m_b; }
    Float32_t alphaF() const { return m_a; }

    template<typename T>
    std::vector<T> toStdVector() const = delete;

    template<>
    std::vector<int> toStdVector<int>() const {
        return std::vector<int>({ red(), green(), blue(), alpha() });
    }

    template<typename T, size_t N>
    Vector<T, N> toVector() const = delete;

    template<>
    Vector<Real_t, 3> toVector<Real_t, 3>() const {
        return Vector3(redF(), greenF(), blueF());
    }

    template<>
    Vector<Real_t, 4> toVector<Real_t, 4>() const {
        return Vector4(redF(), greenF(), blueF(), alphaF());
    }

	/// @}

    /// @name Operators
    /// @{

    bool operator==(const Color& other) const {
        return m_r == other.m_r &&
            m_g == other.m_g &&
            m_b == other.m_b &&
            m_a == other.m_a;
    }

    bool operator!=(const Color& other) const {
        if (m_r != other.m_r) {
            return true;
        }
        else if (m_g != other.m_g) {
            return true;
        }
        else if (m_b != other.m_b) {
            return true;
        }
        else if (m_a != other.m_a) {
            return true;
        }
        else {
            return false;
        }
    }

    /// @}

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson 
    /// @param korObject
    friend void to_json(nlohmann::json& orJson, const Color& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson 
    /// @param orObject 
    friend void from_json(const nlohmann::json& korJson, Color& orObject);


protected:

    /// @brief Convert a float in range [0, 1] to an int in the range[0, 255].
    inline static Uint32_t ToIntColor(Float32_t f) {
        Float32_t clamped = rev::math::clamp(f, 0.0F, 1.0F);
        return floor(clamped == 1.0F ? 255 : clamped * 256.0F);
    }

    /// @brief Convert an int in range 0-255 to a float in the range [0, 1]
    inline static Float32_t ToFloatColor(Int32_t i) {
        Float32_t f = i;
        return f / 255.0F;
    }

    Float32_t m_r{ 0.0F }; ///< Red component of the color
    Float32_t m_g{ 0.0F }; ///< Green component of the color
    Float32_t m_b{ 0.0F }; ///< Blue component of the color
    Float32_t m_a{ 0.0F }; ///< Alpha component of the color
};


} // End rev namespace
