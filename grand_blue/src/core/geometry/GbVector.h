// See:
// https://www.trentreed.net/blog/qt5-opengl-part-1-basic-rendering/

#ifndef GB_VECTOR_H
#define GB_VECTOR_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
#include <typeinfo>

// QT
#include <QString>
#include <array>
#include <QVector3D>
#include <QVector2D>
#include <QVector4D>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <Eigen/Dense>

// Internal
#include "../mixins/GbLoadable.h"
#include "../utils/GbInterpolation.h"

namespace Gb {

class ShaderProgram;

/////////////////////////////////////////////////////////////////////////////////////////////
// Macro
/////////////////////////////////////////////////////////////////////////////////////////////
#ifdef GB_USE_DOUBLE
typedef double real_g;
#else
typedef float real_g;
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////////////////////
static double ERROR_TOLERANCE = 1.0E-9;

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////////////////////
template<class D, size_t N_ROWS, size_t N_COLS>
class Matrix;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** @class Vector
    @brief A class representing a generic vector
	@details Template takes in the numeric type (e.g. float, double), and size of vector
    @note Uses std::move call to instantiate with a std::vector, so does not preserve input
*/
template <class D, size_t N>
class Vector {
public:
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @name Constructors and Destructors
    /// @{
    Vector(D fillValue = D(0)) {
        // Zero vector by default
        std::fill(std::begin(m_array), std::end(m_array), fillValue);
	}
    
    Vector(const QJsonValue& json) {
        loadFromJson(json);
    }

    Vector(const std::vector<D>& vec) {
        initialize(vec);
	}

    Vector(const QVector3D& vector, D wpos) {
        initialize(vector.x(), vector.y(), vector.z(), wpos);
    }
    Vector(const QVector3D& vector) {
        initialize(vector.x(), vector.y(), vector.z());
    }
    Vector(const QVector2D& vector, D zpos, D wpos){
        initialize(vector.x(), vector.y(), zpos, wpos);
    }
    Vector(const QVector2D& vector)
    {
        initialize(vector.x(), vector.y());
    }
    Vector(D xpos, D ypos, D zpos = D(0), D wpos = D(0))
    {
        initialize(xpos, ypos, zpos, wpos);
    }
    Vector(const D* arrayPtr)
    {
        memcpy(m_array.data(), arrayPtr, sizeof(D)*N);
    }

    // TODO: Enforce type D in entries argument
    // FIXME: Throws error when trying to assign from larger vector, e.g. new Vector3(Vector4(...))
    template <size_t M, typename... T>
    Vector(const Vector<D, M>& vec, T&&... entries)
    {
        //constexpr size_t numArgs = entries.size();
        std::vector<D> entryVec{ { static_cast<D>(entries)... } };
        uint numArgs = entryVec.size();
        constexpr size_t size = std::min(N, M);
        memcpy(m_array.data(), vec.m_array.data(), sizeof(D)*size);

        // Populate from packed arguments
        if (numArgs && M < N) {
            constexpr int diff = std::max(int(N) - int(M), 0);
            //memcpy(m_array.data() + M, entryVec.data(), sizeof(D)*numArgs);
            memcpy(m_array.data() + M, entryVec.data(), sizeof(D)*diff);
        }

        if (M + numArgs < N) {
            // Fill remaining entries with zero
            std::fill(std::begin(m_array) + M + numArgs, std::end(m_array), D(0));
        }
    }

    template <size_t M, typename ... Entries>
    Vector(D fillValue, const Vector<D, M>& vec, Entries... entries)
    {
        std::vector<D> entryVec{ { static_cast<D>(entries)... } };
        uint numArgs = entryVec.size();
        size_t size = std::min(N, M);
        memcpy(m_array.data(), vec.m_array.data(), sizeof(D)*size);

        // Populate from packed arguments
        memcpy(m_array.data() + M, entryVec.data(), sizeof(D)*numArgs);

        if (M + numArgs < N) {
            // Fill remaining entries with zero
            std::fill(std::begin(m_array) + M + numArgs, std::end(m_array), fillValue);
        }
    }

    // Want to be explicit with conversions, so removed
    //template <typename T, typename ... Entries>
    //Vector(const Vector<T, N>& vec, Entries... entries)
    //{
    //    std::vector<D> entryVec{ { static_cast<D>(entries)... } };
    //    uint numArgs = entryVec.size();
    //    memcpy(m_array.data(), vec.m_array.data(), sizeof(D)*N);

    //    // Populate from packed arguments
    //    memcpy(m_array.data() + N, entryVec.data(), sizeof(D)*numArgs);

    //    if (N + numArgs < N) {
    //        // Fill remaining entries with zero
    //        std::fill(std::begin(m_array) + N + numArgs, std::end(m_array), D(0));
    //    }
    //}

    ~Vector() {
	}
    /// @}

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @name Operators
    /// @{
    //-----------------------------------------------------------------------------------------------------------------
    inline bool operator< (const Vector<D, N>& other) const {
        return lengthSquared() < other.lengthSquared();
    }
    //-----------------------------------------------------------------------------------------------------------------
    inline D& operator[] (std::size_t i) {

        return m_array[i];
    }
    //-----------------------------------------------------------------------------------------------------------------
    inline const D& operator[] (std::size_t i) const{
        return m_array[i];
    }
	//-----------------------------------------------------------------------------------------------------------------
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

    //-----------------------------------------------------------------------------------------------------------------
    inline Vector<D, N>& operator+= (D factor) {
#ifdef LINALG_USE_EIGEN
        auto thisMap = map();
        thisMap += factor;
#else
        for (size_t i = 0; i < N; i++) {
            m_array[i] += D;
        }
#endif
        return *this;
    }

	//-----------------------------------------------------------------------------------------------------------------
    inline const Vector<D, N> operator+ (const Vector<D, N> &other) const {
		Vector<D, N> result = *this;
		result += other;
		return result;
	}

    //-----------------------------------------------------------------------------------------------------------------
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

	//-----------------------------------------------------------------------------------------------------------------
    inline Vector<D, N>& operator-= (const Vector<D, N> &rhs) {
#ifdef LINALG_USE_EIGEN
        auto thisMap = map();
        thisMap -= rhs.getMap();
#else
		std::vector<D> result;
		result.reserve(size());

		std::transform(m_array.begin(), m_array.end(), rhs.begin(), std::back_inserter(result), std::minus<D>());

		//m_array = std::move(result.data());
		std::move(result.begin(), result.begin() + result.size(), m_array.begin());
#endif
		return *this;
	}

    //-----------------------------------------------------------------------------------------------------------------
    inline Vector<D, N>& operator-= (D factor) {
#ifdef LINALG_USE_EIGEN
        auto thisMap = map();
        thisMap -= factor;
#else
        for (size_t i = 0; i < N; i++) {
            m_array[i] -= D;
        }
#endif
        return *this;
    }

	//-----------------------------------------------------------------------------------------------------------------
    inline const Vector<D, N> operator- (const Vector<D, N> &other) const {
		Vector<D, N> result = *this;
		result -= other;
		return result;
	}

	//-----------------------------------------------------------------------------------------------------------------
    inline const Vector<D, N> operator-() const {
		Vector<D, N> result = *this * -1;
		return result;
	}

	//-----------------------------------------------------------------------------------------------------------------
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

	//-----------------------------------------------------------------------------------------------------------------
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

	//-----------------------------------------------------------------------------------------------------------------
    inline const Vector<D, N> operator* (const Vector<D, N> &other) const {
		Vector<D, N> result = *this;
		result *= other;
		return result;
	}

	//-----------------------------------------------------------------------------------------------------------------
    inline const Vector<D, N> operator* (D other) const {
		Vector<D, N> result = *this;
		result *= other;
		return result;
	}

	//-----------------------------------------------------------------------------------------------------------------
    inline friend const Vector<D, N> operator* (D d, const Vector<D, N> &rhs) {
		Vector<D, N> result = rhs;
		result *= d;
		return result;
	}

	//-----------------------------------------------------------------------------------------------------------------
    inline const Vector<D, N> operator/ (D other) const {
		Vector<D, N> result = *this;
		result *= (1.0 / other);
		return result;
	}

    //-----------------------------------------------------------------------------------------------------------------
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

	//-----------------------------------------------------------------------------------------------------------------
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

	//-----------------------------------------------------------------------------------------------------------------
    inline bool operator!= (const Vector<D, N> &other) const { return !(*this == other); }

	//-----------------------------------------------------------------------------------------------------------------
    /// @brief Implicit conversion to QString for use with qDebug
    operator QString() const {
		QString string;
		string += "vector(";
		for (auto& e : m_array) {
			string += QString::number(e);
			string += ", ";
		}
        string.chop(2);
		string += ")";

		return string;
	}

	//-----------------------------------------------------------------------------------------------------------------
    /// @brief Implicit conversion to std::vector of Ds
    inline operator std::vector<D>() const {
		std::vector<D> vec(m_array.begin(), m_array.end());
		return vec;
	}

	//-----------------------------------------------------------------------------------------------------------------
    /// @brief String stream operator
    friend std::ostream& operator<<(std::ostream& os, const Vector<D, N>& vec) {
		os << "vector(";
		for (auto& e : vec.m_array) {
			os << e;
			os << ", ";
		}
		os << ")";

		return os;
	}

    /// @}

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @name Properties
    /// @{

    inline const D& at(size_t i) const { return m_array.at(i); }
    inline D& at(size_t i)  { return m_array.at(i); }

    inline const D* data() const {return m_array.data();}

    inline size_t size() const { return m_array.size(); }
    
    typename std::array<D, N>::iterator begin() { return m_array.begin(); }
    typename std::array<D, N>::iterator end() { return m_array.end(); }

    typename std::array<D, N>::const_iterator begin() const { return m_array.begin(); }
    typename std::array<D, N>::const_iterator end() const { return m_array.end(); }

    /// @}

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @name Public Methods
    /// @{

    /// @brief Express as a JSON object
    QJsonValue asJson() const{
        QJsonArray array;
        std::copy(m_array.begin(), m_array.end(), std::back_inserter(array));
        return array;
    }

    void loadFromJson(const QJsonValue& json) {
        const QJsonArray& array = json.toArray();
        for (int i = 0; i < array.size(); i++) {
            m_array[i] = D(array.at(i).toDouble());
        }
    }

    /// @brief Get array
    inline const std::array<D, N>& getArray() const{
        return m_array;
    }
    inline std::array<D, N>& array() {
        return m_array;
    }

    /// @brief Get as std::vector
    inline std::vector<D> asStdVector() const{
        return std::vector<D>(m_array.begin(), m_array.end());
    }

    /// @brief return vector components
    inline D x() const {
        static_assert(N > 0, "N must be greater than zero");
        return m_array.at(0); 
    }
    inline D y() const {
        static_assert(N > 1, "N must be greater than one");
        return m_array.at(1); 
    }
    inline D z() const {
        static_assert(N > 2, "N must be greater than two");
        return m_array.at(2); 
    }
    inline D w() const {
        static_assert(N > 3, "N must be greater than three");
        return m_array.at(3); 
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
    typedef std::is_same<D, real_g> is_real;
    typedef std::conditional_t<is_real::value, Vector<real_g, N>&, Vector<real_g, N>> RealVecType;
    inline const RealVecType asReal() const {
        if constexpr(is_real::value) {
            return const_cast<const RealVecType>(*this);
        }
        else {
#ifdef LINALG_USE_EIGEN
            auto thisMap = getMap();
            Eigen::Array<real_g, N, 1> outVec = thisMap.cast<real_g>();
            Vector<real_g, N> result(outVec.data());
#else
            std::vector<real_g> result(m_array.begin(), m_array.end());
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
        //Eigen::Map<const Eigen::Matrix<D, N, 1>> thisMap = getMatrixMap();
        //Eigen::Map<const Eigen::Matrix<D, N, 1>> otherMap = other.getMatrixMap();
        //auto dot = thisMap.dot(otherMap);
        auto dot = std::inner_product(m_array.begin(), m_array.end(), other.m_array.begin(), D(0));
        return dot;

        // Slower
        //Eigen::Map<const Eigen::Array<D, N, 1>> thisMap2 = getMap();
        //Eigen::Map<const Eigen::Array<D, N, 1>> otherMap2 = other.getMap();
        //D res2 = (thisMap2.cwiseProduct(otherMap2)).rowwise().sum()(0);
        //Q_UNUSED(res2);
#else
		Vector<D, N> product = *this * other;
		D sum = std::accumulate(product.begin(), product.end(), 0.0);
		return sum;
#endif
	}

    /// @brief Perform a cross product with another vector
    inline Vector<D, N> cross(const Vector<D, N>& other) const {
#ifdef LINALG_USE_EIGEN
        Eigen::Map<const Eigen::Matrix<D, N, 1>> thisMap = getMatrixMap();
        Eigen::Map<const Eigen::Matrix<D, N, 1>> otherMap = other.getMatrixMap();
        Vector<D, N> result(thisMap.cross(otherMap).data());
        return result;
#else
        static_assert(N == 3, "This function requires a three-element vector");
        return Vector<D, N>(
            m_array.at(1) * other.at(2) - m_array.at(2) * other.at(1),
            m_array.at(2) * other.at(0) - m_array.at(0) * other.at(2),
            m_array.at(0) * other.at(1) - m_array.at(1) * other.at(0));
#endif
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
    inline bool hasNegative() { return std::any_of(m_array.begin(), m_array.end(), [](D entry) {return entry < 0; }); }

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

    /// @}

protected:
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @name Friends
    /// @{
    /// @brief Since the vector class is templated, it needs to be friends with all possible templates
    template<class D, size_t M> friend class Vector;

    friend class Matrix<D, N, N>;
    friend class ShaderProgram;

    /// @}

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @name Protected Methods
    /// @{

    explicit Vector(bool fill) {
        if (fill)
            std::fill(std::begin(m_array), std::end(m_array), D(0));
    }

    inline void initialize(D v0, D v1) {
        m_array[0] = v0;
        m_array[1] = v1;
    }

    inline void initialize(D v0, D v1, D v2) {
        initialize(v0, v1);
        if (N > 2) {
            m_array[2] = v2;
        }
    }

    inline void initialize(D v0, D v1, D v2, D v3) {
        initialize(v0, v1, v2);
        if (N > 3) {
            m_array[3] = v3;
        }
    }

    inline void initialize(const std::vector<D>& vec) {
        //std::move(vec.begin(), vec.begin() + std::min(vec.size(), N), m_array.begin());
        memcpy(m_array.data(), vec.data(), sizeof(D) * N);
    }

    /// @}

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @name Protected Members
    /// @{
    std::array<D, N> m_array;
    /// @}
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Typedefs based on generic vector
typedef Vector<int, 2> Vector2i;
typedef Vector<int, 3> Vector3i;
typedef Vector<int, 4> Vector4i;
typedef Vector<unsigned int, 2> Vector2u;
typedef Vector<unsigned int, 3> Vector3u;
typedef Vector<unsigned int, 4> Vector4u;
typedef Vector<float, 2> Vector2f;
typedef Vector<float, 3> Vector3f;
typedef Vector<float, 4> Vector4f;
typedef Vector<double, 2> Vector2d;
typedef Vector<double, 3> Vector3d;
typedef Vector<double, 4> Vector4d;
typedef Vector<real_g, 2> Vector2;
typedef Vector<real_g, 3> Vector3;
typedef Vector<real_g, 4> Vector4;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Globals
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Convert standard vector to and from JSON
template <typename T>
QJsonValue vectorAsJson(const std::vector<T>& vec) {
    QJsonArray array;
    std::copy(vec.begin(), vec.end(), std::back_inserter(array));
    return array;
}

template <typename T>
std::vector<T> vectorFromJson(const QJsonValue& json) {
    std::vector<float> vec;
    const QJsonArray& array = json.toArray();
    for (int i = 0; i < array.size(); i++) {
        Vec::EmplaceBack(vec, array.at(i).toDouble());
    }
    return vec;
}

template <typename T, size_t N>
QJsonValue vecOfVecAsJson(const std::vector<Vector<T, N>>& vec) {
    QJsonArray array;
    for (const auto& v : vec) {
        array.append(v.asJson());
    }
    return array;
}

template <typename T, size_t N>
std::vector<Vector<T, N>> vecOfVecFromJson(const QJsonValue& json) {
    std::vector<Vector<T, N>> vec;
    const QJsonArray& array = json.toArray();
    for (int i = 0; i < array.size(); i++) {
        Vector<T, N> v;
        v.loadFromJson(array.at(i));
        Vec::EmplaceBack(vec, std::move(v));
    }
    return vec;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Declare metatypes
Q_DECLARE_METATYPE(Gb::Vector2f)
Q_DECLARE_METATYPE(Gb::Vector3f)
Q_DECLARE_METATYPE(Gb::Vector4f)
Q_DECLARE_METATYPE(Gb::Vector2d)
Q_DECLARE_METATYPE(Gb::Vector3d)
Q_DECLARE_METATYPE(Gb::Vector4d)
Q_DECLARE_METATYPE(std::vector<float>)

#endif
