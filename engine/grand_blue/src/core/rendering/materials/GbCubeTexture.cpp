#include "GbCubeTexture.h"

#include <QFileInfo>
#include <QDir>

#include "../../processes/GbProcess.h"
#include "../../GbCoreEngine.h"
#include "../../resource/GbResourceCache.h"
#include "../../resource/GbImage.h"

#include "../../readers/GbJsonReader.h"

#include "../geometry/GbPolygon.h"
#include "../../components/GbCamera.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////////////
CubePaths::CubePaths()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
CubePaths::CubePaths(
    const QString& directoryPath, 
    const QString & right,
    const QString & left,
    const QString & top, 
    const QString & bottom,
    const QString & front, 
    const QString & back):
    m_directoryPath(directoryPath)
{
    // Assign images to image map
    Map::Emplace(m_fileNames, Face::kRight, right);
    Map::Emplace(m_fileNames, Face::kLeft, left);
    Map::Emplace(m_fileNames, Face::kTop, top);
    Map::Emplace(m_fileNames, Face::kBottom, bottom);
    Map::Emplace(m_fileNames, Face::kFront, back);
    Map::Emplace(m_fileNames, Face::kBack, front);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
CubePaths::~CubePaths()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
QString CubePaths::path(const Face & face) const
{
    return QDir::cleanPath(m_directoryPath + QDir::separator() + m_fileNames.at(face));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
CubePaths CubeTexture::loadCubemapFile(const QString & filepath)
{
    QString dirPath = QFileInfo(filepath).dir().path();
    QVariantMap filepaths = JsonReader(filepath).getContentsAsVariantMap();
    return CubePaths(dirPath,
        filepaths["right"].toString(),
        filepaths["left"].toString(),
        filepaths["top"].toString(),
        filepaths["bottom"].toString(), 
        filepaths["front"].toString(), 
        filepaths["back"].toString());
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> CubeTexture::createHandle(CoreEngine * engine, const QString& filepath)
{
    auto handle = ResourceHandle::create(engine, Resource::kCubeTexture);
    handle->setName(filepath);
    handle->setPath(filepath);
    return handle;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
// Cube Texture
/////////////////////////////////////////////////////////////////////////////////////////////////////
CubeTexture::CubeTexture(const CubePaths& imagePaths,
    unsigned int texUnit):
    Resource(kCubeTexture),
    m_imagePaths(imagePaths),
    m_textureUnit(texUnit),
    m_gl(nullptr)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
CubeTexture::CubeTexture(const QString & cubemapFilePath,
    unsigned int texUnit) :
    Resource(kCubeTexture),
    m_textureUnit(texUnit),
    m_gl(nullptr)
{
    m_imagePaths = loadCubemapFile(cubemapFilePath);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
CubeTexture::~CubeTexture()
{
    if (m_gl) {
        delete m_gl;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CubeTexture::initializeTexture()
{
    initializeTexture(m_imagePaths);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CubeTexture::initializeTexture(const CubePaths& imagePaths)
{
    m_gl = new GL::OpenGLFunctions();

    // Create texture ID to reference
    m_gl->glGenTextures(1, &m_textureID); // number of textures, texture ID

    // Bind the cubemap texture
    bind();

    // Set wrapping and mipmap generation modes
    initializeSettings();

    // Iterate through faces and populate cubemap texture data
    int width;
    int height;
    m_cost = 0;
    for (int faceInt = 0; faceInt < 6; ++faceInt) {
        // Load image
        Face face = Face(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceInt);
        const QString& imagePath = imagePaths.path(face);
        auto image = std::make_shared<Image>(
            imagePath,
            QImage::Format_RGBA8888);

        if (faceInt == 0) {
            // Get size of images (all should be same dimensions)
            width = image->m_image.width();
            height = image->m_image.height();
        }

        // Load image into GL
        glTexImage2D((int)face,  // Specify which cubemap face is texture target
            0, // mipmap level
            GL_RGBA8, // internal format to store texture in
            width,  // resulting texture width
            height, // resulting texture height
            0,  // always 0, legacy
            GL_RGBA, // format of source image (pixel data)
            GL_UNSIGNED_BYTE, // data type of source image (affects resolution)
            image->m_image.constBits()); // actual image data

        m_cost += image->sizeInMegaBytes();
#ifdef DEBUG_MODE
        m_gl->printGLError("Failed to initialize cubemap texture " + QString::number(faceInt));
#endif
    }
	
#ifdef DEBUG_MODE
    m_gl->printGLError("Failed to initialize cubemap textures");
#endif

    // Unbind the cubemap texture
    release();

}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CubeTexture::onRemoval(ResourceCache* cache)
{
    Q_UNUSED(cache);
    remove();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CubeTexture::postConstruction()
{
    initializeTexture();

    // Call parent class construction routine
    Resource::postConstruction();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CubeTexture::initializeSettings()
{
    // Set cubemap settings
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // May not be compatible with Open GL ES
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifdef DEBUG_MODE
    m_gl->printGLError("Failed to initialize cubemap texture settings");
#endif
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
