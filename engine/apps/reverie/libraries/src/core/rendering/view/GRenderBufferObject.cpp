#include "core/rendering/view/GRenderBufferObject.h"


#include "core/GCoreEngine.h"
#include <fortress/constants/GConstants.h>
#include "fortress/containers/GColor.h"
#include "core/rendering/view/GFrameBuffer.h"

#define DEBUG_FBO

namespace rev {


// RenderBufferObject

RenderBufferObject::RenderBufferObject(AliasingType format, TextureFormat internalFormat, uint32_t numSamples):
    m_aliasingType(format),
    m_numSamples(numSamples),
    m_internalFormat(internalFormat)
{
    // Get max number of samples to check validity of m_numSamples
    GLint maxNumSamples;
    glGetInternalformativ(
        GL_RENDERBUFFER,
        (int)m_internalFormat,
        GL_SAMPLES, // This will return the max number of samples relating to the internal format, as specified by the OpenGL standard
        1,
        &maxNumSamples
    );

    // Number of samples unsupported, so reduce
    if (m_numSamples > maxNumSamples) {
        Logger::LogWarning(GString::Format("%d samples not supported for the RBO's internal format. Capping at %d", m_numSamples, maxNumSamples));
        m_numSamples = maxNumSamples;
    }

    initializeGL();
}

RenderBufferObject::~RenderBufferObject()
{
    glDeleteRenderbuffers(1, &m_rboID);
}

void RenderBufferObject::bind()
{
    glBindRenderbuffer(GL_RENDERBUFFER, m_rboID);
    m_isBound = true;

#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
    bool error = printGLError("Error, failed to bind RBO");
    if (error) {
        Logger::Throw("Error binding RBO");
    }
#endif
#endif
}

void RenderBufferObject::release()
{
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    m_isBound = false;

#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
    bool error = printGLError("Error, failed to release RBO");
    if (error) {
        Logger::Throw("Error releasing RBO");
    }
#endif
#endif
}

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
        Logger::Throw("Error attaching RBO");
    }
#endif
#endif
}

void RenderBufferObject::setColor(uint32_t w, uint32_t h, uint32_t attachmentIndex)
{
    if (!m_isBound) {
        Logger::Throw("Error, RBO must be bound");
    }

    m_attachmentType = GL_COLOR_ATTACHMENT0 + attachmentIndex;

    switch (m_aliasingType) {
    case AliasingType::kDefault:
    {
        glRenderbufferStorage(GL_RENDERBUFFER, (int)m_internalFormat, w, h);
        break;
    }
    case AliasingType::kMSAA:
    {
#ifdef DEBUG_MODE
        // Asserts based on OpenGL 4.4 specification 
        assert(w <= GL_MAX_RENDERBUFFER_SIZE);
        assert(h <= GL_MAX_RENDERBUFFER_SIZE);
        assert(w >= 0);
        assert(h >= 0);
        assert(m_numSamples >= 0);
#endif

        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 
            m_numSamples,
            (int)m_internalFormat,
            w,
            h);
        break;
    }
    default:
        Logger::Throw("Error, format type not recognized");
        break;
    }

#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
    bool error = printGLError("Error, failed to set color");
    if (error) {
        Logger::Throw("Error initializing RBO as color attachment");
    }
#endif
#endif
}

void RenderBufferObject::setDepthStencil(uint32_t w, uint32_t h)
{
    if (!m_isBound) {
        Logger::Throw("Error, RBO must be bound");
    }

    switch (m_aliasingType) {
    case AliasingType::kDefault:
    {
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
        break;
    }
    case AliasingType::kMSAA:
    {
        glRenderbufferStorageMultisample(GL_RENDERBUFFER,
            m_numSamples,
            GL_DEPTH24_STENCIL8,
            w,
            h);        
        break;
    }
    default:
        Logger::Throw("Error, format type not recognized");
        break;
    }

    m_attachmentType = GL_DEPTH_STENCIL_ATTACHMENT;

#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
    bool error = printGLError("Error, failed to set depth/stencil");
    if (error) {
        Logger::Throw("Error initializing RBO as depth/stencil attachment");
    }
#endif
#endif
}

void RenderBufferObject::initializeGL()
{
    // Create underlying OpenGL RBO
    glGenRenderbuffers(1, &m_rboID);

#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
    bool error = printGLError("Error, failed to create RBO");
    if (error) {
        Logger::Throw("Error creating RBO");
    }
#endif
#endif
}




// End namespaces
}