#include "GbQuaternion.h"
#include "../GbConstants.h"

#include "GbMatrix.h"
#include "../physics/GbPhysics.h"
#include "GbEulerAngles.h"

namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion::Quaternion():
    m_x(0),
    m_y(0),
    m_z(0),
    m_w(1)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion::Quaternion(const QJsonValue & json)
{
    loadFromJson(json);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion::Quaternion(real_g x, real_g y, real_g z, real_g w):
    m_x(x),
    m_y(y),
    m_z(z),
    m_w(w)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion::Quaternion(const Vector3 & vec):
    m_x(vec[0]),
    m_y(vec[1]),
    m_z(vec[2]),
    m_w(0)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion::Quaternion(const Vector4 & vec) :
    m_x(vec[0]),
    m_y(vec[1]),
    m_z(vec[2]),
    m_w(vec[3])
{
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Quaternion::Quaternion(real_g pitch, real_g yaw, real_g roll)
//{
//    *this = std::move(fromEulerAngles(pitch, yaw, roll));
//}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Was being misused for a direct vector initialization, instead of assuming the input was an axis
//Quaternion::Quaternion(const Vector3 & rotationAxis, real_g rotationAngleRad):
//    m_x(rotationAxis.x() * sin(rotationAngleRad / 2.0)),
//    m_y(rotationAxis.y() * sin(rotationAngleRad / 2.0)),
//    m_z(rotationAxis.z() * sin(rotationAngleRad / 2.0)),
//    m_w(cos(rotationAngleRad / 2.0))
//{
//}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion::~Quaternion()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool operator==(const Quaternion& q1, const Quaternion& q2)
{
    return q1.m_w == q2.m_w && q1.m_x == q2.m_x && q1.m_y == q2.m_y && q1.m_z == q2.m_z;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool operator!=(const Quaternion& q1, const Quaternion& q2)
{
    return !operator==(q1, q2);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion & Quaternion::operator+=(const Quaternion & quaternion)
{
    m_w += quaternion.m_w;
    m_x += quaternion.m_x;
    m_y += quaternion.m_y;
    m_z += quaternion.m_z;
    return *this;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion & Quaternion::operator-=(const Quaternion & quaternion)
{
    m_w -= quaternion.m_w;
    m_x -= quaternion.m_x;
    m_y -= quaternion.m_y;
    m_z -= quaternion.m_z;
    return *this;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion & Quaternion::operator*=(real_g factor)
{
    m_w *= factor;
    m_x *= factor;
    m_y *= factor;
    m_z *= factor;
    return *this;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const Quaternion operator*(const Quaternion & q1, const Quaternion & q2)
{
    real_g yy = (q1.m_w - q1.m_y) * (q2.m_w + q2.m_z);
    real_g zz = (q1.m_w + q1.m_y) * (q2.m_w - q2.m_z);
    real_g ww = (q1.m_z + q1.m_x) * (q2.m_x + q2.m_y);
    real_g xx = ww + yy + zz;
    real_g qq = 0.5f * (xx + (q1.m_z - q1.m_x) * (q2.m_x - q2.m_y));
    real_g w = qq - ww + (q1.m_z - q1.m_y) * (q2.m_y - q2.m_z);
    real_g x = qq - xx + (q1.m_x + q1.m_w) * (q2.m_x + q2.m_w);
    real_g y = qq - yy + (q1.m_w - q1.m_x) * (q2.m_y + q2.m_z);
    real_g z = qq - zz + (q1.m_z + q1.m_y) * (q2.m_w - q2.m_x);

    // Unoptimized
    //real_g x = q1.m_x * q2.m_w + q1.m_y * q2.m_z - q1.m_z * q2.m_y + q1.m_w * q2.m_x;
    //real_g y = -q1.m_x * q2.m_z + q1.m_y * q2.m_w + q1.m_z * q2.m_x + q1.m_w * q2.m_y;
    //real_g z = q1.m_x * q2.m_y - q1.m_y * q2.m_x + q1.m_z * q2.m_w + q1.m_w * q2.m_z;
    //real_g w = -q1.m_x * q2.m_x - q1.m_y * q2.m_y - q1.m_z * q2.m_z + q1.m_w * q2.m_w;
    return Quaternion(x, y, z, w);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion & Quaternion::operator*=(const Quaternion & quaternion)
{
    *this = *this * quaternion;
    return *this;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion & Quaternion::operator/=(real_g divisor)
{
    m_w /= divisor;
    m_x /= divisor;
    m_y /= divisor;
    m_z /= divisor;
    return *this;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const Quaternion operator+(const Quaternion& q1, const Quaternion & q2)
{
    return Quaternion(q1.m_x + q2.m_x, q1.m_y + q2.m_y, q1.m_z + q2.m_z, q1.m_w + q2.m_w);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const Quaternion operator-(const Quaternion& q1, const Quaternion & q2)
{
    return Quaternion(q1.m_x - q2.m_x, q1.m_y - q2.m_y, q1.m_z - q2.m_z, q1.m_w - q2.m_w);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const Quaternion operator*(real_g factor, const Quaternion & quaternion)
{
    return Quaternion(quaternion.m_x * factor, quaternion.m_y * factor, quaternion.m_z * factor, quaternion.m_w * factor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const Vector3 operator*(const Quaternion & q, const Vector3 & vec)
{
    return q.rotatedVector(vec);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const Quaternion operator*(const Quaternion & q, real_g factor){
    return Quaternion(q.m_x * factor, q.m_y * factor, q.m_z * factor, q.m_w * factor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const Quaternion operator-(const Quaternion& q)
{
    return Quaternion(-q.m_x, -q.m_y, -q.m_z, -q.m_w);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const Quaternion operator/(const Quaternion &quaternion, real_g divisor)
{
    return Quaternion(quaternion.m_x / divisor, quaternion.m_y / divisor, quaternion.m_z / divisor, quaternion.m_w / divisor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
physx::PxQuat Quaternion::asPhysX() const
{
    return physx::PxQuat(m_x, m_y, m_z, m_w);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
real_g Quaternion::length() const
{
    return sqrt(m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
real_g Quaternion::lengthSquared() const
{
    return m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion Quaternion::normalized() const
{
    // Need some extra precision if the length is very small.
    double len = double(m_x) * double(m_x) +
        double(m_y) * double(m_y) +
        double(m_z) * double(m_z) +
        double(m_w) * double(m_w);
    if (qFuzzyIsNull(len - 1.0f))
        return *this;
    else if (!qFuzzyIsNull(len))
        return *this / sqrt(len);
    else
        return Quaternion(0.0f, 0.0f, 0.0f, 0.0f);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Quaternion::normalize()
{
    // Need some extra precision if the length is very small.
    double len = double(m_x) * double(m_x) +
        double(m_y) * double(m_y) +
        double(m_z) * double(m_z) +
        double(m_w) * double(m_w);

    if (qFuzzyIsNull(len - 1.0f) || qFuzzyIsNull(len)) {
        // Return if null length or if already normalized
        return;
    }

    double k = 1.0 / sqrt(len);
    m_x *= k;
    m_y *= k;
    m_z *= k;
    m_w *= k;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Quaternion::isNull() const
{
    return m_w == 0.0 && m_x == 0.0 && m_y == 0.0 && m_z == 0.0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Quaternion::isIdentity() const
{
    return m_w == 1.0 && m_x == 0.0 && m_y == 0.0 && m_z == 0.0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
real_g Quaternion::dot(const Quaternion & other) const
{
    return m_w * other.m_w + m_x * other.m_x + m_y * other.m_y + m_z * other.m_z;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion Quaternion::inverted() const
{
    double len = double(m_w) * double(m_w) +
        double(m_x) * double(m_x) +
        double(m_y) * double(m_y) +
        double(m_z) * double(m_z);
    if (!qFuzzyIsNull(len)) {
        return Quaternion(real_g(double(-m_x) / len), real_g(double(-m_y) / len), real_g(double(-m_z) / len), real_g(double(m_w) / len));
    }
    return Quaternion(0.0f, 0.0f, 0.0f, 0.0f);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion Quaternion::conjugated() const
{
    return Quaternion(-m_x, -m_y, -m_z, m_w);
}

void Quaternion::setVector(const Vector3 & vec)
{
    m_x = vec.x();
    m_y = vec.y();
    m_z = vec.z();
}

void Quaternion::setVector(real_g aX, real_g aY, real_g aZ)
{
    m_x = aX;
    m_y = aY;
    m_z = aZ;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector3 Quaternion::vector() const
{
    return Vector3(m_x, m_y, m_z);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Quaternion::getAxisAndAngle(Vector3& axis, real_g& angleDeg) const
{
    getAxisAndAngle(axis[0], axis[1], axis[2], angleDeg);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Quaternion::getAxisAndAngle(real_g& x, real_g& y, real_g& z, real_g& angle) const
{
    // The quaternion representing the rotation is
    //   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)
    real_g length = m_x * m_x + m_y * m_y + m_z * m_z;
    if (!qFuzzyIsNull(length)) {
        x = m_x;
        y = m_y;
        z = m_z;
        if (!qFuzzyIsNull(length - 1.0f)) {
            length = std::sqrt(length);
            x /= length;
            y /= length;
            z /= length;
        }
        angle = 2.0f * std::acos(m_w);
    }
    else {
        // angle is 0 (mod 2*pi), so any axis will fit
        x = y = z = angle = 0.0f;
    }

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion Quaternion::fromAxisAngle(const Vector3 & axis, real_g angle)
{
    return fromAxisAngle(axis.x(), axis.y(), axis.z(), angle);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion Quaternion::fromAxisAngle(real_g x, real_g y, real_g z, real_g angle)
{
    real_g length = std::sqrt(x * x + y * y + z * z);
    if (!qFuzzyIsNull(length - 1.0f) && !qFuzzyIsNull(length)) {
        x /= length;
        y /= length;
        z /= length;
    }
    real_g a = angle / 2.0f;
    real_g s = std::sin(a);
    real_g c = std::cos(a);
    return Quaternion(x * s, y * s, z * s, c).normalized();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Quaternion::toAngularVelocity(Vector3& vel) const
{
    if (abs(m_w) > 1023.5f / 1024.0f)
        vel = Vector3();
    real_g angle = acos(abs(m_w));
    real_g sign_w = m_w / abs(m_w);
    real_g gain = sign_w *2.0f * angle / sin(angle);
    vel[0] = m_x * gain;
    vel[1] = m_y * gain;
    vel[2] = m_z * gain;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion Quaternion::fromEulerAngles(const EulerAngles & eulerAngles)
{
    return Quaternion::fromRotationMatrix(eulerAngles.toRotationMatrix());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector3 Quaternion::rotatePoint(const Vector3 & point, const Vector3 & origin, const Vector3 & vector, real_g angle)
{
    real_g vx = vector.x();
    real_g vy = vector.y();
    real_g vz = vector.z();
    real_g C = 1 - cos(angle);
    real_g c = cos(angle);
    real_g s = sin(angle);

    Matrix3x3g rotationMatrix = std::vector<Vector3>{
        {vx * vx * C + c, vy * vx * C + vz * s, vz * vx * C - vy * s}, // first column
        {vx * vy * C - vz * s, vy * vy * C + c, vz * vy * C + vx * s}, // second column
        {vx * vz * C + vy * s, vy * vz * C - vx * s, vz * vz * C + c}  // third column
    };
    Vector3 rotatedPoint = origin + rotationMatrix * (point - origin);

    return rotatedPoint;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion Quaternion::fromAxes(const Vector3 & xAxis, const Vector3 & yAxis, const Vector3 & zAxis)
{
    // This was giving the transpose of what I want
    Matrix3x3g rot3x3(std::array<float, 9>{
        xAxis.x(), xAxis.y(), xAxis.z(),
        yAxis.x(), yAxis.y(), yAxis.z(),
        zAxis.x(), zAxis.y(), zAxis.z()});
    return Quaternion::fromRotationMatrix(rot3x3);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion Quaternion::fromDirection(const Vector3 & direction, const Vector3 & up)
{
    // Return identity quaternion if direction is null
    if (qFuzzyIsNull(direction.x()) && qFuzzyIsNull(direction.y()) && qFuzzyIsNull(direction.z()))
        return Quaternion();

    // z-axis is normalized direction
    Matrix3x3 axes = Matrix3x3::EmptyMatrix();
    Vector3& xAxis = axes.column(0);
    Vector3& yAxis = axes.column(1);
    Vector3& zAxis = axes.column(2);
    zAxis = direction.normalized();
    xAxis = up.cross(zAxis);
    if (qFuzzyIsNull(xAxis.lengthSquared())) {
        // Collinear or invalid up vector; derive shortest arc to new direction
        return Quaternion::rotationTo(Vector3(0.0f, 0.0f, 1.0f), zAxis);
    }
    xAxis.normalize();
    yAxis = zAxis.cross(xAxis);
    return Quaternion::fromRotationMatrix(axes);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion Quaternion::fromAngularVelocity(const Vector3 & vel)
{
    real_g mag = vel.length();
    if (mag <= 0)
        return Quaternion();

    real_g cs = cos(mag * 0.5f);
    real_g siGain = sin(mag * 0.5f) / mag;
    return Quaternion(vel.x() * siGain, vel.y() * siGain, vel.z() * siGain, cs);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion Quaternion::rotationTo(const Vector3 & from, const Vector3 & to)
{
    // Based on Stan Melax's article in Game Programming Gems
    const Vector3d v0 = from.asDouble().normalized();
    const Vector3d v1 = to.asDouble().normalized();
    double d = v0.dot(v1);
    // If dot == 1, vectors are the same
    if (qFuzzyIsNull(d - 1.0f))
    {
        return Quaternion();
    }
    // if dest vector is close to the inverse of source vector, ANY axis of rotation is valid
    if (qFuzzyIsNull(d + 1.0f)) {
        if (false) {
            // TODO: Implement a fallback axis
            // if(fallbackAxis != zero vector)
            // rotate 180 degrees about the fallback axis
            //return Quaternion::fromAxisAngle(fallbackAxis, Constants::PI_2);
        }
        else {
            Vector3d axis = Vector3d(1.0, 0.0, 0.0).cross(v0);
            if (qFuzzyIsNull(axis.lengthSquared()))
                axis = Vector3d(0.0, 1.0, 0.0).cross(v0);
            axis.normalize();
            // same as Quaternion::fromAxisAndAngle(axis, Constants::PI_2)
            return Quaternion(axis.x(), axis.y(), axis.z(), 0.0);
        }
    }

    double s = std::sqrt(2.0 * (d + 1.0));
    double invS = 1.0 / s;
    Vector3d axis = v0.cross(v1) * invS;
    return Quaternion(axis.x(), axis.y(), axis.z(), s * 0.5f).normalized();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector3 Quaternion::rotatedVector(const Vector3 & v) const
{
    // Optimized
    // See: https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion
    //return (*this * Quaternion(v) * conjugated()).vector();
    
    // Extract the vector part of the quaternion
    Vector3 u(m_x, m_y, m_z);

    // Extract the scalar part of the quaternion
    float s = m_w;

    // Do the math
    return 2.0f * u.dot(v) * u + (s*s - u.dot(u)) * v + 2.0f * s * u.cross(v);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion Quaternion::Slerp(const Quaternion * quaternions, const std::vector<float>& weights)
{
    // NOTE: Weights are pre-normalized! See GbBlendQueue.cpp updateCurrentClips for an example
    // TODO: Implement normalization
    // Slerp weights scale each weight to account for successive slerping of multiple quaternions
    //float denom = m_clipWeights[0];
    //for (size_t i = 1; i < numClips; i++) {
    //    float currentClipWeight = m_clipWeights[i];
    //    denom += currentClipWeight;
    //    m_slerpWeights.push_back(currentClipWeight / denom); // TODO: Don't have to do this for last iteration, since denom == 1
    //}

    size_t numSlerps = weights.size();
    if (numSlerps == 0) {
        // If there are no slerps to be done!
        return quaternions[0];
    }

    Quaternion out = Slerp(quaternions[0], quaternions[1], weights[0]);
    for (size_t i = 1; i < numSlerps; i++) {
        out = Slerp(out, quaternions[i + 1], weights[i]);
    }
    out.normalize();
    return out;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion Quaternion::EigenAverage(const Quaternion* quaternions, const std::vector<float>& weights)
{
    // IS VERY SLOW, but accurate
    Quaternion average;

    const size_t N = weights.size();

    if (N == 2) {
        // If only two quaternions, use standard slerp
        float weight = weights[1]/(weights[0] + weights[1]);
        return Quaternion::Slerp(quaternions[0], quaternions[1], weight);
    }
    else if (N == 1) {
        return quaternions[0];
    }

    // Normalize weights (so they total to one)
    auto weightMap = Eigen::Map<const Eigen::VectorXf>(weights.data(), N);
    Eigen::VectorXf weightVec = weightMap;
    float sum = weightVec.sum();
    weightVec *= 1.0 / sum;

    // Construct weighted quaternion matrix
    // See: https://stackoverflow.com/questions/42935944/multiplication-of-each-matrix-column-by-each-vector-element-using-eigen-c-libr
    Eigen::Matrix<real_g, 4, -1> quatMat;
    std::vector<real_g> quatVecs;
    quatVecs.reserve(N);
    for (size_t i = 0; i < N; i++){
        const Quaternion& quat = quaternions[i];
        quatVecs.emplace_back(quat.m_x);
        quatVecs.emplace_back(quat.m_y);
        quatVecs.emplace_back(quat.m_z);
        quatVecs.emplace_back(quat.m_w);
    }
    quatMat = Eigen::MatrixXf::Map(quatVecs.data(), 4, N);
    quatMat = quatMat* weightVec.asDiagonal();

    // Perform eigenvalue decomposition
    Eigen::MatrixXf qq = quatMat * quatMat.transpose();
    Eigen::VectorXf eVals;
    Eigen::MatrixXf eVecs;

    // Matrix is always self-adjoint
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> es;
    es.compute(qq);
    eVals = es.eigenvalues().real();
    eVecs = es.eigenvectors().real();

    // Get eigenvector corresponding to the maximum eigenvalue
    //std::stringstream ss;
    //ss << eVecs;
    //std::string matStr = ss.str();
    int maxIndex;
    eVals.maxCoeff(&maxIndex);
    auto eVec = eVecs.col(maxIndex);

    // Set quaternion value from eigen-vector
    real_g x = eVec.row(0).value();
    real_g y = eVec.row(1).value();
    real_g z = eVec.row(2).value();
    real_g w = eVec.row(3).value();
    average.setX(x);
    average.setY(y);
    average.setZ(z);
    average.setW(w);

    return average;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion Quaternion::Average(const std::vector<Quaternion>& quaternions, const std::vector<float>& weights)
{
#ifdef DEBUG_MODE
    if (quaternions.size() != weights.size()) {
        throw("Error, size mismatch");
    }
#endif
    return Average(quaternions.data(), weights);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion Quaternion::Average(const Quaternion * quaternions, const std::vector<float>& weights)
{
    Quaternion average(0, 0, 0, 0);

    // https://gamedev.stackexchange.com/questions/119688/calculate-average-of-arbitrary-amount-of-quaternions-recursion
    size_t numQuaternions = weights.size();
#ifdef DEBUG_MODE
    if (!numQuaternions) {
        throw("Error, no weights");
    }
#endif

    float weight;
    for(size_t i = 0; i < numQuaternions; i++)
    {
        weight = weights[i];
        const Quaternion& q = quaternions[i];
        average.m_x += weight * q.m_x;
        average.m_y += weight * q.m_y;
        average.m_z += weight * q.m_z;
        average.m_w += weight * q.m_w;
    }

    average.normalize();
    return average;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion Quaternion::IterativeAverage(const Quaternion* quaternions, const std::vector<float>& weights, size_t numIterations)
{
    Quaternion reference;
    for (size_t i = 0; i < numIterations; i++)
    {
        reference = AverageIteration(reference, quaternions, weights);
        reference.normalize(); // Added to try to improve
    }
    return reference;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion Quaternion::AverageIteration(const Quaternion & reference, const Quaternion* quaternions, const std::vector<float>& weights)
{
    Quaternion referenceInverse = reference.inverted();
    Vector3 result;
    size_t numQuats = weights.size();
    for (size_t i = 0; i < numQuats; i++)
    {
        const Quaternion& quat = quaternions[i];
        float weight = weights[i];
        Vector3 vel = Vector3::EmptyVector();
        (referenceInverse * quat).toAngularVelocity(vel);
        result += vel * weight;
    }
    return reference * fromAngularVelocity(result);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion Quaternion::Slerp(const Quaternion & q1, const Quaternion & q2, real_g t)
{
    // Handle the easy cases first.
    if (t <= 0.0f)
        return q1;
    else if (t >= 1.0f)
        return q2;

    // Determine the angle between the two quaternions.
    Quaternion q2b(q2);
    real_g dot = q1.dot(q2);
    if (dot < 0.0f) {
        q2b = -q2b;
        dot = -dot;
    }

    // Get the scale factors.  If they are too small,
    // then revert to simple linear interpolation.
    real_g factor1 = 1.0f - t;
    real_g factor2 = t;
    real_g precision = (real_g)1e-7;
    if ((1.0f - dot) > precision) {
        real_g angle = acos(dot);
        real_g sinOfAngle = sin(angle);
        if (sinOfAngle > precision) {
            factor1 = sin((1.0f - t) * angle) / sinOfAngle;
            factor2 = sin(t * angle) / sinOfAngle;
        }
    }
    // Construct the result quaternion.
    return q1 * factor1 + q2b * factor2;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion Quaternion::Nlerp(const Quaternion & q1, const Quaternion & q2, real_g t)
{
    // Handle the easy cases first.
    if (t <= 0.0f)
        return q1;
    else if (t >= 1.0f)
        return q2;

    // Determine the angle between the two quaternions.
    Quaternion q2b(q2);
    real_g dot = q1.dot(q2);
    if (dot < 0.0f)
        q2b = -q2b;

    // Perform the linear interpolation.
    return (q1 * (1.0f - t) + q2b * t).normalized();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void Quaternion::getEulerAngles(real_g * pitch, real_g * yaw, real_g * roll) const
//{
//    assert(pitch && yaw && roll);
//    // Algorithm from:
//    // http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q37
//    real_g xx = m_x * m_x;
//    real_g xy = m_x * m_y;
//    real_g xz = m_x * m_z;
//    real_g xw = m_x * m_w;
//    real_g yy = m_y * m_y;
//    real_g yz = m_y * m_z;
//    real_g yw = m_y * m_w;
//    real_g zz = m_z * m_z;
//    real_g zw = m_z * m_w;
//    const real_g lengthSquared = xx + yy + zz + m_w * m_w;
//    if (!qFuzzyIsNull(lengthSquared - 1.0f) && !qFuzzyIsNull(lengthSquared)) {
//        xx /= lengthSquared;
//        xy /= lengthSquared; // same as (m_x / length) * (m_y / length)
//        xz /= lengthSquared;
//        xw /= lengthSquared;
//        yy /= lengthSquared;
//        yz /= lengthSquared;
//        yw /= lengthSquared;
//        zz /= lengthSquared;
//        zw /= lengthSquared;
//    }
//    *pitch = std::asin(-2.0f * (yz - xw));
//    if (*pitch < Constants::PI_2) {
//        if (*pitch > -Constants::PI_2) {
//            *yaw = std::atan2(2.0f * (xz + yw), 1.0f - 2.0f * (xx + yy));
//            *roll = std::atan2(2.0f * (xy + zw), 1.0f - 2.0f * (xx + zz));
//        }
//        else {
//            // not a unique solution
//            *roll = 0.0f;
//            *yaw = -std::atan2(-2.0f * (xy - zw), 1.0f - 2.0f * (yy + zz));
//        }
//    }
//    else {
//        // not a unique solution
//        *roll = 0.0f;
//        *yaw = std::atan2(-2.0f * (xy - zw), 1.0f - 2.0f * (yy + zz));
//    }
//}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector4 Quaternion::toVector4() const
{
    return Vector4(m_x, m_y, m_z, m_w);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Matrix3x3g Quaternion::toRotationMatrix() const
{
    // Algorithm from:
    // http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q54
    Matrix3x3g rot3x3;
    const real_g f2x = m_x + m_x;
    const real_g f2y = m_y + m_y;
    const real_g f2z = m_z + m_z;
    const real_g f2xw = f2x * m_w;
    const real_g f2yw = f2y * m_w;
    const real_g f2zw = f2z * m_w;
    const real_g f2xx = f2x * m_x;
    const real_g f2xy = f2x * m_y;
    const real_g f2xz = f2x * m_z;
    const real_g f2yy = f2y * m_y;
    const real_g f2yz = f2y * m_z;
    const real_g f2zz = f2z * m_z;
    rot3x3(0, 0) = 1.0f - (f2yy + f2zz);
    rot3x3(0, 1) = f2xy - f2zw;
    rot3x3(0, 2) = f2xz + f2yw;
    rot3x3(1, 0) = f2xy + f2zw;
    rot3x3(1, 1) = 1.0f - (f2xx + f2zz);
    rot3x3(1, 2) = f2yz - f2xw;
    rot3x3(2, 0) = f2xz - f2yw;
    rot3x3(2, 1) = f2yz + f2xw;
    rot3x3(2, 2) = 1.0f - (f2xx + f2yy);
    return rot3x3;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Matrix4x4g Quaternion::toRotationMatrix4x4() const
{
    // Algorithm from:
    // http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q54
    return Matrix4x4g(toRotationMatrix());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Quaternion::toRotationMatrix(Matrix4x4g & outMatrix) const
{
    const real_g f2x = m_x + m_x;
    const real_g f2y = m_y + m_y;
    const real_g f2z = m_z + m_z;
    const real_g f2xw = f2x * m_w;
    const real_g f2yw = f2y * m_w;
    const real_g f2zw = f2z * m_w;
    const real_g f2xx = f2x * m_x;
    const real_g f2xy = f2x * m_y;
    const real_g f2xz = f2x * m_z;
    const real_g f2yy = f2y * m_y;
    const real_g f2yz = f2y * m_z;
    const real_g f2zz = f2z * m_z;
    outMatrix(0, 0) = 1.0f - (f2yy + f2zz);
    outMatrix(0, 1) = f2xy - f2zw;
    outMatrix(0, 2) = f2xz + f2yw;
    outMatrix(1, 0) = f2xy + f2zw;
    outMatrix(1, 1) = 1.0f - (f2xx + f2zz);
    outMatrix(1, 2) = f2yz - f2xw;
    outMatrix(2, 0) = f2xz - f2yw;
    outMatrix(2, 1) = f2yz + f2xw;
    outMatrix(2, 2) = 1.0f - (f2xx + f2yy);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Quaternion::getAxes(Vector3 * xAxis, Vector3 * yAxis, Vector3 * zAxis) const
{
    assert(xAxis && yAxis && zAxis);
    const Matrix3x3g rot3x3(toRotationMatrix());
    *xAxis = Vector3(rot3x3(0, 0), rot3x3(1, 0), rot3x3(2, 0));
    *yAxis = Vector3(rot3x3(0, 1), rot3x3(1, 1), rot3x3(2, 1));
    *zAxis = Vector3(rot3x3(0, 2), rot3x3(1, 2), rot3x3(2, 2));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Quaternion::asJson() const
{
    return Vector4(m_x, m_y, m_z, m_w).asJson();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Quaternion::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context);

    QJsonArray jsonArray = json.toArray();
    m_x = (real_g)jsonArray[0].toDouble();
    m_y = (real_g)jsonArray[1].toDouble();
    m_z = (real_g)jsonArray[2].toDouble();
    m_w = (real_g)jsonArray[3].toDouble();
}





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}