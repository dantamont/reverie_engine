#include "GResourceCache.h"

#include "GImage.h"

#include <QDirIterator>
#include <QRegExp>

#include "../processes/GProcessManager.h"
#include "../processes/GLoadProcess.h"

#include "../scene/GScenario.h"
#include "../scripting/GPythonScript.h"
#include "../rendering/geometry/GPolygon.h"
#include "../GCoreEngine.h"
#include "../utils/GMemoryManager.h"
#include "../rendering/shaders/GShaderProgram.h"
#include "../rendering/materials/GMaterial.h"
#include "../rendering/geometry/GVertexData.h"
#include "../rendering/models/GModel.h"
#include "../rendering/materials/GCubeTexture.h"
#include "../animation/GAnimation.h"
#include "../rendering/shaders/GShaderPreset.h"

namespace rev {
/////////////////////////////////////////////////////////////////////////////////////////////
ResourceCache::ResourceCache(CoreEngine* core, ProcessManager* processManager, const size_t sizeInMb):
    Manager(core, "ResourceCache"),
    m_maxCost(sizeInMb),
    m_currentCost(0),
    m_processManager(processManager),
    m_polygonCache(std::make_shared<PolygonCache>(core))
{
    // Register resource handle to send over qt
    qRegisterMetaType<std::shared_ptr<ResourceHandle>>("rev::ResourceHandle");

    // Connect post-construction routine to resource cache and emit signal
    // Queued connection will ensure slot is executed in receiver's thread
    QObject::connect(this,
        &ResourceCache::doneLoadingResource,
        this,
        &ResourceCache::runPostConstruction,
        Qt::QueuedConnection);

#ifdef DEBUG_MODE
    if (sizeInMb > getMaxMemoryMb() / 2.0) {
        logWarning("Warning, resource cache allows for more than half of available system memory, may cause performance issues");
    }
#endif

}
/////////////////////////////////////////////////////////////////////////////////////////////
ResourceCache::~ResourceCache()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ResourceCache::clearedRemovable() const
{
    return !std::any_of(m_topLevelResources.begin(), 
        m_topLevelResources.end(),
        [](const std::shared_ptr<ResourceHandle>& r) {
            return !r->isPermanent();
    });
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ResourceCache::insertHandle(const std::shared_ptr<ResourceHandle>& resource)
{
    return insertHandle(resource, nullptr);
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ResourceCache::insertHandle(const std::shared_ptr<ResourceHandle>& resourceHandle, bool* clearedResources)
{
    // Lock in case adding from another thread
    QMutexLocker lock(&s_resourceMutex);

    // Add resourceHandle
    const Uuid& uuid = resourceHandle->getUuid();
    if (m_resources.hasKey(uuid)) {
#ifdef DEBUG_MODE
        const std::shared_ptr<ResourceHandle>& oldHandle = m_resources[uuid];
        Logger::LogInfo("Reloading resource: " + oldHandle->getName());
        throw("Reloading resource");
#endif
        //m_resources[uuid] = resourceHandle;
        resourceHandle->touch();
    }
    else {
        m_resources.emplace(uuid, resourceHandle);

        // Only add to top level list if resource handle is not a child
        if (!resourceHandle->isChild()) {
            m_topLevelResources.push_front(resourceHandle);
        }
    }

    // Return if resource not yet loaded
    if (!resourceHandle->resource()) {
        return true;
    }

    // FIXME: This doesn't get called again if handle is inserted when it has no resource,
    // which means that the current cost is pretty much never going to be accurate
    // This should just be moved to the post-construction routine of a resource

    // Increment total cost of resources
    m_currentCost += resourceHandle->resource()->getCost();

#ifdef DEBUG_MODE
    Logger::LogInfo("Inserted resource");
    logCurrentCost();
#endif

    // Remove resources if over max cost
    bool removedResources = false;
    bool succeeded = true;
    while (m_currentCost > m_maxCost) {
        remove(oldestResource());
        if (clearedRemovable()) {
#ifdef DEBUG_MODE
            Logger::LogError("Could not clear resource cache sufficiently to store new resource");
            Logger::LogError("Error, failed to load resource at" + resourceHandle->getPath()
                + ", resource too large at " + GString::FromNumber(resourceHandle->resource()->getCost()));
#endif
            succeeded = false;
            break;
        }
        removedResources = true;
    }

    // Return flag for whether resources were cleared or not
    if (clearedResources) {
        *clearedResources = removedResources;
    }

    return succeeded;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ResourceCache::getTopLevelHandleWithPath(const GString & filepath) const
{
    //QMutexLocker lock(&s_resourceMutex);

    auto resourceIter = std::find_if(m_topLevelResources.begin(), m_topLevelResources.end(),
        [&](const std::shared_ptr<ResourceHandle>& r) {
        return r->getPath() == filepath;
    });
    if (resourceIter != m_topLevelResources.end()) {
        return *resourceIter;
    }
    else {
        return nullptr;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ResourceCache::guaranteeHandleWithPath(const GString & filepath,
    ResourceType type, ResourceBehaviorFlags flags)
{
    std::shared_ptr<ResourceHandle> handle = getTopLevelHandleWithPath(filepath);
    if (handle) {
#ifdef DEBUG_MODE
        if (handle->getResourceType() != type)
            throw("Handle type mismatch");
#endif
        if (handle->behaviorFlags() != flags) {
            handle->setBehaviorFlags(flags);
        }
    }
    else{
        handle = ResourceHandle::create(m_engine, filepath, type);
        handle->setBehaviorFlags(flags);
        handle->loadResource();
    }

    return handle;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ResourceCache::guaranteeHandleWithPath(const std::vector<GString>& filepaths,
    ResourceType type, ResourceBehaviorFlags flags)
{
    if (type != ResourceType::kShaderProgram)
        throw("Only shader programs are supported with multiple-file construction");

    // Check that a resource associated with any of the input files does not exist
    std::shared_ptr<ResourceHandle> handle;
    for (const GString& path : filepaths) {
        // Ignore any blank paths
        if (path.isEmpty()) continue;

        handle = getTopLevelHandleWithPath(path);
        if (handle) {
#ifdef DEBUG_MODE
            if (handle->getResourceType() != type)
                throw("Handle type mismatch");
#endif
            if (handle->behaviorFlags() != flags) {
                handle->setBehaviorFlags(flags);
            }
            return handle;
        }
    }

    // Create handle from filepaths
    handle = ResourceHandle::create(m_engine, filepaths[0], type);
    for (size_t i = 1; i < filepaths.size(); i++) {
        handle->additionalPaths().push_back(filepaths[i]);
    }
    handle->setBehaviorFlags(flags);
    handle->loadResource();

    return handle;
}
 /////////////////////////////////////////////////////////////////////////////////////////////
 std::shared_ptr<ResourceHandle> ResourceCache::getHandleWithName(const GString & name, ResourceType type) const
 {
     //QMutexLocker lock(&s_resourceMutex);

     auto resourceIter = m_resources.find_if(
         [&](const auto& resourcePair) {
         return resourcePair.second->getName() == name && resourcePair.second->getResourceType() == type;
     });
     if (resourceIter != m_resources.end()) {
         return resourceIter->second;
     }
     else {
         return nullptr;
     }

 }
 /////////////////////////////////////////////////////////////////////////////////////////////
 void ResourceCache::postConstruction()
 {   
     // Load built-ins
     initializeCoreResources();
 }
 /////////////////////////////////////////////////////////////////////////////////////////////
void ResourceCache::clear()
{
    //QMutexLocker lock(&s_resourceMutex);

    // Ensure that main context is current
    m_engine->setGLContext();

    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
#ifdef DEBUG_MODE
    bool error = gl.printGLError("Error before releasing memory");
    if (error) {
        throw("Error before releasing memory");
    }
#endif

    if (m_loadCount && m_engine->isConstructed()) {
        throw("Error, no resources should be loading when clearing cache");
    }

    // Start with a blank slate
    m_currentCost = 0;
    clearResources();

    // Ensure that all OpenGL commands are flushed so resources are cleared
    gl.glFinish(); 

#ifdef DEBUG_MODE
    error = gl.printGLError("Error releasing memory");
    if (error) {
        throw("Error releasing memory");
    }
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ResourceCache::getHandle(const Uuid & uuid) const
{
    //QMutexLocker lock(&s_resourceMutex);

    //auto iter = std::find_if(m_resources.begin(), m_resources.end(),
    //    [&](const std::pair<Uuid, std::shared_ptr<ResourceHandle>>& resourcePair) {
    //    return resourcePair.second->getUuid() == uuid;
    //});
    auto iter = m_resources.find_if(
        [&](const std::pair<Uuid, std::shared_ptr<ResourceHandle>>& resourcePair) {
        return resourcePair.second->getUuid() == uuid;
    });
    if (iter != m_resources.end()) {
        return iter->second;
    }
    else {
        return nullptr;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ResourceCache::getHandle(const QJsonValue & handleJson) const
{
    QJsonObject object = handleJson.toObject();
    if (!object.contains("uuid")) {
        throw("Error, no UUID associated with resource");
    }
    Uuid handleID(handleJson["uuid"].toString());
    if (getHandle(handleID)) {
        return getHandle(handleID);
    }
    else {
        auto handle = ResourceHandle::create(m_engine, ResourceType::kNullType);
        handle->loadFromJson(handleJson);
        handle->loadResource();
        return handle;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceCache::clearResources()
{
    ResourceList coreResources; // Temporary list of core resources
    for (const auto& resourceHandle: m_topLevelResources) {
        if (resourceHandle->isCore())
        {
            // Cache core resource
            coreResources.push_back(resourceHandle);
        }
        else {
            // Erase resource if not a core resource
            resourceHandle->unloadResource(false);
            resourceHandle->removeFromCache(false);
        }
    }

    // Leave only top level resources in deque
    m_topLevelResources.swap(coreResources);
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ResourceCache::remove(const std::shared_ptr<ResourceHandle>& resourceHandle, ResourceDeleteFlags deleteFlags)
{
    bool removed = remove(resourceHandle.get(), deleteFlags);

    // Emit signal for widgets to know that the handle was deleted
    emit m_engine->resourceCache()->resourceDeleted(resourceHandle->getUuid());

    return removed;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ResourceCache::remove(ResourceHandle * resourceHandle, ResourceDeleteFlags deleteFlags)
{
    // Lock in case adding from another thread
    QMutexLocker lock(&s_resourceMutex);

    // Move resource to the front of the queue, since it will either be removed, in which case
    // we don't want it in the back of the queue for another attempted removal, or it is permanent,
    // in which case we also don't want to reattempt removal
    resourceHandle->touch();

    // Skip if a core resource
    if (resourceHandle->isCore()) {
        return false;
    }

    // Skip if resource is not removable, and not forcing deletion
    bool forceDeletion = deleteFlags.testFlag(ResourceDeleteFlag::kForce);
    if (!resourceHandle->isRemovable() && !forceDeletion) {
        return false;
    }

    if (resourceHandle->isChild()) {
        throw("Error, only top-level resources can be removed by resource cache");
    }

    // Reduce cost
    m_currentCost -= resourceHandle->resource()->getCost();

    // Removes resource and child resources
    resourceHandle->unloadResource(false);

    // Remove from resource cache structs if this is a hard delete
    if (deleteFlags.testFlag(ResourceDeleteFlag::kDeleteHandle)) {
        resourceHandle->removeFromCache(true);
    }

    if (m_currentCost < 0) {
        throw("Error, current cost cannot be less than zero");
    }

#ifdef DEBUG_MODE
    Logger::LogInfo("Removed resource");
    logCurrentCost();
#endif

    return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////
qint64 ResourceCache::getMaxMemoryMb()
{
    return qint64(MemoryManager::GET_MAX_MEMORY() / (1024.0 * 1024));
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceCache::logCurrentCost() const
{
    Logger::LogInfo("Current cost: " + QString::number(m_currentCost));
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceCache::initializeCoreResources()
{
    // Create base behavior script
    // See PythonAPI::PythonAPI for py_scripts definition
//#ifndef DEBUG_MODE
//    QString baseBehaviorPath = ":scripts/base_behavior.py";
//#else
//    QString baseBehaviorPath = QFileInfo(QFile("py_scripts:base_behavior.py")).absoluteFilePath();
//#endif
//    auto baseScriptHandle = ResourceHandle::create(m_engine, baseBehaviorPath, ResourceType::kPythonScript);
//    baseScriptHandle->setCore(true);
//    baseScriptHandle->loadResource();

    // Create base listener script
//    QString baseListenerPath;
//#ifndef DEBUG_MODE
//    baseListenerPath = ":scripts/base_listener.py";
//#else
//    baseListenerPath = QFileInfo(QFile("py_scripts:base_listener.py")).absoluteFilePath();
//#endif
//    auto baseListenerHandle = ResourceHandle::create(m_engine, baseListenerPath, ResourceType::kPythonScript);
//    baseListenerHandle->setCore(true);
//    baseListenerHandle->loadResource();

    // Initialize shader resources
    for (const std::pair<QString, Shader::ShaderType>& shaderPair : Shader::Builtins()) {
        QString shaderName = shaderPair.first;
        QString shaderPath = QStringLiteral(":shaders/") + shaderName;
        QString fullShaderPath;
        switch (shaderPair.second) {
        case Shader::ShaderType::kVertex:
            fullShaderPath = shaderPath + ".vert";
            break;
        case Shader::ShaderType::kFragment:
            fullShaderPath = shaderPath + ".frag";
            break;
        case Shader::ShaderType::kCompute:
            fullShaderPath = shaderPath + ".comp";
            break;
        }

        std::shared_ptr<ResourceHandle> shaderHandle;
        if (shaderName.contains("ssao")) {
            // FIXME: Tweak so no longer need a special case for when vertex shader name is different than frag
            shaderHandle = ResourceHandle::create(m_engine, ":shaders/quad.vert", ResourceType::kShaderProgram);
            shaderHandle->additionalPaths().push_back(fullShaderPath);
        }
        else if (shaderName.contains("canvas_billboard")) {
            // FIXME: Tweak so no longer need a special case for when vertex shader name is different than frag
            // Make Canvas Billboard shader
            QString fragShaderPath = QStringLiteral(":shaders/") + "canvas.frag";
            shaderHandle = ResourceHandle::create(m_engine, ":shaders/canvas_billboard.vert", ResourceType::kShaderProgram);
            shaderHandle->additionalPaths().push_back(fragShaderPath);
        }
        else if (shaderName.contains("canvas_gui")) {
            // FIXME: Tweak so no longer need a special case for when vertex shader name is different than frag
            // Make Canvas GUI shader
            QString fragShaderPath = QStringLiteral(":shaders/") + "canvas.frag";
            shaderHandle = ResourceHandle::create(m_engine, ":shaders/canvas_gui.vert", ResourceType::kShaderProgram);
            shaderHandle->additionalPaths().push_back(fragShaderPath);
        }
        else {
            shaderHandle = ResourceHandle::create(m_engine, fullShaderPath, ResourceType::kShaderProgram);
        }
        shaderHandle->setName(shaderName);
        shaderHandle->setCore(true);
        if (shaderPair.second == Shader::ShaderType::kVertex) {
            // Also load fragment shader if this is a vertex shader
            shaderHandle->additionalPaths().push_back(shaderPath + ".frag");

            // Also load geometry shader if there is one
            if (QFileInfo(shaderPath + ".geom").exists()) {
                shaderHandle->additionalPaths().push_back(shaderPath + ".geom");
            }
        }
        shaderHandle->loadResource();
    }

    // Initialize polygons
    m_polygonCache->initializeCoreResources();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceCache::incrementLoadCount()
{
    QMutexLocker lock(&m_engine->resourceCache()->m_loadCountMutex);
    m_loadCount++;
    if (m_engine->resourceCache()->m_loadCount == 1) {
        // Emit started loading signal if first resource getting loaded
        emit m_engine->resourceCache()->startedLoadingResources();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceCache::decrementLoadCount()
{
    QMutexLocker lock(&m_engine->resourceCache()->m_loadCountMutex);

    if (m_loadCount == 0) {
        throw("Error, count has been led astray");
    }

    m_loadCount--;
    if (m_loadCount == 0) {
        emit doneLoadingResources();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceCache::runPostConstruction(const Uuid& handleId) {

    const std::shared_ptr<ResourceHandle>& resourceHandle = getHandle(handleId);
    
    QMutexLocker(&resourceHandle->mutex());

    // Set OpenGL context to main context
    m_engine->setGLContext();

    if (resourceHandle->isConstructed()) {
        throw("Error, resource already constructed");
    }
    resourceHandle->postConstruct();

#ifdef DEBUG_MODE
    Logger::LogInfo("Running post-construction of " + resourceHandle->getName() +
        " on thread: " + Process::GetThreadID());
#endif

}

/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue ResourceCache::asJson(const SerializationContext& context) const
{
    QJsonObject object;

    size_t loadProcessCount = m_processManager->loadProcessCount();
    if (m_loadCount > 0 || loadProcessCount > 0) {
        throw("Error, not all resources are done loading");
    }

    // Sort resources
    std::vector<std::shared_ptr<ResourceHandle>> resourceVec;
    for (const std::shared_ptr<ResourceHandle>& handle : m_topLevelResources) {
        if (!handle->isUnsaved()) {
            if (handle->isLoading()) {
                throw("Something has gone awry, resource not loaded but still reached this point");
            }

            resourceVec.push_back(handle);
        }
    }

    std::sort(resourceVec.begin(), resourceVec.end(),
        [](const auto& handle1, const auto& handle2) {
        return handle1->getName() < handle2->getName();
    });

    // Save resources to json
    QJsonArray resources;
    for (const std::shared_ptr<ResourceHandle>& handle : resourceVec) {
        resources.append(handle->asJson());
    }
    object.insert("resources", resources);

    // Save max cost to json
    object.insert("maxCost", int(m_maxCost));

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceCache::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

#ifdef DEBUG_MODE    
    GL::OpenGLFunctions& functions = *GL::OpenGLFunctions::Functions();
    functions.printGLError("Error before setting context");
#endif

    // Set OpenGL context to main context
    m_engine->setGLContext();

    const QJsonObject& object = json.toObject();

    // Sort resources by type to load dependencies in properly, e.g. meshes before models
    QJsonArray resources = object.value("resources").toArray();
    std::sort(resources.begin(), resources.end(), 
        [](const QJsonValue &v1, const QJsonValue &v2) {
        return v1.toObject()["type"].toInt() < v2.toObject()["type"].toInt();
    });

    // Load resources from json
    for (const auto& resourceJson : resources) {
        QJsonObject handleObject = resourceJson.toObject();

        // Ensuring that UUID is set from cached value. Loading from JSON after
        // creation leads to indexing problems
        std::shared_ptr<ResourceHandle> handle = ResourceHandle::create(m_engine, handleObject);

#ifdef DEBUG_MODE    
        functions.printGLError("Error during resource creation");
#endif
    }

    // Load shader presets from json
    // Deprecated entirely, moved to scenario
    if (object.contains("shaderPresets")) { // legacy check
        const QJsonArray& shaderPresets = object.value("shaderPresets").toArray();
        for (const auto& shaderJson : shaderPresets) {
            auto shaderPreset = std::make_shared<ShaderPreset>(m_engine, shaderJson);
            m_engine->scenario()->settings().shaderPresets().push_back(shaderPreset);
        }
    }

    // Save max cost to json
    m_maxCost = object.value("maxCost").toInt();
}

/////////////////////////////////////////////////////////////////////////////////////////////
QMutex ResourceCache::s_resourceMutex = QMutex();


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces