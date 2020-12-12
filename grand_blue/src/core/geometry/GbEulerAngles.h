#ifndef GB_EULER_ANGLES_H
#define GB_EULER_ANGLES_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// QT
#include <QString>
#include <array>

// Internal
#include "GbVector.h"

namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class D, size_t N> class SquareMatrix;
typedef SquareMatrix<float, 3> Matrix3x3;
typedef SquareMatrix<float, 4> Matrix4x4;
typedef SquareMatrix<double, 3> Matrix3x3d;
typedef SquareMatrix<double, 4> Matrix4x4d;
class Quaternion;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Enum for body axes (for roll, pitch, yaw rotations)
enum class Axis {
    kAxisX = 0,
    kAxisY,
    kAxisZ
};

// Enum for body axes (for roll, pitch, yaw rotations)
enum class RotationSpace {
    kBody = 0, // an intrinsic Euler rotation (about rotated body axes)
    kInertial // an extrinsic Euler rotation (about axes fixed in inertial space)
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** @class Euler Angles
    @brief A class representing a set of euler angles
    @note See: docs\Math\euler_angles
    // See: https://en.wikipedia.org/wiki/Davenport_chained_rotations#Conversion_between_intrinsic_and_extrinsic_rotations
*/
class EulerAngles {
public:
    typedef Vector<Axis, 3> Axes;

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief enum of all different possible rotation axis combinations
    /// @note Any extrinsic rotation is equivalent to an intrinsic rotation by the same angles but with inverted order of 
    /// elemental rotations, and vice versa. For instance, the intrinsic rotations x-y'-z'' by angles a, b, c are equivalent 
    /// to the extrinsic rotations z-y-x by angles c, b, a. Both are represented by a matrix R = X(a)Y(b)Z(c) for pre-multiplication
    enum EulerType {
        kInvalid,
        // Proper Euler Angles
        kZXZ=0,
        kXYX,
        kYZY,
        kZYZ,
        kXZX,
        kYXY,
        // Tait-Bryan Angles
        kXYZ,
        kXZY,
        kYXZ,
        kYZX,
        kZXY,
        kZYX
    };

    /// @brief Convert a list of axes in rotation order to an Euler rotation type
    static EulerType axesToType(const Axes& axisOrder, RotationSpace type);
    static Axes typeToAxes(const EulerType& type, RotationSpace space);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    EulerAngles();
    EulerAngles(const Quaternion& quaternion, EulerType rotationType = EulerAngles::kZYX, RotationSpace rotationSpace = RotationSpace::kBody);
    EulerAngles(float ax, float ay, float az, EulerType rotationType = EulerAngles::kZYX, RotationSpace rotationSpace = RotationSpace::kBody);
    EulerAngles(float ax, float ay, float az, const Axes& rotationOrder, RotationSpace rotationSpace = RotationSpace::kBody);
    EulerAngles(const Vector3f& angles, const Axes& rotationOrder, RotationSpace rotationSpace = RotationSpace::kBody);
    ~EulerAngles();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    const Vector3f& angles() const { return m_angles; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    friend bool operator==(const EulerAngles& q1, const EulerAngles& q2);
    friend bool operator!=(const EulerAngles& q1, const EulerAngles& q2);
    inline float& operator[](int idx) {
        return m_angles[idx];
    }
    inline const float& operator[](int idx) const {
        return m_angles[idx];
    }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Compute the rotation matrix corresponding to this set of Euler angles
    Matrix4x4 toRotationMatrixF() const;
    Matrix4x4d toRotationMatrix() const;

    /// @brief Convert to another Euler Angle type
    EulerAngles toAngles(EulerType order, RotationSpace space) const;

    /// @brief Get the Euler rotation type of these angles
    EulerType getType() const;

    Quaternion toQuaternion() const;

    /// @}

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Compute the extrinsic (inertial) rotation matrix corresponding to the given set of Euler angles
    static Matrix4x4d computeExtrinsicRotationMatrix(const Vector3f & angles, const Axes& axisOrder);

    /// @brief Compute the intrinsic (body) rotation matrix corresponding to the given set of Euler angles
    static Matrix4x4d computeIntrinsicRotationMatrix(const Vector3f & angles, const Axes& axisOrder);

    /// @brief Obtain a set of euler angles (in radians) of the given type from the given rotation matrix
    static EulerAngles toAngles(const Matrix3x3d& m, EulerType type, RotationSpace space);

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Rotation space, either extrinsic or intrinsic
    /// @details Extrinsic rotations are performed with respect to an inertially fixed trio of axes,
    /// whereas intrinsic rotations are performed with respect to the rotated axes of a body
    RotationSpace m_rotationSpace;

    /// @brief Axes order corresponding to the Euler rotation angles
    /// @example An axis order of {kAxisZ, kAxisY, kAxisX} would represent a 1-2-3 (X-Y-Z) Euler rotation
    Axes m_rotationOrder;

    /// @brief the Euler Angles (stored in radians)
    Vector3f m_angles;

    static double clamp(double val, double min = -1.0, double max = 1.0);

    /// @}

};
Q_DECLARE_METATYPE(EulerAngles)


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}

#endif
