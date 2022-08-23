#pragma once

template <typename D>
class Vector<D, 3> {
private:

    static constexpr size_t N = 3;

public:
    static Vector<D, N> EmptyVector() {
        return Vector<D, N>();
    }

    static const Vector<D, N>& Ones() {
        static Vector<D, N> ones(D(1.0));
        return ones;
    }

    static const Vector<D, N>& Right() {
        static Vector<D, N> right(D(1.0), D(0.0), D(0.0));
        return right;
    }
    static const Vector<D, N>& Left() {
        static Vector<D, N> left(D(-1.0), D(0.0), D(0.0));
        return left;
    }
    static const Vector<D, N>& Up() {
        static Vector<D, N> up(D(0.0), D(1.0), D(0.0));
        return up;
    }
    static const Vector<D, N>& Down() {
        static Vector<D, N> down(D(0.0), D(-1.0), D(0.0));
        return down;
    }
    static const Vector<D, N>& Forward() {
        static Vector<D, N> forward(D(0.0), D(0.0), D(1.0));
        return forward;
    }
    static const Vector<D, N>& Back() {
        static Vector<D, N> back(D(0.0), D(0.0), D(-1.0));
        return back;
    }

    constexpr Vector() = default;
    constexpr explicit Vector(D fillValue) :
        m_x(fillValue),
        m_y(fillValue),
        m_z(fillValue)
    {
    }

    Vector(const std::vector<D>& vec) {
        memcpy(data(), vec.data(), sizeof(D) * N);
    }

    constexpr Vector(D x, D y, D z) :
        m_x(x),
        m_y(y),
        m_z(z)
    {
    }

    Vector(const D* arrayPtr)
    {
        memcpy(data(), arrayPtr, sizeof(D) * N);
    }

    Vector(const json& j) :
        m_x(j[0].get<D>()),
        m_y(j[1].get<D>()),
        m_z(j[2].get<D>())
    {
    }

    /// @todo Use C++ 20 to clean up
    /// @see https://stackoverflow.com/questions/3703658/specifying-one-type-for-all-arguments-passed-to-variadic-function-or-variadic-te
    template <size_t M, typename... T/*, typename = std::enable_if_t<are_same_v<D, T...>>*/>
    Vector(const Vector<D, M>& vec, T&&... entries)
    {
        constexpr size_t numArgs = sizeof...(T); // count number of parameter pack entries
        std::array<D, numArgs> entryArray {{ std::forward<T>(entries)...}};

        constexpr size_t size = std::min(N, M);

        // Populate from other vector
        memcpy(data(), vec.data(), sizeof(D) * size);

        // Populate from packed arguments
        if (numArgs && M < N) {
            constexpr int diff = std::max(int(N) - int(M), 0);
            memcpy(data() + M, entryArray.data(), sizeof(D) * diff);
        }
    }

    /// @brief  For conversion between different Vector types of the same size
    template <typename T>
    constexpr explicit Vector(const Vector<T, N>& vec)
    {
        m_x = D(vec.m_x);
        m_y = D(vec.m_y);
        m_z = D(vec.m_z);
    }

    inline bool operator< (const Vector<D, N>& other) const {
        return lengthSquared() < other.lengthSquared();
    }

    inline D& operator[] (std::size_t i) {
        if (i == 0) {
            return m_x;
        }
        else if (i == 1) {
            return m_y;
        }
        else {
            return m_z;
        }
    }

    inline const D& operator[] (std::size_t i) const {
        if (i == 0) {
            return m_x;
        }
        else if (i == 1) {
            return m_y;
        }
        else {
            return m_z;
        }
    }

    inline Vector<D, N>& operator+= (const Vector<D, N>& rhs) {
        m_x += rhs.m_x;
        m_y += rhs.m_y;
        m_z += rhs.m_z;
        return *this;
    }

    inline Vector<D, N>& operator+= (D factor) {
        m_x += factor;
        m_y += factor;
        m_z += factor;
        return *this;
    }

    inline const Vector<D, N> operator+ (const Vector<D, N>& other) const {
        return Vector<D, N>(other.m_x + m_x, other.m_y + m_y, other.m_z + m_z);
    }

    inline const Vector<D, N> operator+ (D other) const {
        return Vector<D, N>(other + m_x, other + m_y, other + m_z);
    }

    inline Vector<D, N>& operator-= (const Vector<D, N>& rhs) {
        m_x -= rhs.m_x;
        m_y -= rhs.m_y;
        m_z -= rhs.m_z;
        return *this;
    }

    inline Vector<D, N>& operator-= (D factor) {
        m_x -= factor;
        m_y -= factor;
        m_z -= factor;
        return *this;
    }

    inline const Vector<D, N> operator- (const Vector<D, N>& other) const {
        return Vector<D, N>(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z);
    }

    /// @brief Unary operator
    inline const Vector<D, N> operator-() const {
        return Vector<D, N>(m_x * -1, m_y * -1, m_z * -1);
    }

    inline const Vector<D, N> operator-(D other) const {
        return Vector<D, N>(m_x - other, m_y - other, m_z - other);
    }

    inline Vector<D, N>& operator*= (const Vector<D, N>& rhs) {
        m_x *= rhs.m_x;
        m_y *= rhs.m_y;
        m_z *= rhs.m_z;
        return *this;
    }

    inline Vector<D, N>& operator*= (D scale) {
        m_x *= scale;
        m_y *= scale;
        m_z *= scale;
        return *this;
    }

    inline const Vector<D, N> operator* (const Vector<D, N>& other) const {
        return Vector<D, N>(m_x * other.m_x, m_y * other.m_y, m_z * other.m_z);
    }

    inline const Vector<D, N> operator* (D other) const {
        return Vector<D, N>(m_x * other, m_y * other, m_z * other);
    }

    inline friend const Vector<D, N> operator* (D d, const Vector<D, N>& rhs) {
        return Vector<D, N>(rhs.m_x * d, rhs.m_y * d, rhs.m_z * d);
    }

    inline const Vector<D, N> operator/ (D other) const {
        D factor = D(1) / other;
        return Vector<D, N>(m_x * factor, m_y * factor, m_z * factor);
    }

    inline Vector<D, N>& operator/= (D scale) {
        D factor = D(1) / scale;
        m_x *= factor;
        m_y *= factor;
        m_z *= factor;
        return *this;
    }

    inline bool operator== (const Vector<D, N>& other) const {
        static constexpr double s_tolerance = 1e-6;
        return math::areClose(m_x, other.m_x, s_tolerance) && math::areClose(m_y, other.m_y, s_tolerance) && math::areClose(m_z, other.m_z, s_tolerance);
    }

    inline bool operator!= (const Vector<D, N>& other) const {
        return !(*this == other);
    }

    /// @brief Implicit conversion to std::string for use with qDebug
    operator std::string() const {
        std::string string;
        string += "vector(";
        string += std::to_string(m_x);
        string += ", ";
        string += std::to_string(m_y);
        string += ", ";
        string += std::to_string(m_z);
        string += ")";

        return string;
    }

    /// @brief Implicit conversion to std::vector of Ds
    inline operator std::vector<D>() const {
        std::vector<D> vec{ m_x, m_y, m_z };
        return vec;
    }

    /// @brief String stream operator
    inline friend std::ostream& operator<<(std::ostream& os, const Vector<D, N>& vec) {
        os << "vector(";
        os << vec.m_x;
        os << ", ";
        os << vec.m_y;
        os << ", ";
        os << vec.m_z;
        os << ")";

        return os;
    }

    inline const D& at(size_t i) const { return (*this)[i]; }
    inline D& at(size_t i) { return (*this)[i]; }

    inline const D* data() const { return &m_x; }

    constexpr size_t size() const { return N; }

    /// @brief Get as std::vector
    inline std::vector<D> asStdVector() const {
        return std::vector<D>{m_x, m_y, m_z};
    }

    inline D x() const {
        return m_x;
    }
    inline void setX(D val) {
        m_x = val;
    }
    inline D y() const {
        return m_y;
    }
    inline void setY(D val) {
        m_y = val;
    }
    inline D z() const {
        return m_z;
    }
    inline void setZ(D val) {
        m_z = val;
    }


    /// @brief return vector as a vector of doubles
    typedef std::is_same<D, double> is_double;
    typedef std::conditional_t<is_double::value, Vector<double, N>&, Vector<double, N>> DoubleVecType;
    inline const DoubleVecType asDouble() const {
        if constexpr (is_double::value) {
            return const_cast<const DoubleVecType>(*this);
        }
        else {
            return DoubleVecType(double(m_x), double(m_y), double(m_z));
        }
    }

    /// @brief return vector as a vector of reals
    /// @details Returns a reference to the vector if it is already real
    typedef std::is_same<D, Real_t> is_real;
    typedef std::conditional_t<is_real::value, Vector<Real_t, N>&, Vector<Real_t, N>> RealVecType;
    inline const RealVecType asReal() const {
        if constexpr (is_real::value) {
            return const_cast<const RealVecType>(*this);
        }
        else {
            return RealVecType(Real_t(m_x), Real_t(m_y), Real_t(m_z));
        }
    }

    /// @brief return vector as a vector of floats
    typedef std::is_same<D, float> is_float;
    typedef std::conditional_t<is_float::value, Vector<float, N>&, Vector<float, N>> FloatVecType;
    inline const FloatVecType asFloat() const {
        if constexpr (is_float::value) {
            return const_cast<const FloatVecType>(*this);
        }
        else {
            return FloatVecType(float(m_x), float(m_y), float(m_z));
        }
    }

    /// @brief return vector sorted in descending order
    inline Vector<D, N> sortedDescending() {
        Vector<D, N> copy = *this;
        copy.sortDescending();
        return copy;
    }

    /// @brief Return a normalized version of the vector
    inline Vector<D, N> normalized() const { return *this / length(); }

    /// @brief Return a normalized version of the vector
    inline void normalize() { *this = *this / length(); }

    /// @brief Perform a dot product with another vector
    inline D dot(const Vector<D, N>& other) const {
        return m_x * other.m_x + m_y * other.m_y + m_z * other.m_z;
    }

    /// @brief Perform a cross product with another vector
    inline Vector<D, N> cross(const Vector<D, N>& other) const {
        return Vector<D, N>(
            m_y * other.m_z - m_z * other.m_y,
            m_z * other.m_x - m_x * other.m_z,
            m_x * other.m_y - m_y * other.m_x);
    }


    /// @brief return the length of this vector
    inline D length() const {
        return sqrt(m_x * m_x + m_y * m_y + m_z * m_z);
    }

    /// @brief return the length of this vector squared
    inline D lengthSquared() const {
        return m_x * m_x + m_y * m_y + m_z * m_z;
    }

    /// @brief Return whether or not any of the vector elements are negative
    inline bool hasNegative() const {
        if (m_x < 0) {
            return true;
        }
        else if (m_y < 0) {
            return true;
        }
        else if (m_z < 0) {
            return true;
        }
        else {
            return false;
        }
    }

#ifdef LINALG_USE_EIGEN
    inline Eigen::Map<const Eigen::Matrix<D, N, 1>> getMatrixMap() const {
        return Eigen::Map<const Eigen::Matrix<D, N, 1>>(getData());
    }
    inline Eigen::Map<Eigen::Matrix<D, N, 1>> matrixMap() {
        return Eigen::Map<Eigen::Matrix<D, N, 1>>(data());
    }
#endif

    inline const D* getData() const {
        return &m_x;
    }
    inline D* data() {
        return &m_x;
    }

    /// @brief Lerp with another vector
    Vector<D, N> lerp(const Vector<D, N>& other, double f) {
        return Interpolation::lerp(*this, other, f);
    }

    /// @brief Get distance squared between this vector and another
    D distanceSquared(const Vector<D, N>& other) const {
        D p1 = m_x - other.m_x;
        D p2 = m_y - other.m_y;
        D p3 = m_z - other.m_z;
        return p1 * p1 + p2 * p2 + p3 * p3;
    }

    /// @brief Get distance between this vector and another
    D distance(const Vector<D, N>& other) const {
        return sqrt(distanceSquared(other));
    }

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json<D, N>(nlohmann::json& orJson, const Vector<D, N>& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    template<typename T, size_t S>
    friend void from_json<D, N>(const nlohmann::json& korJson, Vector<T, S>& orObject);

private:

    /// @brief  Algorithm to sort in ascending order
    void sortAscending()
    {
        if (m_x < m_y) {
            if (m_y < m_z) {
                return;
            }
            else if (m_x < m_z) {
                std::swap(m_y, m_z);
            }
            else {
                D tmp = std::move(m_x);
                m_x = std::move(m_z);
                m_z = std::move(m_y);
                m_y = std::move(tmp);
            }
        }
        else {
            if (m_x < m_z) {
                std::swap(m_x, m_y);
            }
            else if (m_z < m_y) {
                std::swap(m_x, m_z);
            }
            else {
                D tmp = std::move(m_x);
                m_x = std::move(m_y);
                m_y = std::move(m_z);
                m_z = std::move(tmp);
            }
        }
    }

    /// @brief Algorithm to sort in descending order
    void sortDescending()
    {
        if (m_x > m_y) {
            if (m_y > m_z) {
                return;
            }
            else if (m_x > m_z) {
                std::swap(m_y, m_z);
            }
            else {
                D tmp = std::move(m_x);
                m_x = std::move(m_z);
                m_z = std::move(m_y);
                m_y = std::move(tmp);
            }
        }
        else {
            if (m_x > m_z) {
                std::swap(m_x, m_y);
            }
            else if (m_z > m_y) {
                std::swap(m_x, m_z);
            }
            else {
                D tmp = std::move(m_x);
                m_x = std::move(m_y);
                m_y = std::move(m_z);
                m_z = std::move(tmp);
            }
        }
    }

    D m_x{ 0 }; ///< The first value stored by the vector
    D m_y{ 0 }; ///< The second value stored by the vector
    D m_z{ 0 }; ///< The third value stored by the vector
};
