#include "GMousePicking.h"

#include <core/scene/GSceneObject.h>
#include <core/rendering/view/GCamera.h>
#include <core/rendering/renderer/GMainRenderer.h>
#include <core/rendering/renderer/GRenderCommand.h>

namespace rev {
//////////////////////////////////////////////////////////////////////////////////////////////////
MousePicker::MousePicker()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
MousePicker::~MousePicker()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
bool MousePicker::isMouseOver(Renderable * renderable) const
{
    return m_hoverInfo.m_renderablePtr == renderable;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
bool MousePicker::isMouseOver(SceneObject * sceneObject) const
{
    return m_hoverInfo.m_sceneObjectId == sceneObject->id();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void rev::MousePicker::updateMouseOver(const Vector2 & widgetMousePos, const Camera& camera, const MainRenderer & renderer)
{
    // Second color attachment holds normals
    //getPixelColor(m_mouseOverViewNormal, widgetMousePos, camera, renderer, 1, 1);

    // Third color attachment holds unique color IDs
    getPixelColor(m_mouseOverColor, widgetMousePos, camera, renderer, 2, 1);

    if (m_mouseOverColor[0] < 0) {
        m_hoverInfo = {-1, nullptr};
        return;
    }

    //m_mouseOverViewNormal.setW(0);
    //Logger::LogInfo("Normals: " + GString(m_mouseOverViewNormal * 255));
    //Vector4 worldNormal = camera.getViewMatrix().inversed() * m_mouseOverViewNormal;
    //Logger::LogInfo("World normal: " + GString(worldNormal));
    //Logger::LogInfo("View normal: " + GString(m_mouseOverViewNormal));

    // Retrieve as three-channel ID since alpha is at 255 to avoid blending troubles
    //Logger::LogInfo("ID: " + GString(m_mouseOverColor));
    size_t id = GetColorId<3, int>(m_mouseOverColor.data());
    //Logger::LogInfo("ID: " + GString::FromNumber(id));

#ifdef DEBUG_MODE
    if (id >= renderer.readRenderCommands().size()) {
        return;
    }
#endif

    std::shared_ptr<DrawCommand> dc = std::dynamic_pointer_cast<DrawCommand>(renderer.readRenderCommands()[id]);
    m_hoverInfo.m_sceneObjectId = dc->sceneObjectId();
    if (m_hoverInfo.m_sceneObjectId > 0) {
        //Logger::LogInfo("Scene object: " + SceneObject::Get(m_hoverInfo.m_sceneObjectId)->getName());
        m_hoverInfo.m_renderablePtr = dc->renderable();
        //Logger::LogInfo("Renderable: " + GString::FromNumber((int)r));
    }
    else {
        m_hoverInfo.m_renderablePtr = nullptr;
    }
    //Logger::LogInfo("-----------");
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void rev::MousePicker::getPixelColor(Vector<int, 4>& outColor, const Vector3 & widgetPos, const Camera & camera, const MainRenderer & renderer, size_t attachmentIndex, size_t sampleHalfWidth)
{
    // Check if in bounds
    real_g frameX, frameY;
    if (!camera.getViewport().inBounds(widgetPos, renderer, frameX, frameY)) {
        outColor = { -1, -1, -1, -1 };
        return;
    }

    // Convert from clip to frame coordinates
    size_t dim = sampleHalfWidth == 0 ? 1 : sampleHalfWidth;
    std::vector<int> pixels;
    camera.clipToFrame(frameX, frameY, frameX, frameY);

    // Convention is that 0, 0 is bottom-left, window_h - 1, window_h - 1 is top-right corner
    // See: 
    // https://community.khronos.org/t/window-coordinates-in-glreadpixels/20931/2
    // For possible speedup, investigate PBOs:
    // http://www.songho.ca/opengl/gl_pbo.html
    camera.frameBuffers().readBuffer().readColorPixels(attachmentIndex, pixels,
        frameX - sampleHalfWidth, frameY - sampleHalfWidth, dim, dim,
        PixelFormat::kRGBA, PixelType::kUByte8);

    // TODO: Implement color averaging when dim != 1x1
    // For now, only the first sampled pixel is used to convert from an int to a color.
    GetChannelId<4>(pixels[0], outColor.array().data());
    //Logger::LogInfo(QString(colorVec));

    return;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void MousePicker::getPixelColor(Vector4 & outColor, const Vector3 & widgetPos, const Camera & camera, const MainRenderer & renderer, size_t attachmentIndex, size_t sampleHalfWidth)
{
    // Check if in bounds
    real_g frameX, frameY;
    if (!camera.getViewport().inBounds(widgetPos, renderer, frameX, frameY)) {
        outColor = { -1, -1, -1, -1 };
        return;
    }

    // Convert from clip to frame coordinates
    size_t dim = sampleHalfWidth == 0 ? 1 : sampleHalfWidth;
    //std::vector<float> pixels;
    camera.clipToFrame(frameX, frameY, frameX, frameY);

    // Convention is that 0, 0 is bottom-left, window_h - 1, window_h - 1 is top-right corner
    // See: 
    // https://community.khronos.org/t/window-coordinates-in-glreadpixels/20931/2
    // For possible speedup, investigate PBOs:
    // http://www.songho.ca/opengl/gl_pbo.html
    camera.frameBuffers().readBuffer().readColorPixels(attachmentIndex, outColor.data(),
        frameX - sampleHalfWidth, frameY - sampleHalfWidth, dim, dim,
        PixelFormat::kRGBA, PixelType::kFloat32);

    //GString ps = GString::FromNumber(pixels[0]);
    //Logger::LogInfo("Float: " + ps);
    //GetChannelId<4>(pixels[0], outColor.array().data());
    //Logger::LogInfo(QString(colorVec));

    return;
}


//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}
