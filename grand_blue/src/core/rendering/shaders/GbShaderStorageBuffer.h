#ifndef GB_SHADER_STORAGE_BUFFER_H
#define GB_SHADER_STORAGE_BUFFER_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Standard
#include <cstdlib>
#include <memory>
#include <vector>

// QT
#include <QObject>
#include <QString>

// Internal 
#include "../../GbObject.h"
#include "../../containers/GbVariant.h"
#include "../../geometry/GbMatrix.h"
#include "../../containers/GbContainerExtensions.h"
#include "../GbGLFunctions.h"
#include "GbUniform.h"
#include "../renderer/GbRenderContext.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
// TypeDefs
/////////////////////////////////////////////////////////////////////////////////////////////
typedef std::vector<Vector3g> Vec3List;
typedef std::vector<Vector4g> Vec4List;


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
struct ShaderStruct;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class representing a Shader Storage Buffer
// TODO: Make a common buffer class to encapsulate SSBO, UBO, etc.
template<typename T>
class ShaderStorageBuffer : public Object, private GL::OpenGLFunctions{
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Destructor
    /// @{
    /// @}

    ShaderStorageBuffer(RenderContext& context, size_t size):
        m_size(size),
        m_context(context)
    {
        initializeGL();
    }

    ~ShaderStorageBuffer() 
    {
        glDeleteBuffers(1, &m_bufferID);
    }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    //std::vector<T>& data() { return m_data; }
    //const std::vector<T>& data() const { return m_data; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    size_t elementByteSize() const { return sizeof(T); }

    /// @brief Obtain data associated with the Shader Storage buffer, MUST be bound first
    T* map(GL::RangeBufferAccessType access = GL::RangeBufferAccessType::kReadWrite, bool doBind = true) {
        if (m_mapped) {
            return m_data;
        }
        
        if (doBind) { bind(); }

        if (!m_context.isCurrent()) {
            throw("Error, context is not current");
        }


//#ifdef DEBUG_MODE
//        bool error = printGLError("Failed to bind buffer before mapping");;
//        if (error) {
//            throw("Failed to bind buffer before mapping");
//        }
//#endif

        size_t bufferSize = size();
        m_data = (T*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER,
            0, 
            bufferSize, 
            (size_t)access //| GL_MAP_PERSISTENT_BIT
        );

//#ifdef DEBUG_MODE
//        error = printGLError("Failed to map buffer");;
//        if (error) {
//            throw("Failed to bind buffer before mapping");
//        }
//#endif

#ifdef DEBUG_MODE
        if (!m_data) {
            printGLError("Failed to map buffer range");
            throw("Error, failed to map buffer");
        }
#endif  
        m_mapped = true;

        return m_data;
    }
    void unmap(bool doRelease = true) {
        if (!m_mapped) {
            throw("Error, this buffer is not mapped");
        }

        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        m_mapped = false;
        if (doRelease) { release(); }
    }

    bool isBound() const {
        return m_context.boundBuffer() == m_uuid;
    }

    /// @brief Bind and release the buffer for storage
    void bind() {

        if (!m_context.isCurrent()) {
            m_context.makeCurrent();
        }

        if (!isBound()) {
            // Release previously bound buffer
            if (!m_context.boundBuffer().isNull()) {
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

//#ifdef DEBUG_MODE
//                bool error = printGLError("Failed to release previous SSB");;
//                if (error) {
//                    throw("Error, failed to release previous SSB");
//                }
//#endif
            }

            // Bind current buffer
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferID);
            m_context.setBoundBuffer(m_uuid);

//#ifdef DEBUG_MODE
//            bool error = printGLError("Failed to bind SSB");;
//            if (error) {
//                throw("Error, failed to bind SSB");
//            }
//#endif
        }
    }

    void release() {
        if (!isBound()) {
            throw("Error, SSB was not bound");
        }
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        m_context.setBoundBuffer(Uuid());

//#ifdef DEBUG_MODE
//        bool error = printGLError("Failed to release SSB");;
//        if (error) {
//            throw("Error, failed to release SSB");
//        }
//#endif
    }

    /// @brief Creates a new data store for the currently bound buffer object
    void allocateMemory(GL::DrawDataMode mode = GL::DrawDataMode::kDynamic) {
        bind(); // Ensure that buffer is bound to the current context

        if (!isBound()) {
            throw("Error, SSB was not bound");
        }

        glBufferData(GL_SHADER_STORAGE_BUFFER, 
            size(), 0, (size_t)mode);
        //glBufferData(GL_SHADER_STORAGE_BUFFER,
        //    m_size * sizeof(T), 0,
        //    GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);

#ifdef DEBUG_MODE
        bool error = printGLError("Failed to allocate memory");;
        if (error) {
            throw("Failed to allocate memory");
        }
#endif
    }

    /// @brief Bind to a buffer point
    void bindToPoint(size_t bufferPoint)
    {
        bind();
        m_bindingPoint = bufferPoint;
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bufferPoint, m_bufferID);
    }

    /// @brief Release from buffer point
    void releaseFromPoint(size_t bufferPoint)
    {
        bind();
        m_bindingPoint = bufferPoint;
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bufferPoint, 0);
    }

    /// @brief Return the size of the data in this buffer, in bytes
    unsigned int size() const { return m_size * sizeof(T); }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @}

protected:
    friend class ShaderProgram;
    friend struct UniformBufferData;

    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors
    /// @{
    /// @}

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    void initializeGL() {
        glGenBuffers(1, &m_bufferID);

        bool error = printGLError("Failed to create SSB");
        if (error) {
            throw("Error, failed to create SSB");
        }

        allocateMemory();
    }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{
    
    /// @brief Data to store in the shader buffer object
    // Removed, data is stored directly in GL memory
    //std::vector<T> m_data;
    
    /// @brief Size of the buffer
    size_t m_size;

    /// @brief ID of the buffer
    unsigned int m_bufferID = -1;

    unsigned int m_bindingPoint = -1;

    bool m_mapped = false;
    T* m_data = nullptr;

    RenderContext& m_context;

    /// @}

};



    /////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif