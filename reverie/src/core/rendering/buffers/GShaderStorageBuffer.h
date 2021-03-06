#ifndef GB_SHADER_STORAGE_BUFFER_H
#define GB_SHADER_STORAGE_BUFFER_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

// Internal 
#include "GGLBuffer.h"

namespace rev {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
struct ShaderStruct;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class representing a Shader Storage Buffer
class ShaderStorageBuffer : public GLBuffer {
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    /// @}

    ShaderStorageBuffer();
    ShaderStorageBuffer(RenderContext& context, size_t size, GL::BufferStorageMode storageMode = GL::BufferStorageMode::kDynamicDraw, size_t storageFlags = 0);
    ~ShaderStorageBuffer();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Non-copyable
    /// @{
    /// @}

    ShaderStorageBuffer& operator=(ShaderStorageBuffer&&);

    // Need to explicitly delete these or problems arise with copy assignment
    ShaderStorageBuffer& operator=(const GLBuffer&) = delete;
    ShaderStorageBuffer(const GLBuffer&) = delete;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief Bind the GL buffer
    virtual void bind() override;

    /// @brief Unbind the GL buffer
    virtual void release() override;


    /// @brief Return max allowed size of a shader storage buffer
    size_t maxSize();

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
    /// @}


};



    /////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif