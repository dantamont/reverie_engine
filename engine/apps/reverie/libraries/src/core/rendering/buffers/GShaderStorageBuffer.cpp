#include "core/rendering/buffers/GShaderStorageBuffer.h"
#include "core/rendering/renderer/GRenderContext.h"
#include "core/rendering/shaders/GUniform.h"

// QT

// Internal
#include "fortress/system/memory/GPointerTypes.h"

namespace rev {

ShaderStorageBuffer::ShaderStorageBuffer()
{
}

ShaderStorageBuffer::ShaderStorageBuffer(ShaderStorageBuffer&& other):
    GlBuffer(std::move(other))
{
}

ShaderStorageBuffer::ShaderStorageBuffer(RenderContext & context, size_t size, gl::BufferStorageMode storageMode, size_t storageFlags):
    GlBuffer(gl::BufferType::kShaderStorage, size, storageMode, storageFlags)
{
    initialize(context);
}

ShaderStorageBuffer::~ShaderStorageBuffer()
{
}

ShaderStorageBuffer & ShaderStorageBuffer::operator=(ShaderStorageBuffer && other)
{
    GlBuffer::operator=(std::move(other));
    return *this;
}

void ShaderStorageBuffer::bind()
{
#ifdef DEBUG_MODE
    size_t ms = maxSize();
    if (m_size > ms) {
        Logger::Throw("Error, size is greater than maximum allowed SSBO size");
    }
#endif

    GlBuffer::bind();
}

void ShaderStorageBuffer::release()
{
    GlBuffer::release();
}

size_t ShaderStorageBuffer::maxSize()
{
    GLint max;
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &max);

    return (size_t)max;
}



// End namespacing
}