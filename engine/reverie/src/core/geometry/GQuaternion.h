#ifndef GB_QUATERNION_H
#define GB_QUATERNION_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// QT
#include <QString>
#include <array>
#include <QMetaType>

// Internal
#include "../mixins/GLoadable.h"

/////////////////////////////////////////////////////////////////////////////////////////////
// Macros
/////////////////////////////////////////////////////////////////////////////////////////////
#ifdef GB_USE_DOUBLE
typedef double real_g;
#else
typedef float real_g;
#endif

namespace physx {
class PxQuat;
}

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////////////////////
template<class D, size_t N> class Vector;
typedef Vector<real_g, 3> Vector3;
typedef Vector<real_g, 4> Vector4;

class EulerAngles;
template<class D, size_t N> class SquareMatrix;
typedef SquareMatrix<real_g, 2> Matrix2x2g;
typedef SquareMatrix<real_g, 3> Matrix3x3g;
typedef SquareMatrix<real_g, 4> Matrix4x4g;
typedef SquareMatrix<float, 2> Matrix2x2;
typedef SquareMatrix<float, 3> Matrix3x3;
typedef SquareMatrix<float, 4> Matrix4x4;

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
    static Quaternion fromAxisAngle(const Vector3& axis, real_g angle);
    static Quaternion fromAxisAngle(real_g x, real_g y, real_g z, real_g angleDeg);

    /// @brief Obtain a quaternion from a set of Euler Angles
    /// @details Creates a quaternion that corresponds to a rotation of eulerAngles.
    /// eulerAngles.z() radians around the z axis, eulerAngles.x() radians around the x axis, 
    /// and eulerAngles.y() radians around the y axis (in that order).
    static Quaternion fromEulerAngles(const EulerAngles& eulerAngles);

    /// @brief Obtain a quaternion from a rotation matrix
    template<typename T, size_t N>
    static Quaternion fromRotationMatrix(const SquareMatrix<T, N>& rot) {
        static_assert(N >= 3, "Error, matrix not of sufficient size");
        // Algorithm from:
        // http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q55
        real_g scalar;
        real_g axis[3];
        const real_g trace = rot(0, 0) + rot(1, 1) + rot(2, 2);
        if (trace > 0.00000001f) {
            const real_g s = 2.0f * std::sqrt(trace + 1.0f);
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
            const real_g s = 2.0f * std::sqrt(rot(i, i) - rot(j, j) - rot(k, k) + 1.0f);
            axis[i] = 0.25f * s;
            scalar = (rot(k, j) - rot(j, k)) / s;
            axis[j] = (rot(j, i) + rot(i, j)) / s;
            axis[k] = (rot(k, i) + rot(i, k)) / s;
        }
        return Quaternion(axis[0], axis[1], axis[2], scalar);
    }

    /// @brief Rotates a point by the given angle (radians) around a vector originating from an origin
    static Vector3 rotatePoint(const Vector3& point, const Vector3& origin, const Vector3& vector, real_g angle);

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
    static Quaternion Nlerp(const Quaternion &q1, const Quaternion &q2, real_g t);

    ///@brief Spherically interpolates between two quaternions
    /// @details Interpolates along the shortest spherical path between the rotational positions 
    /// q1 and q2. The value t should be between 0 and 1, indicating the distance to travel 
    /// between q1 and q2. 
    static Quaternion Slerp(const Quaternion &q1, const Quaternion &q2, real_g t);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{

    /// @brief Constructs identity quaternion
    Quaternion();

    /// @brief Constructs a quaternion
    Quaternion(const QJsonValue& json);
    Quaternion(real_g x, real_g y, real_g z, real_g w);
    Quaternion(const Vector3& vec);
    Quaternion(const Vector4& vec);
    //Quaternion(real_g pitch, real_g yaw, real_g roll);
    //Quaternion(const Vector3& rotationAxis, real_g rotationAngleRad);

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
    Quaternion& operator*=(real_g factor);
    friend const Quaternion operator*(const Quaternion& q1, const Quaternion& q2);
    Quaternion& operator*=(const Quaternion &quaternion);
    Quaternion& operator/=(real_g divisor);
    friend const Quaternion operator+(const Quaternion& q1, const Quaternion &q2);
    friend const Quaternion operator-(const Quaternion& q1, const Quaternion &q2);
    friend const Quaternion operator*(real_g factor, const Quaternion &quaternion);
    friend const Quaternion operator*(const Quaternion &q, real_g factor);
    friend const Quaternion operator-(const Quaternion &q);
    friend const Quaternion operator/(const Quaternion& q1, real_g divisor);
    friend const Vector3 operator*(const Quaternion& q, const Vector3& vec);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Return as a physX quaternion
    physx::PxQuat asPhysX() const;

    /// @brief Return the length (norm) of the quaternion
    real_g length() const;

    /// @brief Return the length (norm) of the quaternion squared
    real_g lengthSquared() const;

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
    real_g dot(const Quaternion& other) const;

    /// @brief Returns the inverse of this quaternion
    Quaternion inverted() const;

    /// @brief Returns the conjugate of this quaternion
    Quaternion conjugated() const;

    /// @brief Set the xyz components of the quaternion
    void setVector(const Vector3& vec);
    void setVector(real_g aX, real_g aY, real_g aZ);

    /// @brief return the xys components of the quaternion
    Vector3 vector() const;

    /// @brief Return the pointer to the first data entry of the quaternion
    const real_g* getData() const { return &m_x; }

    /// @brief Get rotation axis and angle about that axis corresponding to this quaternion
    /// @details Returned angle is in radians
    void getAxisAndAngle(Vector3& axis, real_g& angle) const;
    void getAxisAndAngle(real_g& x, real_g& y, real_g& z, real_g& angle) const;

    /// @brief Obtain a set of Euler Angles corresponding to this quaternion
    //Vector3g toEulerAngles() const;

    /// @brief Convert to an "angular velocity"
    /// See: https://forum.unity.com/threads/average-quaternions.86898/
    void toAngularVelocity(Vector3& vel) const;

    /// @brief Obtain euler angles corresponding to a quaternion
    /// @details Calculates roll, pitch, yaw Euler angles (in radians) corresponding to this Quaternion
    //void getEulerAngles(real_g *pitch, real_g *yaw, real_g *roll) const;

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
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const ;

    /// @brief Populates this data using a valid json string
    void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty());

    /// @}

protected:
    real_g m_x, m_y, m_z, m_w;

    static Quaternion AverageIteration(const Quaternion& reference, const Quaternion* quaternions, const std::vector<float>& weights);
};
Q_DECLARE_METATYPE(Quaternion)
Q_DECLARE_METATYPE(Quaternion*)


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}

#endif
