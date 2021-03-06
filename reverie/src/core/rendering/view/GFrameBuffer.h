/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_FRAME_BUFFER_H
#define GB_FRAME_BUFFER_H

// QT

// Internal
#include "GRenderBufferObject.h"
#include "../../geometry/GVector.h"
#include "../materials/GTexture.h"

namespace rev {  

/////////////////////////////////////////////////////////////////////////////////////////////
// Definitions
/////////////////////////////////////////////////////////////////////////////////////////////
#define FBO_DEFAULT_TEX_FORMAT          TextureFormat::kRGBA8
#define FBO_SSAO_TEX_FORMAT             TextureFormat::kR8
#define FBO_FLOATING_POINT_TEX_FORMAT   TextureFormat::kRGBA16F
#define FBO_DEFAULT_DEPTH_PRECISION     TextureFormat::kDepth24Stencil8 //TextureFormat::kDepth32FStencil8X24

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class Color;
class Camera;
class ShaderProgram;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class FrameBuffer
/// @brief An abstraction of an OpenGL framebuffer
class FrameBuffer : public Object {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum class BlitMask {
        kColorBit = GL_COLOR_BUFFER_BIT,
        kDepthBit = GL_DEPTH_BUFFER_BIT,
        kStencilBit = GL_STENCIL_BUFFER_BIT,
        kDepthStencilBit = kDepthBit | kStencilBit
    };

    enum class BufferAttachmentType {
        kTexture, // Use textures wherever possible
        kRBO // Use RBOs wherever possible
    };

    enum class BindType {
        kRead = GL_READ_FRAMEBUFFER,
        kWrite = GL_DRAW_FRAMEBUFFER
    };

    /// @brief Release current active FBO
    static void Release();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    FrameBuffer();
    FrameBuffer(const FrameBuffer& other);
    FrameBuffer(QOpenGLContext* currentContext,
        AliasingType internalFormat = AliasingType::kDefault,
        BufferAttachmentType bufferType = BufferAttachmentType::kRBO,
        TextureFormat textureFormat = FBO_FLOATING_POINT_TEX_FORMAT,
        size_t numSamples = 4,
        size_t numColorAttachments = 1,
        bool hasDepth = true);
    ~FrameBuffer();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    FrameBuffer& operator=(const FrameBuffer& other);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    FrameBuffer* blitBuffer() {
        return m_blitBuffer;
    }

    bool hasSize() const {
        return m_size.lengthSquared() > 0;
    }

    size_t width() const { return m_size.x(); }
    size_t height() const { return m_size.y(); }

    bool isBound() const { return m_isBound; }

    std::vector<std::shared_ptr<Texture>>& colorTextures() {
        return m_colorTextures;
    }

    const std::shared_ptr<Texture>& depthTexture() {
        return m_depthStencilTexture;
    }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Draw framebuffer quad
    void drawQuad(Camera& camera, ShaderProgram& shaderProgram, size_t colorAttachmentIndex = 0);

    /// @brief Save color attachment to a file
    void saveColorToFile(size_t attachmentIndex, const GString& filepath, PixelType pixelType = PixelType::kUByte8);
    void saveDepthToFile(const GString& filepath);

    /// @brief Read pixels from specified texture
    template<typename T>
    void readColorPixels(size_t attachmentIndex, std::vector<T>& outColor,
        PixelFormat pixelFormat = PixelFormat::kRGB, PixelType pixelType = PixelType::kByte8) const {
        readColorPixels(attachmentIndex, outColor, 0, 0, m_size.x(), m_size.y(), pixelFormat, pixelType);
    }
    template<typename T>
    void readColorPixels(size_t attachmentIndex, std::vector<T>& outColor,
        GLint x, GLint y, GLsizei width, GLsizei height,
        PixelFormat pixelFormat = PixelFormat::kRGB, PixelType pixelType = PixelType::kByte8) const
    {
        // Resize vector to appropriate size based on pixel format/type
        size_t numColorEntries;
        switch (pixelFormat) {
        case PixelFormat::kBGR:
        case PixelFormat::kRGB:
            numColorEntries = 3;
            break;
        case PixelFormat::kBGRA:
        case PixelFormat::kRGBA:
            numColorEntries = 4;
            break;
        default:
            throw("Unimplemented format");
        }

        size_t vecSize;
        switch (pixelType) {
        case PixelType::kUByte8:
            Q_UNUSED(numColorEntries);
            vecSize = width * height;
            break;
        case PixelType::kFloat16:
        case PixelType::kFloat32:
            vecSize = width * height * numColorEntries;
            break;
        default:
            throw("Unimplemented type");
        }
        outColor.resize(vecSize, -1);

        // Perform read
        readColorPixels(attachmentIndex, outColor.data(), x, y, width, height, pixelFormat, pixelType);
    }
    template<typename T>
    void readColorPixels(size_t attachmentIndex, T* outColor,
        GLint x, GLint y, GLsizei width, GLsizei height,
        PixelFormat pixelFormat = PixelFormat::kRGB, PixelType pixelType = PixelType::kByte8) const
    {
        switch (m_aliasingType) {
        case AliasingType::kDefault:
        {
            GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();

            bind();

            // Read pixels directly
            glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
            gl.glReadPixels(
                x,
                y,
                width,
                height,
                (int)pixelFormat,
                (int)pixelType,
                outColor
            );
            break;
        }
        case AliasingType::kMSAA:
        {
            if (m_attachmentType == BufferAttachmentType::kRBO) {
                // RBO needed for MSAA
                // Need to blit (exchange pixels) with another framebuffer (that is not MSAA)
                blit(BlitMask::kColorBit, *m_blitBuffer, attachmentIndex);
                m_blitBuffer->readColorPixels(attachmentIndex, outColor, x, y, width, height, pixelFormat, pixelType);
            }
            else {
                throw("Error, MSAA FBOs with textures are not supported by current drivers");
            }
            break;
        }
        default:
            throw("Error, format type not recognized");
            break;
        }

#ifdef DEBUG_MODE
#ifdef DEBUG_FBO
        bool error = gl.printGLError("Error, failed to read color pixels");
        if (error) {
            throw("Error, failed to read color pixels");
        }
#endif
#endif

        release();
    }


    void readDepthPixels(std::vector<float>& outDepths) const;

    /// @brief Binds color texture at index
    void bindColorTexture(unsigned int texUnit = 0, unsigned int attachmentIndex = 0);

    /// @brief Binds depth texture to specified texture unit
    void bindDepthTexture(unsigned int texUnit = 0);

    /// @brief Release all bound textures
    // REMOVED, unbind is unnecessary and cumbersome until a global list of bound textures is maintained
    //void releaseTextures();

    /// @brief Initialize the FBO with the specified width and height
    /// @details Uses RBO by default
    // TODO: Add flags to control whether RBOs or textures are used
    void reinitialize(size_t w, size_t h, bool ignoreIncomplete = false);
    void reinitialize(size_t w, size_t h, const std::vector<std::shared_ptr<Texture>>& colorAttachments,
        const std::shared_ptr<Texture>& depthStencilAttachment,
        size_t depthLayer);

    void reinitialize(size_t w, size_t h, const std::vector<std::shared_ptr<Texture>>& colorAttachments);
    void reinitialize(size_t w, size_t h, 
        const std::shared_ptr<Texture>& depthStencilAttachment,
        size_t depthLayer = 0
    );

    /// @brief Bind the framebuffer for use
    void bind() const;

    /// @brief Bind framebuffer for read or write
    void bind(BindType type) const;

    /// @brief Clear the framebuffer, with an optional color
    void clear();
    void clear(const Color& color);

    /// @brief Clear the specified attachment with the given color
    void clear(const Color& color, size_t attachmentIndex);

    /// @brief Blit texture from this framebuffer to the specified framebuffer
    /// @details This does not follow OpenGL's API as closely, but is a cleaner interface
    void blit(BlitMask blitMask, FrameBuffer& other, size_t readAttachmentIndex = 0) const;
    void blit(BlitMask blitMask, FrameBuffer& other, size_t readAttachmentIndex, const std::vector<size_t>& drawAttachmentIndices) const;
    /// @brief Release the framebuffer from use
    void release() const;

    /// @brief Check whether the framebuffer is complete
    bool isComplete();

    void initializeGL(bool reinitialize = true);

    /// @}


protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief blit with another framebuffer
    /// @details This follows OpenGL's API closely, so the framebuffers bound to GL_READ_FRAMEBUFFER
    /// and GL_WRITE_FRAMEBUFFER are used
    void blit(BlitMask blitMask, size_t readColorAttachmentIndex, const std::vector<size_t>& drawAttachmentIndices) const;


    /// @brief Set the framebuffer's color attachment to a new texture
    /// @details w and h are in pixels
    void setColorAttachments(size_t w, size_t h);

    /// @brief Set the framebuffer's depth/stencil attachment to a new texture
    void setDepthStencilAttachment(size_t w, size_t h);

    void createColorAttachment(size_t w, size_t h);
    //void createColorTexture(size_t w, size_t h, unsigned int & outIndex);
    //void createColorTextureMSAA(size_t w, size_t h, unsigned int & outIndex);

    void createDepthStencilAttachment(size_t w, size_t h);

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

    /// @brief Number of color attachments to use
    size_t m_numColorAttachments = 1;
    bool m_hasDepth;

    unsigned int m_fboID = 0;

    mutable bool m_isBound = false;

    /// @brief Textures on color attachments
    std::vector<std::shared_ptr<Texture>> m_colorTextures;

    /// @brief Depth/stencil attachment, assumed to share a texture
    std::shared_ptr<Texture> m_depthStencilTexture = nullptr;
    unsigned int m_depthStencilAttachmentLayer = 0;

    /// @brief Color RBOs, can be used instead of textures
    std::vector<std::shared_ptr<RenderBufferObject>> m_colorRBOs;

    /// @brief Depth/stencil RBO, can be used instead of textures (and is)
    std::shared_ptr<RenderBufferObject> m_depthStencilRBO = nullptr;

    AliasingType m_aliasingType;
    BufferAttachmentType m_attachmentType;
    size_t m_numSamples;

    TextureFormat m_textureFormat;

    /// @brief A textured framebuffer, used to overcome:
    /// Need to use a texture to render to a quad/post-process
    /// Need to use a texture to read from depth buffer
    FrameBuffer* m_blitBuffer = nullptr;



    /// @}
};

//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}

#endif