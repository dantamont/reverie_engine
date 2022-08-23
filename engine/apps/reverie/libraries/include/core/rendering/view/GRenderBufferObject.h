#pragma once

// QT

// Internal
#include "core/rendering/GGLFunctions.h"
#include "core/rendering/materials/GTexture.h"

namespace rev {  

class Color;
class Camera;
class ShaderProgram;

enum class AliasingType {
    kDefault,
    kMSAA
};


/// @class RenderBufferObject
/// @brief An abstraction of an OpenGL RBO
/// @details Render buffers are write-only, so are often used as depth and stencil 
/// attachments, since values will often not be needed from them for SAMPLING. Values
/// can still be used for depth and stencil testing
class RenderBufferObject : private gl::OpenGLFunctions {
public:
    /// @name Constructors/Destructor
    /// @{
    RenderBufferObject(AliasingType format, TextureFormat internalFormat, uint32_t numSamples);
    ~RenderBufferObject();
    /// @}

    /// @name Properties
    /// @{
    unsigned int rboID() const {
        return m_rboID;
    }

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Bind the RBO for use
    void bind();

    /// @brief Release the RBO from use
    void release();

    /// @brief Attach to the currently bound FBO
    void attach();

    /// @brief Set as a color attachment
    void setColor(uint32_t w, uint32_t h, uint32_t attachmentIndex);

    /// @brief Set as a depth/stencil attachment
    void setDepthStencil(uint32_t w, uint32_t h);

    /// @}


protected:
    /// @name Protected Methods
    /// @{

    void initializeGL();

    /// @}

    /// @name Protected Members
    /// @{

    unsigned int m_rboID;
    unsigned int m_attachmentType; // GL attachment type, e.g., GL_DEPTH_STENCIL_ATTACHMENT
    bool m_isBound = false;

    AliasingType m_aliasingType;
    uint32_t m_numSamples;
    TextureFormat m_internalFormat;

    /// @}
};

// End namespaces
}
