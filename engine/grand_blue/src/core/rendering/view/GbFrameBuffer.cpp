#include "GbFrameBuffer.h"

#include "../../GbCoreEngine.h"
#include "../../GbConstants.h"
#include "../../containers/GbColor.h"
#include "../view/GbFrameBuffer.h"
#include <QOpenGLFunctions_3_2_Core> 

#define DEBUG_FBO

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
// RenderBufferObject
/////////////////////////////////////////////////////////////////////////////////////////////
RenderBufferObject::RenderBufferObject(InternalBufferFormat format, size_t numSamples):
    m_internalFormat(format),
    m_numSamples(numSamples)
{
    initializeGL();
}
/////////////////////////////////////////////////////////////////////////////////////////////
RenderBufferObject::~RenderBufferObject()
{
    glDeleteRenderbuffers(1, &m_rboID);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RenderBufferObject::bind()
{
    glBindRenderbuffer(GL_RENDERBUFFER, m_rboID);
    m_isBound = true;

#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
    bool error = printGLError("Error, failed to bind RBO");
    if (error) {
        throw("Error binding RBO");
    }
#endif
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RenderBufferObject::release()
{
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    m_isBound = false;

#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
    bool error = printGLError("Error, failed to release RBO");
    if (error) {
        throw("Error releasing RBO");
    }
#endif
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RenderBufferObject::attach()
{
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
        m_attachmentType,
        GL_RENDERBUFFER,
        m_rboID);

#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
    bool error = printGLError("Error, failed to attach RBO");
    if (error) {
        throw("Error attaching RBO");
    }
#endif
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RenderBufferObject::setColor(size_t w, size_t h, size_t attachmentIndex)
{
    if (!m_isBound) {
        throw("Error, RBO must be bound");
    }

    m_attachmentType = GL_COLOR_ATTACHMENT0 + attachmentIndex;

    switch (m_internalFormat) {
    case InternalBufferFormat::kDefault:
    {
        glRenderbufferStorage(GL_RENDERBUFFER, m_attachmentType, w, h);
        break;
    }
    case InternalBufferFormat::kMSAA:
    {
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 
            m_numSamples,
            GL_RGB,
            w,
            h);
        break;
    }
    default:
        throw("Error, format type not recognized");
        break;
    }

#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
    bool error = printGLError("Error, failed to set color");
    if (error) {
        throw("Error initializing RBO as color attachment");
    }
#endif
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RenderBufferObject::setDepthStencil(size_t w, size_t h)
{
    if (!m_isBound) {
        throw("Error, RBO must be bound");
    }

    switch (m_internalFormat) {
    case InternalBufferFormat::kDefault:
    {
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
        break;
    }
    case InternalBufferFormat::kMSAA:
    {
        glRenderbufferStorageMultisample(GL_RENDERBUFFER,
            m_numSamples,
            GL_DEPTH24_STENCIL8,
            w,
            h);        
        break;
    }
    default:
        throw("Error, format type not recognized");
        break;
    }

    m_attachmentType = GL_DEPTH_STENCIL_ATTACHMENT;

#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
    bool error = printGLError("Error, failed to set depth/stencil");
    if (error) {
        throw("Error initializing RBO as depth/stencil attachment");
    }
#endif
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RenderBufferObject::initializeGL()
{
    // Create underlying OpenGL RBO
    glGenRenderbuffers(1, &m_rboID);

#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
    bool error = printGLError("Error, failed to create RBO");
    if (error) {
        throw("Error creating RBO");
    }
#endif
#endif
}



/////////////////////////////////////////////////////////////////////////////////////////////
// FrameBuffer
/////////////////////////////////////////////////////////////////////////////////////////////
FrameBuffer::FrameBuffer(QOpenGLContext* currentContext,
    InternalBufferFormat format, 
    BufferStorageType bufferType,
    size_t numSamples) :
    m_currentContext(currentContext),
    m_internalFormat(format),
    m_bufferType(bufferType),
    m_numSamples(numSamples)
{
    initializeGL();
}
/////////////////////////////////////////////////////////////////////////////////////////////
FrameBuffer::~FrameBuffer()
{
    clearAttachments();

    // Delete the underlying GL framebuffer
    if (m_fboID) {
        glDeleteFramebuffers(1, &m_fboID);
    }

    if (m_blitBuffer) {
        delete m_blitBuffer;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void FrameBuffer::bindColorAttachment(unsigned int idx)
{
    switch (m_internalFormat) {
    case InternalBufferFormat::kDefault:
    {
        if (m_colorTextures.size() <= idx) {
            throw("Error, no texture found at index");
        }
        glBindTexture(GL_TEXTURE_2D, m_colorTextures[idx]);
        break;
    }
    case InternalBufferFormat::kMSAA:
    {
        if (m_bufferType == BufferStorageType::kRBO) {
            // Need to blit (exchange pixels) with another framebuffer (that is not MSAA)
            // In order to bind a texture for rendering
            // See: https://learnopengl.com/Advanced-OpenGL/Anti-Aliasing (has both methods)
            // See: https://stackoverflow.com/questions/46535341/opengl-msaa-in-2-different-ways-what-are-the-differences
            
            if (m_isBound) {
                throw("Error, framebuffer should not be bound for blit");
            }

            bind(BindType::kRead);
            m_blitBuffer->bind(BindType::kWrite);
            blit();
            m_blitBuffer->bindColorAttachment();
            release();
        }
        else {
            if (m_colorTextures.size() <= idx) {
                throw("Error, no texture found at index");
            }
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_colorTextures[idx]);
        }
        break;
    }
    default:
        throw("Error, format type not recognized");
        break;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void FrameBuffer::reinitialize(size_t w, size_t h)
{
    bind();

    m_size = Vector2i( w, h );

    // Create and set color texture as attachment
    setColorAttachment(w, h);

    // Set up RBO for depth and stencil testing
    setDepthStencilAttachment(w, h);

    if (!isComplete()) {
        throw("Error, framebuffer object is incomplete");
    }

    release();

    // Reinitialize blit buffer
    if (m_blitBuffer) {
        m_blitBuffer->reinitialize(w, h);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void FrameBuffer::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
    m_isBound = true;

#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
    bool error = printGLError("Error, failed to bind FBO");
    if (error) {
        throw("Error binding FBO");
    }
#endif
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void FrameBuffer::bind(BindType type)
{
    glBindFramebuffer((unsigned int)type, m_fboID);

#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
    bool error = printGLError("Error, failed to bind FBO");
    if (error) {
        throw("Error binding FBO");
    }
#endif
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void FrameBuffer::clear(const Color& color)
{
#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
    if (!isComplete()) {
        throw("Error, the FBO is incomplete");
    }
#endif
#endif

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());

#ifdef DEBUG_FBO
#ifdef DEBUG_MODE
    bool error = printGLError("Error, failed to clear FBO");
    if (error) {
        throw("Error, failed to clear FBO");
    }
#endif
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void FrameBuffer::blit(unsigned int mask)
{
    // See: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glBlitFramebuffer.xhtml
    glBlitFramebuffer(
        0, // srcX0
        0, // srcY0
        m_size.x(), // srcX1
        m_size.y(), // srxY1
        0, // destX0
        0, // destY0
        m_size.x(), // destX1
        m_size.y(), // destY1
        mask, // mask
        GL_NEAREST); // interpolation if image stretched, must be GL_NEAREST or GL_LINEAR

#ifdef DEBUG_FBO
#ifdef DEBUG_MODE
    bool error = printGLError("Error, failed to blit FBO");
    if (error) {
        throw("Error blitting FBO");
    }
#endif
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void FrameBuffer::release()
{
#ifdef DEBUG_FBO
#ifdef DEBUG_MODE
    bool error = printGLError("Error before releasing FBO");
    if (error) {
        throw("Error releasing FBO");
    }
#endif
#endif

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_isBound = false;

#ifdef DEBUG_FBO
#ifdef DEBUG_MODE
    error = printGLError("Error, failed to release FBO");
    if (error) {
        throw("Error releasing FBO");
    }
#endif
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void FrameBuffer::setColorAttachment(size_t w, size_t h)
{
    if (!m_isBound) {
        throw("Error, FBO must be bound");
    }

    // Clear current color attachments
    clearColorAttachments();

    // Create new color attachment texture
    createColorAttachment(w, h);

    // Attach texture
    switch (m_internalFormat) {
    case InternalBufferFormat::kDefault:
    {
        // Attach texture
        glFramebufferTexture2D(GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0 + (m_colorTextures.size() - 1), // unnecessary since size is reset
            GL_TEXTURE_2D,
            m_colorTextures.back(),
            0);

        break;
    }
    case InternalBufferFormat::kMSAA:
    {
        if (m_bufferType == BufferStorageType::kTexture) {
            // Using texture for MSAA
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0 + (m_colorTextures.size() - 1),
                GL_TEXTURE_2D_MULTISAMPLE,
                m_colorTextures.back(),
                0);
        }
        else {
            // Using RBO for MSAA
            m_colorRBOs.back()->attach();
        }
        break;
    }
    default:
        throw("Error, format type not recognized");
        break;
    }

#ifdef DEBUG_FBO
#ifdef DEBUG_MODE
    bool error = printGLError("Error, failed to set color attachment");
    if (error) {
        throw("Error setting color attachment");
    }
#endif
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void FrameBuffer::setDepthStencilAttachment(size_t w, size_t h)
{
    if (!m_isBound) {
        throw("Error, FBO must be bound");
    }

    // Clear current attachments
    clearDepthStencilAttachments();

    // Create new color attachment texture
    createDepthStencilAttachment(w, h);

    // Attach texture
    switch (m_internalFormat) {
    case InternalBufferFormat::kDefault:
    {
        // If not using texture, skip this and attach RBO
        if (m_bufferType == BufferStorageType::kTexture) {
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                GL_DEPTH_STENCIL_ATTACHMENT,
                GL_TEXTURE_2D,
                m_depthTexture,
                0);
        }
        else {
            m_depthStencilRBO->attach();
        }
        break;
    }
    case InternalBufferFormat::kMSAA:
    {
        if (m_bufferType == BufferStorageType::kTexture) {
            // Using texture for MSAA
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                GL_DEPTH_STENCIL_ATTACHMENT,
                GL_TEXTURE_2D_MULTISAMPLE,
                m_depthTexture,
                0);
        }
        else {
            m_depthStencilRBO->attach();
        }
        break;
    }
    default:
        throw("Error, format type not recognized");
        break;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool FrameBuffer::isComplete()
{
    bool bound = m_isBound;
    if (!bound) {
        bind();
    }

    bool valid;
    unsigned int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status == GL_FRAMEBUFFER_COMPLETE) {
        valid = true;
    }
    else {
        valid = false;
        if (status == GL_FRAMEBUFFER_UNDEFINED) {
            logError("Framebuffer undefined");
        }
        else if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
            logError("Framebuffer has incomplete attachment");
        }
        else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
            // returned if the framebuffer does not have at least one image attached to it
            logError("Framebuffer has missing attachment");
        }
        else if (status == GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER) {
            logError("Framebuffer has incomplete draw buffer");
        }
        else if (status == GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER) {
            logError("Framebuffer has incomplete read buffer");
        }
        else if (status == GL_FRAMEBUFFER_UNSUPPORTED) {
            logError("Framebuffer unsupported");
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
            logError("Framebuffer has incomplete multisamples");
        }
        else if (status == GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS) {
            logError("Framebuffer has incomplete layer targets");
        }
    }

    if (!bound) {
        release();
    }

    return valid;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void FrameBuffer::initializeGL()
{
    // Create framebuffer in GL and set ID 
    glGenFramebuffers(1, &m_fboID);

    // Initialize blitting framebuffer if type dictates
    if (m_bufferType == BufferStorageType::kRBO && 
        m_internalFormat == InternalBufferFormat::kMSAA) {
        if (m_blitBuffer) {
            throw("Error, blit buffer should be uninitialized");
        }
        m_blitBuffer = new FrameBuffer(m_currentContext,
            InternalBufferFormat::kDefault,
            FrameBuffer::BufferStorageType::kTexture);
    }

    // Initialize so that the framebuffer is complete
    reinitialize(1, 1);

#ifdef DEBUG_FBO
#ifdef DEBUG_MODE
    bool error = printGLError("Error, failed to create FBO");
    if (error) {
        throw("Error creating FBO");
    }
#endif
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void FrameBuffer::createColorAttachment(size_t w, size_t h)
{
    switch (m_internalFormat) {
        case InternalBufferFormat::kDefault:
        {
            m_colorTextures.push_back(0);
            unsigned int& colorTexture = m_colorTextures.back();
            createColorTexture(w, h, colorTexture);
            break;
        }
        case InternalBufferFormat::kMSAA:
        {
            if (m_bufferType == BufferStorageType::kRBO) {
                // Use RBO for MSAA
                m_colorRBOs.push_back(std::make_shared<RenderBufferObject>(m_internalFormat, m_numSamples));
                auto colorBuffer = m_colorRBOs.back();
                colorBuffer->bind();
                colorBuffer->setColor(w, h, m_colorRBOs.size() - 1);
                colorBuffer->release(); // can unbind since memory has been allocated
            }
            else {
                m_colorTextures.push_back(0);
                unsigned int& colorTexture = m_colorTextures.back();
                createColorTextureMSAA(w, h, colorTexture);
            }
            break;
        }
        default:
            throw("Error, format type not recognized");
            break;
    }

#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
    bool error = printGLError("Error, failed to create color attachment");
    if (error) {
        throw("Error creating color attachment");
    }
#endif
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void FrameBuffer::createColorTexture(size_t w, size_t h, unsigned int & outTextureIndex)
{
    // Create texture
    glGenTextures(1, &outTextureIndex);

    glBindTexture(GL_TEXTURE_2D, outTextureIndex);

    glTexImage2D(GL_TEXTURE_2D,
        0, // mipmap level
        GL_RGB8, // internal format to store texture in
        w, // GL screen width in pixels
        h, // GL screen height in pixels
        0, // border, always 0, legacy
        GL_RGB, // format of source image (pixel data)
        GL_UNSIGNED_INT,  // data type of source image (affects resolution), could be GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT
        NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

#ifdef DEBUG_FBO
#ifdef DEBUG_MODE
    bool error = printGLError("Error, failed to create color texture");
    if (error) {
        throw("Error failed to create color texture");
    }
#endif
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void FrameBuffer::createColorTextureMSAA(size_t w, size_t h, unsigned int & outIndex)
{
    // See: https://doc.qt.io/qt-5/qopenglcontext.html#versionFunctions
    // https://stackoverflow.com/questions/38818382/qt5-gltexstorage2d-glbindimagetexture-undefined
    // Could also inherit from a newer core profile, but this makes it clear
    // Which methods rely on newer versions of OpenGL
    QOpenGLFunctions_3_2_Core* funcs = nullptr;
    funcs = m_currentContext->versionFunctions<QOpenGLFunctions_3_2_Core>();

    // Create texture
    glGenTextures(1, &outIndex);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, outIndex);

    funcs->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
        m_numSamples, // GL_TEXTURE_SAMPLES 
        GL_RGB8,
        w,
        h,
        true); //  GL_TEXTURE_FIXED_SAMPLE_LOCATIONS, must be true if mixing with render buffers in FBO

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void FrameBuffer::createDepthStencilAttachment(size_t w, size_t h)
{
    switch (m_internalFormat) {
    case InternalBufferFormat::kDefault:
    {
        // If not using texture, skip this and use RBO
        if (m_bufferType == BufferStorageType::kTexture) {
            createDepthStencilTexture(w, h, m_depthTexture);
            m_stencilTexture = m_depthTexture; // Denotes that depth and stencil share the same texture
        }
        else {
            m_depthStencilRBO = std::make_shared<RenderBufferObject>(m_internalFormat, m_numSamples);
            m_depthStencilRBO->bind();
            m_depthStencilRBO->setDepthStencil(w, h);
            m_depthStencilRBO->release(); // can unbind since memory has been allocated
        }
        break;
    }
    case InternalBufferFormat::kMSAA:
    {
        if (m_bufferType == BufferStorageType::kTexture) {
            // See: https://doc.qt.io/qt-5/qopenglcontext.html#versionFunctions
            // https://stackoverflow.com/questions/38818382/qt5-gltexstorage2d-glbindimagetexture-undefined
            // Could also inherit from a newer core profile, but this makes it clear
            // Which methods rely on newer versions of OpenGL
            QOpenGLFunctions_3_2_Core* funcs = nullptr;
            funcs = m_currentContext->versionFunctions<QOpenGLFunctions_3_2_Core>();

            // Create texture
            glGenTextures(1, &m_depthTexture);
            m_stencilTexture = m_depthTexture; // Denotes that depth and stencil share the same texture

            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_depthTexture);

            funcs->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
                m_numSamples, // GL_TEXTURE_SAMPLES 
                GL_DEPTH24_STENCIL8,
                w,
                h,
                false); //  GL_TEXTURE_FIXED_SAMPLE_LOCATIONS, must be true if mixing with render buffers in FBO

            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
        }
        else {
            m_depthStencilRBO = std::make_shared<RenderBufferObject>(m_internalFormat, m_numSamples);
            m_depthStencilRBO->bind();
            m_depthStencilRBO->setDepthStencil(w, h);
            m_depthStencilRBO->release(); // can unbind since memory has been allocated
        }
        break;
    }
    default:
        throw("Error, format type not recognized");
        break;
    }

#ifdef DEBUG_FBO
#ifdef DEBUG_MODE
    bool error = printGLError("Error, failed to create depth/stencil attachment");
    if (error) {
        throw("Error failed to create depth/stencil attachment");
    }
#endif
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void FrameBuffer::createDepthStencilTexture(size_t w, size_t h, unsigned int & outTextureIndex)
{
    // Create texture
    glGenTextures(1, &outTextureIndex);

    glBindTexture(GL_TEXTURE_2D, outTextureIndex);

    glTexImage2D(GL_TEXTURE_2D,
        0, // mipmap level
        GL_DEPTH24_STENCIL8, // internal format to store texture in
        w, // GL screen width in pixels
        h, // GL screen height in pixels
        0, // border, always 0, legacy
        GL_DEPTH_STENCIL, // format of source image (pixel data)
        GL_UNSIGNED_INT_24_8,  // data type of source image (affects resolution)
        NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

#ifdef DEBUG_FBO
#ifdef DEBUG_MODE
    bool error = printGLError("Error, failed to create depth/stencil texture");
    if (error) {
        throw("Error failed to create depth/stencil texture");
    }
#endif
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void FrameBuffer::clearColorAttachments()
{
    // Clear color textures (RBOs delete themselves)
    for (const auto& tex : m_colorTextures) {
        glDeleteTextures(1, &tex);
    }
    m_colorTextures.clear();
    m_colorRBOs.clear();

#ifdef DEBUG_FBO
#ifdef DEBUG_MODE
    bool error = printGLError("Error, failed to clear color textures");
    if (error) {
        throw("Error failed to clear color textures");
    }
#endif
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void FrameBuffer::clearDepthStencilAttachments()
{
    // Clear depth and stencil textures
    if (m_depthTexture == m_stencilTexture && m_depthTexture != 0) {
        glDeleteTextures(1, &m_depthTexture);
    }
    else {
        if (m_depthTexture) {
            glDeleteTextures(1, &m_depthTexture);
        }

        if (m_stencilTexture) {
            glDeleteTextures(1, &m_stencilTexture);
        }
    }

    m_depthStencilRBO = nullptr;

#ifdef DEBUG_FBO
#ifdef DEBUG_MODE
    bool error = printGLError("Error, failed to clear depth/stencil texture");
    if (error) {
        throw("Error failed to clear depth/stencil texture");
    }
#endif
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void FrameBuffer::clearAttachments()
{
    clearColorAttachments();
    clearDepthStencilAttachments();
}





/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}