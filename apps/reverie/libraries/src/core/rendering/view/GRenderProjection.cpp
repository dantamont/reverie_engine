#include "core/rendering/view/GRenderProjection.h"

#include "core/GCoreEngine.h"
#include <fortress/constants/GConstants.h>
#include "core/components/GCameraComponent.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "core/layer/view/widgets/graphics/GGLWidget.h"
#include "core/loop/GSimLoop.h"

namespace rev {

Matrix4x4 RenderProjection::Orthographic(float left, float right, float bottom, float top, float zNear, float zFar)
{
     Matrix4x4 matrix = Matrix4x4::EmptyMatrix();
     Orthographic(matrix, left, right, bottom, top, zNear, zFar);
     return matrix;
}

void RenderProjection::Orthographic(Matrix4x4& matrix, float left, float right, float bottom, float top, float zNear, float zFar)
{
    matrix.setToIdentity();
    float inverseFrustumLength = 1.0f / (zFar - zNear);
    float width = right - left;
    float inverseWidth = 1.0f / width;
    float height = top - bottom;
    matrix(0, 0) = 2 * inverseWidth;
    matrix(1, 1) = 2 / (top - bottom);
    matrix(2, 2) = -2 * inverseFrustumLength;
    matrix(0, 3) = -(right + left) * inverseWidth;
    matrix(1, 3) = -(top + bottom) / height;
    matrix(2, 3) = -(zFar + zNear) * inverseFrustumLength;
    matrix(3, 3) = 1;
}

Matrix4x4 RenderProjection::Perspective(float fovDeg, float aspectRatio, float zNear, float zFar)
{
    Matrix4x4 projectionMatrix = Matrix4x4::EmptyMatrix();
    Perspective(projectionMatrix, fovDeg, aspectRatio, zNear, zFar);
    return projectionMatrix;
}

void RenderProjection::Perspective(Matrix4x4& projectionMatrix, float fovDeg, float aspectRatio, float zNear, float zFar)
{
    double inverseFrustumLength = 1.0 / (zFar - zNear);
    double zp = zFar + zNear;

    double fovRad = rev::Constants::DegToRad * fovDeg;
    double yScale = (1.0 / tan(0.5*fovRad)) * aspectRatio; // Scaling since using horizontal FOV
    double xScale = yScale / aspectRatio;
    projectionMatrix(0, 0) = xScale;
    projectionMatrix(1, 1) = yScale;
    projectionMatrix(2, 2) = -zp * inverseFrustumLength; // T1 https://www.khronos.org/opengl/wiki/Compute_eye_space_from_window_space
    projectionMatrix(3, 2) = -1.0; // E1
    projectionMatrix(2, 3) = -(2 * zFar * zNear) * inverseFrustumLength; // T2
    projectionMatrix(3, 3) = 0;

}

float RenderProjection::LinearizedDepth(float depth, float zNear, float zFar)
{
    // Map depth back from [0, 1] to [-1, 1]
    // TODO: Make this use current GL Settings
    float depthRange = 2.0 * depth - 1.0;

    // Retrieve true depth value (distance from camera) from depth buffer value
    float linear = 2.0 * zNear * zFar / (zFar + zNear - depthRange * (zFar - zNear));
    return linear;
}

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

RenderProjection::~RenderProjection()
{
}

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

void RenderProjection::setProjectionType(ProjectionType type)
{
    m_projectionType = type;
    computeProjectionMatrix();
}

void RenderProjection::setPerspective(float fov, float aspectRatio, float nearClip, float farClip)
{
    m_projectionType = ProjectionType::kPerspective;

    m_fov = fov;
    m_aspectRatio = aspectRatio;
    m_zNear = nearClip;
    m_zFar = farClip;

    updateProjection();
}

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

float RenderProjection::linearizeDepth(float depth) const
{
    return LinearizedDepth(depth, m_zNear, m_zFar);
}

void RenderProjection::setFOV(double fov)
{
    m_fov = fov;
    computeProjectionMatrix();
}

double RenderProjection::fovX() const
{
    return m_fov;
}

double RenderProjection::fovY() const
{
    return hfov_to_vfov(m_fov, m_aspectRatio);
}

void to_json(json& orJson, const RenderProjection& korObject)
{
    orJson["fov"] = korObject.m_fov;
    orJson["zNear"] = korObject.m_zNear;
    orJson["zFar"] = korObject.m_zFar;
    orJson["left"] = korObject.m_left;
    orJson["right"] = korObject.m_right;
    orJson["bottom"] = korObject.m_bottom;
    orJson["top"] = korObject.m_top;
    orJson["aspectRatio"] = korObject.m_aspectRatio;
    orJson["projType"] = int(korObject.m_projectionType);
}

void from_json(const json& korJson, RenderProjection& orObject)
{
    korJson.at("fov").get_to(orObject.m_fov);
    korJson.at("zNear").get_to(orObject.m_zNear);
    korJson.at("zFar").get_to(orObject.m_zFar);
    korJson.at("aspectRatio").get_to(orObject.m_aspectRatio);
    if (korJson.contains("projType")) {
        orObject.m_projectionType = RenderProjection::ProjectionType(korJson.at("projType").get<Int32_t>());
    }
    if (korJson.contains("left")) {
        korJson.at("left").get_to(orObject.m_left);
        korJson.at("right").get_to(orObject.m_right);
        korJson.at("bottom").get_to(orObject.m_bottom);
        korJson.at("top").get_to(orObject.m_top);
    }
    orObject.computeProjectionMatrix();
}

void RenderProjection::resizeProjection(int width, int height) {
    switch (m_projectionType) {
    case kPerspective:
        // Update perspective dimensions
        m_aspectRatio = double(width) / double(height);
        break;
    case kOrthographic:
        // Orthographic really doesn't depend on widget dimensions (I think), so don't
        // do anything
        //// Update orthographic dimensions
        //m_left = -width/2.0;
        //m_right = width/2.0;
        //m_bottom = -height/2.0;
        //m_top = height/2.0;
        break;
    default:
        Logger::Throw("Error, projection type not recognized");
        break;
    }

    // Update projection matrix
    computeProjectionMatrix();
}

void RenderProjection::updateProjection()
{
    // Update projection matrix
    computeProjectionMatrix();

    // Reconstruct camera's light frustum grid
    if (m_camera) {
        m_camera->updateFrustum(CameraUpdateFlag::kProjectionUpdated);
    }

}

void RenderProjection::computeProjectionMatrix()
{
    /// \see https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml
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

double RenderProjection::hfov_to_vfov(double hfov, double aspect) {
    hfov = rev::Constants::DegToRad * (hfov);
    float vfov = 2.0f * atan(tan(hfov * 0.5f) / aspect);
    return rev::Constants::RadToDeg * (vfov);
}

double RenderProjection::vfov_to_hfov(double vfov, double aspect) {
    vfov = rev::Constants::DegToRad * (vfov);
    float hfov = 2.0f * atan(tan(vfov * 0.5f) * aspect);
    return rev::Constants::RadToDeg * (hfov);
}



// End namespaces
}