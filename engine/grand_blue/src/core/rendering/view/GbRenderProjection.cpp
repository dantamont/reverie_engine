#include "GbRenderProjection.h"

#include "../../GbCoreEngine.h"
#include "../../GbConstants.h"
#include "../../../view/GbWidgetManager.h"
#include "../../../view/GL/GbGLWidget.h"
#include "../../loop/GbSimLoop.h"

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
Matrix4x4f RenderProjection::ortho(float left, float right, float bottom, float top, float zNear, float zFar)
{
     Matrix4x4f matrix;
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
RenderProjection::RenderProjection() :
    m_fov(70),
    m_zNear(0.01),
    m_zFar(1000),
    m_left(-0.5),
    m_right(0.5),
    m_bottom(-0.5),
    m_top(0.5),
    m_projectionType(kPerspective)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
RenderProjection::RenderProjection(View::GLWidget* glWidget):
    m_fov(70),
    m_zNear(0.01),
    m_zFar(1000),
    m_left(-0.5),
    m_right(0.5),
    m_bottom(-0.5),
    m_top(0.5),
    m_projectionType(kPerspective)
{
    addToGLWidget(glWidget);
    //auto glWidget = m_engine->widgetManager()->mainGLWidget();
    //updateAspectRatio(glWidget->lastWidth(), glWidget->lastHeight());
}
/////////////////////////////////////////////////////////////////////////////////////////////
RenderProjection::~RenderProjection()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RenderProjection::setProjectionType(ProjectionType type)
{
    m_projectionType = type;
    computeProjectionMatrix();
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
void RenderProjection::loadFromJson(const QJsonValue & json)
{
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
void RenderProjection::addToGLWidget(View::GLWidget* glWidget)
{
    glWidget->addRenderProjection(this);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RenderProjection::updateAspectRatio(int width, int height) {
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
        break;
    }

    // Update projection matrix
    computeProjectionMatrix();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RenderProjection::computeProjectionMatrix()
{
    // See: https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml
    m_projectionMatrix.setToIdentity(); 

    double frustumLength = m_zFar - m_zNear;
    double zp = m_zFar + m_zNear;

    switch (m_projectionType) {
    case kPerspective:
    {
        double fovRad = Gb::Constants::DEG_TO_RAD * m_fov;
        double yScale = (1.0 / tan(fovRad / 2.0)) * m_aspectRatio; // Scaling since using horizontal FOV
        double xScale = yScale / m_aspectRatio;
        m_projectionMatrix(0, 0) = xScale;
        m_projectionMatrix(1, 1) = yScale;
        m_projectionMatrix(2, 2) = -zp / frustumLength;
        m_projectionMatrix(3, 2) = -1.0;
        m_projectionMatrix(2, 3) = -(2 * m_zFar * m_zNear) / frustumLength;
        m_projectionMatrix(3, 3) = 0;
        break;
    }
    case kOrthographic:
        m_projectionMatrix = ortho(m_left, m_right, m_bottom, m_top, m_zNear, m_zFar);
        break;
    }

    m_projectionMatrix.inversed(m_inverseProjectionMatrix);
}

double RenderProjection::hfov_to_vfov(double hfov, double aspect) {
    hfov = Gb::Constants::DEG_TO_RAD * (hfov);
    float vfov = 2.0f * atan(tan(hfov * 0.5f) / aspect);
    return Gb::Constants::RAD_TO_DEG * (vfov);
}

double RenderProjection::vfov_to_hfov(double vfov, double aspect) {
    vfov = Gb::Constants::DEG_TO_RAD * (vfov);
    float hfov = 2.0f * atan(tan(vfov * 0.5f) * aspect);
    return Gb::Constants::RAD_TO_DEG * (hfov);
}

/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}