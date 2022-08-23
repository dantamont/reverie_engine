#pragma once

// Standard
#include <cstdlib>
#include <memory>
#include <vector>

// QT
#include <QObject>
#include <QString>

// Internal 
#include "fortress/types/GLoadable.h"
#include "fortress/types/GNameable.h"
#include "fortress/types/GIdentifiable.h"
#include "fortress/containers/GVariant.h"
#include "fortress/containers/math/GMatrix.h"
#include "fortress/containers/GContainerExtensions.h"
#include "logging/GLogger.h"
#include "core/rendering/GGLFunctions.h"

namespace rev {

typedef std::vector<Vector3> Vec3List;
typedef std::vector<Vector4> Vec4List;

class RenderContext;

namespace gl {
    enum class BufferAccessType {
        kRead = GL_READ_ONLY,
        kWrite = GL_WRITE_ONLY,
        kReadWrite = GL_READ_WRITE
    };

    enum RangeBufferAccessFlag {
        kRead = GL_MAP_READ_BIT,
        kWrite = GL_MAP_WRITE_BIT,
        kInvalidateRange = GL_MAP_INVALIDATE_RANGE_BIT,
        kInvalidateBuffer = GL_MAP_INVALIDATE_BUFFER_BIT,
        kFlushExplicit = GL_MAP_FLUSH_EXPLICIT_BIT,
        kUnsynchronized = GL_MAP_UNSYNCHRONIZED_BIT,
        kPersistent = GL_MAP_PERSISTENT_BIT,
        kCoherent = GL_MAP_COHERENT_BIT
    };
    typedef QFlags<RangeBufferAccessFlag> RangeBufferaccess;

    enum class BufferBlockType {
        kInvalid = -1,
        kUniformBuffer = GL_UNIFORM_BLOCK, // Storage comes from a UBO
        kShaderStorage = GL_SHADER_STORAGE_BLOCK // Storage comes from an SSB
    };

    // See: https://www.khronos.org/opengl/wiki/Type_Qualifier_(GLSL)
    enum class BufferMemoryQualifier {
        kNone = -1,
        kCoherent,
        kVolatile,
        kRestrict,
        kReadOnly,
        kWriteOnly
    };

    enum class BufferType {
        kInvalid = -1,
        kArrayBuffer = GL_ARRAY_BUFFER,
        kAtomicCounterBuffer = GL_ATOMIC_COUNTER_BUFFER,
        kCopyReadBuffer = GL_COPY_READ_BUFFER,
        kCopyWriteBuffer = GL_COPY_WRITE_BUFFER,
        kDispatchIndirectBuffer = GL_DISPATCH_INDIRECT_BUFFER,
        kDrawIndirectBuffer = GL_DRAW_INDIRECT_BUFFER,
        kElementArrayBuffer = GL_ELEMENT_ARRAY_BUFFER,
        kPixelPackBuffer = GL_PIXEL_PACK_BUFFER,
        kPixelUnpackBuffer = GL_PIXEL_UNPACK_BUFFER,
        kQueryBuffer = GL_QUERY_BUFFER,
        kUniformBuffer = GL_UNIFORM_BUFFER,
        kShaderStorage = GL_SHADER_STORAGE_BUFFER,
        kTextureBuffer = GL_TEXTURE_BUFFER,
        kTransformFeedbackBuffer = GL_TRANSFORM_FEEDBACK_BUFFER
    };

    enum class BufferBindType {
        kShaderStorageBufferBinding = GL_SHADER_STORAGE_BUFFER_BINDING,
        kUniformBufferBinding = GL_UNIFORM_BUFFER_BINDING
    };

    enum class BufferStorageMode {
        kStreamDraw = GL_STREAM_DRAW,
        kStreamRead = GL_STREAM_READ,
        kStreamCopy = GL_STREAM_COPY,
        kStaticDraw = GL_STATIC_DRAW,
        kStaticRead = GL_STATIC_READ,
        kStaticCopy = GL_STATIC_COPY,
        kDynamicDraw = GL_DYNAMIC_DRAW,
        kDynamicRead = GL_DYNAMIC_READ,
        kDynamicCopy = GL_DYNAMIC_COPY
    };

    // See: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glBufferStorage.xhtml
    enum class BufferStorageFlag {
        kReadable = GL_MAP_READ_BIT, // Data store may be mapped by client for read access
        kWriteable = GL_MAP_WRITE_BIT, // Data store may be mapped by client for write access
        kPersistent = GL_MAP_PERSISTENT_BIT, // Requires read or write. Client may request that the server read/write from the buffer while it is mapped
        kCoherent = GL_MAP_COHERENT_BIT, // Requires kPersistent. Shared access to buffers that are simultaneously mapped for client access will be coherent as long as mapping is performed using glMapBufferRange, i.e., data written to the store by either the client or server will be immediately visible to the other
        kDynamicStorage = GL_DYNAMIC_STORAGE_BIT, // Contents may be update through calls to glBufferSubData
        kClientStorage = GL_CLIENT_STORAGE_BIT // Determine whether to use storage that is local to the server or client to serve as backing store for buffer
    };
}


/// @class GlBuffer
/// @brief Class representing a GL Buffer
class GlBuffer : 
    public IdentifiableInterface, 
    public NameableInterface,
    protected gl::OpenGLFunctions{
public:
    /// @name Constructors/Destructor
    /// @{
    /// @}

    GlBuffer();
    GlBuffer(RenderContext& context, gl::BufferType type, size_t sizeInBytes, gl::BufferStorageMode storageMode = gl::BufferStorageMode::kDynamicDraw, size_t storageFlags = 0);
    virtual ~GlBuffer();

    /// @}

    /// @name Non-copyable
    /// @{
    /// @}

    GlBuffer& operator=(GlBuffer&& other);

    // Need to explicitly delete these or problems arise with copy assignment
    GlBuffer& operator=(const GlBuffer&) = delete;
    GlBuffer(const GlBuffer&) = delete;

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Whether or not the buffer is a valid type
    bool hasValidType() const { return m_bufferType == gl::BufferType::kInvalid; }

    /// @brief Size in bytes of the buffer
    size_t byteSize() const { return m_size; }

    /// @brie Return length of buffer given a type
    template<typename T>
    inline size_t length() const {
        return m_size / sizeof(T);
    }

    /// @brief Set binding point of the buffer
    void setBindPoint(uint32_t point) {
        m_bindingPoint = point;
    }

    /// @brief Whether or not the GL buffer is bound to a context
    bool isBound() const;

    /// @brief Bind the GL buffer
    virtual void bind();

    /// @brief Unbind the GL buffer
    virtual void release();

    /// @brief Copy data into another buffer
    void copyInto(GlBuffer& other);

    /// @brief Map buffer and return data casted to the specified type
    template<typename T>
    T* data(size_t access = size_t(gl::RangeBufferAccessFlag::kRead | gl::RangeBufferAccessFlag::kWrite)) {
        return (T*)map(access, true);
    }

    /// @params[in] length The number of elements T to return
    template<typename T>
    T* data(size_t offset, size_t length, size_t access = size_t(gl::RangeBufferAccessFlag::kRead | gl::RangeBufferAccessFlag::kWrite)) {
        return (T*)map(offset * sizeof(T), length * sizeof(T), access, true);
    }

    /// @brief Copy len entries of data from buffer into a vector
    template<typename T>
    void copyData(std::vector<T>& outData, size_t len, size_t startIndex = 0, bool unmap = true) {
        T* bufferData = data<T>(gl::RangeBufferAccessFlag::kRead);
        outData = std::vector<T>(bufferData + startIndex, bufferData + len + startIndex);
        if (unmap) {
            GlBuffer::unmap(true);
        }
    }

    /// @brief Substitutes data into the buffer, no mapping or unmapping required
    /// @param[in] offset offset in number of elements T from the start of the buffer
    /// @param[in] size number of elements T to substitute into the buffer
    template<typename T>
    inline void subData(const T& val, size_t offset) {
        subData(&val, offset, 1);
    }
    template<typename T>
    inline void subData(const T* val, size_t offset, size_t size = 1) {
        subData((const void*)val, offset * sizeof(T), size * sizeof(T));
    }
    inline void subData(const void* val, size_t offsetBytes, size_t sizeBytes) {
        bool wasBound = isBound();
        if (!wasBound) {
            bind();
        }

        glBufferSubData((int)m_bufferType,
            offsetBytes,
            sizeBytes,
            val);

        if (!wasBound) {
            release();
        }

#ifdef DEBUG_MODE
        bool error = printGLError("Error on glBufferSubData");
        if (error) {
            Logger::Throw("Error on glBufferSubData");
        }
#endif
    }

    /// @brief Set data from a vector of inputs
    /// @details The GL_MAP_INVALIDATE_BUFFER_BIT is apparently very helpful for write performance
    /// @note This maps the buffer, which requires an unmap. SubData is recommended instead
    template<typename T>
    void setData(const std::vector<T>& vec, size_t access = size_t(gl::RangeBufferAccessFlag::kInvalidateBuffer | gl::RangeBufferAccessFlag::kWrite)) {
        T* bufferData = data<T>(access);
        for (size_t i = 0; i < vec.size(); i++) {
            bufferData[i] = vec[i];
        }
    }
    template<typename T>
    void setData(const T& val, size_t offset = 0, size_t access = size_t(gl::RangeBufferAccessFlag::kInvalidateBuffer | gl::RangeBufferAccessFlag::kWrite)) {
        T* bufferData = data<T>(access);
        bufferData[offset] = val;
    }

    /// @brief Map and obtain data associated with the GL buffer, MUST be bound first
    void* map(size_t access = size_t(gl::RangeBufferAccessFlag::kRead | gl::RangeBufferAccessFlag::kWrite), bool doBind = true);
    
    /// @params[in] sizeBytes the offset in bytes to start map
    /// @params[in] sizeBytes the size in bytes to map
    void* map(size_t offsetBytes, size_t sizeBytes, size_t access = size_t(gl::RangeBufferAccessFlag::kRead | gl::RangeBufferAccessFlag::kWrite), bool doBind = true);
    void unmap(bool doRelease = true);

    /// @brief Allocate memory for the buffer in OpenGL
    void allocateMemory();
    void allocateMemory(size_t size, gl::BufferStorageMode mode, size_t storageFlags);

    /// @brief Bind to a binding point
    void bindToPoint();
    void bindToPoint(size_t bindingPoint);

    /// @brief Bind a part of the buffer to a binding point, starting at offset and for sizeInBytes bytes
    void bindToPoint(size_t bindingPoint, size_t offset, size_t sizeInBytes);

    /// @brief Release from buffer point
    // Removed, doesn't make sense since 0 is a valid GL binding point
    void releaseFromPoint(size_t bufferPoint);
    void releaseFromPoint();

    /// @brief Return number of elements in the buffer data, assuming the given element size
    size_t count(size_t stride);

    /// @brief Get current binding ID from GL
    GLint getBoundBuffer();

    /// @brief Clear the buffer's contents
    void clear();

    /// @}

protected:
    friend class ShaderProgram;
    friend struct UniformBufferData;

    /// @name Protected methods
    /// @{

    void initialize();
    void createBuffer();

    /// @}

    /// @name Protected members
    /// @{

    static constexpr Uint32_t s_maxIntSize = std::numeric_limits<unsigned int>::max();

    /// @details Binding point won't change for GL buffers, but may be dynamic for other buffers
    unsigned int m_bindingPoint{ s_maxIntSize }; ///< The binding point of the buffer
    unsigned int m_bufferID{ s_maxIntSize }; ///< the GL ID of the GL buffer
    size_t m_size; ///< Size in bytes of underlying buffer data
    bool m_isMapped = false; ///< Whether or not the buffer is currently mapped
    void* m_data = nullptr; ///< The data mapped to this buffer in GPU memory
    gl::BufferType m_bufferType = gl::BufferType::kInvalid; ///< Type of buffer
    gl::BufferStorageMode m_storageMode; ///< The storage mode of the buffer
    size_t m_storageFlags = 0; ///< Storage flags for the buffer
    RenderContext* m_context = nullptr; ///< The OpenGL context that this buffers is bound within
    
    /// @}

};


} // End namespaces
