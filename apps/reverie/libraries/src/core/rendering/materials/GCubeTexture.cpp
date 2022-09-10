#include "core/rendering/materials/GCubeTexture.h"

#include <QFileInfo>
#include <QDir>

#include "fortress/process/GProcess.h"
#include "core/GCoreEngine.h"
#include "core/resource/GResourceCache.h"

#include "fortress/json/GJson.h"

#include "core/rendering/geometry/GPolygon.h"
#include "core/components/GCameraComponent.h"

namespace rev {


CubePaths::CubePaths()
{
}

CubePaths::CubePaths(
    const GString& directoryPath, 
    const GString & right,
    const GString & left,
    const GString & top, 
    const GString & bottom,
    const GString & front, 
    const GString & back):
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

CubePaths::~CubePaths()
{
}

GString CubePaths::path(const CubeMapFace & face) const
{
    int offset = (int)CubeMapFace::kRight;
    return QDir::cleanPath(QString(m_directoryPath.c_str()) + QDir::separator() + (const char*)m_fileNames.at((int)face - offset)).toStdString();
}

CubePaths CubeTexture::loadPathsFromCubemapFile(const GString & filepath)
{
    GString dirPath = QFileInfo(filepath.c_str()).dir().path().toStdString();
    json pathsJson;
    GJson::FromFile(filepath, pathsJson);
    pathsJson.get_to(pathsJson);
    return CubePaths(dirPath,
        pathsJson.at("right").get_ref<const std::string&>().c_str(),
        pathsJson.at("left").get_ref<const std::string&>().c_str(),
        pathsJson.at("top").get_ref<const std::string&>().c_str(),
        pathsJson.at("bottom").get_ref<const std::string&>().c_str(),
        pathsJson.at("front").get_ref<const std::string&>().c_str(),
        pathsJson.at("back").get_ref<const std::string&>().c_str()
    );
}

std::shared_ptr<ResourceHandle> CubeTexture::CreateHandle(CoreEngine * engine, const GString& filepath)
{
    auto handle = ResourceHandle::Create(engine, (GResourceType)EResourceType::eCubeTexture);
    handle->setName(filepath);
    handle->setPath(filepath);
    return handle;
}




// Cube Texture

CubeTexture::CubeTexture(const CubePaths& imagePaths) :
    CubeTexture(1, 1, 1)
{
    m_imagePaths = imagePaths;
    loadImages();
}

CubeTexture::CubeTexture(const GString & cubemapFilePath):
    CubeTexture(1, 1, 1)
{
    m_imagePaths = loadPathsFromCubemapFile(cubemapFilePath);
    loadImages();
}

CubeTexture::CubeTexture(uint32_t width, uint32_t height, uint32_t depth, bool isArray, TextureFormat internalFormat):
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

CubeTexture::~CubeTexture()
{
}

void CubeTexture::loadImages()
{
    for (int faceInt = 0; faceInt < 6; ++faceInt) {
        // Load images into cubemap
        CubeMapFace face = CubeMapFace(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceInt);
        GString imagePath = m_imagePaths.path(face);
        m_images.emplace_back(GString(imagePath.c_str()), false, Image::ColorFormat::kRGBA8888);

        // Set cubemap dimensions
        if (faceInt == 0) {
            m_width = m_images.back().width();
            m_height = m_images.back().height();
        }
    }
}



void to_json(nlohmann::json& orJson, const CubeTexture& korObject)
{
}

void from_json(const nlohmann::json& korJson, CubeTexture& orObject)
{
}

} // End namespaces
