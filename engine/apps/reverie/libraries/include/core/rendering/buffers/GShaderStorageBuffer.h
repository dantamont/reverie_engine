#ifndef GB_SHADER_STORAGE_BUFFER_H
#define GB_SHADER_STORAGE_BUFFER_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

// Internal 
#include "GGlBuffer.h"

namespace rev {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
struct ShaderStruct;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class representing a Shader Storage Buffer
class ShaderStorageBuffer : public GlBuffer {
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
    ShaderStorageBuffer(ShaderStorageBuffer&& other);
    ShaderStorageBuffer(RenderContext& context, size_t size, gl::BufferStorageMode storageMode = gl::BufferStorageMode::kDynamicDraw, size_t storageFlags = 0);
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
    ShaderStorageBuffer& operator=(const GlBuffer&) = delete;
    ShaderStorageBuffer(const GlBuffer&) = delete;

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
    /// @name Friend Functions
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