#include "GRenderBufferObject.h"


#include "../../GCoreEngine.h"
#include "../../GConstants.h"
#include "../../containers/GColor.h"
#include "../view/GFrameBuffer.h"

#define DEBUG_FBO

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
// RenderBufferObject
/////////////////////////////////////////////////////////////////////////////////////////////
RenderBufferObject::RenderBufferObject(AliasingType format, TextureFormat internalFormat, size_t numSamples):
    m_aliasingType(format),
    m_numSamples(numSamples),
    m_internalFormat(internalFormat)
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

    switch (m_aliasingType) {
    case AliasingType::kDefault:
    {
        glRenderbufferStorage(GL_RENDERBUFFER, (int)m_internalFormat, w, h);
        break;
    }
    case AliasingType::kMSAA:
    {
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 
            m_numSamples,
            (int)m_internalFormat,
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
// End namespaces
}