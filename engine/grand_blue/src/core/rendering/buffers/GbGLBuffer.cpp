#include "GbGLBuffer.h"

// QT

// Internal
#include "../renderer/GbRenderContext.h"
#include "../../utils/GbMemoryManager.h"
#include <QOpenGLFunctions_4_4_Core> 

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
GLBuffer::GLBuffer()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
GLBuffer::GLBuffer(RenderContext & context, GL::BufferType type, size_t size, GL::BufferStorageMode storageMode, size_t storageFlags) :
    m_context(&context),
    m_bufferType(type),
    m_size(size),
    m_storageMode(storageMode),
    m_storageFlags(storageFlags)
{
    initialize();
}
/////////////////////////////////////////////////////////////////////////////////////////////
GLBuffer::~GLBuffer()
{
    if (m_bufferID < MAX_UNSIGNED_INT) {
        glDeleteBuffers(1, &m_bufferID);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
GLBuffer & GLBuffer::operator=(GLBuffer && other)
{
    // Set attributes
    m_bufferID = other.m_bufferID;
    m_bindingPoint = other.m_bindingPoint;
    m_size = other.m_size;
    m_isMapped = other.m_isMapped;
    m_data = other.m_data;
    m_bufferType = other.m_bufferType;
    m_storageMode = other.m_storageMode;
    m_storageFlags = other.m_storageFlags;
    m_context = other.m_context;

    // Clear ID from other buffer so deletion doesn't remove OpenGL buffer
    other.m_bufferID = MAX_UNSIGNED_INT;
    return *this;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool GLBuffer::isBound() const
{
    return m_context->boundBuffer(m_bufferType) == m_uuid;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLBuffer::bind()
{
    //GLuint boundBuffer = getBoundBuffer();

    if (!m_context->isCurrent()) {
        m_context->makeCurrent();
    }

#ifdef DEBUG_MODE
    bool error = printGLError("Error before buffer bind");
    if (error) {
        throw("Error before buffer bind");
    }
#endif

#ifdef DEBUG_MODE
    if (isBound()) {
        GLuint boundBuffer = getBoundBuffer();
        if (!boundBuffer) {
            throw("Something has gone awry");
        }
    }
#endif

    if (!isBound()) {
        // Release previously bound buffer
        if (!m_context->boundBuffer(m_bufferType).isNull()) {
            glBindBuffer((size_t)m_bufferType, 0);

#ifdef DEBUG_MODE
            bool error = printGLError("Failed to release previous GL Buffer");
            if (error) {
                throw("Error, failed to release previous GL Buffer");
            }
#endif
        }

        GLuint boundBuffer = getBoundBuffer();

        // Bind current buffer
        glBindBuffer((size_t)m_bufferType, m_bufferID);
        m_context->setBoundBuffer(m_bufferType, m_uuid);

        boundBuffer = getBoundBuffer();

#ifdef DEBUG_MODE
        error = printGLError("Failed to bind GL Buffer");
        if (error) {
            throw("Error, failed to bind GL Buffer");
        }
#endif
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLBuffer::release()
{
    if (!isBound()) {
        throw("Error, buffer was not bound");
    }
    glBindBuffer((size_t)m_bufferType, 0);

#ifdef DEBUG_MODE
    GLint boundBuffer = getBoundBuffer();
    if (boundBuffer) {
        throw("Error, buffer should not be bound");
    }
#endif

    m_context->setBoundBuffer(m_bufferType, Uuid::NullID());

#ifdef DEBUG_MODE
    bool error = printGLError("Failed to release SSB");;
    if (error) {
        throw("Error, failed to release SSB");
    }
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLBuffer::copyInto(GLBuffer & other)
{
    if (m_size != other.m_size) {
        throw("Error, buffer size mismatch");
    }

    if (m_bufferID == other.m_bufferID) {
        throw("Error, buffer copying into itself");
    }

    glBindBuffer(GL_COPY_READ_BUFFER, m_bufferID);
    glBindBuffer(GL_COPY_WRITE_BUFFER, other.m_bufferID);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, m_size);
    glBindBuffer(GL_COPY_READ_BUFFER, 0);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

#ifdef DEBUG_MODE
    bool error = printGLError("Failed to copy into SSBO");;
    if (error) {
        throw("Error, failed to copy into SSBO");
    }
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
void * GLBuffer::map(size_t access, bool doBind)
{
    return map(0, m_size, access, doBind);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void * GLBuffer::map(size_t offsetBytes, size_t sizeBytes, size_t access, bool doBind)
{
    if (m_isMapped) {
        throw("Error, already smapped this buffer");
        return m_data;
    }

#ifdef DEBUG_MODE
    bool error = printGLError("Failed before binding buffer");;
    if (error) {
        throw("Failed before binding buffer");
    }
#endif


    if (doBind) { bind(); }

    if (!m_context->isCurrent()) {
        throw("Error, context is not current");
    }


#ifdef DEBUG_MODE
    error = printGLError("Failed to bind buffer before mapping");;
    if (error) {
        throw("Failed to bind buffer before mapping");
    }
#endif

    // TODO: Look into glBufferSubData instead, probably faster
    m_data = glMapBufferRange((size_t)m_bufferType,
        offsetBytes,
        sizeBytes,
        (size_t)access //| GL_MAP_PERSISTENT_BIT
    );

    //#ifdef DEBUG_MODE
    //    error = printGLError(QStringLiteral("Failed to map buffer");;
    //    if (error) {
    //        throw("Failed to bind buffer before mapping");
    //    }
    //#endif

#ifdef DEBUG_MODE
    if (!m_data) {
        printGLError("Failed to map buffer range");
        throw("Error, failed to map buffer");
    }
#endif  

    // Mark the buffer as mapped
    m_isMapped = true;

    return m_data;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLBuffer::unmap(bool doRelease)
{
    // Need to bind this guy to unmap it
    bool wasBound = isBound();
    if (!wasBound) {
        bind();
    }

    if (!m_isMapped) {
        throw("Error, this buffer is not mapped");
    }

    glUnmapBuffer((size_t)m_bufferType);
    m_isMapped = false;
    if (doRelease || !wasBound) { release(); }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLBuffer::allocateMemory()
{
    allocateMemory(m_size, m_storageMode, m_storageFlags);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLBuffer::allocateMemory(size_t size, GL::BufferStorageMode mode, size_t storageFlags)
{
    bind(); // Ensure that buffer is bound to the current context

    if (!isBound()) {
        throw("Error, GL Buffer was not bound");
    }

    if (!storageFlags) {
        // Allocate dynamic memory for the buffer
        glBufferData((size_t)m_bufferType,
            size,
            NULL, // Data used to initialize
            (size_t)mode);
    }
    else {
        // Allocate immutable memory, using glBufferStorage
        QOpenGLFunctions_4_4_Core* functions = m_context->context()->versionFunctions<QOpenGLFunctions_4_4_Core>();
        functions->initializeOpenGLFunctions();
        if (!functions) {
            throw("Error, 4_4 core functions not supported");
        }
        functions->glBufferStorage((size_t)m_bufferType, 
            size,
            NULL,
            storageFlags
            );
    }

#ifdef DEBUG_MODE
    bool error = printGLError("Failed to allocate memory");
    if (error) {
        throw("Failed to allocate memory");
    }
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLBuffer::bindToPoint()
{
    if (m_bindingPoint < MAX_UNSIGNED_INT) {
        bindToPoint(m_bindingPoint);
    }
    else {
        throw("Error, no binding point set for this buffer");
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLBuffer::bindToPoint(size_t bindingPoint)
{
    //bind(); // Already done by glBindBufferBase

#ifdef DEBUG_MODE
    bool error = printGLError("Error, failed to bind buffer");
    if (error) {
        throw("Error, failed to bind buffer");
    }
#endif

    m_bindingPoint = bindingPoint;
    glBindBufferBase((size_t)m_bufferType, bindingPoint, m_bufferID);
    m_context->setBoundBuffer(m_bufferType, m_uuid); // Set bound buffer to this

#ifdef DEBUG_MODE
    error = printGLError("Error, failed to bind buffer to point");
    if (error) {
        throw("Error, failed to bind buffer to point");
    }
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLBuffer::bindToPoint(size_t bindingPoint, size_t offset, size_t sizeInBytes)
{
    bind();
    m_bindingPoint = bindingPoint;
    glBindBufferRange((size_t)m_bufferType,
        bindingPoint, 
        m_bufferID, 
        offset, 
        sizeInBytes);

#ifdef DEBUG_MODE
    bool error = printGLError("Error, failed to bind buffer to point");
    if (error) {
        throw("Error, failed to bind buffer to point");
    }
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////
void GLBuffer::releaseFromPoint()
{
    if (m_bindingPoint < MAX_UNSIGNED_INT) {
        releaseFromPoint(m_bindingPoint);
    }
    else {
        throw("Error, no binding point set for this buffer");
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLBuffer::releaseFromPoint(size_t bufferPoint)
{
    // Bind not necessary
    //bind();

    if (m_bindingPoint != bufferPoint) {
        throw("Error, binding point mismatch");
    }
    m_bindingPoint = bufferPoint;
    glBindBufferBase((size_t)m_bufferType, bufferPoint, 0);

    // Will unbind buffer in GL, so need to capture this on CPU side
    m_context->setBoundBuffer(m_bufferType, Uuid::NullID());

#ifdef DEBUG_MODE
    bool error = printGLError("Error, failed to release buffer from point");
    if (error) {
        throw("Error, failed to release buffer from point");
    }
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
size_t GLBuffer::count(size_t stride)
{
    float c = m_size / stride;
    if (c != floor(c)) {
        throw("Error, buffer size is incompatible with the specified type");
    }

    return (size_t)c;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLBuffer::initialize()
{
#ifdef DEBUG_MODE
    bool error = printGLError("Failed before initializing GL buffer");
    if (error) {
        throw("Error, failed before initializing GL buffer");
    }
#endif

    createBuffer();

    allocateMemory();

}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLBuffer::createBuffer()
{
    glGenBuffers(1, &m_bufferID);

#ifdef DEBUG_MODE
    bool error = printGLError("Failed to create GL buffer");
    if (error) {
        throw("Error, failed to create GL buffer");
    }
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
GLint GLBuffer::getBoundBuffer()
{
    GL::BufferBindType bindType;
    switch (m_bufferType) {
    case GL::BufferType::kShaderStorage:
        bindType = GL::BufferBindType::kShaderStorageBufferBinding;
        break;
    case GL::BufferType::kUniformBuffer:
        bindType = GL::BufferBindType::kUniformBufferBinding;
        break;
    default:
        throw("Error, type not implemented");
    }

    GLint outType;
    glGetIntegerv((size_t)bindType, &outType);

    return outType;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLBuffer::clear()
{
    bool wasBound = isBound();
    
    if (!wasBound) {
        bind();
    }
    allocateMemory();

    if (!wasBound) {
        release();
    }
}




/////////////////////////////////////////////////////////////////////////////////////////////
// End namespacing
}