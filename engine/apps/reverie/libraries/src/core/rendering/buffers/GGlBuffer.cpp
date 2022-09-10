#include "core/rendering/buffers/GGlBuffer.h"

// QT

// Internal
#include "core/rendering/renderer/GRenderContext.h"
#include "fortress/system/memory/GPointerTypes.h"
#include <QOpenGLFunctions_4_4_Core> 

namespace rev {

GlBuffer::GlBuffer():
    gl::OpenGLFunctions(false)
{
}

GlBuffer::GlBuffer(GlBuffer&& other)
{
    *this = std::move(other);
}

GlBuffer::GlBuffer(gl::BufferType type, size_t size, gl::BufferStorageMode storageMode, size_t storageFlags) :
    gl::OpenGLFunctions(true),
    m_bufferType(type),
    m_size(size),
    m_storageMode(storageMode),
    m_storageFlags(storageFlags)
{
}

GlBuffer::~GlBuffer()
{
    destroy();
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

    // Need to make sure OpenGL functions are initialized
    if (m_context) {
        m_context->makeCurrent();
        initializeOpenGLFunctions();
    }

    // Clear ID from other buffer so deletion doesn't remove OpenGL buffer
    other.m_bufferID = s_maxIntSize;
    return *this;
}

void GlBuffer::setRenderContext(RenderContext& context)
{
    // Set context, and ensure that OpenGL functions are initialized
    m_context = &context;
    initializeOpenGLFunctions();
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

    // Bind current buffer
    glBindBuffer((size_t)m_bufferType, m_bufferID);

#ifdef DEBUG_MODE
    error = printGLError("Failed to bind GL Buffer");
    if (error) {
        Logger::Throw("Error, failed to bind GL Buffer");
    }
#endif
}

void GlBuffer::release()
{
    glBindBuffer((size_t)m_bufferType, 0);

#ifdef DEBUG_MODE
    bool error = printGLError("Failed to release GL buffer");;
    if (error) {
        Logger::Throw("Error, failed to release GL buffer");
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
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, (size_t)m_size);
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
        Logger::Throw("Error, already mapped this buffer");
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

    m_data = glMapBufferRange((size_t)m_bufferType,
        offsetBytes,
        sizeBytes,
        (size_t)access //| GL_MAP_PERSISTENT_BIT
    );

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
    bind();

    if (!m_isMapped) {
        Logger::Throw("Error, this buffer is not mapped");
    }

    glUnmapBuffer((size_t)m_bufferType);
    m_isMapped = false;
    if (doRelease) { 
        release(); 
    }
}

void GlBuffer::allocateMemory()
{
    allocateMemory(m_size, m_storageMode, m_storageFlags);
}

void GlBuffer::allocateMemory(size_t size, gl::BufferStorageMode mode, size_t storageFlags)
{
    // Ensure that the buffer is initialized
    if (!hasValidId()) {
        createBuffer();
    }

    bind(); // Ensure that buffer is bound to the current context

    if (!storageFlags) {
        // Allocate dynamic memory for the buffer
        glBufferData((size_t)m_bufferType,
            size,
            NULL, // Data used to initialize
            (size_t)mode);
    }
    else {
        // Allocate immutable memory, using glBufferStorage and the given storage flags
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
#ifdef DEBUG_MODE
    if (m_bindingPoint == s_maxIntSize) {
        assert(m_bindingPoint < s_maxIntSize && "Error, no binding point set for this buffer");
    }
#endif

    bindToPoint(m_bindingPoint);
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

void GlBuffer::initialize(RenderContext& context)
{
#ifdef DEBUG_MODE
    bool error = printGLError("Failed before initializing GL buffer");
    if (error) {
        Logger::Throw("Error, failed before initializing GL buffer");
    }
#endif

    m_context = &context;

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

void GlBuffer::clear()
{    
    bind();

    allocateMemory();

    release();
}

void GlBuffer::destroy()
{
    if (m_bufferID < s_maxIntSize) {
        glDeleteBuffers(1, &m_bufferID);
        m_bufferID = s_maxIntSize;
        m_size = -1;
    }
}





// End namespacing
}