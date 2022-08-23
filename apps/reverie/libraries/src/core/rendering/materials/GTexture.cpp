#include "core/rendering/materials/GTexture.h"

// Qt
#include <QDebug>

// internal

#include "fortress/containers/GColor.h"
#include "fortress/image/GImage.h"
#include "fortress/system/memory/GPointerTypes.h"
#include "fortress/system/path/GFile.h"
#include "fortress/system/path/GPath.h"

//#include <QOpenGLFunctions_3_2_Core> 
#include "core/rendering/GGLFunctions.h"
#include "core/GCoreEngine.h"
#include "core/resource/GResourceCache.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "fortress/process/GProcess.h"
#include "fortress/json/GJson.h"
#include "core/rendering/GGLFunctions.h"
//#include "QGLFramebufferObject" // TODO: Replace with my own FBO class
#include "core/rendering/view/GFrameBuffer.h"

namespace rev {


TextureData::TextureData(bool isBump)
{
    m_textureFileName[0] = 0; // Initialize file name to null string
    initialize(isBump);
}

TextureData::~TextureData()
{
}

void TextureData::initialize(bool isBump)
{
    if (isBump) {
        m_properties.m_imfChannel = 'l';
    }
    else {
        m_properties.m_imfChannel = 'm';
    }
}

void TextureData::setFileName(const GString& fileName)
{
    if (fileName.length() > MAX_TEX_PATH_SIZE) {
        Logger::Throw("Error, path size too long");
    }
    memcpy(m_textureFileName.data(), fileName.c_str(), fileName.length());
    m_textureFileName[fileName.length()] = 0;
}



uint32_t Texture::MaxDivisibleTextureSize()
{
    uint32_t maxSize = GL_MAX_TEXTURE_SIZE;

    uint32_t digits = 0;
    while (maxSize > 0) {
        maxSize >>= 1;
        digits++;
    }

    return 1 << (digits - 1);
}

uint32_t Texture::MaxTextureSize()
{
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    GLint maxSize;
    gl.glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    return (uint32_t)maxSize;
}


uint32_t Texture::MaxNumTextureLayers()
{
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    GLint maxSize;
    gl.glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxSize);
    return (uint32_t)maxSize;
}

uint32_t Texture::TextureBoundToActiveUnit()
{
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    GLint whichID;
    gl.glGetIntegerv(GL_TEXTURE_BINDING_2D, &whichID);
    return (uint32_t)whichID;
}


const GString& Texture::GetUniformName(TextureUsageType type)
{
    /// Map of texture types and corresponding uniform name
    static const std::vector<GString> s_typeToUniformVec = {
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

    return s_typeToUniformVec.at((int)type);
}

std::shared_ptr<ResourceHandle> Texture::CreateHandle(CoreEngine * engine,
    const GString & texturePath,
    TextureUsageType type,
    bool loadSerially)
{
    // Create texture handle and add to cache
    auto textureHandle = prot_make_shared<ResourceHandle>(engine,
        texturePath, EResourceType::eTexture);

    // Set handle attributes
    json oJson;
    oJson["texUsageType"] = (int)type;
    textureHandle->setCachedResourceJson(oJson);

    // Add handle to resource cache
    ResourceCache::Instance().insertHandle(textureHandle);

    // Load texture
    textureHandle->loadResource(loadSerially);

    return textureHandle;
}

std::shared_ptr<ResourceHandle> Texture::CreateHandle(const GString & texturePath,
    TextureUsageType type,
    ResourceHandle& material)
{
    // Create texture handle and add to cache
    auto textureHandle = prot_make_shared<ResourceHandle>(material.engine(),
        texturePath, EResourceType::eTexture);

    // Need to set as a child handle before inserting into resource cache
    textureHandle->setChild(true);
    ResourceCache::Instance().insertHandle(textureHandle);

    // Set handle attributes
    json oJson;
    oJson["texUsageType"] = (int)type;
    textureHandle->setCachedResourceJson(oJson);

    // Actually add to material as a child
    material.addChild(textureHandle);

    return textureHandle;
}

std::shared_ptr<ResourceHandle> Texture::CreateHandle(CoreEngine* core,
    Image & image, TextureUsageType type,
    TextureFilter minFilter,
    TextureFilter maxFilter,
    TextureWrapMode wrapMode)
{
    std::shared_ptr<ResourceHandle> handle = ResourceHandle::create(core, (GResourceType)EResourceType::eTexture);
    //handle->setResourceType(EResourceType::eTexture);

    // Create texture, passing ownership of image to the texture
    auto texture = prot_make_unique<Texture>(image,
        type, minFilter, maxFilter, wrapMode);
    handle->setResource(std::move(texture), false);

    // Emit signal to run post-construction
    handle->setIsLoading(true);
    ResourceCache::Instance().incrementLoadCount();
    emit ResourceCache::Instance().doneLoadingResource(handle->getUuid()); // run post-construction
    return handle;
}


Texture::Texture(const QString & filePath, TextureUsageType type):
    m_usageType(type),
    m_targetType(TextureTargetType::k2D),
    m_depth(1)
{
    // Load image
    initialize(filePath);
}

Texture::Texture(Image & image, TextureUsageType type,
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
    initialize(image);
}

Texture::Texture(uint32_t width,
    uint32_t height, 
    TextureTargetType targetType,
    TextureUsageType type,
    TextureFilter minFilter, 
    TextureFilter magFilter,
    TextureWrapMode wrapMode,
    TextureFormat format, 
    uint32_t depth) :
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

Texture::~Texture()
{
    // Destroy underlying GL texture on destruction
    destroy();
}

Image Texture::getImage(PixelFormat pixelFormat, PixelType pixelType, TextureFormat texFormat) const
{
    // TODO: This might not be correct generally
    /// \see https://stackoverflow.com/questions/34497195/difference-between-format-and-internalformat
    // Maybe use m_internalFormat so this doesn't need to be passed in
    // Note: Sized types are typically used internally, unsized are for backwards compatibility
    static tsl::robin_map<int, Image::ColorFormat> texFormatMap = {
        {GL_RGB16F, Image::ColorFormat::kRGB888},
        {GL_RGB8, Image::ColorFormat::kRGB888},
        {GL_RGBA8, Image::ColorFormat::kRGBA8888},
        {GL_RGBA16F, Image::ColorFormat::kRGBA8888},
        {GL_R8, Image::ColorFormat::kGrayscale8},
        {GL_RED, Image::ColorFormat::kGrayscale8}
    };

    if (!Map::HasKey(texFormatMap, (int)texFormat)) {
        Logger::Throw("Internal type unsupported");
    }
    Image::ColorFormat format = texFormatMap[(int)texFormat];

    std::vector<int> pixels; // 8 bytes per int, intentionally
    pixels.resize(m_width * m_height * m_depth * 6, -1);
    getData(pixels.data(), pixelFormat, pixelType);

    // Create image, taking ownership of the buffer
    Image image((const Uint8_t*)pixels.data(), m_width, m_height, format);

    // Hard-coded to test cubemaps
    //const char* fmt = "C:\\Users\\dante\\Documents\\Projects\\grand-blue-engine\\depth\\%d.jpg";
    //for (uint32_t i = 0; i < m_depth; i++) {
    //    for (uint32_t j = 0; j < 6; j++) {
    //        uint32_t layerIndex = j + 6 * i;
    //        uint32_t offset = layerIndex * (m_width * m_height);
    //        image = QImage((const unsigned char*)(pixels.data() + offset), m_width, m_height, format);
    //        std::string path = GString::Format(fmt, layerIndex);
    //        image.mirrored().save(path.c_str());
    //    }
    //}
    return image;
}

TextureData Texture::asTexdata() const
{
    TextureData data;
    
    // Copy filename
    GFile textureFile(m_handle->getPath());
    GString fileName = textureFile.getFileName(true, true);
    data.setFileName(fileName);

    data.m_properties.m_usageType = m_usageType;

    return data;
}

void Texture::getData(void * outData, PixelFormat format, PixelType pixelType) const
{
    bind();

    //gl::OpenGLFunctions& functions = *gl::OpenGLFunctions::Functions();

    switch (m_targetType) {
    case TextureTargetType::k2D:
    case TextureTargetType::kCubeMapArray:
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
    // TODO: See https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetTextureSubImage.xhtml
    default:
        Logger::Throw("Error, unimplemented");
    }

#ifdef DEBUG_MODE
    gl::OpenGLFunctions& functions = *gl::OpenGLFunctions::Functions();
    bool error = functions.printGLError("Error, failed to get texture data");
    if (error) {
        Logger::Throw("Error, failed to get texture data");
    }
#endif

    release();
}

void Texture::setData(const void * data, PixelFormat format, PixelType pixelType)
{
    bind();

    gl::OpenGLFunctions& functions = *gl::OpenGLFunctions::Functions();

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
        Logger::Throw("Texture type unimplemented");
        break;
    }

#ifdef DEBUG_MODE
    bool error = functions.printGLError("Error, failed to set texture data");
    if (error) {
        Logger::Throw("Error, failed to set texture data");
    }
#endif

    functions.glFlush();

    // Generate mip maps automatically (requires OpenGL 3.0+)
    if (hasMipMaps()) {
        generateMipMaps();
    }

    release();
}

void Texture::setBorderColor(const Color & color)
{
    Vector4 colorVec = color.toVector<Real_t, 4>();
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, colorVec.data());
}

void Texture::attach(FrameBuffer & fbo, int attachment, uint32_t depthLayer)
{
    gl::OpenGLFunctions& functions = *gl::OpenGLFunctions::Functions();

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
        /// \see https://gamedev.stackexchange.com/questions/109199/how-to-attach-a-framebuffer-to-a-whole-cube-map-from-a-gl-texture-cube-map-array
        //functions.glFramebufferTextureLayer(GL_FRAMEBUFFER,
        //    attachment, // e.g. GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT
        //    m_textureID,
        //    0, // Mipmap level of texture array to attach
        //    depthLayer * 6); // Layer must be multiplied by 6, since 6 faces per layer


        // "For glFramebufferTexture and glNamedFramebufferTexture, if texture is the name of a 
        // three-dimensional, cube map array, cube map, one- or two-dimensional array, or
        // two-dimensional multisample array texture, the specified texture level is an array of 
        // images, and the framebuffer attachment is considered to be layered."

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
        Logger::Throw("Error, texture type not yet implemented for attachments");
        break;
    }

#ifdef DEBUG_MODE
    bool error = functions.printGLError("Error attaching texture");
    if (error) {
        Logger::Throw("Error attaching texture");
    }
#endif

    if (!wasBound) {
        fbo.release();
    }
}

void Texture::setReadMode(DepthStencilMode mode)
{
    gl::OpenGLFunctions& functions = *gl::OpenGLFunctions::Functions();

    if (m_textureOptions.m_internalFormat != TextureFormat::kDepth24Stencil8 &&
        m_textureOptions.m_internalFormat != TextureFormat::kDepth32FStencil8X24) {
        Logger::Throw("Error, incorrect texture format to set depth stencil mode");
    }

    functions.glTexParameteri((int)m_targetType, GL_DEPTH_STENCIL_TEXTURE_MODE, (int)mode);
}

void Texture::bind(int texUnit) const
{
    //if (m_isBound) {
    //    Logger::Throw("Error, texture already bound");
    //}
    //m_isBound = true;

    gl::OpenGLFunctions& functions = *gl::OpenGLFunctions::Functions();

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
        Logger::Throw("Error, failed to bind texture");
    }
#endif
}

void Texture::release() const
{
    //if (!m_isBound) {
    //    return;
    //}

    //m_isBound = false;

    // Release is just binding the default texture
    gl::OpenGLFunctions& functions = *gl::OpenGLFunctions::Functions();

    functions.glBindTexture((int)m_targetType, 0);

#ifdef DEBUG_MODE
    bool error = functions.printGLError("Error, failed to release texture");
    if (error) {
        Logger::Throw("Error, failed to release texture");
    }
#endif
}

void Texture::onRemoval(ResourceCache* cache)
{
    Q_UNUSED(cache)
    // Destroy GL-side texture on resource removal
    // Removed, done on delete now
    //destroy();
}

void Texture::postConstruction()
{
#ifdef DEBUG_MODE
    gl::OpenGLFunctions& functions = *gl::OpenGLFunctions::Functions();
    bool error = functions.printGLError("Error before constructing texture");
#endif

#ifdef DEBUG_MODE
    if (isCreated()) {
        Logger::LogWarning("Texture " + m_handle->getName() + " already created, skipping post-construction");
        Logger::Throw("Texture " + m_handle->getName() + " already created, skipping post-construction");
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
        Logger::Throw("Error, failed to construct texture");
    }
#endif

    // Release texture
    release();
	
	// Call parent class construction routine
	Resource::postConstruction();
}

void Texture::flushData()
{
    //if (!m_isBound) {
    //    Logger::Throw("Error, texture must be bound to set data");
    //}

    gl::OpenGLFunctions& functions = *gl::OpenGLFunctions::Functions();

    // Check that image is valid
    if (!m_images.size()) {
        Logger::Throw("Error, null image when trying to set texture data");
    }

    // Get pixel type and format of input image
    static constexpr int InputPixelFormat = (int)PixelFormat::kRGBA;
    static constexpr int PixelDataType = (int)PixelType::kUByte8;

    // Store pixel data
    switch (m_targetType) {
    case(TextureTargetType::k2D):
    {
        // Convert image to internal-format friendly version
        Image glImage = m_images.back().convertToFormat(Image::StandardColorFormat);
        const Uint8_t* bits = glImage.buffer();

        functions.glTexSubImage2D(
            (GLint)m_targetType, //GL_TEXTURE_2D, or any of the cubemap faces!
            0, // mip-map level
            0, // x-offset
            0, // y-offset
            m_width, // texture width
            m_height, // texture height
            InputPixelFormat, // pixel format of input texture, is necessarily unsized
            PixelDataType, // pixel data type, this is where the size comes in
            bits
        );
        break;
    }
    case(TextureTargetType::k2DMultisample):
        // Can only allocate data into an MSAA texture via framebuffer
        Logger::Throw("SetData unsupported for this type");
        break;
    case(TextureTargetType::k2DArray):
    {
        uint32_t numImages = (uint32_t)m_images.size();
        for (uint32_t depthLevel = 0; depthLevel < numImages; depthLevel++) {
            // Convert image to internal-format friendly version
            Image glImage = m_images[depthLevel].convertToFormat(Image::StandardColorFormat);
            const uchar* bits = glImage.buffer();

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
                InputPixelFormat, // pixel format of input texture, is necessarily unsized
                PixelDataType, // pixel data type, this is where the size comes in
                bits
            ); 
        }

        break;
    }
    case TextureTargetType::kCubeMap: 
    {
        uint32_t numFaces = (uint32_t)m_images.size();
        for (uint32_t faceInt = 0; faceInt < numFaces; ++faceInt) {
            // Get image, be sure to convert to same format as internal storage
            Image glImage = m_images[faceInt].convertToFormat(Image::StandardColorFormat);
            const Uint8_t* bits = glImage.buffer();

            // Get size of images (all should be same dimensions)
            if (m_width != glImage.width()) {
                Logger::Throw("Error, size mismatch");
            }
            if (m_height != glImage.height()) {
                Logger::Throw("Error, size mismatch");
            }

            // Load image into GL
            glTexSubImage2D(
                (int)GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceInt,  // Specify which cubemap face is texture target
                0, // mip-map level
                0, // x-offset
                0, // y-offset
                m_width,  // resulting texture width
                m_height, // resulting texture height
                InputPixelFormat, // pixel format of input texture, is necessarily unsized
                PixelDataType, // pixel data type, this is where the size comes in
                bits); // actual image data

#ifdef DEBUG_MODE
            functions.printGLError("flushData:: Failed to initialize cubemap texture");
#endif
        }
    }
        break;
    case TextureTargetType::kCubeMapArray: // Only supported in OpenGL 4.0+
    {
        uint32_t numImages = (uint32_t)m_images.size();
        for (uint32_t cubemap = 0; cubemap < numImages; cubemap+=6) {

            for (uint32_t face = 0; face < 6; face++) {
                // Convert image to internal-format friendly version
                Image glImage = m_images[cubemap].convertToFormat(Image::StandardColorFormat);
                const Uint8_t* bits = glImage.buffer();

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
                    InputPixelFormat, // pixel format of input texture, is necessarily unsized
                    PixelDataType, // pixel data type, this is where the size comes in
                    bits
                );
            }
        }

        break;
    }
    default:
        Logger::Throw("Texture type unimplemented");
        break;
    }

#ifdef DEBUG_MODE
    bool error = functions.printGLError("Error, failed to set texture data");
    if (error) {
        Logger::Throw("Error, failed to set texture data");
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

void Texture::generateMipMaps()
{
    //if (!m_isBound) {
    //    Logger::Throw("Error, texture must be bound to generate mip maps");
    //}

    gl::OpenGLFunctions& functions = *gl::OpenGLFunctions::Functions();
    functions.glGenerateMipmap((int)m_targetType);

#ifdef DEBUG_MODE
    bool error = functions.printGLError("Error, failed to generate mip maps");
    if (error) {
        Logger::Throw("Error, failed to generate mip maps");
    }
#endif
}

void Texture::setTextureParameters()
{

    gl::OpenGLFunctions& functions = *gl::OpenGLFunctions::Functions();
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
        Logger::Throw("Error, failed to set texture parameters");
    }
#endif
}

void Texture::allocateMemory()
{
    //if (!m_isBound) {
    //    Logger::Throw("Error, texture must be bound to allocate memory");
    //}

    gl::OpenGLFunctions& functions = *gl::OpenGLFunctions::Functions();

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
        Logger::Throw("Texture type unimplemented");
        break;
    }

    // Once memory is allocated for the texture, the cost is known
    setCost();

#ifdef DEBUG_MODE
    bool error = functions.printGLError("Error, failed to allocate memory");
    if (error) {
        Logger::Throw("Error, failed to allocate memory");
    }
#endif
}

void Texture::setCost()
{
    // Cost is the size of image in Mb, since no image is cached on CPU side
    /// \see https://community.khronos.org/t/does-glteximage2d-copy-the-texture-data/1952
    m_cost = sizeof(Texture);
    size_t pixelSize = sizeof(GLubyte); // Internal format is kUByte8: GL_UNSIGNED_BYTE
    size_t imageCost = (size_t)m_width * (size_t)m_height * (size_t)m_depth * pixelSize / 1000.0;
    if (m_targetType == TextureTargetType::kCubeMap) {
        // Cost for cubemap images
        imageCost *= 6;
    }
    m_cost += imageCost;
}

void Texture::destroy()
{
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    if (isCreated()) {
        //m_width = m_height = m_depth = 1;
        //bind();
        //allocateMemory(); // FIXME: Needs texImage, can't resize. Reallocated memory in case deletion is deferred
        //release();
        gl.glDeleteTextures(1, &m_textureID);
    }
    else {
        Logger::Throw("Error attempted to destroy uninitialized texture");
    }
}

uint32_t Texture::maxMipMapLevel()
{
    /// \see 
    // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexStorage2D.xhtml
    uint32_t max = 0;
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

void Texture::initializeGL()
{
    gl::OpenGLFunctions& functions = *gl::OpenGLFunctions::Functions();

    if (isCreated()) {
        Logger::Throw("Error, cannot recreate texture");
    }

    // Create actual texture
    functions.glGenTextures(1, &m_textureID);

    // Set as initialized
    m_textureFlags.setFlag(kInitialized, true);

#ifdef DEBUG_MODE
    bool error = functions.printGLError("Error, failed to create texture");
    if (error) {
        Logger::Throw("Error, failed to create texture");
    }
#endif
}

void Texture::initialize(Image & img)
{
    /// Create image without duplicating buffer
    m_images.emplace_back(img);
    Image& image = m_images.back();

    /// Set dimensions based on image
    m_height = image.size().height();
    m_width = image.size().width();
    setCost();
}

void Texture::initialize(const QString& filePath)
{
    /// Load image from file path and check validity
    /// @note Need to flip image vertically due to OpenGL image convention
    //        Mirrored to have (0,0) texture coordinate at the lower left
    bool exists = QFile(filePath).exists();
    if (exists) {
        initialize(Image(GString(filePath.toStdString().c_str()), true));
    }
    else {
#ifdef DEBUG_MODE
        Logger::Throw("Error, filepath does not exist");
#endif
        Logger::LogError("Error, filepath does not exist");
    }

}

/// @todo, implement
void to_json(nlohmann::json& orJson, const Texture& korObject)
{
}

void from_json(const nlohmann::json& korJson, Texture& orObject)
{
}

// End namespaces
}