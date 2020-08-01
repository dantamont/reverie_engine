/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_FRAME_BUFFER_H
#define GB_FRAME_BUFFER_H

// QT

// Internal
#include "../../GbObject.h"
#include "../GbGLFunctions.h"
#include "../../geometry/GbVector.h"

namespace Gb {  

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class Color;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
//////////////////////////////////////////////////////////////////////////////////////////////////

enum class InternalBufferFormat {
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
    RenderBufferObject(InternalBufferFormat format, size_t numSamples);
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

    InternalBufferFormat m_internalFormat;
    size_t m_numSamples;

    /// @}
};



//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class FrameBuffer
/// @brief An abstraction of an OpenGL framebuffer
// TODO: Get MSAA textures working, doesn't seem supported by my hardware
class FrameBuffer : public Object, private GL::OpenGLFunctions {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum class BufferStorageType {
        kTexture, // Use textures wherever possible
        kRBO // Use RBOs wherever possible
    };

    enum class BindType {
        kRead = GL_READ_FRAMEBUFFER,
        kWrite = GL_DRAW_FRAMEBUFFER
    };

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    FrameBuffer(QOpenGLContext* currentContext,
        InternalBufferFormat internalFormat = InternalBufferFormat::kDefault,
        BufferStorageType bufferType = BufferStorageType::kRBO,
        size_t numSamples = 4);
    ~FrameBuffer();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    std::vector<unsigned int>& colorTextures() {
        return m_colorTextures;
    }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Returns color attachment at index
    void bindColorAttachment(unsigned int idx = 0);

    /// @brief Initialize the FBO with the specified width and height
    /// @details Uses RBO by default
    // TODO: Add flags to control whether RBOs or textures are used
    void reinitialize(size_t w, size_t h);

    /// @brief Bind the framebuffer for use
    void bind();

    /// @brief Bind framebuffer for read or write
    void bind(BindType type);

    /// @brief Clear the framebuffer, with an optional color
    void clear(const Color& color);

    /// @brief blit with another framebuffer
    void blit(unsigned int mask = GL_COLOR_BUFFER_BIT);

    /// @brief Release the framebuffer from use
    void release();

    /// @brief Check whether the framebuffer is complete
    bool isComplete();

    void initializeGL();

    /// @}


protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Set the framebuffer's color attachment to a new texture
    /// @details w and h are in pixels
    void setColorAttachment(size_t w, size_t h);

    /// @brief Set the framebuffer's depth/stencil attachment to a new texture
    void setDepthStencilAttachment(size_t w, size_t h);

    void createColorAttachment(size_t w, size_t h);
    void createColorTexture(size_t w, size_t h, unsigned int & outIndex);
    void createColorTextureMSAA(size_t w, size_t h, unsigned int & outIndex);

    void createDepthStencilAttachment(size_t w, size_t h);
    void createDepthStencilTexture(size_t w, size_t h, unsigned int & outIndex);

    void clearColorAttachments();
    void clearDepthStencilAttachments();
    void clearAttachments();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief OpenGL context used by the framebuffer
    QOpenGLContext* m_currentContext;

    /// @brief Enforce the same size for all attachments
    Vector2i m_size = { 0, 0 };

    unsigned int m_fboID = 0;

    bool m_isBound = false;

    /// @brief Textures on color attachments
    std::vector<unsigned int> m_colorTextures;

    /// @brief Depth/stencil attachments (may be same texture)
    unsigned int m_depthTexture = 0;
    unsigned int m_stencilTexture = 0;

    /// @brief Color RBOs, can be used instead of textures
    std::vector<std::shared_ptr<RenderBufferObject>> m_colorRBOs;

    /// @brief Depth/stencil RBO, can be used instead of textures (and is)
    std::shared_ptr<RenderBufferObject> m_depthStencilRBO = nullptr;

    InternalBufferFormat m_internalFormat;
    BufferStorageType m_bufferType;
    size_t m_numSamples;

    FrameBuffer* m_blitBuffer = nullptr;

    /// @}
};

//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}

#endif