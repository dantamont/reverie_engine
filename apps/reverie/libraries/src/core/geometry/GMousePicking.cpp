#include "core/geometry/GMousePicking.h"

#include <core/scene/GSceneObject.h>
#include <core/rendering/view/GCamera.h>
#include <core/rendering/renderer/GOpenGlRenderer.h>
#include <core/rendering/renderer/GRenderCommand.h>
#include "fortress/numeric/GFloat16.h"

namespace rev {

MousePicker::MousePicker()
{
    static constexpr Uint32_t pixelDataCount = 4; // Number of entries per pixel
    static constexpr Uint32_t pixelEntrySize = sizeof(Float16_t); //< Size of each pixel data entry
    static constexpr Uint32_t s_numSamples = std::max(Uint32_t(1), s_sampleWidth * s_sampleWidth);
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    gl.glGenBuffers(m_pboIds.size(), m_pboIds.data());
    gl.glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pboIds[0]);
    gl.glBufferData(GL_PIXEL_PACK_BUFFER, s_numSamples * pixelDataCount * pixelEntrySize/*data size*/, 0, GL_STREAM_READ);
    gl.glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pboIds[1]);
    gl.glBufferData(GL_PIXEL_PACK_BUFFER, s_numSamples * pixelDataCount * pixelEntrySize/*data size*/, 0, GL_STREAM_READ);
    gl.glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

MousePicker::~MousePicker()
{
}

bool MousePicker::isMouseOver(Renderable * renderable) const
{
    return m_hoverInfo.m_renderablePtr == renderable;
}

bool MousePicker::isMouseOver(SceneObject * sceneObject) const
{
    return m_hoverInfo.m_sceneObjectId == sceneObject->id();
}

void rev::MousePicker::updateMouseOver(const Vector2 & widgetMousePos, const Camera& camera, const OpenGlRenderer & renderer)
{
    // Second color attachment holds normals
    //getPixelColor(m_mouseOverViewNormal, widgetMousePos, camera, renderer, 1, 1);

    // Third color attachment holds unique color IDs
    getPixelColor(m_mouseOverColor, widgetMousePos, camera, renderer, 2);

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
    uint32_t id = (uint32_t)GetColorId<3, int>(m_mouseOverColor.data());
    //Logger::LogInfo("ID: " + GString::FromNumber(id));

    if (id >= renderer.readRenderCommands().size()) {
        return;
    }

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
}


void rev::MousePicker::getPixelColor(Vector<int, 4>& outColor, const Vector3 & widgetPos, const Camera & camera, const OpenGlRenderer & renderer, uint32_t attachmentIndex)
{
    // Check if in bounds
    Real_t frameX, frameY;
    if (!camera.getViewport().inBounds(widgetPos, renderer, frameX, frameY)) {
        outColor = { -1, -1, -1, -1 };
        return;
    }

    static_assert(s_sampleWidth == 0, "Non-zero widths not implemented");

    /// @todo Use my abstractions for this (subclass GlBuffer as PboBuffer).
    // Convert from clip to frame coordinates
    uint32_t dim = s_sampleWidth == 0 ? 1 : s_sampleWidth;
    std::vector<int> pixels; // Since each pixel is 4 bytes, int is just fine
    camera.clipToFrame(frameX, frameY, frameX, frameY);

    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    // "index" is used to read pixels from framebuffer to a PBO
    // "nextIndex" is used to update pixels in the other PBO

    m_pboIndex = (m_pboIndex + 1) % 2;
    Int32_t nextIndex = (m_pboIndex + 1) % 2;

    // read pixels from framebuffer to PBO
    /// @note glReadPixels() should return immediately, since this work happens entirely on the GPU.
    /// @note If a non-zero named buffer object is bound to the GL_PIXEL_PACK_BUFFER 
    /// target (see glBindBuffer) while a block of pixels is requested, data is treated 
    /// as a byte offset into the buffer object's data store rather than a pointer to 
    /// client memory.
    gl.glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pboIds[m_pboIndex]);

    /// @note The pixel type and format here are absolutely essential (particularly the type)
    /// If the type does not match the texture format of the FBO, and the pixel format doesn't 
    /// match the GPU internal storage type (BGRA for modern GPUs), then this will block
    /// the CPU as a conversion is made to the requested type.
    /// @see https://stackoverflow.com/questions/34497195/difference-between-format-and-internalformat
    // Most GPUs store as BGRA internally
    Uint32_t sampleHalfWidth = s_sampleWidth / 2;
    camera.frameBuffers().readBuffer().readColorPixels(attachmentIndex, (void*)0,
        frameX - sampleHalfWidth, frameY - sampleHalfWidth, dim, dim,
        PixelFormat::kBGRA, PixelType::kFloat16);

#ifdef DEBUG_MODE
    const char* errStrUnbind = "Error reading pixels";
    bool error = gl.printGLError(errStrUnbind);
    if (error) {
        Logger::Throw(errStrUnbind);
    }
#endif

    // Map the PBO to process its data by CPU
    // Ideally, the pointer would be a 16 bit float, if that existed
    gl.glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pboIds[nextIndex]);
    Float16_t* ptr = (Float16_t*)gl.glMapBufferRange(
        GL_PIXEL_PACK_BUFFER,
        0,
        dim * dim,
        GL_MAP_READ_BIT); 

    if (ptr)
    {
        // Convert from BGRA half floats to RGBA ints
        outColor[2] = Int32_t(Float32_t(ptr[0]) * 255.0F);
        outColor[1] = Int32_t(Float32_t(ptr[1]) * 255.0F);
        outColor[0] = Int32_t(Float32_t(ptr[2]) * 255.0F);
        outColor[3] = Int32_t(Float32_t(ptr[3]) * 255.0F);
        gl.glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    }

    // back to conventional pixel operation
    gl.glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    m_previousColor = outColor;
    return;
}

    
// End namespaces
}
