#ifndef GB_QUATERNION_H
#define GB_QUATERNION_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// QT
#include <array>

// Internal
#include <fortress/types/GSizedTypes.h>
#include <fortress/json/GJson.h>

/////////////////////////////////////////////////////////////////////////////////////////////
// Macros
/////////////////////////////////////////////////////////////////////////////////////////////
namespace physx {
class PxQuat;
}

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////////////////////
template<class D, size_t N> class Vector;
typedef Vector<Real_t, 3> Vector3;
typedef Vector<Real_t, 4> Vector4;

class EulerAngles;
template<class D, size_t R, size_t C> class Matrix;
typedef Matrix<Real_t, 2, 2> Matrix2x2g;
typedef Matrix<Real_t, 3, 3> Matrix3x3g;
typedef Matrix<Real_t, 4, 4> Matrix4x4g;
typedef Matrix<float, 2, 2> Matrix2x2;
typedef Matrix<float, 3, 3> Matrix3x3;
typedef Matrix<float, 4, 4> Matrix4x4;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** @class Quaternion
    @brief A class representing a quaternion rotation
    @notes See http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
*/

class Quaternion {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @brief Get quaternion from an axis angle pair
    /// @details Angle is given in radians
    static Quaternion fromAxisAngle(const Vector3& axis, Real_t angle);
    static Quaternion fromAxisAngle(Real_t x, Real_t y, Real_t z, Real_t angleDeg);

    /// @brief Obtain a quaternion from a set of Euler Angles
    /// @details Creates a quaternion that corresponds to a rotation of eulerAngles.
    /// eulerAngles.z() radians around the z axis, eulerAngles.x() radians around the x axis, 
    /// and eulerAngles.y() radians around the y axis (in that order).
    static Quaternion fromEulerAngles(const EulerAngles& eulerAngles);

    /// @brief Obtain a quaternion from a rotation matrix
    template<typename T, size_t N>
    static Quaternion fromRotationMatrix(const Matrix<T, N, N>& rot) {
        static_assert(N >= 3, "Error, matrix not of sufficient size");
        // Algorithm from:
        // http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q55
        Real_t scalar;
        Real_t axis[3];
        const Real_t trace = rot(0, 0) + rot(1, 1) + rot(2, 2);
        if (trace > 0.00000001f) {
            const Real_t s = 2.0f * std::sqrt(trace + 1.0f);
            scalar = 0.25f * s;
            axis[0] = (rot(2, 1) - rot(1, 2)) / s;
            axis[1] = (rot(0, 2) - rot(2, 0)) / s;
            axis[2] = (rot(1, 0) - rot(0, 1)) / s;
        }
        else {
            static int s_next[3] = { 1, 2, 0 };
            int i = 0;
            if (rot(1, 1) > rot(0, 0))
                i = 1;
            if (rot(2, 2) > rot(i, i))
                i = 2;
            int j = s_next[i];
            int k = s_next[j];
            const Real_t s = 2.0f * std::sqrt(rot(i, i) - rot(j, j) - rot(k, k) + 1.0f);
            axis[i] = 0.25f * s;
            scalar = (rot(k, j) - rot(j, k)) / s;
            axis[j] = (rot(j, i) + rot(i, j)) / s;
            axis[k] = (rot(k, i) + rot(i, k)) / s;
        }
        return Quaternion(axis[0], axis[1], axis[2], scalar);
    }

    /// @brief Rotates a point by the given angle (radians) around a vector originating from an origin
    static Vector3 rotatePoint(const Vector3& point, const Vector3& origin, const Vector3& vector, Real_t angle);

    /// @brief Obtain a quaternion from principle axes
    /// @note Axes are assumed to be orthonormal
    static Quaternion fromAxes(const Vector3 &xAxis, const Vector3 &yAxis, const Vector3 &zAxis);

    /// @brief Obtain a quaternion from a direction and up vector
    /// @details This is essentially a "lookAt" rotation
    static Quaternion fromDirection(const Vector3 &direction, const Vector3 &up);

    /// @brief Convert from an "angular velocity"
    /// See: https://forum.unity.com/threads/average-quaternions.86898/
    static Quaternion fromAngularVelocity(const Vector3& vel);

    /// @brief Rotate from the "from" vector to the "to" vector
    /// @details Returns the shortest arc quaternion to rotate from the direction described
    /// by the vector "from" to the direction described by the vector "to".
    static Quaternion rotationTo(const Vector3 &from, const Vector3& to);

    /// @brief Rotates vector with this quaternion to produce a new vector in 3D space
    Vector3 rotatedVector(const Vector3& vector) const;

    /// @brief Chain a slerp between quaternions
    static Quaternion Slerp(const Quaternion* quaternions, const std::vector<float>& weights);

    /// @brief Find the weighted average of multiple quaternions
    /// See: https://forum.unity.com/threads/average-quaternions.86898/
    static Quaternion EigenAverage(const Quaternion* quaternions, const std::vector<float>& weights);

    /// @brief Iterative averaging, much cheaper (faster) alternative to slerp
    // FIXME: None of these work with more than two quaternions
    static Quaternion Average(const std::vector<Quaternion>& quaternions, const std::vector<float>& weights);
    static Quaternion Average(const Quaternion* quaternions, const std::vector<float>& weights);
    static Quaternion IterativeAverage(const Quaternion* quaternions, const std::vector<float>& weights, size_t numIterations);

    /// @brief Linearly interpolates between two quaternions, normalized lerp
    /// @details Interpolates along the shortest linear path between the rotational positions 
    /// q1 and q2. The value t should be between 0 and 1, indicating the distance to travel 
    /// between q1 and q2. The result will be normalized().
    static Quaternion Nlerp(const Quaternion &q1, const Quaternion &q2, Real_t t);

    ///@brief Spherically interpolates between two quaternions
    /// @details Interpolates along the shortest spherical path between the rotational positions 
    /// q1 and q2. The value t should be between 0 and 1, indicating the distance to travel 
    /// between q1 and q2. 
    static Quaternion Slerp(const Quaternion &q1, const Quaternion &q2, Real_t t);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{

    /// @brief Constructs identity quaternion
    Quaternion();

    /// @brief Constructs a quaternion
    Quaternion(const nlohmann::json& json);
    Quaternion(Real_t x, Real_t y, Real_t z, Real_t w);
    Quaternion(const Vector3& vec);
    Quaternion(const Vector4& vec);

    ~Quaternion();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    double x() const { return m_x; }
    double y() const { return m_y; }
    double z() const { return m_z; }
    double w() const { return m_w; }

    void setX(double x) { m_x = x; }
    void setY(double y) { m_y = y; }
    void setZ(double z) { m_z = z; }
    void setW(double w) { m_w = w; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    friend bool operator==(const Quaternion& q1, const Quaternion& q2);
    friend bool operator!=(const Quaternion& q1, const Quaternion& q2);
    Quaternion&  operator+=(const Quaternion &quaternion);
    Quaternion& operator-=(const Quaternion &quaternion);
    Quaternion& operator*=(Real_t factor);
    friend const Quaternion operator*(const Quaternion& q1, const Quaternion& q2);
    Quaternion& operator*=(const Quaternion &quaternion);
    Quaternion& operator/=(Real_t divisor);
    friend const Quaternion operator+(const Quaternion& q1, const Quaternion &q2);
    friend const Quaternion operator-(const Quaternion& q1, const Quaternion &q2);
    friend const Quaternion operator*(Real_t factor, const Quaternion &quaternion);
    friend const Quaternion operator*(const Quaternion &q, Real_t factor);
    friend const Quaternion operator-(const Quaternion &q);
    friend const Quaternion operator/(const Quaternion& q1, Real_t divisor);
    friend const Vector3 operator*(const Quaternion& q, const Vector3& vec);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Return the length (norm) of the quaternion
    Real_t length() const;

    /// @brief Return the length (norm) of the quaternion squared
    Real_t lengthSquared() const;

    /// @brief Return the normalized unit form of the quaternion
    Quaternion normalized() const;

    /// @brief Normalizes this quaternion
    void normalize();

    /// @brief Determines whether quaternion is null
    bool isNull() const;

    /// @brief Determines whether quaternion is identity
    bool isIdentity() const;

    /// @brief Dot product
    /// @details Gives the angle between the quaternions
    Real_t dot(const Quaternion& other) const;

    /// @brief Returns the inverse of this quaternion
    Quaternion inverted() const;

    /// @brief Returns the conjugate of this quaternion
    Quaternion conjugated() const;

    /// @brief Set the xyz components of the quaternion
    void setVector(const Vector3& vec);
    void setVector(Real_t aX, Real_t aY, Real_t aZ);

    /// @brief return the xys components of the quaternion
    Vector3 vector() const;

    /// @brief Return the pointer to the first data entry of the quaternion
    const Real_t* getData() const { return &m_x; }

    /// @brief Get rotation axis and angle about that axis corresponding to this quaternion
    /// @details Returned angle is in radians
    void getAxisAndAngle(Vector3& axis, Real_t& angle) const;
    void getAxisAndAngle(Real_t& x, Real_t& y, Real_t& z, Real_t& angle) const;

    /// @brief Obtain a set of Euler Angles corresponding to this quaternion
    //Vector3g toEulerAngles() const;

    /// @brief Convert to an "angular velocity"
    /// See: https://forum.unity.com/threads/average-quaternions.86898/
    void toAngularVelocity(Vector3& vel) const;

    /// @brief Obtain euler angles corresponding to a quaternion
    /// @details Calculates roll, pitch, yaw Euler angles (in radians) corresponding to this Quaternion
    //void getEulerAngles(Real_t *pitch, Real_t *yaw, Real_t *roll) const;

    /// @brief Convert the quaternion to a vector
    Vector4 toVector4() const;

    /// @brief Convert the quaternion to a rotation matrix
    Matrix3x3g toRotationMatrix() const;
    Matrix4x4g toRotationMatrix4x4() const;
    void toRotationMatrix(Matrix4x4g& outMatrix) const;

    /// @brief Returns the orthonormal axes xAxis, yAxis, zAxis defining this quaternion
    void getAxes(Vector3 *xAxis, Vector3 *yAxis, Vector3 *zAxis) const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const Quaternion& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, Quaternion& orObject);


    /// @}

protected:
    Real_t m_x;
    Real_t m_y;
    Real_t m_z;
    Real_t m_w;

    static Quaternion AverageIteration(const Quaternion& reference, const Quaternion* quaternions, const std::vector<float>& weights);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}

#endif
