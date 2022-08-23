#include "core/rendering/buffers/GGlBuffer.h"

// QT

// Internal
#include "core/rendering/renderer/GRenderContext.h"
#include "fortress/system/memory/GPointerTypes.h"
#include <QOpenGLFunctions_4_4_Core> 

namespace rev {

GlBuffer::GlBuffer()
{
}

GlBuffer::GlBuffer(RenderContext & context, gl::BufferType type, size_t size, gl::BufferStorageMode storageMode, size_t storageFlags) :
    m_context(&context),
    m_bufferType(type),
    m_size(size),
    m_storageMode(storageMode),
    m_storageFlags(storageFlags)
{
    initialize();
}

GlBuffer::~GlBuffer()
{
    if (m_bufferID < s_maxIntSize) {
        glDeleteBuffers(1, &m_bufferID);
    }
}

GlBuffer & GlBuffer::operator=(GlBuffer && other)
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
    other.m_bufferID = s_maxIntSize;
    return *this;
}

bool GlBuffer::isBound() const
{
    return m_context->boundBuffer(m_bufferType) == m_uuid;
}

void GlBuffer::bind()
{
    //GLuint boundBuffer = getBoundBuffer();

    if (!m_context->isCurrent()) {
        m_context->makeCurrent();
    }

#ifdef DEBUG_MODE
    bool error = printGLError("Error before buffer bind");
    if (error) {
        Logger::Throw("Error before buffer bind");
    }
#endif

#ifdef DEBUG_MODE
    if (isBound()) {
        GLuint boundBuffer = getBoundBuffer();
        if (!boundBuffer) {
            Logger::Throw("Something has gone awry");
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
                Logger::Throw("Error, failed to release previous GL Buffer");
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
            Logger::Throw("Error, failed to bind GL Buffer");
        }
#endif
    }
}

void GlBuffer::release()
{
    if (!isBound()) {
        Logger::Throw("Error, buffer was not bound");
    }
    glBindBuffer((size_t)m_bufferType, 0);

#ifdef DEBUG_MODE
    GLint boundBuffer = getBoundBuffer();
    if (boundBuffer) {
        Logger::Throw("Error, buffer should not be bound");
    }
#endif

    m_context->setBoundBuffer(m_bufferType, Uuid::NullID());

#ifdef DEBUG_MODE
    bool error = printGLError("Failed to release SSB");;
    if (error) {
        Logger::Throw("Error, failed to release SSB");
    }
#endif
}

void GlBuffer::copyInto(GlBuffer & other)
{
    if (m_size != other.m_size) {
        Logger::Throw("Error, buffer size mismatch");
    }

    if (m_bufferID == other.m_bufferID) {
        Logger::Throw("Error, buffer copying into itself");
    }

    glBindBuffer(GL_COPY_READ_BUFFER, m_bufferID);
    glBindBuffer(GL_COPY_WRITE_BUFFER, other.m_bufferID);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, m_size);
    glBindBuffer(GL_COPY_READ_BUFFER, 0);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

#ifdef DEBUG_MODE
    bool error = printGLError("Failed to copy into SSBO");;
    if (error) {
        Logger::Throw("Error, failed to copy into SSBO");
    }
#endif
}


void * GlBuffer::map(size_t access, bool doBind)
{
    return map(0, m_size, access, doBind);
}

void * GlBuffer::map(size_t offsetBytes, size_t sizeBytes, size_t access, bool doBind)
{
    if (m_isMapped) {
        Logger::Throw("Error, already smapped this buffer");
        return m_data;
    }

#ifdef DEBUG_MODE
    bool error = printGLError("Failed before binding buffer");;
    if (error) {
        Logger::Throw("Failed before binding buffer");
    }
#endif


    if (doBind) { bind(); }

    if (!m_context->isCurrent()) {
        Logger::Throw("Error, context is not current");
    }


#ifdef DEBUG_MODE
    error = printGLError("Failed to bind buffer before mapping");;
    if (error) {
        Logger::Throw("Failed to bind buffer before mapping");
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
    //        Logger::Throw("Failed to bind buffer before mapping");
    //    }
    //#endif

#ifdef DEBUG_MODE
    if (!m_data) {
        printGLError("Failed to map buffer range");
        Logger::Throw("Error, failed to map buffer");
    }
#endif  

    // Mark the buffer as mapped
    m_isMapped = true;

    return m_data;
}

void GlBuffer::unmap(bool doRelease)
{
    // Need to bind this guy to unmap it
    bool wasBound = isBound();
    if (!wasBound) {
        bind();
    }

    if (!m_isMapped) {
        Logger::Throw("Error, this buffer is not mapped");
    }

    glUnmapBuffer((size_t)m_bufferType);
    m_isMapped = false;
    if (doRelease || !wasBound) { release(); }
}

void GlBuffer::allocateMemory()
{
    allocateMemory(m_size, m_storageMode, m_storageFlags);
}

void GlBuffer::allocateMemory(size_t size, gl::BufferStorageMode mode, size_t storageFlags)
{
    bind(); // Ensure that buffer is bound to the current context

    if (!isBound()) {
        Logger::Throw("Error, GL Buffer was not bound");
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
            Logger::Throw("Error, 4_4 core functions not supported");
        }
        functions->glBufferStorage((GLenum)m_bufferType, 
            size,
            NULL,
            storageFlags
            );
    }

#ifdef DEBUG_MODE
    bool error = printGLError("Failed to allocate memory");
    if (error) {
        Logger::Throw("Failed to allocate memory");
    }
#endif
}

void GlBuffer::bindToPoint()
{
    if (m_bindingPoint < s_maxIntSize) {
        bindToPoint(m_bindingPoint);
    }
    else {
        Logger::Throw("Error, no binding point set for this buffer");
    }
}

void GlBuffer::bindToPoint(size_t bindingPoint)
{
    //bind(); // Already done by glBindBufferBase

#ifdef DEBUG_MODE
    bool error = printGLError("Error, failed to bind buffer");
    if (error) {
        Logger::Throw("Error, failed to bind buffer");
    }
#endif

    m_bindingPoint = bindingPoint;
    glBindBufferBase((size_t)m_bufferType, bindingPoint, m_bufferID);
    m_context->setBoundBuffer(m_bufferType, m_uuid); // Set bound buffer to this

#ifdef DEBUG_MODE
    error = printGLError("Error, failed to bind buffer to point");
    if (error) {
        Logger::Throw("Error, failed to bind buffer to point");
    }
#endif
}

void GlBuffer::bindToPoint(size_t bindingPoint, size_t offset, size_t sizeInBytes)
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
        Logger::Throw("Error, failed to bind buffer to point");
    }
#endif
}

void GlBuffer::releaseFromPoint()
{
    if (m_bindingPoint < s_maxIntSize) {
        releaseFromPoint(m_bindingPoint);
    }
    else {
        Logger::Throw("Error, no binding point set for this buffer");
    }
}

void GlBuffer::releaseFromPoint(size_t bufferPoint)
{
    // Bind not necessary
    //bind();

    if (m_bindingPoint != bufferPoint) {
        Logger::Throw("Error, binding point mismatch");
    }
    m_bindingPoint = bufferPoint;
    glBindBufferBase((uint32_t)m_bufferType, bufferPoint, 0);

    // Will unbind buffer in GL, so need to capture this on CPU side
    m_context->setBoundBuffer(m_bufferType, Uuid::NullID());

#ifdef DEBUG_MODE
    bool error = printGLError("Error, failed to release buffer from point");
    if (error) {
        Logger::Throw("Error, failed to release buffer from point");
    }
#endif
}

size_t GlBuffer::count(size_t stride)
{
    float c = m_size / stride;
    if (c != floor(c)) {
        Logger::Throw("Error, buffer size is incompatible with the specified type");
    }

    return (size_t)c;
}

void GlBuffer::initialize()
{
#ifdef DEBUG_MODE
    bool error = printGLError("Failed before initializing GL buffer");
    if (error) {
        Logger::Throw("Error, failed before initializing GL buffer");
    }
#endif

    createBuffer();

    allocateMemory();

}

void GlBuffer::createBuffer()
{
    glGenBuffers(1, &m_bufferID);

#ifdef DEBUG_MODE
    bool error = printGLError("Failed to create GL buffer");
    if (error) {
        Logger::Throw("Error, failed to create GL buffer");
    }
#endif
}

GLint GlBuffer::getBoundBuffer()
{
    gl::BufferBindType bindType;
    switch (m_bufferType) {
    case gl::BufferType::kShaderStorage:
        bindType = gl::BufferBindType::kShaderStorageBufferBinding;
        break;
    case gl::BufferType::kUniformBuffer:
        bindType = gl::BufferBindType::kUniformBufferBinding;
        break;
    default:
        Logger::Throw("Error, type not implemented");
    }

    GLint outType;
    glGetIntegerv((size_t)bindType, &outType);

    return outType;
}

void GlBuffer::clear()
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





// End namespacing
}