#include "core/rendering/view/GFrameBuffer.h"

#include <QOpenGLFunctions_3_2_Core> 

#include "core/GCoreEngine.h"
#include <fortress/constants/GConstants.h>
#include "fortress/containers/GColor.h"

#include "core/rendering/geometry/GMesh.h"
#include "core/rendering/materials/GTexture.h"
#include "core/rendering/models/GModel.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/buffers/GUniformBufferObject.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"
#include "core/rendering/shaders/GUniformContainer.h"
#include "core/resource/GResourceCache.h"
#include "core/rendering/geometry/GPolygon.h"
#include "core/components/GCameraComponent.h"

#define DEBUG_FBO

namespace rev {


// FrameBuffer

FrameBuffer::FrameBuffer()
{
}

FrameBuffer::FrameBuffer(const FrameBuffer & other):
    m_currentContext(other.m_currentContext),
    m_aliasingType(other.m_aliasingType),
    m_attachmentType(other.m_attachmentType),
    m_numSamples(other.m_numSamples),
    m_textureFormat(other.m_textureFormat),
    m_numColorAttachments(other.m_numColorAttachments),
    m_hasDepth(other.m_hasDepth)
{
    initializeGL();
    switch (other.m_attachmentType) {
    case BufferAttachmentType::kTexture:
        reinitialize(other.m_size.x(), other.m_size.y(), other.m_colorTextures,
            other.m_depthStencilTexture, other.m_depthStencilAttachmentLayer);
        break;
    default:
        Logger::Throw("Copy is not supported for non-texture framebuffer type");
        break;
    }
}

FrameBuffer::FrameBuffer(QOpenGLContext* currentContext,
    AliasingType format, 
    BufferAttachmentType bufferType,
    TextureFormat textureFormat,
    uint32_t numSamples,
    uint32_t numColorAttachments,
    bool hasDepth) :
    m_currentContext(currentContext),
    m_aliasingType(format),
    m_attachmentType(bufferType),
    m_numSamples(numSamples),
    m_textureFormat(textureFormat),
    m_numColorAttachments(numColorAttachments),
    m_hasDepth(hasDepth)
{
    initializeGL();
}

FrameBuffer::~FrameBuffer()
{
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();

#ifdef DEBUG_MODE
    bool error = gl.printGLError("Failed before deleting framebuffer");
    if (error) {
        Logger::LogError("Failed before deleting framebuffer");
    }
#endif

    clearAttachments();

    // Delete the underlying GL framebuffer
    if (m_fboID) {
        gl.glDeleteFramebuffers(1, &m_fboID);
    }

    if (m_blitBuffer) {
        delete m_blitBuffer;
    }
}

FrameBuffer & FrameBuffer::operator=(const FrameBuffer & other)
{
    m_currentContext = other.m_currentContext;
    m_aliasingType = other.m_aliasingType;
    m_attachmentType = other.m_attachmentType;
    m_numSamples = other.m_numSamples;
    m_textureFormat = other.m_textureFormat;
    m_numColorAttachments = other.m_numColorAttachments;

    initializeGL(false); // Don't call reinitialize yet
    switch (other.m_attachmentType) {
    case BufferAttachmentType::kTexture:
        reinitialize(other.m_size.x(), other.m_size.y(), other.m_colorTextures,
            other.m_depthStencilTexture, other.m_depthStencilAttachmentLayer);
        break;
    default:
        Logger::Throw("Copy is not supported for non-texture framebuffer type");
        break;
    }

    return *this;
}

void FrameBuffer::drawQuad(Camera& camera, ShaderProgram& shaderProgram, uint32_t attachmentIndex)
{
    //if (!m_size.lengthSquared()) {
    //    // Return if size is uninitialized
    //    return;
    //}
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();

    // TODO: Maybe just make this a singleton
    CoreEngine* core = shaderProgram.handle()->engine();

    // Obtain quad
    Mesh* quad = ResourceCache::Instance().polygonCache()->getSquare();

    //// Obtain shader
    //auto quadShaderProgram = ResourceCache::Instance().getHandleWithName("quad",
    //    EResourceType::eShaderProgram)->resourceAs<ShaderProgram>();

    // Bind shader and set uniforms
    /// @todo Only set underlying values when they actually change
    UniformContainer& uc = shaderProgram.handle()->engine()->openGlRenderer()->renderContext().uniformContainer();
    float clipDepth = camera.viewport().getClipDepth();
    m_uniforms.m_quadOffsets.setValue(Vector3(camera.viewport().m_xn, camera.viewport().m_yn, clipDepth), uc);
    m_uniforms.m_quadScale.setValue(Vector2(camera.viewport().m_width, camera.viewport().m_height), uc);

    shaderProgram.bind();
    shaderProgram.setUniformValue(shaderProgram.uniformMappings().m_offsets, m_uniforms.m_quadOffsets);
    shaderProgram.setUniformValue(shaderProgram.uniformMappings().m_scale, m_uniforms.m_quadScale);
    shaderProgram.updateUniforms(uc);

#ifdef DEBUG_MODE
    bool error = gl.printGLError("Camera::drawFrameBufferQuad:: Error binding shader");
    if (error) {
        Logger::LogInfo("Error in Camera::drawFrameBufferQuad");
        //logInfo("Error in Camera::drawFrameBufferQuad: " + m_name);
    }
#endif

    // Bind texture
    bindColorTexture(0, attachmentIndex);
    //bindColorTexture(0, attachmentIndex);
    //bindDepthTexture(1);

#ifdef DEBUG_MODE
    error = gl.printGLError("Camera::drawFrameBufferQuad:: Error binding texture");
    if (error) {
        Logger::LogInfo("Error in Camera::drawFrameBufferQuad");
        //logInfo("Error in Camera::drawFrameBufferQuad: " + m_name);
    }
#endif

    // Draw the quad
    quad->vertexData().drawGeometry(PrimitiveMode::kTriangles, 1);

    // Release shader
    shaderProgram.release();
}

void FrameBuffer::saveColorToFile(uint32_t attachmentIndex, const GString & filepath, PixelType pixelType)
{
    static tsl::robin_map<int, QImage::Format> formatMap = {
        {GL_RGB16F, QImage::Format_RGB888},
        {GL_RGB8, QImage::Format_RGB888},
        {GL_RGBA8, QImage::Format_RGBA8888},
        {GL_RGBA16F, QImage::Format_RGBA8888},
        {GL_R8, QImage::Format_Grayscale8},
        {GL_RED, QImage::Format_Grayscale8}
    };

    /// \see https://www.khronos.org/opengl/wiki/GLAPI/glReadPixels
    // Official reference page is incomplete/wrong
    static tsl::robin_map<int, PixelFormat> pixelFormatMap = {
        {GL_RGB16F, PixelFormat::kRGB},
        {GL_RGB8, PixelFormat::kRGB},
        {GL_RGBA16F, PixelFormat::kRGBA},
        {GL_RGBA8, PixelFormat::kRGBA},
        //{GL_R8, PixelFormat::kRed_Integer},
        //{GL_RED, PixelFormat::kRed_Integer}
        {GL_R8, PixelFormat::kRed},
        {GL_RED, PixelFormat::kRed}
    };


    if (!Map::HasKey(formatMap, (int)m_textureFormat)) {
        Logger::Throw("Internal type unsupported");
    }
    QImage::Format format = formatMap[(int)m_textureFormat];
    PixelFormat pixelFormat = pixelFormatMap[(int)m_textureFormat];

    std::vector<int> pixels;
    readColorPixels(attachmentIndex, pixels, pixelFormat, pixelType);
    QImage image((const unsigned char*)pixels.data(), m_size.x(), m_size.y(), format);
    image.mirrored().save(filepath.c_str());
}

void FrameBuffer::saveDepthToFile(const GString & filepath)
{
    // Read depth
    std::vector<float> pixels;
    readDepthPixels(pixels);

    // TODO: Render to a quad and do this in depth_to_screen.frag shader
    // Normalize and convert to color
    float zNear = 0.1f;
    float zFar = 1000.0f;
    float linearizedDepth;
    uint32_t depthInt;
    uint32_t size = (uint32_t)pixels.size();
    std::vector<UINT8> outPixels;
    outPixels.resize(size);
    uint32_t maxDepth = 0;
    uint32_t minDepth = 1000;
    for (uint32_t i = 0; i < size; i++) {
        linearizedDepth = RenderProjection::LinearizedDepth(pixels[i], zNear, zFar) / zFar;
        UINT8& currentDepth = outPixels[i];
        depthInt = 255 * linearizedDepth;
        currentDepth = (UINT8)depthInt;
        minDepth = std::min((uint32_t)currentDepth, minDepth);
        maxDepth = std::max((uint32_t)currentDepth, maxDepth);
        //currentDepth = 0x000000FF;
        //currentDepth |= depthInt << 24;
        //currentDepth |= depthInt << 16;
        //currentDepth |= depthInt << 8;
    }
    QImage image((const unsigned char*)outPixels.data(), m_size.x(), m_size.y(), QImage::Format_Grayscale8);
    image.mirrored().save(filepath.c_str());
}

void FrameBuffer::readDepthPixels(std::vector<float>& outDepths) const
{
    bind();
    outDepths.resize(m_size.x() * m_size.y(), -1);

    switch (m_aliasingType) {
    case AliasingType::kDefault:
    {
        // Read pixels directly
        //glReadBuffer(GL_NONE);
        glReadPixels(
            0,
            0,
            m_size.x(),
            m_size.y(),
            GL_DEPTH_COMPONENT,
            GL_FLOAT,
            outDepths.data()
        );
        break;
    }
    case AliasingType::kMSAA:
    {
        if (m_attachmentType == BufferAttachmentType::kRBO) {
            // RBO needed for MSAA
            // Need to blit (exchange pixels) with another framebuffer (that is not MSAA)
            blit(BlitMask::kDepthBit, *m_blitBuffer);
            m_blitBuffer->readDepthPixels(outDepths);
        }
        else {
            Logger::Throw("Error, MSAA FBOs with textures are not supported by current drivers");
        }
        break;
    }
    default:
        Logger::Throw("Error, format type not recognized");
        break;
    }

#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    bool error = gl.printGLError("Error, failed to read depth pixels");
    if (error) {
        Logger::Throw("Error, failed to read depth pixels");
    }
#endif
#endif

    release();
}

void FrameBuffer::bindColorTexture(unsigned int texUnit, unsigned int attachmentIndex)
{
    // Return if no color textures
    //if (!m_colorTextures.size()) {
    //    return;
    //}

    switch (m_aliasingType) {
    case AliasingType::kDefault:
    {
        if (m_colorTextures.size() <= attachmentIndex) {
            Logger::Throw("Error, no texture found at index");
        }

        // Bind texture
        m_colorTextures[attachmentIndex]->bind(texUnit);
        break;
    }
    case AliasingType::kMSAA:
    {
        if (m_attachmentType == BufferAttachmentType::kRBO) {
            // Need to blit (exchange pixels) with another framebuffer (that is not MSAA)
            // In order to bind a texture for rendering
            /// \see https://learnopengl.com/Advanced-OpenGL/Anti-Aliasing (has both methods)
            /// \see https://stackoverflow.com/questions/46535341/opengl-msaa-in-2-different-ways-what-are-the-differences
            
            if (m_isBound) {
                Logger::Throw("Error, framebuffer should not be bound for blit");
            }

            blit(BlitMask::kColorBit, *m_blitBuffer, attachmentIndex);
            m_blitBuffer->bindColorTexture(texUnit, attachmentIndex);
        }
        else {
            if (m_colorTextures.size() <= attachmentIndex) {
                Logger::Throw("Error, no texture found at index");
            }
            //glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_colorTextures[attachmentIndex]);
            m_colorTextures[attachmentIndex]->bind(texUnit);
        }
        break;
    }
    default:
        Logger::Throw("Error, format type not recognized");
        break;
    }

#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    bool error = gl.printGLError("Error, failed to bind color attachment");
    if (error) {
        Logger::Throw("Error binding color attachment");
    }
#endif
#endif
}

void FrameBuffer::bindDepthTexture(unsigned int texUnit)
{
    switch (m_aliasingType) {
    case AliasingType::kDefault:
    {
        // Bind texture
        m_depthStencilTexture->bind(texUnit);

        // This is actually the default, so not strictly necessary
        m_depthStencilTexture->setReadMode(DepthStencilMode::kDepthComponent);

        break;
    }
    case AliasingType::kMSAA:
    {
        if (m_attachmentType == BufferAttachmentType::kRBO) {
            // Need to blit (exchange pixels) with another framebuffer (that is not MSAA)
            // In order to bind a texture for rendering
            /// \see https://learnopengl.com/Advanced-OpenGL/Anti-Aliasing (has both methods)
            /// \see https://stackoverflow.com/questions/46535341/opengl-msaa-in-2-different-ways-what-are-the-differences

            if (m_isBound) {
                Logger::Throw("Error, framebuffer should not be bound for blit");
            }

            blit(BlitMask::kDepthBit, *m_blitBuffer);
            m_blitBuffer->bindDepthTexture(texUnit);
        }
        else {
            m_depthStencilTexture->bind(texUnit);
        }
        break;
    }
    default:
        Logger::Throw("Error, format type not recognized");
        break;
    }

#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    bool error = gl.printGLError("Error, failed to bind depth attachment");
    if (error) {
        Logger::Throw("Error binding depth attachment");
    }
#endif
#endif
}

void FrameBuffer::reinitialize(uint32_t w, uint32_t h, bool ignoreIncomplete)
{
    bind();

    m_size = Vector2u( w, h );

    // Create and set color texture as attachment
    setColorAttachments(w, h);

    // Set up RBO for depth and stencil testing
    if (m_hasDepth) {
        setDepthStencilAttachment(w, h);
    }

    bool complete = isComplete();
    if (!complete) {
        if (!ignoreIncomplete) {
            Logger::Throw("Error, framebuffer object is incomplete");
        }
    }

    release();

    // Reinitialize blit buffer
    if (m_blitBuffer && complete) {
        m_blitBuffer->reinitialize(w, h, ignoreIncomplete);
    }
}

void FrameBuffer::reinitialize(uint32_t w, uint32_t h, const std::vector<std::shared_ptr<Texture>>& colorAttachments, const std::shared_ptr<Texture>& depthStencilAttachment, uint32_t depthLayer)
{
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();

    if (m_attachmentType != BufferAttachmentType::kTexture) {
        Logger::Throw("Error, framebuffer does not use textures");
    }

    bind();

    m_size = Vector2u(w, h);

    clearColorAttachments();
    clearDepthStencilAttachments();

    // Set color textures as attachments
    m_colorTextures = colorAttachments;
    uint32_t numTextures = (uint32_t)m_colorTextures.size();
    for (uint32_t i = 0; i < numTextures; i++) {
        m_colorTextures[i]->attach(*this, i);
    }

    if (m_colorTextures.size() == 0) {
        // Since FBO is not complete without a colorbuffer, need to explicitly
        // tell OpenGL that we're not rendering any color data
        glDrawBuffer(GL_NONE); // Not supported in OpenGL ES
        gl.glReadBuffer(GL_NONE);
    }
    else {
        m_numColorAttachments = (uint32_t)m_colorTextures.size();
        for (uint32_t i = 0; i < m_numColorAttachments; i++) {
            m_colorTextures[i]->attach(*this, GL_COLOR_ATTACHMENT0 + i);
        }
    }

    // Set up texture for depth and stencil testing
    if (depthStencilAttachment) {
        // Use given attachment
        m_depthStencilTexture = depthStencilAttachment;
        m_depthStencilAttachmentLayer = depthLayer;
        m_depthStencilTexture->attach(*this, GL_DEPTH_STENCIL_ATTACHMENT, depthLayer);
    }
    else {
        // Set up empty texture for depth and stencil testing
        setDepthStencilAttachment(w, h);
    }

    if (!isComplete()) {
        Logger::Throw("Error, framebuffer object is incomplete");
    }

    release();
}

void FrameBuffer::reinitialize(uint32_t w, uint32_t h, const std::vector<std::shared_ptr<Texture>>& colorAttachments)
{
    reinitialize(w, h, colorAttachments, nullptr, 0);
}

void FrameBuffer::reinitialize(uint32_t w, uint32_t h, const std::shared_ptr<Texture>& depthStencilAttachment, uint32_t depthLayer)
{
    reinitialize(w, h, {}, depthStencilAttachment, depthLayer);
}

void FrameBuffer::bind() const
{
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();

#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
    QOpenGLContext* cc = QOpenGLContext::currentContext();
    if (m_currentContext != cc) {
        Logger::Throw("Current context is not current");
    }

    bool error = gl.printGLError("Error, before binding FBO");
    if (error) {
        Logger::Throw("Error before binding FBO");
    }
#endif
#endif

    gl.glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
    m_isBound = true;

#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
    error = gl.printGLError("Error, failed to bind FBO");
    if (error) {
        Logger::Throw("Error binding FBO");
    }
#endif
#endif

    // If has multiple color attachments, set draw buffers
    /// \see https://stackoverflow.com/questions/51030120/concept-what-is-the-use-of-gldrawbuffer-and-gldrawbuffers
    // Note, glDrawBuffers links to layout qualifiers, so this assumes that
    // attachment points correspond 1:1 with qualifiers
    std::vector<GLenum> buffers;
    buffers.reserve(m_numColorAttachments);
    for (uint32_t i = 0; i < m_numColorAttachments; i++) {
        buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
    }
    gl.glDrawBuffers(m_numColorAttachments, buffers.data());

#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
    error = gl.printGLError("Error, failed to set draw buffers");
    if (error) {
        Logger::Throw("Errorsetting draw buffers");
    }
#endif
#endif
}

void FrameBuffer::bind(BindType type) const
{
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    gl.glBindFramebuffer((unsigned int)type, m_fboID);
    m_isBound = true;

#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
    bool error = gl.printGLError("Error, failed to bind FBO");
    if (error) {
        Logger::Throw("Error binding FBO");
    }
#endif
#endif
}

void FrameBuffer::clear()
{
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();

#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
    if (!isComplete()) {
        Logger::LogWarning("Error, the FBO is incomplete");
        return;
    }
#endif

    if (!m_isBound) {
        Logger::Throw("Error, did not bind buffer before clear");
    }
#endif

    gl.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

#ifdef DEBUG_FBO
#ifdef DEBUG_MODE
    bool error = gl.printGLError("Error, failed to clear FBO");
    if (error) {
        Logger::Throw("Error, failed to clear FBO");
    }
#endif
#endif
}

void FrameBuffer::clear(const Color& color)
{
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    gl.glClearColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    clear();
}

void FrameBuffer::clear(const Color & color, uint32_t attachmentIndex)
{
    // UNTESTED, buyer beware
#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
    if (!isComplete()) {
        Logger::LogWarning("Error, the FBO is incomplete");
        return;
    }

    if (!m_isBound) {
        Logger::Throw("Error, did not bind buffer before clear");
    }
#endif
#endif

    /// \see https://stackoverflow.com/questions/44756898/opengl-different-clear-color-for-individual-color-attachments
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();

    glDrawBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
    gl.glClearColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    gl.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

#ifdef DEBUG_FBO
#ifdef DEBUG_MODE
    bool error = gl.printGLError((const char*)("Error, failed to clear FBO attachment " + GString::FromNumber(attachmentIndex)));
    if (error) {
        Logger::Throw("Error, failed to clear FBO");
    }
#endif
#endif
}

void FrameBuffer::blit(BlitMask mask, uint32_t readAttachmentIndex, const std::vector<uint32_t>& drawAttachmentIndices) const
{
    // No blitting to be done if framebuffer is uninitialized
    if (m_size.lengthSquared() == 0) {
        return;
    }

    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();

    // For color attachments not at 0, need to edit current color buffer
    /// \see https://stackoverflow.com/questions/17691702/does-glblitframebuffer-copy-all-color-attachments-if-gl-color-buffer-bit-mask-is
    // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glReadBuffer.xhtml
    if (mask == BlitMask::kColorBit) {
        // Set read buffer
        gl.glReadBuffer(GL_COLOR_ATTACHMENT0 + readAttachmentIndex);
        
        // Set draw buffers
        uint32_t numDrawAttachments = (uint32_t)drawAttachmentIndices.size();
        std::vector<GLenum> drawBuffers;
        drawBuffers.reserve(drawAttachmentIndices.size());
        for (uint32_t i = 0; i < numDrawAttachments; i++) {
            drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + drawAttachmentIndices[i]);
        }
        gl.glDrawBuffers(1, drawBuffers.data());
    }

    /// \see https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glBlitFramebuffer.xhtml
    gl.glBlitFramebuffer(
        0, // srcX0
        0, // srcY0
        m_size.x(), // srcX1
        m_size.y(), // srxY1
        0, // destX0
        0, // destY0
        m_size.x(), // destX1
        m_size.y(), // destY1
        (GLint)mask, // mask
        GL_NEAREST); // interpolation if image stretched, must be GL_NEAREST or GL_LINEAR

#ifdef DEBUG_FBO
#ifdef DEBUG_MODE
    bool error = gl.printGLError("Error, failed to blit FBO");
    if (error) {
        Logger::Throw("Error blitting FBO");
    }
#endif
#endif
}

void FrameBuffer::blit(BlitMask blitMask, FrameBuffer & other, uint32_t colorAttachment) const
{
    blit(blitMask, other, colorAttachment, { colorAttachment });
}

void FrameBuffer::blit(BlitMask blitMask, FrameBuffer & other, uint32_t readAttachmentIndex, const std::vector<uint32_t>& drawAttachmentIndices) const
{
    // Blit contents of current framebuffer into other
    bind(BindType::kRead);
    other.bind(BindType::kWrite);
    blit(blitMask, readAttachmentIndex, drawAttachmentIndices);

    // Release as read/write framebuffers
    other.release();
    release();
}

void FrameBuffer::Release()
{
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();

#ifdef DEBUG_FBO
#ifdef DEBUG_MODE
    bool error = gl.printGLError("Error before releasing FBO");
    if (error) {
        Logger::Throw("Error releasing FBO");
    }
#endif
#endif

    gl::OpenGLFunctions::Functions()->glBindFramebuffer(GL_FRAMEBUFFER, 0);

#ifdef DEBUG_FBO
#ifdef DEBUG_MODE
    error = gl.printGLError("Error, failed to release FBO");
    if (error) {
        Logger::Throw("Error releasing FBO");
    }
#endif
#endif
}

void FrameBuffer::release() const
{
    Release();
    m_isBound = false;
}

void FrameBuffer::setColorAttachments(uint32_t w, uint32_t h)
{
    if (!m_isBound) {
        Logger::Throw("Error, FBO must be bound");
    }

    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();

    // Clear current color attachments
    clearColorAttachments();

    if (m_numColorAttachments == 0) {
        // Since FBO is not complete without a colorbuffer, need to explicitly
        // tell OpenGL that we're not rendering any color data
        glDrawBuffer(GL_NONE); // Not supported in OpenGL ES
        gl.glReadBuffer(GL_NONE);

#ifdef DEBUG_FBO
#ifdef DEBUG_MODE
        bool error = gl.printGLError("Error, failed to set color attachment");
        if (error) {
            Logger::Throw("Error setting color attachment");
        }
#endif
#endif

        return;
    }

    // Create new color attachment texture
    for (uint32_t i = 0; i < m_numColorAttachments; i++) {
        createColorAttachment(w, h);
    }

#ifdef DEBUG_FBO
#ifdef DEBUG_MODE
    bool error = gl.printGLError("Error, failed to set color attachment");
    if (error) {
        Logger::Throw("Error setting color attachment");
    }
#endif
#endif
}

void FrameBuffer::setDepthStencilAttachment(uint32_t w, uint32_t h)
{
    if (!m_isBound) {
        Logger::Throw("Error, FBO must be bound");
    }

    // Clear current attachments
    clearDepthStencilAttachments();

    // Create new color attachment texture
    createDepthStencilAttachment(w, h);
}

bool FrameBuffer::isComplete()
{
    bool bound = m_isBound;
    if (!bound) {
        bind();
    }

    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();

    bool valid;
    unsigned int status = gl.glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status == GL_FRAMEBUFFER_COMPLETE) {
        valid = true;
    }
    else {
        valid = false;
        const char* errorStr;
        if (status == GL_FRAMEBUFFER_UNDEFINED) {
            errorStr = "Framebuffer undefined";
        }
        else if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
            errorStr = "Framebuffer has incomplete attachment";
        }
        else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
            // returned if the framebuffer does not have at least one image attached to it
            errorStr = "Framebuffer has missing attachment";
        }
        else if (status == GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER) {
            errorStr = "Framebuffer has incomplete draw buffer";
        }
        else if (status == GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER) {
            errorStr = "Framebuffer has incomplete read buffer";
        }
        else if (status == GL_FRAMEBUFFER_UNSUPPORTED) {
            errorStr = "Framebuffer unsupported";
        }
        else if (status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE) {
            // GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE is returned if the value 
            // of GL_RENDERBUFFER_SAMPLES is not the same for all attached 
            // renderbuffers; if the value of GL_TEXTURE_SAMPLES is the not same
            // for all attached textures; or , if the attached images are a mix 
            // of renderbuffers and textures, the value of GL_RENDERBUFFER_SAMPLES 
            // does not match the value of GL_TEXTURE_SAMPLES.

            // GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE is also returned if the
            // value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not the same for 
            // all attached textures; or, if the attached images are a mix of 
            // renderbuffers and textures, the value of
            // GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not GL_TRUE for all attached 
            // textures. 
            errorStr = "Framebuffer has incomplete multisamples";
        }
        else if (status == GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS) {
            errorStr = "Framebuffer has incomplete layer targets";
        }
        Logger::LogError(errorStr);
    }

    if (!bound) {
        release();
    }

    return valid;
}

void FrameBuffer::initializeGL(bool reinitialize_)
{
    // Create framebuffer in GL and set ID 
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    gl.glGenFramebuffers(1, &m_fboID);

    // Initialize blitting framebuffer if type dictates
    if (m_attachmentType == BufferAttachmentType::kRBO && 
        m_aliasingType == AliasingType::kMSAA) {
        if (m_blitBuffer) {
            Logger::Throw("Error, blit buffer should be uninitialized");
        }

        // ----------------------------------------------------
        // CREATE BLIT BUFFER
        // ----------------------------------------------------
        m_blitBuffer = new FrameBuffer(m_currentContext,
            AliasingType::kDefault,
            FrameBuffer::BufferAttachmentType::kTexture,
            m_textureFormat,
            m_numSamples, 
            m_numColorAttachments);
    }

    // Initialize so that the framebuffer is complete
    // Don't bother initializing, doesn't need to be complete until sized
    if (reinitialize_) {
        reinitialize(1, 1);
    }
#ifdef DEBUG_FBO
#ifdef DEBUG_MODE
    bool error = gl.printGLError("Error, failed to create FBO");
    if (error) {
        Logger::Throw("Error creating FBO");
    }
#endif
#endif
}

void FrameBuffer::createColorAttachment(uint32_t w, uint32_t h)
{
    switch (m_aliasingType) {
        case AliasingType::kDefault:
        {
            auto texture = std::make_shared<Texture>(
                w,
                h,
                TextureTargetType::k2D,
                TextureUsageType::kNone,
                TextureFilter::kLinear,
                TextureFilter::kLinear,
                TextureWrapMode::kRepeat,
                m_textureFormat
                );
            texture->setNumSamples(m_numSamples);
            texture->postConstruction();
            m_colorTextures.push_back(texture);
            break;
        }
        case AliasingType::kMSAA:
        {
            if (m_attachmentType == BufferAttachmentType::kRBO) {
                // Use RBO for MSAA
                m_colorRBOs.push_back(std::make_shared<RenderBufferObject>(m_aliasingType, m_textureFormat, m_numSamples));
                auto colorBuffer = m_colorRBOs.back();
                colorBuffer->bind();
                colorBuffer->setColor(w, h, (uint32_t)m_colorRBOs.size() - 1);
                colorBuffer->release(); // can unbind since memory has been allocated
            }
            else {
                auto texture = std::make_shared<Texture>(
                    w,
                    h,
                    TextureTargetType::k2DMultisample,
                    TextureUsageType::kNone,
                    TextureFilter::kNearest,
                    TextureFilter::kNearest,
                    TextureWrapMode::kRepeat,
                    m_textureFormat
                    );
                texture->setNumSamples(m_numSamples);
                texture->postConstruction();
                m_colorTextures.push_back(texture);
            }
            break;
        }
        default:
            Logger::Throw("Error, format type not recognized");
            break;
    }

    // Attach texture
    switch (m_aliasingType) {
    case AliasingType::kDefault:
    {
        // Attach texture
        m_colorTextures.back()->attach(*this,
            GL_COLOR_ATTACHMENT0 + ((uint32_t)m_colorTextures.size() - 1)); // unnecessary since size is reset

        break;
    }
    case AliasingType::kMSAA:
    {
        if (m_attachmentType == BufferAttachmentType::kTexture) {
            // Using texture for MSAA
            m_colorTextures.back()->attach(*this,
                GL_COLOR_ATTACHMENT0 + ((uint32_t)m_colorTextures.size() - 1)); // unnecessary since size is reset
        }
        else {
            // Using RBO for MSAA
            m_colorRBOs.back()->attach();
        }
        break;
    }
    default:
        Logger::Throw("Error, format type not recognized");
        break;
    }

#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    bool error = gl.printGLError("Error, failed to create color attachment");
    if (error) {
        Logger::Throw("Error creating color attachment");
    }
#endif
#endif
}

void FrameBuffer::createDepthStencilAttachment(uint32_t w, uint32_t h)
{
    switch (m_aliasingType) {
    case AliasingType::kDefault:
    {
        // If not using texture, skip this and use RBO
        if (m_attachmentType == BufferAttachmentType::kTexture) {
            auto texture = std::make_shared<Texture>(
                w,
                h,
                TextureTargetType::k2D,
                TextureUsageType::kNone,
                TextureFilter::kLinear,
                TextureFilter::kLinear,
                TextureWrapMode::kRepeat,
                FBO_DEFAULT_DEPTH_PRECISION
                );
            texture->setNumSamples(m_numSamples);
            texture->postConstruction();
            m_depthStencilTexture = texture;
        }
        else {
            m_depthStencilRBO = std::make_shared<RenderBufferObject>(
                m_aliasingType, 
                FBO_DEFAULT_DEPTH_PRECISION, 
                m_numSamples);
            m_depthStencilRBO->bind();
            m_depthStencilRBO->setDepthStencil(w, h);
            m_depthStencilRBO->release(); // can unbind since memory has been allocated
        }
        break;
    }
    case AliasingType::kMSAA:
    {
        if (m_attachmentType == BufferAttachmentType::kTexture) {
            /// \see https://doc.qt.io/qt-5/qopenglcontext.html#versionFunctions
            // https://stackoverflow.com/questions/38818382/qt5-gltexstorage2d-glbindimagetexture-undefined
            auto texture = std::make_shared<Texture>(
                w,
                h,
                TextureTargetType::k2DMultisample,
                TextureUsageType::kNone,
                TextureFilter::kLinear,
                TextureFilter::kLinear,
                TextureWrapMode::kRepeat,
                FBO_DEFAULT_DEPTH_PRECISION
                );
            texture->setNumSamples(m_numSamples);
            texture->setFixedSampleLocations(false); //  GL_TEXTURE_FIXED_SAMPLE_LOCATIONS, must be true if mixing with render buffers in FBO
            texture->postConstruction();
            m_depthStencilTexture = texture;
        }
        else {
            m_depthStencilRBO = std::make_shared<RenderBufferObject>(m_aliasingType, FBO_DEFAULT_DEPTH_PRECISION, m_numSamples);
            m_depthStencilRBO->bind();
            m_depthStencilRBO->setDepthStencil(w, h);
            m_depthStencilRBO->release(); // can unbind since memory has been allocated
        }
        break;
    }
    default:
        Logger::Throw("Error, format type not recognized");
        break;
    }

#ifdef DEBUG_FBO
#ifdef DEBUG_MODE
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    bool error = gl.printGLError("Error, failed to create depth/stencil attachment");
    if (error) {
        Logger::Throw("Error failed to create depth/stencil attachment");
    }
#endif
#endif

    // Attach texture
    switch (m_aliasingType) {
    case AliasingType::kDefault:
    case AliasingType::kMSAA:
    {
        // If not using texture, skip this and attach RBO
        if (m_attachmentType == BufferAttachmentType::kTexture) {
            // For MSAA mode, attach will handle different target type
            m_depthStencilTexture->attach(*this, GL_DEPTH_STENCIL_ATTACHMENT);
        }
        else {
            m_depthStencilRBO->attach();
        }
        break;
    }
    default:
        Logger::Throw("Error, format type not recognized");
        break;
    }

#ifdef DEBUG_FBO
#ifdef DEBUG_MODE
    if (error) {
        Logger::Throw("Error failed to attach depth/stencil attachment");
    }
#endif
#endif
}


void FrameBuffer::clearColorAttachments()
{
    // Clear color textures (RBOs delete themselves)
    m_colorTextures.clear();
    m_colorRBOs.clear();

#ifdef DEBUG_FBO
#ifdef DEBUG_MODE
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    bool error = gl.printGLError("Error, failed to clear color textures");
    if (error) {
        Logger::Throw("Error failed to clear color textures");
    }
#endif
#endif
}

void FrameBuffer::clearDepthStencilAttachments()
{
    // Clear depth and stencil textures
    if (m_depthStencilTexture) {
        m_depthStencilTexture = nullptr;
    }

    m_depthStencilRBO = nullptr;

#ifdef DEBUG_FBO
#ifdef DEBUG_MODE
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    bool error = gl.printGLError("Error, failed to clear depth/stencil texture");
    if (error) {
        Logger::Throw("Error failed to clear depth/stencil texture");
    }
#endif
#endif
}

void FrameBuffer::clearAttachments()
{
#ifdef DEBUG_FBO
#ifdef DEBUG_MODE
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    bool error = gl.printGLError("Error, before clearing attachments");
    if (error) {
        Logger::Throw("Error failure before clearing attachments");
    }
#endif
#endif

    clearColorAttachments();
    clearDepthStencilAttachments();
}






// End namespaces
}