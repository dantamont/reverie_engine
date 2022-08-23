#include "core/processes/GLoadProcess.h"
#include "core/processes/GProcessManager.h"

#include "fortress/system/memory/GPointerTypes.h"
#include "fortress/system/path/GPath.h"

#include "core/GCoreEngine.h"
#include "core/events/GEventManager.h"
#include "core/resource/GResource.h"
#include "core/resource/GResourceCache.h"

#include "core/sound/GAudioResource.h"
#include "core/rendering/materials/GMaterial.h"
#include "core/rendering/materials/GCubeTexture.h"
#include "core/rendering/geometry/GMesh.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"
#include "core/rendering/renderer/GRenderContext.h"
#include "core/readers/models/GModelReader.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/scripting/GPythonScript.h"

#include "geppetto/qt/widgets/GWidgetManager.h"
#include "core/layer/view/widgets/graphics/GGLWidget.h"

#include "enums/GBasicPolygonTypeEnum.h"

namespace rev {


LoadProcess::LoadProcess(const std::shared_ptr<ResourceHandle>& resourceHandle):
    ThreadedProcess(&ResourceCache::Instance().processManager()->threadedProcessQueue()),
    m_resourceHandle(resourceHandle)
{
    // Set flag on handle to denote that resource is being loaded
    resourceHandle->setIsLoading(true);
}

LoadProcess::~LoadProcess()
{
}

void LoadProcess::onInit()
{
    // Increment load process count
    s_loadProcessCount++;

    try {
        QMutexLocker locker(&m_resourceHandle->mutex());

        // Call parent class initialization
        ThreadedProcess::onInit();

        // Was quitting if resource had already been loaded, but now can be reloaded
        if (m_resourceHandle->resource()) {
            //fail();
#ifdef DEBUG_MODE
            Logger::LogWarning("Reloading resource");
#endif
            //return;
        }

        // Load the resource
        std::unique_ptr<Resource> resource = nullptr;
        switch ((EResourceType)m_resourceHandle->getResourceType()) {
        case EResourceType::eTexture:
            resource = loadTexture();
            break;
        case EResourceType::eMaterial:
            resource = loadMaterial();
            break;
        case EResourceType::eMesh:
            resource = loadMesh();
            break;
        case EResourceType::eAnimation:
            resource = loadAnimation();
            break;
        case EResourceType::eModel:
            resource = loadModel();
            break;
        case EResourceType::eShaderProgram:
            resource = loadShaderProgram();
            break;
        case EResourceType::ePythonScript:
            resource = loadPythonScript();
            break;
        case EResourceType::eCubeTexture:
            resource = loadCubeTexture();
            break;
        case EResourceType::eAudio:
            resource = loadAudio();
            break;
        default:
#ifdef DEBUG_MODE
            Logger::LogError("This resource type is not implemented");
            Logger::Throw("This resource type is not implemented");
#endif
            break;
        }

        // Add resource to handle if not already set
        // Shaders and materials will have set the handle already
        if (!m_resourceHandle->resource()) {
            m_resourceHandle->setResource(std::move(resource), false);
        }

        // Touch resource handle
        ResourceCache::Instance().resourceMapMutex().lock();
        m_resourceHandle->touch();
        ResourceCache::Instance().resourceMapMutex().unlock();

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

void LoadProcess::onSuccess()
{
    // Don't emit post-construction signal if this resource is a child
    if (!m_resourceHandle->isChild()) {
        emit ResourceCache::Instance().doneLoadingResource(m_resourceHandle->getUuid());
    }

    /// @todo Find a non-qt mechanism to call this
    emit ResourceCache::Instance().processManager()->deleteThreadedProcess(getUuid());

    s_loadProcessCount--;
} 

void LoadProcess::onFail()
{
    ThreadedProcess::onFail();

#ifdef DEBUG_MODE
    Logger::LogError("Error, failed to load resource");
#endif
}

void LoadProcess::onAbort()
{
    ThreadedProcess::onAbort();
    s_loadProcessCount--;
}

std::unique_ptr<Resource> LoadProcess::loadTexture()
{
    const GString& texPath = m_resourceHandle->getPath();

    auto texture = prot_make_unique<Texture>(texPath.c_str());
    const json& cachedJson = m_resourceHandle->cachedResourceJson();
    int typeInt = 0;
    if (!cachedJson.is_null()) {
        typeInt = cachedJson.value("texUsageType", 0);
    }
    TextureUsageType type = TextureUsageType(typeInt);
    texture->setUsageType(type);

    texture->generateMipMaps(true); // Set texture to generate mip maps

    // Since return type is different than type in function body, need to explicitly move
    return std::move(texture);
}

std::unique_ptr<Resource> LoadProcess::loadModel()
{
    std::unique_ptr<Resource> model;
    
    // Read in from json if possible
    const json& object = m_resourceHandle->cachedResourceJson();
    if (!object.empty()) {
        // For user-created models, load from JSON
        if (!m_resourceHandle->isRuntimeGenerated()) {
            Logger::Throw("Error, for now, JSON loading is only supported for user-generated models");
        }
        model = std::make_unique<Model>(*m_resourceHandle); // Load from JSON in post-construction
    }
    else {
        // For models direct-from-file, use ModelReader
        auto reader = std::make_shared<ModelReader>(&ResourceCache::Instance(), *m_resourceHandle);
        reader->loadModel();
        model = std::move(m_resourceHandle->m_resource);
    }

    if (!model) {
        Logger::Throw("Error, model not loaded");
    }

    // Since return type is different than type in function body, need to explicitly move
    return std::move(model);
}

std::unique_ptr<Resource> LoadProcess::loadMesh()
{
    // Create mesh if it is a polygon
    const GString& name = m_resourceHandle->getName();
    if (PolygonCache::IsPolygonName(name)) {
        auto mesh = ResourceCache::Instance().polygonCache()->createPolygon(name, m_resourceHandle);
        // Since return type is different than type in function body, need to explicitly move
        return std::move(mesh);
    }
    else {
        // Independent mesh load is not yet supported
        Logger::Throw("Mesh loading independently not implemented");
        return nullptr;
    }
}

std::unique_ptr<Resource> LoadProcess::loadCubeTexture()
{
    //static QMutex cubeTextureMutex;
    //QMutexLocker lock(&cubeTextureMutex);

    const GString& filePath = m_resourceHandle->getPath();
    auto cubeTexture = std::make_unique<CubeTexture>(filePath.c_str());

    // Since return type is different than type in function body, need to explicitly move
    return std::move(cubeTexture);
}

std::unique_ptr<Resource> LoadProcess::loadMaterial()
{
    std::unique_ptr<Material> material;

    // Read in from json if data provided
    if (!m_resourceHandle->cachedResourceJson().empty()) {
        material = prot_make_unique<Material>(); /// @todo This constructor should be totally private
        material->initializeUniformValues(m_resourceHandle->engine()->openGlRenderer()->renderContext().uniformContainer());
        m_resourceHandle->setResource(std::move(material), false); // Need to set before loading from JSON
        m_resourceHandle->cachedResourceJson().get_to(*m_resourceHandle->resourceAs<Material>());
    }
    else {
        Logger::Throw("Material loading directly from file not implemented");
    }

    // Since return type is different than type in function body, need to explicitly move
    return std::move(material);
}

std::unique_ptr<Resource> LoadProcess::loadAnimation()
{
    Logger::Throw("Loading animations independently is not implemented");
    return nullptr;
}

std::unique_ptr<Resource> LoadProcess::loadShaderProgram()
{    
    // Initialize path variables
    GString vertPath, fragPath, geomPath, compPath;
    if (m_resourceHandle->additionalPaths().size() == 2) {
        vertPath = m_resourceHandle->getPath();
        fragPath = m_resourceHandle->additionalPaths().front();
        geomPath = m_resourceHandle->additionalPaths()[1];

#ifdef DEBUG_MODE
        if (ResourceCache::Instance().getTopLevelHandleWithPath(vertPath)->isConstructed()) {
            Logger::Throw("Error, attempting to load a shader program that's already been loaded");
        }
#endif
    }
    else if (m_resourceHandle->additionalPaths().size() == 1) {
        vertPath = m_resourceHandle->getPath();
        fragPath = m_resourceHandle->additionalPaths().front();

#ifdef DEBUG_MODE
        if (ResourceCache::Instance().getTopLevelHandleWithPath(vertPath)->isConstructed()) {
            Logger::Throw("Error, attempting to load a shader program that's already been loaded");
        }
#endif
    }
    else if (m_resourceHandle->additionalPaths().size() == 0) {
        compPath = m_resourceHandle->getPath();

#ifdef DEBUG_MODE
        if (ResourceCache::Instance().getTopLevelHandleWithPath(compPath)->isConstructed()) {
            Logger::Throw("Error, attempting to load a shader program that's already been loaded");
        }
#endif
    }
    else {
        Logger::Throw("Unsupported number of paths");
    }

    std::unique_ptr<ShaderProgram> shaderProgram;
    if (!m_resourceHandle->cachedResourceJson().empty()) {
        // Load from json if possible
        shaderProgram = std::make_unique<ShaderProgram>();
        m_resourceHandle->cachedResourceJson().get_to(*shaderProgram);
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
    RenderContext& context = m_resourceHandle->engine()->openGlRenderer()->renderContext();
    shaderProgram->m_handle = m_resourceHandle.get();
    shaderProgram->initializeShaderProgram(context);
    shaderProgram->release();
    shaderProgram->m_handle = nullptr;

    // Since return type is different than type in function body, need to explicitly move
    return std::move(shaderProgram);
}

std::unique_ptr<Resource> LoadProcess::loadPythonScript()
{
    // TODO: Search for file using a FileManager
    // Throw error if script already added
    //ResourceCache& cache = ResourceCache::Instance();
#ifdef DEBUG_MODE
    if (m_resourceHandle->isConstructed()) {
        Logger::Throw("Error, attempting to reload a python script");
    }
#endif

    // Check if filepath exists
    const GString& filepath = m_resourceHandle->getPath();
    if (GPath::Exists(filepath.c_str())) {
        std::unique_ptr<PythonClassScript> script;
        if (!m_resourceHandle->cachedResourceJson().empty()) {
            // Load from JSON if possible
            script = std::make_unique<PythonClassScript>(m_resourceHandle->engine(), m_resourceHandle->cachedResourceJson());
        }
        else {
            script = std::make_unique<PythonClassScript>(m_resourceHandle->engine(), (QString)filepath.c_str());
        }

        return std::move(script);
    }

#ifdef DEBUG_MODE
    Logger::LogError("Failed to load file at path " + filepath);
    Logger::Throw("Failed to load file");
#endif
    return nullptr;

}

std::unique_ptr<Resource> LoadProcess::loadAudio()
{
    // TODO: Catch thread errors
    // https://stackoverflow.com/questions/25282620/catching-exception-from-worker-thread-in-the-main-thread
    const json& resourceJson = m_resourceHandle->cachedResourceJson();
    if (!resourceJson.contains("sourceType")) {
        Logger::Throw("Error, no audio type specified");
    }

    const GString& audioPath = m_resourceHandle->getPath();
    AudioResource::SourceType sourceType = AudioResource::SourceType(resourceJson.at("sourceType").get<Int32_t>());
    auto audio = prot_make_unique<AudioResource>(sourceType);
    m_resourceHandle->setPath(audioPath);
    audio->loadAudioSource(audioPath);

    // Update source with JSON attributes
    resourceJson["sourceSettings"].get_to(audio->audioSourceSettings());
    audio->cacheSettings();

    return audio;
}

std::atomic<Uint32_t> LoadProcess::s_loadProcessCount = 0;

} // End namespaces