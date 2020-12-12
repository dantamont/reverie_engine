#include "GbViewport.h"

#include "GbRenderProjection.h"
#include "GbFrameBuffer.h"
#include "../renderer/GbMainRenderer.h"

#include "../../../view/GbWidgetManager.h"
#include "../../../view/GL/GbGLWidget.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// Viewport
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Viewport::asJson() const
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
    fbo.reinitialize(w, h);
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::array<int, 2> Viewport::s_depthBounds = { std::numeric_limits<int>::infinity(), -std::numeric_limits<int>::infinity() };


//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}
