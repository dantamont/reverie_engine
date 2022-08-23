#include "core/rendering/view/GViewport.h"

#include "core/rendering/view/GRenderProjection.h"
#include "core/rendering/view/GFrameBuffer.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"

#include "geppetto/qt/widgets/GWidgetManager.h"
#include "core/layer/view/widgets/graphics/GGLWidget.h"

namespace rev {


// Viewport

Vector2i Viewport::ScreenDimensions()
{
    // Since the primary screen may not correspond with the screen of the main window, obtain the 
    // screen at the main window's midpoint
    // Note: This assumes that the main window is the first window in the QGuiApplication list, which may 
    // not be the case. However, if all windows are on the same screen, this will not matter.
    //QList<QScreen*> screens = QGuiApplication::screens();
    //QScreen *screen = QGuiApplication::primaryScreen();
    QWindowList windows = QGuiApplication::topLevelWindows();
    QWindow* mainWindow = windows[0];
    QPoint windowPos = mainWindow->mapToGlobal({ mainWindow->width() / 2, 0 });
    QSize windowSize = mainWindow->size();
    //QScreen* screen = QGuiApplication::screenAt(windowPos);
    QScreen* screen = mainWindow->screen();

    // Could also use QGuiApplication::devicePixelRatio
    // The true screen size is the device-pixel-ratio (typically 1.0, but 2.0+ for 4k monitors or similar) \
    // multiplied by the screen geometry
    QSize screenGeometry = screen->size();
    float dpr = screen->devicePixelRatio();
    Vector2i dims = Vector2i(screenGeometry.width(), screenGeometry.height()) * dpr;
    return dims;
}

float Viewport::ScreenDPI()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    return screen->logicalDotsPerInch();
}

float Viewport::ScreenDPIX()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    return screen->logicalDotsPerInchX();
}

float Viewport::ScreenDPIY()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    return screen->logicalDotsPerInchY();
}

void to_json(json& orJson, const Viewport& korObject)
{
    orJson["x"] = korObject.m_xn;
    orJson["y"] = korObject.m_yn;
    orJson["w"] = korObject.m_width;
    orJson["h"] = korObject.m_height;
    orJson["d"] = korObject.m_depth;
}

void from_json(const json& korJson, Viewport& orObject)
{
    korJson.at("x").get_to(orObject.m_xn);
    korJson.at("y").get_to(orObject.m_yn);
    korJson.at("w").get_to(orObject.m_width);
    korJson.at("h").get_to(orObject.m_height);
    orObject.setDepth(korJson.at("d").get<Float32_t>());
}

void Viewport::setDepth(int depth)
{
    m_depth = depth;
    s_depthBounds[0] = std::min(m_depth, s_depthBounds[0]);
    s_depthBounds[1] = std::max(m_depth, s_depthBounds[1]);

    if (s_depthBounds[0] == s_depthBounds[1]) {
        s_depthBounds[1] += 1;
    }
}

bool Viewport::inBounds(const Vector2 & widgetPos, const OpenGlRenderer & renderer, float & outClipX, float & outClipY) const
{
    widgetToClip(widgetPos, renderer, outClipX, outClipY);
    return outClipX <= 1 && outClipX >= -1 && outClipY <= 1 && outClipY >= -1;
}

void Viewport::widgetToClip(const Vector2 & widgetSpace, const OpenGlRenderer & renderer, float & outClipX, float & outClipY) const
{
    // First, get in homogeneous coordinates for the entire widget -----------------
    /// \see https://antongerdelan.net/opengl/raycasting.html
    // Start in widget space; range [0:widget_width, widget_height:0]
    // Convert from widget space to viewport coordinates
    // range [0:viewport_width, viewport_height:0]
    uint32_t widgetWidth = renderer.widget()->width();
    uint32_t widgetHeight = renderer.widget()->height();

    // Convert from viewport to normalized device coordinates
    // range[-1:1, -1 : 1, -1 : 1]
    // Also convert from normalized device coordinates to homogeneous clip coordinates
    // range[-1:1, -1 : 1, -1 : 1, -1 : 1]
    outClipX = 2 * (widgetSpace.x() / widgetWidth) - 1.0;
    outClipY = 1.0 - 2 * (widgetSpace.y() / widgetHeight); // Invert (flip), since widget-space origin is top-left

    // Then, use the viewport properties to convert to framebuffer coords ----------
    outClipX = (outClipX - m_xn) / (Real_t)m_width;
    outClipY = (outClipY - m_yn) / (Real_t)m_height;
}

float Viewport::getClipDepth()
{
    // Normalize to [0, 1] range
    float depth = m_depth - s_depthBounds[0];
    depth /= float(s_depthBounds[1] - s_depthBounds[0]);

    // Convert to [-1, 0] range
    depth = depth - 1.0;

    return depth;
}

void Viewport::setGLViewport(const OpenGlRenderer& r) const
{
    // Changed this, offset is now used for screen quads in shader
    // Remaps the scene AFTER projection
    /// \see https://gamedev.stackexchange.com/questions/75499/glviewport-like-camera
    //int xw = (m_xn)*(r.m_widget->width());  // not required with framebuffer, quad itself is moved
    //int yw = (m_yn)*(r.m_widget->height()); // not required with framebuffer, quad itself is moved
    int width = r.m_widget->pixelWidth()* m_width;
    int height = r.m_widget->pixelHeight()*m_height;
    glViewport(0, 0, width, height);
}

void Viewport::setGLViewport(const FrameBuffer& fbo) const
{
    // Use framebuffer dimensions to set viewport
    glViewport(0, 0, fbo.width(), fbo.height());
}

void Viewport::resizeFrameBuffer(uint32_t width, uint32_t height, FrameBuffer & fbo) const
{
    // Remaps the scene AFTER projection
    /// \see https://gamedev.stackexchange.com/questions/75499/glviewport-like-camera
    int w = (m_width)*(width);
    int h = (m_height)*(height);
    bool ignoreError = w == 0 && h == 0; // ignore reinitialize error if 0 width and height speciified
    fbo.reinitialize(w, h, ignoreError);
}

std::array<int, 2> Viewport::s_depthBounds = { std::numeric_limits<int>::infinity(), -std::numeric_limits<int>::infinity() };


    
// End namespaces
}
