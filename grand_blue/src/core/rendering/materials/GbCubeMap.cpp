#include "GbCubeMap.h"

#include <QFileInfo>
#include <QDir>

#include "../../processes/GbProcess.h"
#include "../../GbCoreEngine.h"
#include "../../resource/GbResourceCache.h"
#include "../../resource/GbImage.h"

#include "../../readers/GbJsonReader.h"

#include "../geometry/GbPolygon.h"
#include "../shaders/GbShaders.h"
#include "../../geometry/GbTransform.h"
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
/////////////////////////////////////////////////////////////////////////////////////////////////////
// Cube Texture
/////////////////////////////////////////////////////////////////////////////////////////////////////
CubeTexture::CubeTexture(const CubePaths& imagePaths,
    unsigned int texUnit):
    //ResourceHandle(QFileInfo(rightPath).dir().path()),
    Resource(kCubeTexture),
    m_imagePaths(imagePaths),
    m_textureUnit(texUnit),
    m_gl(nullptr)
{
    // Run post-construction if on the main thread, otherwise return
    if (Process::isMainThread()) {
        postConstruction();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
CubeTexture::CubeTexture(const QString & cubemapFilePath,
    unsigned int texUnit) :
    Resource(kCubeTexture),
    m_textureUnit(texUnit),
    m_gl(nullptr)
{
    m_imagePaths = loadCubemapFile(cubemapFilePath);
    // Run post-construction if on the main thread, otherwise return
    if (Process::isMainThread()) {
        postConstruction();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
CubeTexture::~CubeTexture()
{
    if (m_gl) {
        delete m_gl;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CubeTexture::onRemoval(ResourceCache* cache)
{
    Q_UNUSED(cache)
    // Delete texture in GL
    glDeleteTextures(1, &m_textureID);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CubeTexture::postConstruction()
{
#ifdef DEBUG_MODE
    logConstructedWarning();
#endif

    initializeTexture(m_imagePaths);
	
	// Call parent class construction routine
	Resource::postConstruction();
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
/////////////////////////////////////////////////////////////////////////////////////////////////////
// Cubemap
/////////////////////////////////////////////////////////////////////////////////////////////////////
Gb::CubeMap::CubeMap(CoreEngine* engine,
    const QString& name,
    const std::shared_ptr<ResourceHandle>& cubeTexture) :
    Model(engine, name, kCubeMap),
    m_cubeTextureHandle(cubeTexture),
    m_color(255, 255, 255)
{
    initialize();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
CubeMap::CubeMap(CoreEngine * engine, const QJsonValue & json) :
    Model(engine, json.toObject().value("name").toString(), kCubeMap)
{
    loadFromJson(json);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
Gb::CubeMap::~CubeMap()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////////////s
void CubeMap::draw(const std::shared_ptr<ShaderProgram>& shaderProgram, RenderSettings* settings)
{
    Q_UNUSED(settings)
    if (m_cubeTextureHandle->getIsLoading()) {
#ifdef DEBUG_MODE
        logInfo("Cube texture not yet loaded, returning");
#endif
        return;
    }

    // Lock mutex to mesh handle
    QMutexLocker locker(&m_meshHandle->mutex());

    // Cast resource
    auto cubeTexture = std::static_pointer_cast<CubeTexture>(m_cubeTextureHandle->resource(false));

    // Return if no resource
    if (!cubeTexture) {
        return;
    }

    if (!cubeTexture->isConstructed()) {
        return;
    }

    if (!checkMesh()) {
        return;
    }

    // Turn off depth mask (stop writing to depth buffer)
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL); //  The depth buffer will be filled with values of 1.0 for the skybox, so we need to make sure the skybox passes the depth tests with values less than or equal to the depth buffer instead of less than. 

    // Bind shader program
    shaderProgram->bind();

    // Set shader uniforms
    bindUniforms(shaderProgram, cubeTexture);
    shaderProgram->updateUniforms();

    // Bind texture
    cubeTexture->bind();

#ifdef DEBUG_MODE
    printGLError("Error binding cube texture");
#endif

    // Draw geometry (is only one set of mesh data for a cubemap mesh)
    std::shared_ptr<Mesh> meshPtr = mesh();
    meshPtr->m_meshData.begin()->second->drawGeometry(GL_TRIANGLES);

    // Release texture
    cubeTexture->release();

    // Turn depth mask (depth writing) back on
    glDepthMask(GL_TRUE);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue CubeMap::asJson() const
{
    QJsonObject object = Model::asJson().toObject();
    object.insert("texture", m_cubeTextureHandle->asJson());
    object.insert("transform", m_transform.asJson());
    object.insert("diffuseColor", m_color.toVector3g().asJson());
    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CubeMap::loadFromJson(const QJsonValue & json)
{
    Model::loadFromJson(json);
    const QJsonObject& object = json.toObject();
    m_transform.loadFromJson(object["transform"]);
    m_cubeTextureHandle = m_engine->resourceCache()->getResourceHandle(object.value("texture"));
    if (object.contains("diffuseColor")) {
        m_color = Color(Vector3f(object["diffuseColor"]));
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CubeMap::initialize()
{
    m_meshHandle = m_engine->resourceCache()->polygonCache()->getCube();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CubeMap::bindUniforms(const std::shared_ptr<ShaderProgram>&  shaderProgram, 
    std::shared_ptr<CubeTexture> cubeTexture)
{
    // Set texture
    shaderProgram->setUniformValue("cubeTexture", int(cubeTexture->m_textureUnit));

    // Set diffuse color
    shaderProgram->setUniformValue("diffuseColor", m_color.toVector3g());

    // Set world matrix uniform
    auto worldMatrix = m_transform.worldMatrix();
    shaderProgram->setUniformValue("worldMatrix", worldMatrix);

}


/////////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
