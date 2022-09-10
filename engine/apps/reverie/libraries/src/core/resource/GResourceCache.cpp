#include "core/resource/GResourceCache.h"

#include <QDirIterator>
#include <QRegExp>

#include "core/processes/GProcessManager.h"
#include "core/processes/GLoadProcess.h"

#include "core/scene/GScenario.h"
#include "core/scripting/GPythonScript.h"
#include "core/rendering/geometry/GPolygon.h"
#include "core/GCoreEngine.h"
#include "fortress/system/memory/GPointerTypes.h"
#include "fortress/system/memory/GMemoryMonitor.h"
#include "core/rendering/shaders/GShaderProgram.h"  
#include "core/rendering/materials/GMaterial.h"
#include "core/rendering/geometry/GVertexData.h"
#include "core/rendering/models/GModel.h"
#include "core/rendering/materials/GCubeTexture.h"
#include "core/animation/GAnimation.h"
#include "core/rendering/shaders/GShaderPreset.h"

namespace rev {

ResourceCache::ResourceCache(CoreEngine* core, ProcessManager* processManager, const size_t sizeInMb):
    Manager(core, "ResourceCache"),
    m_maxCost(sizeInMb),
    m_currentCost(0),
    m_processManager(processManager),
    m_polygonCache(std::make_shared<PolygonCache>(core))
{
    // Connect post-construction routine to resource cache and emit signal
    // Queued connection will ensure slot is executed in receiver's thread
    /// @fixme Make the timing of this synchronous so that behavior is more deterministic
    QObject::connect(this,
        &ResourceCache::doneLoadingResource,
        this,
        &ResourceCache::queuePostConstruction,
        Qt::QueuedConnection);

#ifdef DEBUG_MODE
    if (sizeInMb > MemoryMonitor::GetMaxMemoryMb() / 2.0) {
        Logger::LogWarning("Warning, resource cache allows for more than half of available system memory, may cause performance issues");
    }
#endif

}

ResourceCache::~ResourceCache()
{
}

bool ResourceCache::clearedRemovable() const
{
    return !std::any_of(m_topLevelResources.begin(), 
        m_topLevelResources.end(),
        [](const std::shared_ptr<ResourceHandle>& r) {
            return !r->isPermanent();
    });
}

bool ResourceCache::insertHandle(const std::shared_ptr<ResourceHandle>& resource)
{
    return insertHandle(resource, nullptr);
}

bool ResourceCache::insertHandle(const std::shared_ptr<ResourceHandle>& resourceHandle, bool* clearedResources)
{
    // Lock in case adding from another thread
    std::unique_lock lock(m_resourceMutex);

    // Add resourceHandle
    const Uuid& uuid = resourceHandle->getUuid();
    if (m_resources.hasKey(uuid)) {
#ifdef DEBUG_MODE
        const std::shared_ptr<ResourceHandle>& oldHandle = m_resources[uuid];
        Logger::LogInfo("Reloading resource: " + oldHandle->getName());
        Logger::Throw("Reloading resource");
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

std::shared_ptr<ResourceHandle> ResourceCache::getTopLevelHandleWithPath(const GString & filepath) const
{
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

std::shared_ptr<ResourceHandle> ResourceCache::guaranteeHandleWithPath(const GString & filepath,
    GResourceType type, ResourceBehaviorFlags flags)
{
    std::shared_ptr<ResourceHandle> handle = getTopLevelHandleWithPath(filepath);
    if (handle) {
#ifdef DEBUG_MODE
        if (handle->getResourceType() != type)
            Logger::Throw("Handle type mismatch");
#endif
        if (handle->behaviorFlags() != flags) {
            handle->setBehaviorFlags(flags);
        }
    }
    else{
        handle = ResourceHandle::Create(m_engine, filepath, type);
        handle->setBehaviorFlags(flags);
        handle->loadResource();
    }

    return handle;
}

std::shared_ptr<ResourceHandle> ResourceCache::guaranteeHandleWithPath(const std::vector<GString>& filepaths,
    GResourceType type, ResourceBehaviorFlags flags)
{
    if (type != EResourceType::eShaderProgram)
        Logger::Throw("Only shader programs are supported with multiple-file construction");

    // Check that a resource associated with any of the input files does not exist
    std::shared_ptr<ResourceHandle> handle;
    for (const GString& path : filepaths) {
        // Ignore any blank paths
        if (path.isEmpty()) continue;

        handle = getTopLevelHandleWithPath(path);
        if (handle) {
#ifdef DEBUG_MODE
            if (handle->getResourceType() != type)
                Logger::Throw("Handle type mismatch");
#endif
            if (handle->behaviorFlags() != flags) {
                handle->setBehaviorFlags(flags);
            }
            return handle;
        }
    }

    // Create handle from filepaths
    handle = ResourceHandle::Create(m_engine, filepaths[0], type);
    for (size_t i = 1; i < filepaths.size(); i++) {
        handle->additionalPaths().push_back(filepaths[i]);
    }
    handle->setBehaviorFlags(flags);
    handle->loadResource();

    return handle;
}
 
 std::shared_ptr<ResourceHandle> ResourceCache::getHandleWithName(const GString & name, GResourceType type) const
 {
     auto resourceIter = m_resources.find_if(
         [&](const auto& resourcePair) {
         return resourcePair.second->getName() == name && resourcePair.second->getResourceType() == type;
     });
     if (resourceIter != m_resources.end()) {
         std::shared_ptr<ResourceHandle> oResourceHandle = resourceIter->second;
         return oResourceHandle;
     }
     else {
         const Int32_t iNumResources = m_resources.size();
         if (0 == iNumResources) {
             Logger::LogWarning("No resources found in map");
         }
         return nullptr;
     }

 }
 
 void ResourceCache::postConstruction()
 {   
     // Load built-ins
     initializeCoreResources();
 }
 
void ResourceCache::clear()
{
    // Ensure that main context is current
    m_engine->setGLContext();

    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
#ifdef DEBUG_MODE
    bool error = gl.printGLError("Error before releasing memory");
    if (error) {
        Logger::Throw("Error before releasing memory");
    }
#endif

    if (m_loadCount && m_engine->isConstructed()) {
        Logger::Throw("Error, no resources should be loading when clearing cache");
    }

    // Start with a blank slate
    m_currentCost = 0;
    clearResources();

    // Ensure that all OpenGL commands are flushed so resources are cleared
    gl.glFinish(); 

#ifdef DEBUG_MODE
    error = gl.printGLError("Error releasing memory");
    if (error) {
        Logger::Throw("Error releasing memory");
    }
#endif
}

std::shared_ptr<ResourceHandle> ResourceCache::getHandle(const Uuid & uuid) const
{
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

std::shared_ptr<ResourceHandle> ResourceCache::getHandle(const json & handleJson) const
{
    if (!handleJson.contains("uuid")) {
        Logger::Throw("Error, no UUID associated with resource");
    }
    Uuid handleID = handleJson["uuid"];
    if (getHandle(handleID)) {
        return getHandle(handleID);
    }
    else {
        auto handle = ResourceHandle::Create(m_engine, (GResourceType)EResourceType::eINVALID);
        handleJson.get_to(*handle);
        handle->loadResource();
        return handle;
    }
}


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

bool ResourceCache::remove(const std::shared_ptr<ResourceHandle>& resourceHandle, ResourceDeleteFlags deleteFlags)
{
    bool removed = remove(resourceHandle.get(), deleteFlags);

    // Emit signal for widgets to know that the handle was deleted
    if (!resourceHandle->isHidden()) {
        ResourceCache::Instance().m_resourceDeleted.emitForAll(resourceHandle->getUuid());
    }
    return removed;
}

bool ResourceCache::remove(ResourceHandle * resourceHandle, ResourceDeleteFlags deleteFlags)
{
    std::unique_lock lock(m_resourceMutex);

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
        Logger::Throw("Error, only top-level resources can be removed by resource cache");
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
        Logger::Throw("Error, current cost cannot be less than zero");
    }

#ifdef DEBUG_MODE
    Logger::LogInfo("Removed resource");
    logCurrentCost();
#endif

    return true;
}

void ResourceCache::logCurrentCost() const
{
    Logger::LogInfo("Current cost: " + GString::FromNumber(m_currentCost));
}

void ResourceCache::initializeCoreResources()
{
    // Initialize core resources, mainly shader resources
    /// For info on prefix:
    /// @see https://doc.qt.io/archives/qt-4.8/resources.html#using-resources-in-the-application
    for (const std::pair<QString, Shader::ShaderType>& shaderPair : Shader::Builtins()) {
        GString shaderName = shaderPair.first.toStdString();
        GString shaderPath = GString(":shaders/") + shaderName;
        GString fullShaderPath;
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
            shaderHandle = ResourceHandle::Create(m_engine, ":shaders/quad.vert", EResourceType::eShaderProgram);
            shaderHandle->additionalPaths().push_back(fullShaderPath);
        }
        else if (shaderName.contains("canvas_billboard")) {
            // FIXME: Tweak so no longer need a special case for when vertex shader name is different than frag
            // Make Canvas Billboard shader
            GString fragShaderPath = GString(":shaders/") + "canvas.frag";
            shaderHandle = ResourceHandle::Create(m_engine, ":shaders/canvas_billboard.vert", EResourceType::eShaderProgram);
            shaderHandle->additionalPaths().push_back(fragShaderPath);
        }
        else if (shaderName.contains("canvas_gui")) {
            // FIXME: Tweak so no longer need a special case for when vertex shader name is different than frag
            // Make Canvas GUI shader
            GString fragShaderPath = GString(":shaders/") + "canvas.frag";
            shaderHandle = ResourceHandle::Create(m_engine, ":shaders/canvas_gui.vert", EResourceType::eShaderProgram);
            shaderHandle->additionalPaths().push_back(fragShaderPath);
        }
        else {
            shaderHandle = ResourceHandle::Create(m_engine, fullShaderPath, EResourceType::eShaderProgram);
        }
        shaderHandle->setName(shaderName);
        shaderHandle->setCore(true);
        if (shaderPair.second == Shader::ShaderType::kVertex) {
            // Also load fragment shader if this is a vertex shader
            shaderHandle->additionalPaths().push_back(shaderPath + ".frag");
        }
        shaderHandle->loadResource();
    }

    // Initialize polygons
    m_polygonCache->initializeCoreResources();
}

void ResourceCache::queuePostConstruction(const Uuid& uuid)
{
    std::unique_lock lock(m_postConstructMutex);
    m_resourcesForPostConstruction.push_back(uuid);
}

void ResourceCache::addPostConstructionData(const Uuid& handleId, const ResourcePostConstructionData& data)
{
    std::unique_lock lock(m_postConstructMutex);
    m_resourcePostConstructionData[handleId] = data;
}

void ResourceCache::postConstructResources()
{
    std::unique_lock lock(m_postConstructMutex);
    for (const Uuid& id : m_resourcesForPostConstruction) {
        runPostConstruction(id);
        m_resourcePostConstructionData.erase(id);
    }
    m_resourcesForPostConstruction.clear();
}

void ResourceCache::incrementLoadCount()
{
    m_loadCount.fetch_add(1);
    if (m_loadCount.load() == 1) {
        // Emit started loading signal if first resource getting loaded
        emit ResourceCache::Instance().startedLoadingResources();
    }
}

void ResourceCache::decrementLoadCount()
{
    if (m_loadCount.load() == 0) {
        Logger::Throw("Error, count has been led astray");
    }

    m_loadCount.fetch_sub(1);
    if (m_loadCount.load() == 0) {
        emit doneLoadingResources();
    }
}

void ResourceCache::runPostConstruction(const Uuid& handleId) {

    const std::shared_ptr<ResourceHandle>& resourceHandle = getHandle(handleId);

    QMutexLocker lock(&resourceHandle->mutex());

    // Set OpenGL context to main context
    m_engine->setGLContext();

    if (resourceHandle->isConstructed()) {
        Logger::Throw("Error, resource already constructed");
    }

    if (m_resourcePostConstructionData.contains(handleId)) {
        resourceHandle->postConstruct(m_resourcePostConstructionData[handleId]);
    }
    else {
        resourceHandle->postConstruct(ResourcePostConstructionData());
    }

#ifdef DEBUG_MODE
    Logger::LogInfo("Running post-construction of " + resourceHandle->getName());
#endif

}


void to_json(json& orJson, const ResourceCache& korObject)
{
    size_t loadProcessCount = LoadProcess::Count();

    if (korObject.m_loadCount > 0 || loadProcessCount > 0) {
        Logger::LogWarning("to_json:: Warning, not all resources are done loading");
    }

    // Sort resources
    std::vector<std::shared_ptr<ResourceHandle>> resourceVec;
    for (const std::shared_ptr<ResourceHandle>& handle : korObject.m_topLevelResources) {
        if (!handle->isUnsaved()) {
            if (handle->isLoading()) {
                Logger::LogWarning("to_json:: Resource not loaded, skipping JSON serialization");
            }
            else {
                resourceVec.push_back(handle);
            }
        }
    }

    std::sort(resourceVec.begin(), resourceVec.end(),
        [](const auto& handle1, const auto& handle2) {
        return handle1->getName() < handle2->getName();
    });

    // Save resources to json
    orJson["resources"] = json::array();
    for (const std::shared_ptr<ResourceHandle>& handle : resourceVec) {
        json handleJson = *handle;
        orJson["resources"].push_back(handleJson);
    }

    // Save max cost to json
    orJson["maxCost"] = int(korObject.m_maxCost);
}

void from_json(const json& korJson, ResourceCache& orObject)
{

#ifdef DEBUG_MODE    
    gl::OpenGLFunctions& functions = *gl::OpenGLFunctions::Functions();
    functions.printGLError("Error before setting context");
#endif

    // Set OpenGL context to main context
    orObject.m_engine->setGLContext();

    // Sort resources by type to load dependencies in properly, e.g. meshes before models
    json resources = korJson.at("resources");
    std::sort(resources.begin(), resources.end(), 
        [](const json& v1, const json& v2) {
        return v1.at("type").get<Int32_t>() < v2.at("type").get<Int32_t>();
    });

    // Load resources from json
    for (const json& resourceJson : resources) {
        // Ensuring that UUID is set from cached value. Loading from JSON after
        // creation leads to indexing problems
        std::shared_ptr<ResourceHandle> handle = ResourceHandle::Create(orObject.m_engine, resourceJson);

#ifdef DEBUG_MODE    
        functions.printGLError("Error during resource creation");
#endif
    }

    // Load shader presets from json
    // Deprecated entirely, moved to scenario
    if (korJson.contains("shaderPresets")) { // legacy check
        const json& shaderPresets = korJson.at("shaderPresets");
        for (const json& shaderJson : shaderPresets) {
            auto shaderPreset = std::make_shared<ShaderPreset>(orObject.m_engine, shaderJson);
            orObject.m_engine->scenario()->settings().addShaderPreset(shaderPreset);
        }
    }

    // Save max cost to json
    korJson.at("maxCost").get_to(orObject.m_maxCost);
}


} // End namespaces