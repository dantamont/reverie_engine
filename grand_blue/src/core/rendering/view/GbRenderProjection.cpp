#include "GbRenderProjection.h"

#include "../../GbCoreEngine.h"
#include "../../GbConstants.h"
#include "../../components/GbCameraComponent.h"
#include "../../rendering/shaders/GbShaders.h"
#include "../../rendering/renderer/GbMainRenderer.h"
#include "../../../view/GbWidgetManager.h"
#include "../../../view/GL/GbGLWidget.h"
#include "../../loop/GbSimLoop.h"

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
Matrix4x4 RenderProjection::Orthographic(float left, float right, float bottom, float top, float zNear, float zFar)
{
     Matrix4x4 matrix;
     float frustumLength = zFar - zNear;
     float width = right - left;
     float height = top - bottom;
     matrix(0, 0) = 2 / width;
     matrix(1, 1) = 2 / (top - bottom);
     matrix(2, 2) = -2 / frustumLength;
     matrix(0, 3) = -(right + left) / width;
     matrix(1, 3) = -(top + bottom) / height;
     matrix(2, 3) = -(zFar + zNear) / frustumLength;
     matrix(3, 3) = 1;

     return matrix;
}
/////////////////////////////////////////////////////////////////////////////////////////////
Matrix4x4 RenderProjection::Perspective(float fovDeg, float aspectRatio, float zNear, float zFar)
{
    double frustumLength = zFar - zNear;
    double zp = zFar + zNear;

    Matrix4x4 projectionMatrix;
    double fovRad = Gb::Constants::DEG_TO_RAD * fovDeg;
    double yScale = (1.0 / tan(fovRad / 2.0)) * aspectRatio; // Scaling since using horizontal FOV
    double xScale = yScale / aspectRatio;
    projectionMatrix(0, 0) = xScale;
    projectionMatrix(1, 1) = yScale;
    projectionMatrix(2, 2) = -zp / frustumLength; // T1 https://www.khronos.org/opengl/wiki/Compute_eye_space_from_window_space
    projectionMatrix(3, 2) = -1.0; // E1
    projectionMatrix(2, 3) = -(2 * zFar * zNear) / frustumLength; // T2
    projectionMatrix(3, 3) = 0;

    return projectionMatrix;
}
/////////////////////////////////////////////////////////////////////////////////////////////
float RenderProjection::LinearizedDepth(float depth, float zNear, float zFar)
{
    // Map depth back from [0, 1] to [-1, 1]
    // TODO: Make this use current GL Settings
    float depthRange = 2.0 * depth - 1.0;

    // Retrieve true depth value (distance from camera) from depth buffer value
    float linear = 2.0 * zNear * zFar / (zFar + zNear - depthRange * (zFar - zNear));
    return linear;
}
/////////////////////////////////////////////////////////////////////////////////////////////
RenderProjection::RenderProjection() :
    m_fov(70),
    m_zNear(0.01),
    m_zFar(1000),
    m_left(-0.5),
    m_right(0.5),
    m_bottom(-0.5),
    m_top(0.5),
    m_projectionType(kPerspective),
    m_camera(nullptr)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
RenderProjection::RenderProjection(const RenderProjection & other) :
    m_projectionType(other.m_projectionType),
    m_camera(other.m_camera),
    m_fov(other.m_fov),
    m_zNear(other.m_zNear),
    m_zFar(other.m_zFar),
    m_left(other.m_left),
    m_right(other.m_right),
    m_bottom(other.m_bottom),
    m_top(other.m_top),
    m_aspectRatio(other.m_aspectRatio)
{
    updateProjection();
}
/////////////////////////////////////////////////////////////////////////////////////////////
RenderProjection::RenderProjection(Camera* camera) :
    m_fov(70),
    m_zNear(0.01),
    m_zFar(1000),
    m_left(-0.5),
    m_right(0.5),
    m_bottom(-0.5),
    m_top(0.5),
    m_projectionType(kPerspective),
    m_camera(camera)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
RenderProjection::~RenderProjection()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
RenderProjection & RenderProjection::operator=(const RenderProjection & other)
{
    m_projectionType = other.m_projectionType;
    m_camera = other.m_camera;
    m_fov = other.m_fov;
    m_zNear = other.m_zNear;
    m_zFar = other.m_zFar;
    m_left = other.m_left;
    m_right = other.m_right;
    m_bottom = other.m_bottom;
    m_top = other.m_top;
    m_aspectRatio = other.m_aspectRatio;

    updateProjection();

    return *this;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RenderProjection::setProjectionType(ProjectionType type)
{
    m_projectionType = type;
    computeProjectionMatrix();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RenderProjection::setPerspective(float fov, float aspectRatio, float nearClip, float farClip)
{
    m_projectionType = ProjectionType::kPerspective;

    m_fov = fov;
    m_aspectRatio = aspectRatio;
    m_zNear = nearClip;
    m_zFar = farClip;

    updateProjection();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RenderProjection::setOrthographic(float left, float right, float bottom, float top, float zNear, float zFar)
{
    m_projectionType = ProjectionType::kOrthographic;

    m_left = left;
    m_right = right;
    m_bottom = bottom;
    m_top = top;
    m_zNear = zNear;
    m_zFar = zFar;

    updateProjection();
}
/////////////////////////////////////////////////////////////////////////////////////////////
float RenderProjection::linearizeDepth(float depth) const
{
    return LinearizedDepth(depth, m_zNear, m_zFar);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RenderProjection::setFOV(double fov)
{
    m_fov = fov;
    computeProjectionMatrix();
}
/////////////////////////////////////////////////////////////////////////////////////////////
double RenderProjection::fovX() const
{
    return m_fov;
}
/////////////////////////////////////////////////////////////////////////////////////////////
double RenderProjection::fovY() const
{
    return hfov_to_vfov(m_fov, m_aspectRatio);
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue RenderProjection::asJson() const
{
    QJsonObject object;
    object.insert("fov", m_fov);
    object.insert("zNear", m_zNear);
    object.insert("zFar", m_zFar);
    object.insert("left", m_left);
    object.insert("right", m_right);
    object.insert("bottom", m_bottom);
    object.insert("top", m_top);
    object.insert("aspectRatio", m_aspectRatio);
    object.insert("projType", int(m_projectionType));
    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RenderProjection::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context);

    const QJsonObject& object = json.toObject();

    m_fov = object.value("fov").toDouble();
    m_zNear = object.value("zNear").toDouble();
    m_zFar = object.value("zFar").toDouble();
    m_aspectRatio = object.value("aspectRatio").toDouble();
    if (object.contains("projType")) {
        m_projectionType = ProjectionType(object.value("projType").toInt());
    }
    if (object.contains("left")) {
        m_left = object.value("left").toDouble();
        m_right = object.value("right").toDouble();
        m_bottom = object.value("bottom").toDouble();
        m_top = object.value("top").toDouble();
    }
    computeProjectionMatrix();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RenderProjection::resizeProjection(int width, int height) {
    switch (m_projectionType) {
    case kPerspective:
        // Update perspective dimensions
        m_aspectRatio = double(width) / double(height);
        break;
    case kOrthographic:
        //// Update orthographic dimensions
        //m_left = -width/2.0;
        //m_right = width/2.0;
        //m_bottom = -height/2.0;
        //m_top = height/2.0;
        m_left = -1.0/2.0;
        m_right = 1.0/2.0;
        m_bottom = -1.0/2.0;
        m_top = 1.0/2.0;
        break;
    default:
        throw("Error, projection type not recognized");
        break;
    }

    // Update projection matrix
    computeProjectionMatrix();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RenderProjection::updateProjection()
{
    // Update projection matrix
    computeProjectionMatrix();

    // Reconstruct camera's light frustum grid
    if (m_camera) {
        m_camera->updateFrustum(CameraUpdateFlag::kProjectionUpdated);
    }

}
/////////////////////////////////////////////////////////////////////////////////////////////
void RenderProjection::computeProjectionMatrix()
{
    // See: https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml
    m_projectionMatrix.setToIdentity();
    switch (m_projectionType) {
    case kPerspective:
    {
        m_projectionMatrix = Perspective(m_fov, m_aspectRatio, m_zNear, m_zFar);
        break;
    }
    case kOrthographic:
        m_projectionMatrix = Orthographic(m_left, m_right, m_bottom, m_top, m_zNear, m_zFar);
        break;
    }

    Matrix4x4d inverseProjection;
    m_projectionMatrix.toDoubleMatrix().inversed(inverseProjection);
    m_inverseProjectionMatrix = inverseProjection.toFloatMatrix();
}
/////////////////////////////////////////////////////////////////////////////////////////////
double RenderProjection::hfov_to_vfov(double hfov, double aspect) {
    hfov = Gb::Constants::DEG_TO_RAD * (hfov);
    float vfov = 2.0f * atan(tan(hfov * 0.5f) / aspect);
    return Gb::Constants::RAD_TO_DEG * (vfov);
}
/////////////////////////////////////////////////////////////////////////////////////////////
double RenderProjection::vfov_to_hfov(double vfov, double aspect) {
    vfov = Gb::Constants::DEG_TO_RAD * (vfov);
    float hfov = 2.0f * atan(tan(vfov * 0.5f) * aspect);
    return Gb::Constants::RAD_TO_DEG * (hfov);
}


/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}