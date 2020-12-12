/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_RENDER_BUFFER_OBJECT_H
#define GB_RENDER_BUFFER_OBJECT_H

// QT

// Internal
#include "../../GbObject.h"
#include "../GbGLFunctions.h"
#include "../materials/GbTexture.h"

namespace Gb {  

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class Color;
class Camera;
class ShaderProgram;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
//////////////////////////////////////////////////////////////////////////////////////////////////

enum class AliasingType {
    kDefault,
    kMSAA
};


/// @class RenderBufferObject
/// @brief An abstraction of an OpenGL RBO
/// @details Render buffers are write-only, so are often used as depth and stencil 
/// attachments, since values will often not be needed from them for SAMPLING. Values
/// can still be used for depth and stencil testing
class RenderBufferObject : public Object, private GL::OpenGLFunctions {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    RenderBufferObject(AliasingType format, TextureFormat internalFormat, size_t numSamples);
    ~RenderBufferObject();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
    unsigned int rboID() const {
        return m_rboID;
    }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Bind the RBO for use
    void bind();

    /// @brief Release the RBO from use
    void release();

    /// @brief Attach to the currently bound FBO
    void attach();

    /// @brief Set as a color attachment
    void setColor(size_t w, size_t h, size_t attachmentIndex);

    /// @brief Set as a depth/stencil attachment
    void setDepthStencil(size_t w, size_t h);

    /// @}


protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    void initializeGL();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    unsigned int m_rboID;
    unsigned int m_attachmentType; // GL attachment type, e.g., GL_DEPTH_STENCIL_ATTACHMENT
    bool m_isBound = false;

    AliasingType m_aliasingType;
    size_t m_numSamples;
    TextureFormat m_internalFormat;

    /// @}
};


//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}

#endif