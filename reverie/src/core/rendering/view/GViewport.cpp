#include "GViewport.h"

#include "GRenderProjection.h"
#include "GFrameBuffer.h"
#include "../renderer/GMainRenderer.h"

#include "../../../view/GWidgetManager.h"
#include "../../../view/GL/GGLWidget.h"

namespace rev {
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// Viewport
//////////////////////////////////////////////////////////////////////////////////////////////////
QSize Viewport::ScreenDimensions()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    return screenGeometry.size();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector2 Viewport::ScreenDimensionsVec()
{
    QSize size = ScreenDimensions();
    return Vector2(size.width(), size.height());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float Viewport::ScreenDPI()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    return screen->logicalDotsPerInch();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float Viewport::ScreenDPIX()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    return screen->logicalDotsPerInchX();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float Viewport::ScreenDPIY()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    return screen->logicalDotsPerInchY();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Viewport::asJson(const SerializationContext& context) const
{
    QJsonObject object;
    object.insert("x", m_xn);
    object.insert("y", m_yn);
    object.insert("w", m_width);
    object.insert("h", m_height);
    object.insert("d", m_depth);

    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Viewport::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    const QJsonObject& object = json.toObject();
    m_xn = object.value("x").toDouble();
    m_yn = object.value("y").toDouble();
    m_width = object.value("w").toDouble();
    m_height = object.value("h").toDouble();
    setDepth(object.value("d").toDouble());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Viewport::setDepth(int depth)
{
    m_depth = depth;
    s_depthBounds[0] = std::min(m_depth, s_depthBounds[0]);
    s_depthBounds[1] = std::max(m_depth, s_depthBounds[1]);

    if (s_depthBounds[0] == s_depthBounds[1]) {
        s_depthBounds[1] += 1;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
bool Viewport::inBounds(const Vector2 & widgetPos, const MainRenderer & renderer, float & outClipX, float & outClipY) const
{
    widgetToClip(widgetPos, renderer, outClipX, outClipY);
    return outClipX <= 1 && outClipX >= -1 && outClipY <= 1 && outClipY >= -1;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Viewport::widgetToClip(const Vector2 & widgetSpace, const MainRenderer & renderer, float & outClipX, float & outClipY) const
{
    // First, get in homogeneous coordinates for the entire widget -----------------
    // See: https://antongerdelan.net/opengl/raycasting.html
    // Start in widget space; range [0:widget_width, widget_height:0]
    // Convert from widget space to viewport coordinates
    // range [0:viewport_width, viewport_height:0]
    size_t widgetWidth = renderer.widget()->width();
    size_t widgetHeight = renderer.widget()->height();

    // Convert from viewport to normalized device coordinates
    // range[-1:1, -1 : 1, -1 : 1]
    // Also convert from normalized device coordinates to homogeneous clip coordinates
    // range[-1:1, -1 : 1, -1 : 1, -1 : 1]
    outClipX = 2 * (widgetSpace.x() / widgetWidth) - 1.0;
    outClipY = 1.0 - 2 * (widgetSpace.y() / widgetHeight); // Invert (flip), since widget-space origin is top-left

    // Then, use the viewport properties to convert to framebuffer coords ----------
    outClipX = (outClipX - m_xn) / (real_g)m_width;
    outClipY = (outClipY - m_yn) / (real_g)m_height;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
float Viewport::getClipDepth()
{
    // Normalize to [0, 1] range
    float depth = m_depth - s_depthBounds[0];
    depth /= float(s_depthBounds[1] - s_depthBounds[0]);

    // Convert to [-1, 0] range
    depth = depth - 1.0;

    return depth;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Viewport::setGLViewport(const MainRenderer& r) const
{
    // Changed this, offset is now used for screen quads in shader
    // Remaps the scene AFTER projection
    // See: https://gamedev.stackexchange.com/questions/75499/glviewport-like-camera
    //int xw = (m_xn)*(r.m_widget->width());  // not required with framebuffer, quad itself is moved
    //int yw = (m_yn)*(r.m_widget->height()); // not required with framebuffer, quad itself is moved
    int width = r.m_widget->width()* m_width;
    int height = r.m_widget->height()*m_height;
    glViewport(0, 0, width, height);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Viewport::setGLViewport(const FrameBuffer& fbo) const
{
    // Use framebuffer dimensions to set viewport
    glViewport(0, 0, fbo.width(), fbo.height());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Viewport::resizeFrameBuffer(size_t width, size_t height, FrameBuffer & fbo) const
{
    // Remaps the scene AFTER projection
    // See: https://gamedev.stackexchange.com/questions/75499/glviewport-like-camera
    int w = (m_width)*(width);
    int h = (m_height)*(height);
    bool ignoreError = w == 0 && h == 0; // ignore reinitialize error if 0 width and height speciified
    fbo.reinitialize(w, h, ignoreError);
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::array<int, 2> Viewport::s_depthBounds = { std::numeric_limits<int>::infinity(), -std::numeric_limits<int>::infinity() };


//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}
