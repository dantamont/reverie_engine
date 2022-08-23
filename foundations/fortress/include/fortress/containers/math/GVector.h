#pragma once
#define G_USE_VECTOR_SPECIALIZATIONS

#include <typeinfo>

// Standard
#include <array>
#pragma warning(push, 0)        
#include <Eigen/Dense>
#pragma warning(pop)       
#include <numeric>
#include <string>

// Internal
#include "fortress/json/GJson.h"
#include "fortress/math/GInterpolation.h"
#include "fortress/math/GMath.h"
#include "fortress/types/GSizedTypes.h"
#include "fortress/templates/GTemplates.h"

namespace rev {

static double ERROR_TOLERANCE = 1.0E-9;

template<class D, size_t N_ROWS, size_t N_COLS>
class Matrix;

/// @brief Need forward declarations for templated friends to work correctly 
template <typename D, size_t N>
class Vector;

template <typename D, size_t N>
void to_json(json& orJson, const Vector<D, N>& korObject);

template <typename D, size_t N>
void from_json(const nlohmann::json& korJson, Vector<D, N>& orObject);


/// @class Vector
/// @brief A class representing a generic vector
///	@details Template takes in the numeric type (e.g. float, double), and size of vector
/// @note Uses std::move call to instantiate with a std::vector, so does not preserve input
template <typename D, size_t N>
class Vector {
public:
    /// @name Static
    /// @{

    static Vector<D, N> EmptyVector() {
        return Vector<D, N>(false);
    }

    static const Vector<D, N>& Ones() {
        static Vector<D, N> ones(D(1.0));
        return ones;
    }

    static const Vector<D, N>& Right() {
        static_assert(N >= 3, "Incorrect size");
        static Vector<D, N> right(D(1.0), D(0.0), D(0.0));
        return right;
    }
    static const Vector<D, N>& Left() {
        static_assert(N >= 3, "Incorrect size");
        static Vector<D, N> left(D(-1.0), D(0.0), D(0.0));
        return left;
    }
    static const Vector<D, N>& Up() {
        static_assert(N >= 3, "Incorrect size");
        static Vector<D, N> up(D(0.0), D(1.0), D(0.0));
        return up;
    }
    static const Vector<D, N>& Down() {
        static_assert(N >= 3, "Incorrect size");
        static Vector<D, N> down(D(0.0), D(-1.0), D(0.0));
        return down;
    }
    static const Vector<D, N>& Forward(){
        static_assert(N >= 3, "Incorrect size");
        static Vector<D, N> forward(D(0.0), D(0.0), D(1.0));
        return forward;
    }
    static const Vector<D, N>& Back() {
        static_assert(N >= 3, "Incorrect size");
        static Vector<D, N> back(D(0.0), D(0.0), D(-1.0));
        return back;
    }
    static Vector<D, N>* EmptyVectorPointer() {
        return new Vector<D, N>(false);
    }

    /// @}

    /// @name Constructors and Destructors
    /// @{
    explicit Vector(D fillValue = D(0)) {
        // Zero vector by default
        std::fill(std::begin(m_array), std::end(m_array), fillValue);
	}
    
    template<size_t M>
    explicit Vector(const std::array<D, M>& arr){
        initialize(arr);
    }

    Vector(const std::vector<D>& vec) {
        initialize(vec);
	}


    Vector(D xpos, D ypos, D zpos = D(0), D wpos = D(0))
    {
        initialize(xpos, ypos, zpos, wpos);
    }

    Vector(const D* arrayPtr)
    {
        memcpy(m_array.data(), arrayPtr, sizeof(D)*N);
    }

    /// @todo Use C++ 20 to clean up
    /// @see https://stackoverflow.com/questions/3703658/specifying-one-type-for-all-arguments-passed-to-variadic-function-or-variadic-te
    template <size_t M, typename... T/*, typename = std::enable_if_t<are_same_v<D, T...>>*/>
    Vector(const Vector<D, M>& vec, T&&... entries)
    {
        constexpr size_t numArgs = sizeof...(T); // count number of parameter pack entries
        std::array<D, numArgs> entryArray{ { std::forward<T>(entries)...} };

        constexpr size_t size = std::min(N, M);

        // Populate from other vector
        memcpy(data(), vec.data(), sizeof(D) * size);

        // Populate from packed arguments
        if (numArgs && M < N) {
            constexpr int diff = std::max(int(N) - int(M), 0);
            memcpy(m_array.data() + M, entryArray.data(), sizeof(D)*diff);
        }

        if (M + numArgs < N) {
            // Fill remaining entries with zero
            std::fill(std::begin(m_array) + M + numArgs, std::end(m_array), D(0));
        }
    }

    template <size_t M, typename ... Entries/*, typename = std::enable_if_t<are_same_v<D, T...>>*/>
    Vector(D fillValue, const Vector<D, M>& vec, Entries... entries)
    {
        constexpr size_t numArgs = sizeof...(T); // count number of parameter pack entries
        std::array<D, numArgs> entryArray{ { std::forward<T>(entries)...} };
        size_t size = std::min(N, M);
        memcpy(m_array.data(), vec.m_array.data(), sizeof(D)*size);

        // Populate from packed arguments
        memcpy(m_array.data() + M, entryArray.data(), sizeof(D)*numArgs);

        if (M + numArgs < N) {
            // Fill remaining entries with zero
            std::fill(std::begin(m_array) + M + numArgs, std::end(m_array), fillValue);
        }
    }

    /// @brief  For conversion between different Vector types of the same size
    /// @tparam T 
    /// @param vec 
    template <typename T>
    explicit Vector(const Vector<T, N>& vec)
    {
        for (size_t i = 0; i < N; i++) {
            m_array[i] = D(vec[i]);
        }
    }

    ~Vector() {
	}
    /// @}

    /// @name Operators
    /// @{

    inline bool operator< (const Vector<D, N>& other) const {
        return lengthSquared() < other.lengthSquared();
    }

    inline D& operator[] (std::size_t i) {

        return m_array[i];
    }

    inline const D& operator[] (std::size_t i) const{
        return m_array[i];
    }

    inline Vector<D, N>& operator+= (const Vector<D, N> &rhs) {
#ifdef LINALG_USE_EIGEN
        auto thisMap = map();
        thisMap += rhs.getMap();
#else
		std::vector<D> result;
		result.reserve(size());

		std::transform(m_array.begin(), m_array.end(), rhs.begin(), std::back_inserter(result), std::plus<D>());

		//m_array = std::move(result.data());
		std::move(result.begin(), result.begin() + result.size(), m_array.begin());
#endif
		return *this;
	}

    inline Vector<D, N>& operator+= (D factor) {
#ifdef LINALG_USE_EIGEN
        auto thisMap = map();
        thisMap += factor;
#else
        for (size_t i = 0; i < N; i++) {
            m_array[i] += factor;
        }
#endif
        return *this;
    }

    inline const Vector<D, N> operator+ (const Vector<D, N> &other) const {
		Vector<D, N> result = *this;
		result += other;
		return result;
	}

    inline const Vector<D, N> operator+ (D other) const {
#ifdef LINALG_USE_EIGEN
        auto thisMap = getMap();
        Eigen::Array<D, N, 1> outMap = thisMap + other;
        Vector<D, N> result(outMap.data());
#else
        Vector<D, N> result = *this;
        for (size_t i=0; i < m_array.size(); i++) {
            result.m_array[i] += other;
        }
#endif
        return result;

    }

    inline Vector<D, N>& operator-= (const Vector<D, N> &rhs) {
//#ifdef LINALG_USE_EIGEN
//        auto thisMap = map();
//        thisMap -= rhs.getMap();
//#else
		//std::vector<D> result;
		//result.reserve(size());

		std::transform(m_array.begin(), m_array.end(), rhs.begin(), m_array.begin(), std::minus<D>());

		//std::move(result.begin(), result.begin() + result.size(), m_array.begin());
//#endif
		return *this;
	}

    inline Vector<D, N>& operator-= (D factor) {
#ifdef LINALG_USE_EIGEN
        auto thisMap = map();
        thisMap -= factor;
#else
        for (size_t i = 0; i < N; i++) {
            m_array[i] -= factor;
        }
#endif
        return *this;
    }

    inline const Vector<D, N> operator- (const Vector<D, N> &other) const {
		Vector<D, N> result = *this;
		result -= other;
		return result;
	}

    /// @brief Unary operator
    inline const Vector<D, N> operator-() const {
		Vector<D, N> result = *this * -1;
		return result;
	}

    inline const Vector<D, N> operator-(D other) const {
        Vector<D, N> result = *this;
        result -= other;
        return result;
    }

    inline Vector<D, N>& operator*= (const Vector<D, N> &rhs) {
#ifdef LINALG_USE_EIGEN
        auto thisMap = map();
        thisMap *= rhs.getMap();
#else
		std::vector<D> result;
		result.reserve(size());

		std::transform(m_array.begin(), m_array.end(), rhs.begin(), std::back_inserter(result), std::multiplies<D>());

		//m_array = std::move(result.data());
		std::move(result.begin(), result.begin() + result.size(), m_array.begin());

#endif
        return *this;
	}

    inline Vector<D, N>& operator*= (D scale) {
#ifdef LINALG_USE_EIGEN
        auto thisMap = map();
        thisMap *= scale;
#else
		std::vector<D> result;
		result.reserve(size());

		std::transform(m_array.begin(), m_array.end(), std::back_inserter(result),
			[&](D& entry) {return entry * scale; }
		);

		//m_array = std::move(result.data());
		std::move(result.begin(), result.begin() + result.size(), begin());

#endif
        return *this;
	}

    inline const Vector<D, N> operator* (const Vector<D, N> &other) const {
		Vector<D, N> result = *this;
		result *= other;
		return result;
	}

    inline const Vector<D, N> operator* (D other) const {
		Vector<D, N> result = *this;
		result *= other;
		return result;
	}

    inline friend const Vector<D, N> operator* (D d, const Vector<D, N> &rhs) {
		Vector<D, N> result = rhs;
		result *= d;
		return result;
	}

    inline const Vector<D, N> operator/ (D other) const {
		Vector<D, N> result = *this;
		result *= (1.0 / other);
		return result;
	}

    inline Vector<D, N>& operator/= (D scale) {
#ifdef LINALG_USE_EIGEN
        auto thisMap = map();
        thisMap /= scale;
#else
        std::vector<D> result;
        result.reserve(size());

        std::transform(m_array.begin(), m_array.end(), std::back_inserter(result),
            [&](D& entry) {return entry / scale; }
        );

        //m_array = std::move(result.data());
        std::move(result.begin(), result.begin() + result.size(), begin());

#endif
        return *this;
    }

    inline bool operator== (const Vector<D, N> &other) const {
#ifdef LINALG_USE_EIGEN
        return getMap().isApprox(other.getMap());
#else
		unsigned int length = size();
		if (length != other.size()) return false;
		bool isEqual = true;

		for (unsigned int i = 0; i < length; i++) {
			isEqual &= abs(m_array[i] - other[i]) < 1.0e-6;
		}
		return isEqual;
#endif
	}

    inline bool operator!= (const Vector<D, N> &other) const { return !(*this == other); }

    /// @brief Implicit conversion to std::string for use with qDebug
    operator std::string() const {
        std::string string;
		string += "vector(";
		for (const D& e : m_array) {
			string += std::to_string(e);
			string += ", ";
		}
        string.pop_back();
        string.pop_back();
		string += ")";

		return string;
	}

    /// @brief Implicit conversion to std::vector of Ds
    inline operator std::vector<D>() const {
		std::vector<D> vec(m_array.begin(), m_array.end());
		return vec;
	}

    /// @brief String stream operator
    inline friend std::ostream& operator<<(std::ostream& os, const Vector<D, N>& vec) {
		os << "vector(";
		for (auto& e : vec.m_array) {
			os << e;
			os << ", ";
		}
		os << ")";

		return os;
	}

    /// @}

    /// @name Properties
    /// @{

    inline const D& at(size_t i) const { return m_array.at(i); }
    inline D& at(size_t i)  { return m_array.at(i); }

    inline const D* data() const {return m_array.data();}

    constexpr size_t size() const { return m_array.size(); }
    
    typename std::array<D, N>::iterator begin() { return m_array.begin(); }
    typename std::array<D, N>::iterator end() { return m_array.end(); }

    typename std::array<D, N>::const_iterator begin() const { return m_array.begin(); }
    typename std::array<D, N>::const_iterator end() const { return m_array.end(); }

    /// @}

    /// @name Public Methods
    /// @{


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

    /// @brief Get as std::vector
    inline std::vector<D> asStdVector() const{
        return std::vector<D>(m_array.begin(), m_array.end());
    }

    /// @brief return vector components
    inline D x() const {
        static_assert(N > 0, "N must be greater than zero");
        return m_array.at(0); 
    }
    inline void setX(D val) {
        static_assert(N > 0, "N must be greater than zero");
        m_array[0] = val;
    }
    inline D y() const {
        static_assert(N > 1, "N must be greater than one");
        return m_array.at(1); 
    }
    inline void setY(D val) {
        static_assert(N > 1, "N must be greater than one");
        m_array[1] = val;
    }

    inline D z() const {
        static_assert(N > 2, "N must be greater than two");
        return m_array.at(2); 
    }
    inline void setZ(D val) {
        static_assert(N > 2, "N must be greater than two");
        m_array[2] = val;
    }

    inline D w() const {
        static_assert(N > 3, "N must be greater than three");
        return m_array.at(3); 
    }
    inline void setW(D val) {
        static_assert(N > 3, "N must be greater than three");
        m_array[3] = val;
    }

	/// @brief return vector as a vector of doubles
    typedef std::is_same<D, double> is_double;
    typedef std::conditional_t<is_double::value, Vector<double, N>&, Vector<double, N>> DoubleVecType;
    inline const DoubleVecType asDouble() const {
        if constexpr (is_double::value) {
            return const_cast<const DoubleVecType>(*this);
        }
        else {
#ifdef LINALG_USE_EIGEN
            auto thisMap = getMap();
            Eigen::Array<double, N, 1> outVec = thisMap.cast<double>();
            Vector<double, N> result(outVec.data());
#else
            std::vector<double> result(m_array.begin(), m_array.end());
#endif
            return result;
    }
    }

    /// @brief return vector as a vector of reals
    /// @details Returns a reference to the vector if it is already real
    typedef std::is_same<D, Real_t> is_real;
    typedef std::conditional_t<is_real::value, Vector<Real_t, N>&, Vector<Real_t, N>> RealVecType;
    inline const RealVecType asReal() const {
        if constexpr(is_real::value) {
            return const_cast<const RealVecType>(*this);
        }
        else {
#ifdef LINALG_USE_EIGEN
            auto thisMap = getMap();
            Eigen::Array<Real_t, N, 1> outVec = thisMap.cast<Real_t>();
            Vector<Real_t, N> result(outVec.data());
#else
            std::vector<Real_t> result(m_array.begin(), m_array.end());
#endif
            return result;
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
#ifdef LINALG_USE_EIGEN
            auto thisMap = getMap();
            Eigen::Array<float, N, 1> outVec = thisMap.cast<float>();
            Vector<float, N> result(outVec.data());
#else
            std::vector<float> result(m_array.begin(), m_array.end());
#endif
            return result;
    }
    }

    /// @brief return vector sorted in descending order
    inline Vector<D, N> sortedDescending() {
		Vector<D, N> sorted = *this;
		std::sort(sorted.begin(), sorted.end(), std::greater<D>());
		return sorted;
	}

    /// @brief Return a normalized version of the vector
    inline Vector<D, N> normalized() const { return *this / length(); }

    /// @brief Return a normalized version of the vector
    inline void normalize() { *this = *this / length(); }

    /// @brief Perform a dot product with another vector
    inline D dot(const Vector<D, N>& other) const {
#ifdef LINALG_USE_EIGEN
        auto dot = std::inner_product(m_array.begin(), m_array.end(), other.m_array.begin(), D(0));
        return dot;
#else
		Vector<D, N> product = *this * other;
		D sum = std::accumulate(product.begin(), product.end(), 0.0);
		return sum;
#endif
	}

    /// @brief Perform a cross product with another vector
    inline Vector<D, N> cross(const Vector<D, N>& other) const {
        static_assert(N == 3, "This function requires a three-element vector");
//#ifdef LINALG_USE_EIGEN
//        Eigen::Map<const Eigen::Matrix<D, N, 1>> thisMap = getMatrixMap();
//        Eigen::Map<const Eigen::Matrix<D, N, 1>> otherMap = other.getMatrixMap();
//        Vector<D, N> result(thisMap.cross(otherMap).data());
//        return result;
//#else
        return Vector<D, N>(
            m_array.at(1) * other.at(2) - m_array.at(2) * other.at(1),
            m_array.at(2) * other.at(0) - m_array.at(0) * other.at(2),
            m_array.at(0) * other.at(1) - m_array.at(1) * other.at(0));
//#endif
    }

    /// @brief return the length of this vector
    inline D length() const {
#ifdef LINALG_USE_EIGEN
        Eigen::Map<const Eigen::Matrix<D, N, 1>> thisMap = getMatrixMap();
        return thisMap.norm();
#else
		Vector<D, N> square = *this * *this;
		D squareSum = std::accumulate(square.begin(), square.end(), 0.0);
		return sqrt(squareSum);
#endif
	}

    /// @brief return the length of this vector squared
    inline D lengthSquared() const {
#ifdef LINALG_USE_EIGEN
        Eigen::Map<const Eigen::Matrix<D, N, 1>> thisMap = getMatrixMap();
        return thisMap.squaredNorm();
#else
        Vector<D, N> square = *this * *this;
        D squareSum = std::accumulate(square.begin(), square.end(), 0.0);
        return squareSum;
#endif
    }

    /// @brief Return whether or not any of the vector elements are negative
    inline bool hasNegative() const { return std::any_of(m_array.begin(), m_array.end(), [](D entry) {return entry < 0; }); }

#ifdef LINALG_USE_EIGEN
    inline Eigen::Map<const Eigen::Array<D, N, 1>> getMap() const {
        return Eigen::Map<const Eigen::Array<D, N, 1>>(getData());
    }
    inline Eigen::Map<const Eigen::Matrix<D, N, 1>> getMatrixMap() const {
        return Eigen::Map<const Eigen::Matrix<D, N, 1>>(getData());
    }
    inline Eigen::Map<Eigen::Array<D, N, 1>> map() {
        return Eigen::Map<Eigen::Array<D, N, 1>>(data());
    }
    inline Eigen::Map<Eigen::Matrix<D, N, 1>> matrixMap() {
        return Eigen::Map<Eigen::Matrix<D, N, 1>>(data());
    }
#endif

    inline const D* getData() const {
        return m_array.data();
    }
    inline D* data() {
        return m_array.data();
    }

    /// @brief Lerp with another vector
    Vector<D, N> lerp(const Vector<D, N>& other, double f) {
        return Interpolation::lerp(*this, other, f);
    }

    /// @brief Get distance squared between this vector and another
    D distanceSquared(const Vector<D, N>& other) const {
        static_assert(N == 3 || N == 4, "Only vectors of length 3 or four supported");
        D p1, p2, p3;
        p1 = m_array[0] - other.m_array[0];
        p2 = m_array[1] - other.m_array[1];
        p3 = m_array[2] - other.m_array[2];
        return p1*p1 + p2*p2 + p3*p3;
    }

    /// @brief Get distance between this vector and another
    D distance(const Vector<D, N>& other) const {
        static_assert(N == 3 || N == 4, "Only vectors of length 3 or four supported");
        return sqrt(distanceSquared(other));
    }

    /// @}

protected:
    /// @name Friends
    /// @{

    /// @brief Since the vector class is templated, it needs to be friends with all possible templates
    template<class D, size_t M> friend class Vector;
    friend class Matrix<D, N, N>;

    /// @}

    /// @name Protected Methods
    /// @{

    explicit Vector(bool fill) {
        if (fill) {
            m_array.fill(D(0));
        }
    }

    inline void initialize(D v0, D v1) {
        m_array[0] = v0;
        m_array[1] = v1;
    }

    inline void initialize(D v0, D v1, D v2) {
        initialize(v0, v1);
        if constexpr(N > 2) {
            m_array[2] = v2;
        }
    }

    inline void initialize(D v0, D v1, D v2, D v3) {
        initialize(v0, v1, v2);
        if constexpr (N > 3) {
            m_array[3] = v3;
        }
    }

    inline void initialize(const std::vector<D>& vec) {
        //std::move(vec.begin(), vec.begin() + std::min(vec.size(), N), m_array.begin());
        memcpy(m_array.data(), vec.data(), sizeof(D) * N);
    }

    template<size_t M>
    inline void initialize(const std::array<D, M>& vec) {
        //std::move(vec.begin(), vec.begin() + std::min(vec.size(), N), m_array.begin());
        constexpr size_t n = M > N ? N : M;
        memcpy(m_array.data(), vec.data(), sizeof(D) * n);
    }

    /// @}

    /// @name Protected Members
    /// @{
    std::array<D, N> m_array;
    /// @}
};

#ifdef G_USE_VECTOR_SPECIALIZATIONS
#include "fortress/containers/math/private/GVectorOne.hpp"
#include "fortress/containers/math/private/GVectorTwo.hpp"
#include "fortress/containers/math/private/GVectorThree.hpp"
#include "fortress/containers/math/private/GVectorFour.hpp"
#endif

template <typename D, size_t N>
void to_json(nlohmann::json& orJson, const Vector<D, N>& korObject)
{
#ifdef G_USE_VECTOR_SPECIALIZATIONS
    if constexpr (N == 1) 
    {
        orJson = korObject.m_value;
    }
    else if constexpr (N == 2)
    {
        orJson = json::array();
        orJson.push_back(korObject.m_x);
        orJson.push_back(korObject.m_y);
    }
    else if constexpr (N == 3)
    {
        orJson = json::array();
        orJson.push_back(korObject.m_x);
        orJson.push_back(korObject.m_y);
        orJson.push_back(korObject.m_z);
    }
    else if constexpr (N == 4)
    {
        orJson = json::array();
        orJson.push_back(korObject.m_x);
        orJson.push_back(korObject.m_y);
        orJson.push_back(korObject.m_z);
        orJson.push_back(korObject.m_w);
    }
    else
    {
#endif
        orJson = json::array();
        for (const D& val : korObject.m_array) {
            orJson.push_back(val);
        }

#ifdef G_USE_VECTOR_SPECIALIZATIONS
    }
#endif
}

template <typename D, size_t N>
void from_json(const nlohmann::json& korJson, Vector<D, N>& orObject)
{
#ifdef G_USE_VECTOR_SPECIALIZATIONS
    if constexpr (N == 1)
    {
        orObject.m_value = korJson.get<D>();
    }
    else if constexpr (N == 2)
    {
        orObject.m_x = korJson[0].get<D>();
        orObject.m_y = korJson[1].get<D>();
    }
    else if constexpr (N == 3)
    {
        orObject.m_x = korJson[0].get<D>();
        orObject.m_y = korJson[1].get<D>();
        orObject.m_z = korJson[2].get<D>();
    }
    else if constexpr (N == 4)
    {
        orObject.m_x = korJson[0].get<D>();
        orObject.m_y = korJson[1].get<D>();
        orObject.m_z = korJson[2].get<D>();
        orObject.m_w = korJson[3].get<D>();
    }
    else {
#endif
        size_t n = std::min(N, korJson.size());
        for (Uint32_t i = 0; i < n; i++) {
            orObject.m_array[i] = korJson[i].get<D>();
        }
#ifdef G_USE_VECTOR_SPECIALIZATIONS
    }
#endif
}



// Typedefs based on generic vector
typedef Vector<Int32_t, 2> Vector2i;
typedef Vector<Int32_t, 3> Vector3i;
typedef Vector<Int32_t, 4> Vector4i;
typedef Vector<Uint32_t, 2> Vector2u;
typedef Vector<Uint32_t, 3> Vector3u;
typedef Vector<Uint32_t, 4> Vector4u;
typedef Vector<Float32_t, 2> Vector2f;
typedef Vector<Float32_t, 3> Vector3f;
typedef Vector<Float32_t, 4> Vector4f;
typedef Vector<Float64_t, 2> Vector2d;
typedef Vector<Float64_t, 3> Vector3d;
typedef Vector<Float64_t, 4> Vector4d;
typedef Vector<Real_t, 2> Vector2;
typedef Vector<Real_t, 3> Vector3;
typedef Vector<Real_t, 4> Vector4;


} // End namespaces
