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
#include "../mixins/GbLoadable.h"

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

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////////////////////
template<class D, size_t N> class Vector;
typedef Vector<real_g, 3> Vector3g;
typedef Vector<real_g, 4> Vector4g;

template<class D, size_t N> class SquareMatrix;
typedef SquareMatrix<real_g, 2> Matrix2x2g;
typedef SquareMatrix<real_g, 3> Matrix3x3g;
typedef SquareMatrix<real_g, 4> Matrix4x4g;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** @class Quaternion
    @brief A class representing a quaternion rotation
    @notes See http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
*/

class Quaternion: public Serializable {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @brief Get quaternion from an axis angle pair
    /// @details Angle is given in radians
    static Quaternion fromAxisAngle(const Vector3g& axis, real_g angle);
    static Quaternion fromAxisAngle(real_g x, real_g y, real_g z, real_g angleDeg);

    /// @brief Obtain a quaternion from a set of Euler Angles
    /// @details Creates a quaternion that corresponds to a rotation of eulerAngles.
    /// eulerAngles.z() radians around the z axis, eulerAngles.x() radians around the x axis, 
    /// and eulerAngles.y() radians around the y axis (in that order).
    static Quaternion fromEulerAngles(const Vector3g& eulerAngles);

    /// @brief Obtain a quaternion from a set of Euler Angles
    /// @details Qt documentation states that this 
    /// creates a quaternion that corresponds to a rotation of roll radians 
    /// around the z axis, pitch radians around the x axis, and yaw radians around the y axis 
    /// (in that order), but this appears to be reversed
    /// @note See: docs\Math\euler_angles for body 3-1-2 sequence
    static Quaternion fromEulerAngles(real_g pitch, real_g yaw, real_g roll);

    /// @brief Obtain a quaternion from a rotation matrix
    static Quaternion fromRotationMatrix(const Matrix3x3g& rot3x3);
    static Quaternion fromRotationMatrix(const Matrix4x4g& rot4x4);

    /// @brief Rotates a point by the given angle (radians) around a vector originating from an origin
    static Vector3g rotatePoint(const Vector3g& point, const Vector3g& origin, const Vector3g& vector, real_g angle);

    /// @brief Obtain a quaternion from principle axes
    /// @note Axes are assumed to be orthonormal
    static Quaternion fromAxes(const Vector3g &xAxis, const Vector3g &yAxis, const Vector3g &zAxis);

    /// @brief Obtain a quaternion from a direction and up vector
    /// @details This is essentially a "lookAt" rotation
    static Quaternion fromDirection(const Vector3g &direction, const Vector3g &up);

    /// @brief Convert from an "angular velocity"
    /// See: https://forum.unity.com/threads/average-quaternions.86898/
    static Quaternion fromAngularVelocity(const Vector3g& vel);

    /// @brief Rotate from the "from" vector to the "to" vector
    /// @details Returns the shortest arc quaternion to rotate from the direction described
    /// by the vector "from" to the direction described by the vector "to".
    static Quaternion rotationTo(const Vector3g &from, const Vector3g& to);

    /// @brief Rotates vector with this quaternion to produce a new vector in 3D space
    Vector3g rotatedVector(const Vector3g& vector) const;

    /// @brief Find the weighted average of multiple quaternions
    /// See: https://forum.unity.com/threads/average-quaternions.86898/
    static Quaternion slerp(const std::vector<Quaternion>& quaternions, const std::vector<float>& weights);
    
    /// @brief Iterative averaging, much cheaper (faster) alternative to slerp
    static Quaternion average(const std::vector<Quaternion>& quaternions, const std::vector<float>& weights);
    static Quaternion average(const std::vector<Quaternion>& quaternions, const std::vector<float>& weights, size_t numIterations);
    static Quaternion average(const Quaternion& reference, const std::vector<Quaternion>& quaternions, const std::vector<float>& weights);

    /// @brief Linearly interpolates between two quaternions, normalized lerp
    /// @details Interpolates along the shortest linear path between the rotational positions 
    /// q1 and q2. The value t should be between 0 and 1, indicating the distance to travel 
    /// between q1 and q2. The result will be normalized().
    static Quaternion nlerp(const Quaternion &q1, const Quaternion &q2, real_g t);

    ///@brief Spherically interpolates between two quaternions
    /// @details Interpolates along the shortest spherical path between the rotational positions 
    /// q1 and q2. The value t should be between 0 and 1, indicating the distance to travel 
    /// between q1 and q2. 
    static Quaternion slerp(const Quaternion &q1, const Quaternion &q2, real_g t);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{

    /// @brief Constructs identity quaternion
    Quaternion();

    /// @brief Constructs a quaternion
    Quaternion(const QJsonValue& json);
    Quaternion(real_g x, real_g y, real_g z, real_g w);
    Quaternion(const Vector4g& vec);
    Quaternion(real_g pitch, real_g yaw, real_g roll);
    Quaternion(const Vector3g& rotationAxis, real_g rotationAngleRad);

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
    friend const Vector3g operator*(const Quaternion& q, const Vector3g& vec);

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
    void setVector(const Vector3g& vec);
    void setVector(real_g aX, real_g aY, real_g aZ);

    /// @brief return the xys components of the quaternion
    Vector3g vector() const;

    /// @brief Return the pointer to the first data entry of the quaternion
    const real_g* getData() const { return &m_x; }

    /// @brief Get rotation axis and angle about that axis corresponding to this quaternion
    /// @details Returned angle is in radians
    void getAxisAndAngle(Vector3g& axis, real_g& angle) const;
    void getAxisAndAngle(real_g& x, real_g& y, real_g& z, real_g& angle) const;

    /// @brief Obtain a set of Euler Angles corresponding to this quaternion
    Vector3g toEulerAngles() const;

    /// @brief Convert to an "angular velocity"
    /// See: https://forum.unity.com/threads/average-quaternions.86898/
    void toAngularVelocity(Vector3g& vel) const;

    /// @brief Obtain euler angles corresponding to a quaternion
    /// @details Calculates roll, pitch, yaw Euler angles (in radians) corresponding to this Quaternion
    void getEulerAngles(real_g *pitch, real_g *yaw, real_g *roll) const;

    /// @brief Convert the quaternion to a vector
    Vector4g toVector4() const;

    /// @brief Convert the quaternion to a rotation matrix
    Matrix3x3g toRotationMatrix() const;
    Matrix4x4g toRotationMatrix4x4() const;

    /// @brief Returns the orthonormal axes xAxis, yAxis, zAxis defining this quaternion
    void getAxes(Vector3g *xAxis, Vector3g *yAxis, Vector3g *zAxis) const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

protected:
    real_g m_x, m_y, m_z, m_w;
};
Q_DECLARE_METATYPE(Quaternion)
Q_DECLARE_METATYPE(Quaternion*)


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}

#endif
