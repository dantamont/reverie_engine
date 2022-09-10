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
        kVertexBuffer = GL_ARRAY_BUFFER,
        kAtomicCounterBuffer = GL_ATOMIC_COUNTER_BUFFER,
        kCopyReadBuffer = GL_COPY_READ_BUFFER,
        kCopyWriteBuffer = GL_COPY_WRITE_BUFFER,
        kDispatchIndirectBuffer = GL_DISPATCH_INDIRECT_BUFFER,
        kDrawIndirectBuffer = GL_DRAW_INDIRECT_BUFFER,
        kIndexBuffer = GL_ELEMENT_ARRAY_BUFFER,
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
        kUniformBufferBinding = GL_UNIFORM_BUFFER_BINDING,
        kVertexBufferBinding = GL_ARRAY_BUFFER_BINDING,
        kIndexBufferBinding = GL_ELEMENT_ARRAY_BUFFER_BINDING
    };

    /// @see https://www.reddit.com/r/opengl/comments/57i9cl/examples_of_when_to_use_gl_dynamic_draw/
    // GL_STATIC_DRAW basically means "I will load this vertex data once and then never change it." This would include any static props or level geometry, but also animated models / particles if you are doing all the animation with vertex shaders on the GPU(modern engines with skeletal animation do this, for example).
    // GL_STREAM_DRAW basically means "I am planning to change this vertex data basically every frame." If you are manipulating the vertices a lot on the CPU, and it's not feasible to use shaders instead, you probably want to use this one. Sprites or particles with complex behavior are often best served as STREAM vertices. While STATIC+shaders is preferable for animated geometry, modern hardware can spew incredible amounts of vertex data from the CPU to the GPU every frame without breaking a sweat, so you will generally not notice the performance impact.
    // GL_DYNAMIC_DRAW basically means "I may need to occasionally update this vertex data, but not every frame." This is the least common one.It's not really suited for most forms of animation since those usually require very frequent updates. Animations where the vertex shader interpolates between occasional keyframe updates are one possible case. A game with Minecraft-style dynamic terrain might try using DYNAMIC, since the terrain changes occur less frequently than every frame. DYNAMIC also tends to be useful in more obscure scenarios, such as if you're batching different chunks of model data in the same vertex buffer, and you occasionally need to move them around.
    // Keep in mind these 3 flags don't imply any hard and fast rules within OpenGL or the hardware. They are just hints so the driver can set things up in a way that it thinks will be the most efficient.
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


} // end GL namespace


/// @class GlBuffer
/// @brief Class representing a GL Buffer
/// @todo Maybe make the class a templated type so that switch statements aren't necessary
class GlBuffer :
    public IdentifiableInterface, 
    public NameableInterface,
    protected gl::OpenGLFunctions
{
protected:
    static constexpr Uint32_t s_maxIntSize = std::numeric_limits<unsigned int>::max();

public:
    /// @name Constructors/Destructor
    /// @{
    /// @}

    GlBuffer();
    GlBuffer(GlBuffer&& other);
    GlBuffer(gl::BufferType type, size_t sizeInBytes, gl::BufferStorageMode storageMode = gl::BufferStorageMode::kDynamicDraw, size_t storageFlags = 0);
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
    Int32_t byteSize() const { return m_size; }

    /// @brief Return length of buffer given a type
    template<typename T>
    inline size_t length() const {
        return m_size / sizeof(T);
    }

    /// @brief Set the context for the buffer
    void setRenderContext(RenderContext& context);

    /// @brief Set binding point of the buffer
    void setBindPoint(uint32_t point) {
        m_bindingPoint = point;
    }

    /// @brief Whether or not the buffer has a valid OpenGL ID
    bool hasValidId() const {
        return m_bufferID != s_maxIntSize;
    }

    /// @brief Bind the GL buffer
    virtual void bind();

    /// @brief Unbind the GL buffer
    virtual void release();

    /// @brief Copy data into another buffer
    void copyInto(GlBuffer& other);

    /// @brief Bind and map buffer and return data casted to the specified type
    /// @note Data must be unmapped after use
    template<typename T>
    T* dataMap(size_t access = size_t(gl::RangeBufferAccessFlag::kRead | gl::RangeBufferAccessFlag::kWrite)) {
        return (T*)map(access, true);
    }

    /// @params[in] length The number of elements T to return
    /// @note data must be unmapped after use
    template<typename T>
    T* dataMap(size_t offset, size_t length, size_t access = size_t(gl::RangeBufferAccessFlag::kRead | gl::RangeBufferAccessFlag::kWrite)) {
        return (T*)map(offset * sizeof(T), length * sizeof(T), access, true);
    }

    /// @brief Copy len entries of data from buffer into a vector
    template<typename T>
    void copyData(std::vector<T>& outData, size_t len, size_t startIndex = 0, bool unmap = true) {
        T* bufferData = dataMap<T>(gl::RangeBufferAccessFlag::kRead);
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
    inline void subData(const T* val, size_t offset, size_t count = 1) {
        subData((const void*)val, offset * sizeof(T), count * sizeof(T));
    }
    template<>
    inline void subData(const void* val, size_t offsetBytes, size_t sizeBytes) {
        bind();

        glBufferSubData((int)m_bufferType,
            offsetBytes,
            sizeBytes,
            val);

        release();

#ifdef DEBUG_MODE
        bool error = printGLError("Error on glBufferSubData");
        if (error) {
            Logger::Throw("Error on glBufferSubData");
        }
#endif
    }

    /// @brief Set data from a vector of inputs
    /// @details The GL_MAP_INVALIDATE_BUFFER_BIT is apparently very helpful for write performance
    /// @note This maps the buffer, which requires an unmap. subData is recommended instead
    template<typename T>
    void setDataMap(const T* inData, size_t dataCount, size_t access = size_t(gl::RangeBufferAccessFlag::kInvalidateBuffer | gl::RangeBufferAccessFlag::kWrite)) {
        T* bufferData = dataMap<T>(access);
        for (size_t i = 0; i < dataCount; i++) {
            bufferData[i] = inData[i];
        }
    }
    template<typename T>
    void setDataMap(const T& val, size_t offset = 0, size_t access = size_t(gl::RangeBufferAccessFlag::kInvalidateBuffer | gl::RangeBufferAccessFlag::kWrite)) {
        T* bufferData = dataMap<T>(access);
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
    void allocateMemory(size_t byteSize, gl::BufferStorageMode mode, size_t storageFlags);

    /// @brief Allocate memory for the buffer, initializing with the contents of data
    template<typename T>
    void allocateMemory(const T* data, size_t count) {
        m_size = count * sizeof(T);
        allocateMemory(m_size, m_storageMode, m_storageFlags);
        subData(data, 0, count);
    }

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

    /// @brief Create the buffer in OpenGL and allocate memory
    void initialize(RenderContext& context);

    /// @brief Clear the buffer's data contents, but do not deallocate the memory
    void clear();

    /// @brief Destroy the buffer's OpenGL contents, deallocating the associated memory
    void destroy();

    /// @}

protected:
    friend class ShaderProgram;
    friend struct UniformBufferData;

    /// @name Protected methods
    /// @{

    void createBuffer();

    /// @}

    /// @name Protected members
    /// @{

    /// @details Binding point won't change for GL buffers, but may be dynamic for other buffers
    unsigned int m_bindingPoint{ s_maxIntSize }; ///< The binding point of the buffer
    unsigned int m_bufferID{ s_maxIntSize }; ///< the GL ID of the GL buffer
    Int32_t m_size{ -1 }; ///< Size in bytes of underlying buffer data
    bool m_isMapped = false; ///< Whether or not the buffer is currently mapped
    void* m_data = nullptr; ///< The data mapped to this buffer in GPU memory
    gl::BufferType m_bufferType = gl::BufferType::kInvalid; ///< Type of buffer
    gl::BufferStorageMode m_storageMode; ///< The storage mode of the buffer
    size_t m_storageFlags = 0; ///< Storage flags for the buffer
    RenderContext* m_context = nullptr; ///< The OpenGL context that this buffers is bound within
    
    /// @}

};


} // End namespaces
