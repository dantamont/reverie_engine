#include "GbTexture.h"

// Qt
#include <QDebug>

// internal
#include "../../utils/GbMemoryManager.h"
#include "../../GbCoreEngine.h"
#include "../../resource/GbResourceCache.h"
#include "../shaders/GbShaders.h"
#include "../../processes/GbProcess.h"
#include "../../readers/GbJsonReader.h"
#include "../GbGLFunctions.h"
#include "QGLFramebufferObject"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
TextureData::TextureData(bool isBump)
{
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
const std::unordered_map<Texture::TextureType, QString> Texture::TYPE_TO_UNIFORM_MAP = {
    {kDiffuse, "material.diffuseTexture"},
    {kNormalMap, "material.normalMap"}
};

/////////////////////////////////////////////////////////////////////////////////////////////
Texture::Texture(const QString & filePath, TextureType type):
    Resource(kTexture),
    m_texture(QOpenGLTexture::Target2D), // Does not create underlying OpenGL object, so safe without context
    m_type(type)
{
    // Load image
    initialize(filePath);

    // Run post-construction if on the main thread, otherwise return
    //if (Process::isMainThread()) {
    //    postConstruction();
    //}
}
/////////////////////////////////////////////////////////////////////////////////////////////
Texture::Texture(const Image & image, TextureType type,
    QOpenGLTexture::Filter minFilter,
    QOpenGLTexture::Filter magFilter,
    QOpenGLTexture::WrapMode wrapMode):
    Resource(kTexture),
    m_image(image),
    m_texture(QOpenGLTexture::Target2D), // Does not create underlying OpenGL object, so safe without context
    m_type(type),
    m_minFilter(minFilter),
    m_magFilter(magFilter),
    m_wrapMode(wrapMode)
{
    setCost();

    // Run post-construction if on the main thread, otherwise return
    //if (Process::isMainThread()) {
    //    postConstruction();
    //}
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> Texture::createHandle(CoreEngine * engine,
    const QString & texturePath, 
    TextureType type)
{
    // Create texture handle and add to cache
    auto textureHandle = prot_make_shared<ResourceHandle>(engine,
        texturePath, Resource::kTexture);

    // Set handle attributes
    textureHandle->attributes()["type"] = (int)type;

    // Add handle to resource cache
    engine->resourceCache()->insertHandle(textureHandle);

    // Load texture
    textureHandle->loadResource();

    return textureHandle;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> Texture::createHandle(const QString & texturePath,
    TextureType type, 
    ResourceHandle& material)
{
    // Create texture handle and add to cache
    auto textureHandle = prot_make_shared<ResourceHandle>(material.engine(), 
        texturePath, Resource::kTexture);

    // Need to set as a child handle before inserting into resource cache
    textureHandle->setChild(true);
    material.engine()->resourceCache()->insertHandle(textureHandle);

    // Set handle attributes
    textureHandle->attributes()["type"] = (int)type;

    // Actually add to material as a child
    material.addChild(textureHandle);

    return textureHandle;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> Texture::createHandle(CoreEngine* core, 
    const Image & image, TextureType type,
    QOpenGLTexture::Filter minFilter, 
    QOpenGLTexture::Filter maxFilter, 
    QOpenGLTexture::WrapMode wrapMode)
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
Texture::~Texture()
{
    // Destroy underlying GL texture on destruction
    // Requires a current valid OpenGL context
    //if (!QOpenGLContext::currentContext()) {
    //    throw("Error, no current OpenGL context found");
    //}
    destroy();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::bind(int texUnit)
{
    if (texUnit > -1) {
        // Texture unit was specified
        m_texture.bind(size_t(texUnit));
    }
    else {
        m_texture.bind();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::onRemoval(ResourceCache* cache)
{
    Q_UNUSED(cache)
    // Destroy GL-side texture on resource removal
    destroy();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::postConstruction()
{
#ifdef DEBUG_MODE
    GL::OpenGLFunctions functions;
    bool error = functions.printGLError("Error before constructing texture");
#endif

#ifdef DEBUG_MODE
    if (m_texture.isCreated()) {
        logWarning("Texture " + m_handle->getName()
            + " already created, skipping post-construction");
        return;
    }
#endif

    // Set image for GL texture, mirrored to have (0,0) texture coordinate at the lower left
    m_texture.setData(m_image.m_image);
    //m_texture.setData(m_image.m_image.convertToFormat(QImage::Format::Format_RGBA8888));

    // Set mipmap generation settings
    m_texture.setMinificationFilter(m_minFilter);
    m_texture.setMagnificationFilter(m_magFilter);
    m_texture.setWrapMode(m_wrapMode);

    // Print any GL errors
#ifdef DEBUG_MODE
    error = functions.printGLError("Error post-constructing texture");
#endif
	
	// Call parent class construction routine
	Resource::postConstruction();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::setCost()
{
    // Cost is twice the size of image in Mb, since image is also copied by GL
    // See: https://community.khronos.org/t/does-glteximage2d-copy-the-texture-data/1952
    // TODO: Possible remove image as a member
    if (!m_image.m_image.isNull()) {
        m_cost = 2 * m_image.sizeInMegaBytes();
    }
    else {
        m_cost = 0;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::initialize(const QString& filePath)
{
    // Load image from file path and check validity
    // Need to flip image due to Qt image convention
    // Mirrored to have (0,0) texture coordinate at the lower left
    bool exists = QFile(filePath).exists();
    if (exists) {
        // For .tga: https://forum.qt.io/topic/74712/qimage-from-tga-with-alpha/11
        // https://forum.qt.io/topic/101971/qimage-and-tga-support-in-c/5
        //QImage qImage = QImage(filePath);
        //QImageReader reader(filePath);
        //QImage qImage = reader.read();
        m_image = Gb::Image(filePath);
        m_image.m_image = m_image.m_image.mirrored(false, true);
    }
    else {
#ifdef DEBUG_MODE
        throw("Error, filepath does not exist");
#endif
    }

    setCost();
  
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::getGLImage(Image& outImage)
{
    GL::OpenGLFunctions functions;

    // See: https://interest.qt-project.narkive.com/BfolPsvj/qimage-from-shadereffectsource
    // Note, glGetTexImage does not exist in OpenGL ES, would need to bind texture into an FBO
    // and use glReadPixels, which has similar semantics
    bind(0);
    functions.glActiveTexture(GL_TEXTURE0);

    GLint id = getID();
    GLint boundID;
    functions.glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundID);
    if (!id == boundID) throw("Error, failed to bind current texture");

    GLint internalFormat;
    functions.glGetTexLevelParameteriv(GL_TEXTURE_2D, // target
        0, // level
        GL_TEXTURE_INTERNAL_FORMAT,
        &internalFormat); // get internal format type of GL texture
    int w = width();
    int h = height();
    if (w == 0) throw("Error, image has no width");
    if (h == 0) throw("Error, image has no height");
    //outImage.m_image = QImage(width(), height(), QImage::Format_RGB32);
    outImage.m_image = QImage(width(), height(), QImage::Format_RGBX8888);

    // FIXME: Doesn't work
    //uchar* bits = outImage.m_image.bits();
    //glGetTexImage(GL_TEXTURE_2D, 
    //    0, 
    //    internalFormat, 
    //    GL_RGBA8, // GL_RGBA GL_UNSIGNED_BYTE, GL_UNSIGNED_INT_8_8_8_8,
    //    bits); // TODO: Make portable with OpenGL ES

    // See: http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
    // http://www.songho.ca/opengl/gl_fbo.html
    auto* buffer = new QGLFramebufferObject(w, h, GL_TEXTURE_2D);
    buffer->bind();
    functions.glFramebufferTexture2D(GL_FRAMEBUFFER, // target
        GL_COLOR_ATTACHMENT0, // attachment
        GL_TEXTURE_2D,
        id, // name of texture
        0); // level

    // define an array of buffers into which outputs from the fragment shader data will be written
    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    functions.glDrawBuffers(1, // Size of buffers array
        drawBuffers);

    GLint status = functions.glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
        throw("Messed up FBO");
    
    outImage.m_image = buffer->toImage();
    buffer->release();
    delete buffer;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}