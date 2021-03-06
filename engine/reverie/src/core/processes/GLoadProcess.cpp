#include "GLoadProcess.h"
#include "GProcessManager.h"

#include "../utils/GMemoryManager.h"

#include "../GCoreEngine.h"
#include "../events/GEventManager.h"
#include "../resource/GResource.h"
#include "../resource/GResourceCache.h"

#include "../sound/GAudioResource.h"
#include "../resource/GImage.h"
#include "../rendering/materials/GMaterial.h"
#include "../rendering/materials/GCubeTexture.h"
#include "../rendering/geometry/GMesh.h"
#include "../readers/models/GModelReader.h"
#include "../rendering/shaders/GShaderProgram.h"
#include "../scripting/GPythonScript.h"

#include "../../view/GWidgetManager.h"
#include "../../view/GL/GGLWidget.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
LoadProcess::LoadProcess(CoreEngine* engine, const std::shared_ptr<ResourceHandle>& resourceHandle):
    ThreadedProcess(engine, engine->resourceCache()->processManager()),
    m_resourceHandle(resourceHandle)
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
    try {
        QMutexLocker locker(&m_resourceHandle->mutex());

        // Call parent class initialization
        ThreadedProcess::onInit();

        // Was quitting if resource had already been loaded, but now can be reloaded
        if (m_resourceHandle->resource()) {
            //fail();
#ifdef DEBUG_MODE
            logMessage("Reloading resource", LogLevel::Warning);
#endif
            //return;
        }

        // Load the resource
        std::unique_ptr<Resource> resource = nullptr;
        switch (m_resourceHandle->getResourceType()) {
        case ResourceType::kImage:
            resource = loadImage();
            break;
        case ResourceType::kTexture:
            resource = loadTexture();
            break;
        case ResourceType::kMaterial:
            resource = loadMaterial();
            break;
        case ResourceType::kMesh:
            resource = loadMesh();
            break;
        case ResourceType::kAnimation:
            resource = loadAnimation();
            break;
        case ResourceType::kModel:
            resource = loadModel();
            break;
        case ResourceType::kShaderProgram:
            resource = loadShaderProgram();
            break;
        case ResourceType::kPythonScript:
            resource = loadPythonScript();
            break;
        case ResourceType::kCubeTexture:
            resource = loadCubeTexture();
            break;
        case ResourceType::kAudio:
            resource = loadAudio();
            break;
        default:
#ifdef DEBUG_MODE
            Logger::LogError("This resource type is not implemented");
            throw("This resource type is not implemented");
#endif
            break;
        }

        // Add resource to handle if not already set
        // Shaders and materials will have set the handle already
        if (!m_resourceHandle->resource()) {
            m_resourceHandle->setResource(std::move(resource), false);
        }

        // Touch resource handle
        m_engine->resourceCache()->resourceMapMutex().lock();
        m_resourceHandle->touch();
        m_engine->resourceCache()->resourceMapMutex().unlock();

        // Determine process success or failure, terminate either way
        Resource* r = m_resourceHandle->resource();
        if (r) {
            succeed();
        }
        else {
            fail();
        }
    }
    catch (...) {
        m_exceptionLock.lock();
        m_exceptionPtr = std::current_exception();
        m_exceptionLock.unlock();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void LoadProcess::onSuccess()
{
    // Don't emit post-construction signal if this resource is a child
    if (!m_resourceHandle->isChild()) {
        emit m_engine->resourceCache()->doneLoadingResource(m_resourceHandle->getUuid());
    }
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
std::unique_ptr<Resource> LoadProcess::loadImage()
{
    // TODO: UNTESTED
    QImage::Format format = QImage::Format(m_resourceHandle->resourceJson()["format"].toInt(0));
    auto image = std::make_unique<Image>(m_resourceHandle->getPath().c_str(), format);

    // Since return type is different than type in function body, need to explicitly move
    return std::move(image);
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<Resource> LoadProcess::loadTexture()
{
    const GString& texPath = m_resourceHandle->getPath();

    auto texture = prot_make_unique<Texture>(texPath.c_str());
    int typeInt = m_resourceHandle->resourceJson()["texUsageType"].toInt(0);
    TextureUsageType type = TextureUsageType(typeInt);
    texture->setUsageType(type);

    // TODO: Make this a flag
    texture->generateMipMaps(true); // Set texture to generate mip maps

    // Since return type is different than type in function body, need to explicitly move
    return std::move(texture);
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<Resource> LoadProcess::loadModel()
{
    std::unique_ptr<Resource> model;
    
    // Read in from json if possible
    const QJsonObject& object = m_resourceHandle->resourceJson();
    if (!object.isEmpty()) {
        // For user-created models, load from JSON
        if (!m_resourceHandle->isRuntimeGenerated()) {
            throw("Error, for now, JSON loading is only supported for user-generated models");
        }
        model = std::make_unique<Model>(*m_resourceHandle); // Load from JSON in post-construction
    }
    else {
        // For models direct-from-file, use ModelReader
        auto reader = std::make_shared<ModelReader>(m_engine->resourceCache(), *m_resourceHandle);
        reader->loadModel();
        model = std::move(m_resourceHandle->m_resource);
    }

    if (!model) {
        throw("Error, model not loaded");
    }

    // Since return type is different than type in function body, need to explicitly move
    return std::move(model);
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<Resource> LoadProcess::loadMesh()
{
    // Create mesh if it is a polygon
    const GString& name = m_resourceHandle->getName();
    if (PolygonCache::isPolygonName(name)) {
        auto mesh = m_engine->resourceCache()->polygonCache()->createPolygon(name, m_resourceHandle);
        // Since return type is different than type in function body, need to explicitly move
        return std::move(mesh);
    }
    else {
        // Independent mesh load is not yet supported
        throw("Not implemented");
        return nullptr;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<Resource> LoadProcess::loadCubeTexture()
{
    //static QMutex cubeTextureMutex;
    //QMutexLocker lock(&cubeTextureMutex);

    const GString& filePath = m_resourceHandle->getPath();
    auto cubeTexture = std::make_unique<CubeTexture>(filePath.c_str());

    // Since return type is different than type in function body, need to explicitly move
    return std::move(cubeTexture);
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<Resource> LoadProcess::loadMaterial()
{
    std::unique_ptr<Material> material;

    // Read in from json if data provided
    if (!m_resourceHandle->resourceJson().isEmpty()) {
        material = prot_make_unique<Material>();
        m_resourceHandle->setResource(std::move(material), false); // Need to set before loading from JSON
        m_resourceHandle->resourceAs<Material>()->loadFromJson(m_resourceHandle->resourceJson(), { m_engine });
    }
    else {
        throw("Material loading directly from file not implemented");
    }

    // Since return type is different than type in function body, need to explicitly move
    return std::move(material);
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<Resource> LoadProcess::loadAnimation()
{
    throw("Not implemented");
    return nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<Resource> LoadProcess::loadShaderProgram()
{    
    // Initialize path variables
    GString vertPath, fragPath, geomPath, compPath;
    if (m_resourceHandle->additionalPaths().size() == 2) {
        vertPath = m_resourceHandle->getPath();
        fragPath = m_resourceHandle->additionalPaths().front();
        geomPath = m_resourceHandle->additionalPaths()[1];

#ifdef DEBUG_MODE
        if (m_engine->resourceCache()->getTopLevelHandleWithPath(vertPath)->isConstructed()) {
            throw("Error, attempting to load a shader program that's already been loaded");
        }
#endif
    }
    else if (m_resourceHandle->additionalPaths().size() == 1) {
        vertPath = m_resourceHandle->getPath();
        fragPath = m_resourceHandle->additionalPaths().front();

#ifdef DEBUG_MODE
        if (m_engine->resourceCache()->getTopLevelHandleWithPath(vertPath)->isConstructed()) {
            throw("Error, attempting to load a shader program that's already been loaded");
        }
#endif
    }
    else if (m_resourceHandle->additionalPaths().size() == 0) {
        compPath = m_resourceHandle->getPath();

#ifdef DEBUG_MODE
        if (m_engine->resourceCache()->getTopLevelHandleWithPath(compPath)->isConstructed()) {
            throw("Error, attempting to load a shader program that's already been loaded");
        }
#endif
    }
    else {
        throw("Unsupported number of paths");
    }

    std::unique_ptr<ShaderProgram> shaderProgram;
    if (!m_resourceHandle->resourceJson().isEmpty()) {
        // Load from json if possible
        shaderProgram = std::make_unique<ShaderProgram>();
        shaderProgram->loadFromJson(m_resourceHandle->resourceJson());
    }
    else {
        if (compPath.isEmpty()) {
            // Is not a compute shader
            shaderProgram = std::make_unique<ShaderProgram>(vertPath.c_str(), fragPath.c_str(), geomPath.c_str());
        }
        else {
            // Is a compute shader
            shaderProgram = std::make_unique<ShaderProgram>(compPath.c_str());
        }
    }
    GString shaderName = shaderProgram->createName();
    if (m_resourceHandle->getName().isEmpty()) {
        m_resourceHandle->setName(shaderName);
    }
    else {
        shaderName = m_resourceHandle->getName();
    }

    // Initialize the shader once its name has been set in handle
    shaderProgram->m_handle = m_resourceHandle.get();
    shaderProgram->initializeShaderProgram();
    shaderProgram->release();
    shaderProgram->m_handle = nullptr;

    // Since return type is different than type in function body, need to explicitly move
    return std::move(shaderProgram);
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<Resource> LoadProcess::loadPythonScript()
{
    // TODO: Search for file using a FileManager
    // Throw error if script already added
    //ResourceCache& cache = *m_engine->resourceCache();
#ifdef DEBUG_MODE
    if (m_resourceHandle->isConstructed()) {
        throw("Error, attempting to reload a python script");
    }
#endif

    // Check if filepath exists
    const GString& filepath = m_resourceHandle->getPath();
    if (FileReader::FileExists(filepath.c_str())) {
        std::unique_ptr<PythonClassScript> script;
        if (!m_resourceHandle->resourceJson().isEmpty()) {
            // Load from JSON if possible
            script = std::make_unique<PythonClassScript>(m_engine, m_resourceHandle->resourceJson());
        }
        else {
            script = std::make_unique<PythonClassScript>(m_engine, (QString)filepath.c_str());
        }

        return std::move(script);
    }

#ifdef DEBUG_MODE
    Logger::LogError("Failed to load file at path " + filepath);
    throw("Failed to load file");
#endif
    return nullptr;

}
/////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<Resource> LoadProcess::loadAudio()
{
    // TODO: Catch thread errors
    // https://stackoverflow.com/questions/25282620/catching-exception-from-worker-thread-in-the-main-thread
    const QJsonObject& resourceJson = m_resourceHandle->resourceJson();
    if (!resourceJson.contains("sourceType")) {
        throw("Error, no audio type specified");
    }

    const GString& audioPath = m_resourceHandle->getPath();
    AudioResource::SourceType sourceType = AudioResource::SourceType(resourceJson["sourceType"].toInt());
    auto audio = prot_make_unique<AudioResource>(sourceType);
    m_resourceHandle->setPath(audioPath);
    audio->loadAudioSource(audioPath);

    // Update source with JSON attributes
    audio->audioSourceSettings().loadFromJson(resourceJson["sourceSettings"]);
    audio->cacheSettings();

    return audio;
}

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces