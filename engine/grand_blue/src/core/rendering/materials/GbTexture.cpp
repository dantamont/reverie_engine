#include "GbTexture.h"

// Qt
#include <QDebug>

// internal
//#include <QOpenGLFunctions_3_2_Core> 
#include "../GbGLFunctions.h"
#include "../../containers/GbColor.h"
#include "../../utils/GbMemoryManager.h"
#include "../../GbCoreEngine.h"
#include "../../resource/GbResourceCache.h"
#include "../shaders/GbShaders.h"
#include "../../processes/GbProcess.h"
#include "../../readers/GbJsonReader.h"
#include "../GbGLFunctions.h"
//#include "QGLFramebufferObject" // TODO: Replace with my own FBO class
#include "../view/GbFrameBuffer.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
TextureData::TextureData(bool isBump)
{
    m_textureFileName[0] = 0; // Initialize file name to null string
    initialize(isBump);
}
/////////////////////////////////////////////////////////////////////////////////////////////
TextureData::~TextureData()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void TextureData::initialize(bool isBump)
{
    if (isBump) {
        m_properties.m_imfChannel = 'l';
    }
    else {
        m_properties.m_imfChannel = 'm';
    }
}



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Texture
/////////////////////////////////////////////////////////////////////////////////////////////
const std::vector<GString> Texture::s_typeToUniformVec = {
    "material.diffuseTexture",
    "material.normalMap",
    "material.ambientTexture",
    "material.specularTexture",
    "material.specularHighlightTexture",
    "material.bumpMap",
    "material.displacementTexture",
    "material.opacityTexture",
    "material.reflectionTexture",
    "material.lightmapTexture",
    "material.albedoTexture_pbr",
    "material.bumpTexture_pbr",
    "material.ambientOcclusionTexture_pbr",
    "material.roughnessTexture_pbr",
    "material.metallicTexture_pbr",
    "material.shininessTexture_pbr",
    "material.emissiveTexture_pbr"
};
/////////////////////////////////////////////////////////////////////////////////////////////
size_t Texture::MaxDivisibleTextureSize()
{
    size_t maxSize = GL_MAX_TEXTURE_SIZE;

    size_t digits = 0;
    while (maxSize > 0) {
        maxSize >>= 1;
        digits++;
    }

    return 1 << (digits - 1);
}
/////////////////////////////////////////////////////////////////////////////////////////////
size_t Texture::MaxTextureSize()
{
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
    GLint maxSize;
    gl.glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    return (size_t)maxSize;
}

/////////////////////////////////////////////////////////////////////////////////////////////
size_t Texture::MaxNumTextureLayers()
{
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
    GLint maxSize;
    gl.glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxSize);
    return (size_t)maxSize;
}
/////////////////////////////////////////////////////////////////////////////////////////////
size_t Texture::TextureBoundToActiveUnit()
{
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
    GLint whichID;
    gl.glGetIntegerv(GL_TEXTURE_BINDING_2D, &whichID);
    return (size_t)whichID;
}

/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> Texture::createHandle(CoreEngine * engine,
    const QString & texturePath,
    TextureUsageType type)
{
    // Create texture handle and add to cache
    auto textureHandle = prot_make_shared<ResourceHandle>(engine,
        texturePath, Resource::kTexture);

    // Set handle attributes
    textureHandle->resourceJson().insert("texUsageType", (int)type);

    // Add handle to resource cache
    engine->resourceCache()->insertHandle(textureHandle);

    // Load texture
    textureHandle->loadResource();

    return textureHandle;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> Texture::createHandle(const QString & texturePath,
    TextureUsageType type,
    ResourceHandle& material)
{
    // Create texture handle and add to cache
    auto textureHandle = prot_make_shared<ResourceHandle>(material.engine(),
        texturePath, Resource::kTexture);

    // Need to set as a child handle before inserting into resource cache
    textureHandle->setChild(true);
    material.engine()->resourceCache()->insertHandle(textureHandle);

    // Set handle attributes
    textureHandle->resourceJson().insert("texUsageType", (int)type);

    // Actually add to material as a child
    material.addChild(textureHandle);

    return textureHandle;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> Texture::createHandle(CoreEngine* core,
    const Image & image, TextureUsageType type,
    TextureFilter minFilter,
    TextureFilter maxFilter,
    TextureWrapMode wrapMode)
{
    std::shared_ptr<ResourceHandle> handle = ResourceHandle::create(core,
        Resource::kTexture);
    //handle->setResourceType(Resource::kTexture);

    std::shared_ptr<Texture> texture = prot_make_shared<Texture>(image,
        type, minFilter, maxFilter, wrapMode);
    handle->setResource(texture, false);

    // Emit signal to run post-construction
    handle->setIsLoading(true);
    core->resourceCache()->incrementLoadCount();
    emit core->resourceCache()->doneLoadingResource(handle); // run post-construction
    return handle;
}

/////////////////////////////////////////////////////////////////////////////////////////////
Texture::Texture(const QString & filePath, TextureUsageType type):
    m_usageType(type),
    m_targetType(TextureTargetType::k2D),
    m_depth(1)
{
    // Load image
    initialize(filePath);
}
/////////////////////////////////////////////////////////////////////////////////////////////
Texture::Texture(const Image & image, TextureUsageType type,
    TextureFilter minFilter,
    TextureFilter magFilter,
    TextureWrapMode wrapMode,
    TextureTargetType targetType):
    m_targetType(targetType),
    m_usageType(type),
    m_depth(1)
{
    m_textureOptions.m_minFilter = minFilter;
    m_textureOptions.m_magFilter = magFilter;
    m_textureOptions.m_wrapMode = wrapMode;

    // Don't mirror, assume image has already been mirrored
    initialize(image, false);
}
/////////////////////////////////////////////////////////////////////////////////////////////
Texture::Texture(size_t width,
    size_t height, 
    TextureTargetType targetType,
    TextureUsageType type,
    TextureFilter minFilter, 
    TextureFilter magFilter,
    TextureWrapMode wrapMode,
    TextureFormat format, 
    size_t depth) :
    m_targetType(targetType),
    m_usageType(type),
    m_width(width),
    m_height(height),
    m_depth(depth)
{
    m_textureOptions.m_internalFormat = format;
    m_textureOptions.m_minFilter = minFilter;
    m_textureOptions.m_magFilter = magFilter;
    m_textureOptions.m_wrapMode = wrapMode;

    setCost();
}
/////////////////////////////////////////////////////////////////////////////////////////////
Texture::~Texture()
{
    // Destroy underlying GL texture on destruction
    destroy();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::getData(void * outData, PixelFormat format, PixelType pixelType)
{
    bind();

    //GL::OpenGLFunctions& functions = *GL::OpenGLFunctions::Functions();

    switch (m_targetType) {
    case(TextureTargetType::k2D):
    {
        glGetTexImage(
            (GLint)m_targetType, //GL_TEXTURE_2D, or any of the cubemap faces!
            0, // mip-map level
            (int)format, // pixel format of input texture, is necessarily unsized
            (int)pixelType, // pixel data type, this is where the size comes in
            outData
        );
        break;
    }
    default:
        throw("Error, unimplemented");
    }

#ifdef DEBUG_MODE
    GL::OpenGLFunctions& functions = *GL::OpenGLFunctions::Functions();
    bool error = functions.printGLError("Error, failed to get texture data");
    if (error) {
        throw("Error, failed to get texture data");
    }
#endif

    release();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::setData(const void * data, PixelFormat format, PixelType pixelType)
{
    bind();

    GL::OpenGLFunctions& functions = *GL::OpenGLFunctions::Functions();

    // Store pixel data
    switch (m_targetType) {
    case(TextureTargetType::k2D):
    {
        functions.glTexSubImage2D(
            (GLint)m_targetType, //GL_TEXTURE_2D, or any of the cubemap faces!
            0, // mip-map level
            0, // x-offset
            0, // y-offset
            m_width, // texture width
            m_height, // texture height
            (int)format, // pixel format of input texture, is necessarily unsized
            (int)pixelType, // pixel data type, this is where the size comes in
            data
        );
        break;
    }
    case TextureTargetType::kCubeMap:
    case TextureTargetType::kCubeMapArray: // Only supported in OpenGL 4.0+
    default:
        throw("Texture type unimplemented");
        break;
    }

#ifdef DEBUG_MODE
    bool error = functions.printGLError("Error, failed to set texture data");
    if (error) {
        throw("Error, failed to set texture data");
    }
#endif

    functions.glFlush();

    // Generate mip maps automatically (requires OpenGL 3.0+)
    if (hasMipMaps()) {
        generateMipMaps();
    }

    release();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::setBorderColor(const Color & color)
{
    Vector4 colorVec = color.toVector4g();
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, colorVec.data());
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::attach(FrameBuffer & fbo, int attachment, size_t depthLayer)
{
    GL::OpenGLFunctions& functions = *GL::OpenGLFunctions::Functions();

    bool wasBound = fbo.isBound();
    if (!wasBound) {
        fbo.bind();
    }

    switch (m_targetType) {
    case TextureTargetType::k2D:
        // Standard texture
        functions.glFramebufferTexture2D(GL_FRAMEBUFFER,
            attachment, // e.g. GL_COLOR_ATTACHMENT0
            (int)m_targetType,
            m_textureID,
            0); // Mip-map level, must be 0
        break;
    case TextureTargetType::k2DArray:
        // Texture array (3D texture)
        functions.glFramebufferTextureLayer(GL_FRAMEBUFFER,
            attachment, // e.g. GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT
            m_textureID, 
            0, // Mipmap level of texture array to attach
            depthLayer); // Depth layer of the texture object
        break;
    case TextureTargetType::kCubeMap:
    case TextureTargetType::kCubeMapArray: // Only supported in OpenGL 4.0+
    {
        // Cubemap array (3D texture)
        // To attach a single face, call glFrameBufferTexture2D
        // Cubemap arrays are incompatible with glFramebufferTexture3D
        // See: https://gamedev.stackexchange.com/questions/109199/how-to-attach-a-framebuffer-to-a-whole-cube-map-from-a-gl-texture-cube-map-array
        //functions.glFramebufferTextureLayer(GL_FRAMEBUFFER,
        //    attachment, // e.g. GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT
        //    m_textureID,
        //    0, // Mipmap level of texture array to attach
        //    depthLayer * 6); // Layer must be multiplied by 6, since 6 faces per layer


        // Works, but links entire cubemap, which is sad
        // Since the array clears everything on update, only one layer
        // can be properly updated at a time
        functions.glFramebufferTexture(GL_FRAMEBUFFER,
            attachment, // e.g. GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT
            m_textureID,
            0); // Mipmap level of texture array to attach
        break;
    }
    default:
        throw("Error, texture type not yet implemented for attachments");
        break;
    }

#ifdef DEBUG_MODE
    bool error = functions.printGLError("Error attaching texture");
    if (error) {
        throw("Error attaching texture");
    }
#endif

    if (!wasBound) {
        fbo.release();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::setReadMode(DepthStencilMode mode)
{
    GL::OpenGLFunctions& functions = *GL::OpenGLFunctions::Functions();

    if (m_textureOptions.m_internalFormat != TextureFormat::kDepth24Stencil8 &&
        m_textureOptions.m_internalFormat != TextureFormat::kDepth32FStencil8X24) {
        throw("Error, incorrect texture format to set depth stencil mode");
    }

    functions.glTexParameteri((int)m_targetType, GL_DEPTH_STENCIL_TEXTURE_MODE, (int)mode);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::bind(int texUnit)
{
    //if (m_isBound) {
    //    throw("Error, texture already bound");
    //}
    //m_isBound = true;

    GL::OpenGLFunctions& functions = *GL::OpenGLFunctions::Functions();

    // Texture unit was specified
    if (texUnit > -1) {
        // Set active texture unit so that texture gets bound to it
        functions.glActiveTexture(GL_TEXTURE0 + texUnit);
    }

    // Bind texture to texture unit
    functions.glBindTexture((int)m_targetType, m_textureID);

#ifdef DEBUG_MODE
    bool error = functions.printGLError("Error, failed to bind texture");
    if (error) {
        throw("Error, failed to bind texture");
    }
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::release()
{
    //if (!m_isBound) {
    //    return;
    //}

    //m_isBound = false;

    // Release is just binding the default texture
    GL::OpenGLFunctions& functions = *GL::OpenGLFunctions::Functions();

    functions.glBindTexture((int)m_targetType, 0);

#ifdef DEBUG_MODE
    bool error = functions.printGLError("Error, failed to release texture");
    if (error) {
        throw("Error, failed to release texture");
    }
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::onRemoval(ResourceCache* cache)
{
    Q_UNUSED(cache)
    // Destroy GL-side texture on resource removal
    // Removed, done on delete now
    //destroy();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::postConstruction()
{
#ifdef DEBUG_MODE
    GL::OpenGLFunctions& functions = *GL::OpenGLFunctions::Functions();
    bool error = functions.printGLError("Error before constructing texture");
#endif

#ifdef DEBUG_MODE
    if (isCreated()) {
        logWarning("Texture " + m_handle->getName() + " already created, skipping post-construction");
        throw("Texture " + m_handle->getName() + " already created, skipping post-construction");
        return;
    }
#endif

    // Create texture in OpenGL
    initializeGL();

    // Bind texture
    bind();

    // Allocate storage
    allocateMemory();

    if (m_images.size()) {
        // Set image for GL texture
        flushData();
    }

    // Set mipmap rendering settings
    setTextureParameters();

    // Print any GL errors
#ifdef DEBUG_MODE
    error = functions.printGLError("Error post-constructing texture");
    if (error) {
        throw("Error, failed to construct texture");
    }
#endif

    // Release texture
    release();
	
	// Call parent class construction routine
	Resource::postConstruction();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::flushData()
{
    //if (!m_isBound) {
    //    throw("Error, texture must be bound to set data");
    //}

    GL::OpenGLFunctions& functions = *GL::OpenGLFunctions::Functions();

    // Check that image is valid
    if (!m_images.size()) {
        throw("Error, null image when trying to set texture data");
    }

    // Get pixel type and format of input image
    int format = (int)PixelFormat::kRGBA;

    // Store pixel data
    switch (m_targetType) {
    case(TextureTargetType::k2D):
    {
        // Convert image to internal-format friendly version
        QImage glImage = m_images.back().m_image.convertToFormat(QImage::Format_RGBA8888);
        const uchar* bits = glImage.constBits();

        functions.glTexSubImage2D(
            (GLint)m_targetType, //GL_TEXTURE_2D, or any of the cubemap faces!
            0, // mip-map level
            0, // x-offset
            0, // y-offset
            m_width, // texture width
            m_height, // texture height
            (int)format, // pixel format of input texture, is necessarily unsized
            (int)PixelType::kUByte8, // pixel data type, this is where the size comes in
            bits
        );
        break;
    }
    case(TextureTargetType::k2DMultisample):
        // Can only allocate data into an MSAA texture via framebuffer
        throw("SetData unsupported for this type");
        break;
    case(TextureTargetType::k2DArray):
    {
        size_t numImages = m_images.size();
        for (size_t depthLevel = 0; depthLevel < numImages; depthLevel++) {
            // Convert image to internal-format friendly version
            QImage glImage = m_images[depthLevel].m_image.convertToFormat(QImage::Format_RGBA8888);
            const uchar* bits = glImage.constBits();

            // Texture array
            functions.glTexSubImage3D(
                (GLint)m_targetType, //GL_TEXTURE_2D, or any of the cubemap faces!
                0, // mip-map level
                0, // x-offset
                0, // y-offset
                depthLevel, // z-offset
                m_width, // texture width
                m_height, // texture height,
                m_depth, // texture depth
                (int)format, // pixel format of input texture, is necessarily unsized
                (int)PixelType::kUByte8, // pixel data type, this is where the size comes in
                bits
            ); 
        }

        break;
    }
    case TextureTargetType::kCubeMap: 
    {
        size_t numFaces = m_images.size();
        for (size_t faceInt = 0; faceInt < numFaces; ++faceInt) {
            // Get image, be sure to convert to same format as internal storage
            QImage glImage = m_images[faceInt].m_image.convertToFormat(QImage::Format_RGBA8888);
            const uchar* bits = glImage.constBits();

            // Get size of images (all should be same dimensions)
            if (m_width != glImage.width()) {
                throw("Error, size mismatch");
            }
            if (m_height != glImage.height()) {
                throw("Error, size mismatch");
            }

            // Load image into GL
            glTexSubImage2D(
                (int)GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceInt,  // Specify which cubemap face is texture target
                0, // mip-map level
                0, // x-offset
                0, // y-offset
                m_width,  // resulting texture width
                m_height, // resulting texture height
                (int)format, // pixel format of input texture, is necessarily unsized
                (int)PixelType::kUByte8, // pixel data type, this is where the size comes in
                bits); // actual image data

#ifdef DEBUG_MODE
            functions.printGLError("flushData:: Failed to initialize cubemap texture");
#endif
        }
    }
        break;
    case TextureTargetType::kCubeMapArray: // Only supported in OpenGL 4.0+
    {
        size_t numImages = m_images.size();
        for (size_t cubemap = 0; cubemap < numImages; cubemap+=6) {

            for (size_t face = 0; face < 6; face++) {
                // Convert image to internal-format friendly version
                QImage glImage = m_images[cubemap].m_image.convertToFormat(QImage::Format_RGBA8888);
                const uchar* bits = glImage.constBits();

                // Texture array
                functions.glTexSubImage3D(
                    (GLint)m_targetType, // GL_TEXTURE_CUBE_MAP_ARRAY
                    0, // mip-map level
                    0, // x-offset
                    0, // y-offset
                    cubemap + face, // z-offset, if you want to upload to just the positive z face of the second layer in the array, you would use zoffset parameter 10 (layer 1 * 6 faces per layer + face index (4))
                    m_width, // texture width
                    m_height, // texture height,
                    1, // number of faces to upload
                    (int)format, // pixel format of input texture, is necessarily unsized
                    (int)PixelType::kUByte8, // pixel data type, this is where the size comes in
                    bits
                );
            }
        }

        break;
    }
    default:
        throw("Texture type unimplemented");
        break;
    }

#ifdef DEBUG_MODE
    bool error = functions.printGLError("Error, failed to set texture data");
    if (error) {
        throw("Error, failed to set texture data");
    }
#endif

    functions.glFlush();

    // Clear cached images, no longer needed once the buffer lives in GL
    m_images.clear();

    // Generate mip maps automatically (requires OpenGL 3.0+)
    if (hasMipMaps()) {
        generateMipMaps();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::generateMipMaps()
{
    //if (!m_isBound) {
    //    throw("Error, texture must be bound to generate mip maps");
    //}

    GL::OpenGLFunctions& functions = *GL::OpenGLFunctions::Functions();
    functions.glGenerateMipmap((int)m_targetType);

#ifdef DEBUG_MODE
    bool error = functions.printGLError("Error, failed to generate mip maps");
    if (error) {
        throw("Error, failed to generate mip maps");
    }
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::setTextureParameters()
{

    GL::OpenGLFunctions& functions = *GL::OpenGLFunctions::Functions();
    if (m_textureOptions.m_minFilter != TextureFilter::kUnset) {
        functions.glTexParameteri((int)m_targetType, GL_TEXTURE_MIN_FILTER, (int)m_textureOptions.m_minFilter);
    }
    if (m_textureOptions.m_magFilter != TextureFilter::kUnset) {
        functions.glTexParameteri((int)m_targetType, GL_TEXTURE_MAG_FILTER, (int)m_textureOptions.m_magFilter);
    }
    functions.glTexParameteri((int)m_targetType, GL_TEXTURE_WRAP_S, (int)m_textureOptions.m_wrapMode);
    functions.glTexParameteri((int)m_targetType, GL_TEXTURE_WRAP_T, (int)m_textureOptions.m_wrapMode);
   
    // Set target-specific parameters
    switch (m_targetType) {
    case TextureTargetType::k2DArray:
        // Make sure that 0 is base mip-map level (should be default anyway)
        glTexParameteri((int)m_targetType, GL_TEXTURE_BASE_LEVEL, 0);

        // Make sure that no mip maps are required, 
        // Possibly unnecessary, but don't want attaching to an FBO to raise
        // an incomplete attachment error
        glTexParameteri((int)m_targetType, GL_TEXTURE_MAX_LEVEL, 0);
        break;
    case TextureTargetType::kCubeMap:
    case TextureTargetType::kCubeMapArray:
        // May not be compatible with Open GL ES
        functions.glTexParameteri((int)m_targetType, GL_TEXTURE_WRAP_R, (int)m_textureOptions.m_wrapMode);
        break;
    }

#ifdef DEBUG_MODE
    bool error = functions.printGLError("Error, failed to set texture parameters");
    if (error) {
        throw("Error, failed to set texture parameters");
    }
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::allocateMemory()
{
    //if (!m_isBound) {
    //    throw("Error, texture must be bound to allocate memory");
    //}

    GL::OpenGLFunctions& functions = *GL::OpenGLFunctions::Functions();

    int mipMapLevels = hasMipMaps() ? maxMipMapLevel() : 1;
    switch (m_targetType) {
    case TextureTargetType::k2D:
    case TextureTargetType::kCubeMap: // Cubemap works with this, see OpenGL spec
        // Standard Texture
        // Create immutable storage, as opposed to glTexImage2D
        functions.glTexStorage2D(
            (int)m_targetType, // Target
            mipMapLevels, // number of mip-map levels
            (int)m_textureOptions.m_internalFormat, // Internal format, must be sized
            m_width, // Width
            m_height // Height
        );
        break;
    case TextureTargetType::k2DMultisample:
        // Create immutable storage, as opposed to glTexImage2D
        // For framebuffers, fixedSampleLocations should be true
        functions.glTexStorage2DMultisample(
            (int)m_targetType, // Target
            m_textureOptions.m_numSamples, // number of samples
            (int)m_textureOptions.m_internalFormat, // Internal format, must be sized
            m_width, // Width
            m_height, // Height
            m_textureOptions.m_fixedSampleLocations // Whether the image will use identical sample locations and same number ofsamples for all texels in image
        );
        break;
    case TextureTargetType::k2DArray:
        // Texture Array
        // Create immutable storage, as opposed to glTexImage3D
        functions.glTexStorage3D(
            (int)m_targetType, // Target
            mipMapLevels, // number of mip-map levels
            (int)m_textureOptions.m_internalFormat, // Internal format, must be sized
            m_width, // Width
            m_height, // Height
            m_depth // Depth (number of sub-images for texture array)
        );
        break;
    case TextureTargetType::kCubeMapArray:
        // Cubemap array
        // Create immutable storage, as opposed to glTexImage3D
        functions.glTexStorage3D(
            (int)m_targetType, // Target
            mipMapLevels, // number of mip-map levels
            (int)m_textureOptions.m_internalFormat, // Internal format, must be sized
            m_width, // Width
            m_height, // Height
            m_depth * 6 // number of cubemap faces in cubemap array
        );
        break;
    default:
        throw("Texture type unimplemented");
        break;
    }

    // Once memory is allocated for the texture, the cost is known
    setCost();

#ifdef DEBUG_MODE
    bool error = functions.printGLError("Error, failed to allocate memory");
    if (error) {
        throw("Error, failed to allocate memory");
    }
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::setCost()
{
    // Cost is the size of image in Mb, since no image is cached on CPU side
    // See: https://community.khronos.org/t/does-glteximage2d-copy-the-texture-data/1952
    m_cost = sizeof(Texture);
    size_t pixelSize = sizeof(GLubyte); // Internal format is kUByte8: GL_UNSIGNED_BYTE
    size_t imageCost = m_width * m_height * m_depth * pixelSize / 1000.0;
    if (m_targetType == TextureTargetType::kCubeMap) {
        // Cost for cubemap images
        imageCost *= 6;
    }
    m_cost += imageCost;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::destroy()
{
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
    if (isCreated()) {
        //m_width = m_height = m_depth = 1;
        //bind();
        //allocateMemory(); // FIXME: Needs texImage, can't resize. Reallocated memory in case deletion is deferred
        //release();
        gl.glDeleteTextures(1, &m_textureID);
    }
    else {
        throw("Error attempted to destroy uninitialized texture");
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
size_t Texture::maxMipMapLevel()
{
    // See:
    // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexStorage2D.xhtml
    size_t max = 0;
    switch (m_targetType) {
    case TextureTargetType::k1DArray:
        max = abs(log2(m_width) + 1);
        break;
    default:
        max = abs(log2(std::max(m_width, m_height)) + 1);
        break;
    }
    return max;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::initializeGL()
{
    GL::OpenGLFunctions& functions = *GL::OpenGLFunctions::Functions();

    if (isCreated()) {
        throw("Error, cannot recreate texture");
    }

    // Create actual texture
    functions.glGenTextures(1, &m_textureID);

    // Set as initialized
    m_textureFlags.setFlag(kInitialized, true);

#ifdef DEBUG_MODE
    bool error = functions.printGLError("Error, failed to create texture");
    if (error) {
        throw("Error, failed to create texture");
    }
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::initialize(const Image & img, bool mirror)
{
    m_images.push_back(img);
    Image& image = m_images.back();
    if (mirror) {
        // Only images loaded from file as QImage need to be mirrored
        // Things like generated bitmaps are not (and should not be) mirrored
        image.m_image = image.m_image.mirrored(false, true);
    }
    m_height = image.size().height();
    m_width = image.size().width();

    setCost();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::initialize(const QString& filePath)
{
    // Load image from file path and check validity
    // Need to flip image due to Qt image convention
    // Mirrored to have (0,0) texture coordinate at the lower left
    bool exists = QFile(filePath).exists();
    if (exists) {
        initialize(Gb::Image(filePath), true);
    }
    else {
#ifdef DEBUG_MODE
        throw("Error, filepath does not exist");
#endif
    }

}
///////////////////////////////////////////////////////////////////////////////////////////////
//void Texture::getGLImage(Image& outImage)
//{
//    GL::OpenGLFunctions& functions = *GL::OpenGLFunctions::Functions();
//
//    // See: https://interest.qt-project.narkive.com/BfolPsvj/qimage-from-shadereffectsource
//    // Note, glGetTexImage does not exist in OpenGL ES, would need to bind texture into an FBO
//    // and use glReadPixels, which has similar semantics
//    bind(0);
//    functions.glActiveTexture(GL_TEXTURE0);
//
//    GLint id = getID();
//    GLint boundID;
//    functions.glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundID);
//    if (!id == boundID) throw("Error, failed to bind current texture");
//
//    GLint internalFormat;
//    functions.glGetTexLevelParameteriv(GL_TEXTURE_2D, // target
//        0, // level
//        GL_TEXTURE_INTERNAL_FORMAT,
//        &internalFormat); // get internal format type of GL texture
//    int w = width();
//    int h = height();
//    if (w == 0) throw("Error, image has no width");
//    if (h == 0) throw("Error, image has no height");
//    //outImage.m_image = QImage(width(), height(), QImage::Format_RGB32);
//    outImage.m_image = QImage(width(), height(), QImage::Format_RGBX8888);
//
//    // FIXME: Doesn't work
//    //uchar* bits = outImage.m_image.bits();
//    //glGetTexImage(GL_TEXTURE_2D, 
//    //    0, 
//    //    internalFormat, 
//    //    GL_RGBA8, // GL_RGBA GL_UNSIGNED_BYTE, GL_UNSIGNED_INT_8_8_8_8,
//    //    bits); // TODO: Make portable with OpenGL ES
//
//    // See: http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
//    // http://www.songho.ca/opengl/gl_fbo.html
//    auto* buffer = new QGLFramebufferObject(w, h, GL_TEXTURE_2D);
//    buffer->bind();
//    functions.glFramebufferTexture2D(GL_FRAMEBUFFER, // target
//        GL_COLOR_ATTACHMENT0, // attachment
//        GL_TEXTURE_2D,
//        id, // name of texture
//        0); // level
//
//    // define an array of buffers into which outputs from the fragment shader data will be written
//    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
//    functions.glDrawBuffers(1, // Size of buffers array
//        drawBuffers);
//
//    GLint status = functions.glCheckFramebufferStatus(GL_FRAMEBUFFER);
//    if (status != GL_FRAMEBUFFER_COMPLETE)
//        throw("Messed up FBO");
//    
//    outImage.m_image = buffer->toImage();
//    buffer->release();
//    delete buffer;
//}

/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}