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
typedef SquareMatrix<float, 3> Matrix3x3f;
typedef SquareMatrix<float, 4> Matrix4x4f;

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
enum class RotationType {
    kBody = 0, // an intrinsic Euler rotation (about rotated body axes)
    kSpace // an extrinsic Euler rotation (about axes fixed in inertial space)
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** @class Euler Angles
    @brief A class representing a set of euler angles
    @note See: docs\Math\euler_angles
*/
class EulerAngles {
public:
    typedef Vector<Axis, 3> Axes;

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    EulerAngles();
    EulerAngles(float ax, float ay, float az, const Axes& rotationOrder, RotationType rotationType = RotationType::kBody);
    EulerAngles(const Vector3f& angles, const Axes& rotationOrder, RotationType rotationType = RotationType::kBody);
    ~EulerAngles();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Compute the rotation matrix corresponding to this set of Euler angles
    Matrix4x4f toRotationMatrix() const;

    /// @}

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Compute the extrinsic (inertial) rotation matrix corresponding to the given set of Euler angles
    static Matrix4x4f computeExtrinsicRotationMatrix(const Vector3f & angles, const Axes& axisOrder);

    /// @brief Compute the intrinsic (body) rotation matrix corresponding to the given set of Euler angles
    static Matrix4x4f computeIntrinsicRotationMatrix(const Vector3f & angles, const Axes& axisOrder);

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Rotation type, either extrinsic or intrinsic
    /// @details Extrinsic rotations are performed with respect to an inertially fixed trio of axes,
    /// whereas intrinsic rotations are performed with respect to the rotated axes of a body
    RotationType m_rotationType;

    /// @brief Axes order corresponding to the Euler rotation angles
    /// @example An axis order of {kAxisZ, kAxisY, kAxisX} would represent a 1-2-3 Euler rotation
    Axes m_rotationOrder;

    /// @brief the Euler Angles (stored in radians)
    Vector3f m_angles;

    /// @}

};
Q_DECLARE_METATYPE(EulerAngles)


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}

#endif
