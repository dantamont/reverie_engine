#include "GbLoadProcess.h"
#include "GbProcessManager.h"

#include "../GbCoreEngine.h"
#include "../events/GbEventManager.h"
#include "../resource/GbResource.h"
#include "../resource/GbResourceCache.h"

#include "../resource/GbImage.h"
#include "../rendering/materials/GbMaterial.h"
#include "../rendering/materials/GbCubeMap.h"
#include "../readers/models/GbModelReader.h"

#include "../../view/GbWidgetManager.h"
#include "../../view/GL/GbGLWidget.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
LoadProcess::LoadProcess(CoreEngine * engine, 
    ProcessManager* manager,
    std::shared_ptr<ResourceHandle> resourceHandle,
    const ResourceAttributes& resourceAttributes):
    ThreadedProcess(engine, manager),
    m_resourceHandle(resourceHandle),
    m_resourceAttributes(resourceAttributes)
{
    // Set flag on handle to denote that resource is being loaded
    resourceHandle->setIsLoading(true);
}
/////////////////////////////////////////////////////////////////////////////////////////////
LoadProcess::~LoadProcess()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void LoadProcess::onInit()
{
    QMutexLocker locker(&m_resourceHandle->mutex());

    // Call parent class initialization
    ThreadedProcess::onInit();

    // Was quitting if resource has already been loaded, but now can be reloaded
    auto cache = m_engine->resourceCache();
    if (m_resourceHandle->resource(false)) {
        //fail();
#ifdef DEBUG_MODE
        logMessage("Reloading resource", LogLevel::Warning);
#endif
        //return;
    }

    // Load the resource
    std::shared_ptr<Resource> resource = nullptr;
    switch (m_resourceHandle->getType()) {
    case Resource::kImage:
        resource = loadImage();
        break;
    case Resource::kTexture:
        resource = loadTexture();
        break;
    case Resource::kMaterial:
        resource = loadMaterial();
        break;
    case Resource::kMesh:
        resource = loadMesh();
        break;
    case Resource::kCubeTexture:
        resource = loadCubeTexture();
        break;
    default:
#ifdef DEBUG_MODE
        logError("This resource type is not implemented");
        throw("This resource type is not implemented");
#endif
        break;
    }

    m_resourceHandle->setResource(resource, false);

    // Set resource attributes in handle
    if (m_resourceAttributes.m_attributes.size() > 0) {
        m_resourceHandle->setAttributes(m_resourceAttributes);
    }

    // Reinsert resource into cache
    cache->insert(m_resourceHandle);

    // Determine process success or failure, terminate either way
    if (resource) {
        succeed();
    }
    else {
        fail();
    }

    // Flag loading as complete
    m_resourceHandle->setIsLoading(false);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void LoadProcess::onSuccess()
{
    emit m_engine->eventManager()->doneLoadingResource(m_resourceHandle);
    ThreadedProcess::onSuccess();
} 
/////////////////////////////////////////////////////////////////////////////////////////////
void LoadProcess::onFail()
{
    ThreadedProcess::onFail();

#ifdef DEBUG_MODE
    logMessage("Error, failed to load resource");
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void LoadProcess::onAbort()
{
    ThreadedProcess::onAbort();
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Image> LoadProcess::loadImage()
{

    QImage::Format format = QImage::Format_Invalid;
    if (m_resourceAttributes.hasAttribute("format")) {
        format = QImage::Format(m_resourceAttributes.at("format").get<int>());
    }
    std::shared_ptr<Image> image = std::make_shared<Image>(m_resourceHandle->getPath(), format);

    return image;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Resource> LoadProcess::loadTexture()
{
    const QString& texPath = m_resourceHandle->getPath();

    std::shared_ptr<Texture> texture = std::make_shared<Texture>(texPath);
    int typeInt = 0;
    if (m_resourceAttributes.m_attributes.count("type")) {
        if (!m_resourceAttributes.at("type").valid()) {
            typeInt = 0;
        }
        else {
            typeInt = m_resourceAttributes.at("type").get<int>();
        }
    }
    Texture::TextureType type = Texture::TextureType(typeInt);
    texture->setType(type);
    return texture;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Resource> LoadProcess::loadMesh()
{
    // Instantiate mesh
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(
        FileReader::pathToName(m_resourceHandle->getPath()),
        QOpenGLBuffer::DynamicDraw);

    // Read in mesh as model
    ModelReader reader = ModelReader(m_engine->resourceCache(), m_resourceHandle->getPath());
    reader.loadModel(*mesh);

#ifdef DEBUG_MODE
    size_t cost = mesh->getCost();
    logInfo("Added mesh at " + m_resourceHandle->getPath() + " with size " + QString::number(cost) + " Mb");
#endif

    return mesh;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Resource> LoadProcess::loadCubeTexture()
{
    const QString& filePath = m_resourceHandle->getPath();
    std::shared_ptr<CubeTexture> cubeTexture = std::make_shared<CubeTexture>(filePath);
    return cubeTexture;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Resource> LoadProcess::loadMaterial()
{
    throw("Not implemented");
    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces