#include "GbCubeTexture.h"

#include <QFileInfo>
#include <QDir>

#include "../../processes/GbProcess.h"
#include "../../GbCoreEngine.h"
#include "../../resource/GbResourceCache.h"
#include "../../resource/GbImage.h"

#include "../../readers/GbJsonReader.h"

#include "../geometry/GbPolygon.h"
#include "../../components/GbCameraComponent.h"

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
    int offset = (int)CubeMapFace::kRight;
    m_fileNames[(int)CubeMapFace::kRight - offset] = right;
    m_fileNames[(int)CubeMapFace::kLeft - offset] = left;
    m_fileNames[(int)CubeMapFace::kTop - offset] = top;
    m_fileNames[(int)CubeMapFace::kBottom - offset] = bottom;
    m_fileNames[(int)CubeMapFace::kFront - offset] = back;
    m_fileNames[(int)CubeMapFace::kBack - offset] = front;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
CubePaths::~CubePaths()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
QString CubePaths::path(const CubeMapFace & face) const
{
    int offset = (int)CubeMapFace::kRight;
    return QDir::cleanPath(m_directoryPath + QDir::separator() + m_fileNames.at((int)face - offset));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
CubePaths CubeTexture::loadPathsFromCubemapFile(const QString & filepath)
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
CubeTexture::CubeTexture(const CubePaths& imagePaths) :
    CubeTexture(1, 1, 1)
{
    m_imagePaths = imagePaths;
    loadImages();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
CubeTexture::CubeTexture(const QString & cubemapFilePath):
    CubeTexture(1, 1, 1)
{
    m_imagePaths = loadPathsFromCubemapFile(cubemapFilePath);
    loadImages();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
CubeTexture::CubeTexture(size_t width, size_t height, size_t depth, bool isArray, TextureFormat internalFormat):
    Texture(
    width,
    height,
    isArray ? TextureTargetType::kCubeMapArray : TextureTargetType::kCubeMap,
    TextureUsageType::kNone,
    TextureFilter::kLinear,
    TextureFilter::kLinear,
    TextureWrapMode::kClampToEdge,
    internalFormat,
    depth)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
CubeTexture::~CubeTexture()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CubeTexture::loadImages()
{
    for (int faceInt = 0; faceInt < 6; ++faceInt) {
        // Load images into cubemap
        CubeMapFace face = CubeMapFace(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceInt);
        QString imagePath = m_imagePaths.path(face);
        m_images.emplace_back(imagePath, QImage::Format_RGBA8888); // QImage is implicitly shared, so copy is cheap
    
        // Set cubemap dimensions
        if (faceInt == 0) {
            m_width = m_images.back().m_image.width();
            m_height = m_images.back().m_image.height();
        }
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
