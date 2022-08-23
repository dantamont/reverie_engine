#pragma once

template <typename D>
class Vector<D, 1> {
public:
    static Vector<D, 1> EmptyVector() {
        return Vector<D, 1>();
    }

    static const Vector<D, 1>& Ones() {
        static Vector<D, 1> ones(D(1.0));
        return ones;
    }

    constexpr Vector() = default;

    constexpr Vector(D fillValue):
        m_value(fillValue)
    {
    }

    Vector(const std::vector<D>& vec) :
        m_value(vec[0])
    {
    }

    inline D& operator[] (std::size_t i) {

        return &m_value;
    }

    inline const D& operator[] (std::size_t i) const {
        return &m_value;
    }

    inline bool operator== (const Vector<D, 1>& other) const {
        return m_value == other.m_value;
    }

    inline bool operator!= (const Vector<D, 1>& other) const { 
        return m_value != other.m_value;
    }

    constexpr size_t size() const { return 1; }

    /// @brief Get as std::vector
    inline std::vector<D> asStdVector() const {
        return std::vector<D>{m_x};
    }

    /// @brief return vector components
    inline D x() const {
        return m_value;
    }
    inline void setX(D val) {
        m_value = val;
    }

    /// @brief Return a normalized version of the vector
    inline Vector<D, 1> normalized() const { return Vector<D, 1>(D(1)); }

    /// @brief Return a normalized version of the vector
    inline void normalize() { m_value = 1; }

    /// @brief Perform a dot product with another vector
    inline D dot(const Vector<D, 1>& other) const {
        return m_value * other.m_value;
    }

    /// @brief return the length of this vector
    inline D length() const {
        return m_value;
    }

    /// @brief return the length of this vector squared
    inline D lengthSquared() const {
        return m_value * m_value;
    }

    /// @brief Return whether or not any of the vector elements are negative
    inline bool hasNegative() { 
        return m_value < 0;
    }

    inline const D* getData() const {
        return &m_value;
    }
    inline D* data() {
        return &m_value;;
    }

    /// @brief Lerp with another vector
    Vector<D, 1> lerp(const Vector<D, 1>& other, double f) {
        return Vector<D, 1>(Interpolation::lerp(m_value, other.m_value, f));
    }

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json<D, 1>(nlohmann::json& orJson, const Vector<D, 1>& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    template<typename T, size_t S>
    friend void from_json<D, 1>(const nlohmann::json& korJson, Vector<T, S>& orObject);

protected:

    /// @brief Since the vector class is templated, it needs to be friends with all possible templates
    template<class D, size_t M> friend class Vector;
    friend class Matrix<D, 1, 1>;

private:
    D m_value{ 0 }; ///< The value stored by the vector
};
