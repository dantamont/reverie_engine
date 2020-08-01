#include "GbResourceCache.h"

#include "GbImage.h"

#include <QDirIterator>
#include <QRegExp>

#include "../processes/GbProcessManager.h"
#include "../processes/GbLoadProcess.h"

#include "../scripting/GbPythonScript.h"
#include "../rendering/geometry/GbPolygon.h"
#include "../GbCoreEngine.h"
#include "../utils/GbMemoryManager.h"
#include "../rendering/shaders/GbShaders.h"
#include "../rendering/materials/GbMaterial.h"
#include "../rendering/geometry/GbVertexData.h"
#include "../rendering/models/GbModel.h"
#include "../rendering/materials/GbCubeTexture.h"
#include "../animation/GbAnimation.h"
#include "../rendering/shaders/GbShaderPreset.h"

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
ResourceCache::ResourceCache(CoreEngine* core, ProcessManager* processManager, const size_t sizeInMb):
    Manager(core, "ResourceCache"),
    m_maxCost(sizeInMb),
    m_currentCost(0),
    m_processManager(processManager),
    m_polygonCache(std::make_shared<PolygonCache>(core))
{
    // Register resource handle to send over qt
    qRegisterMetaType<std::shared_ptr<ResourceHandle>>("Gb::ResourceHandle");

    // Initialize connections
    //connect(this, &ResourceCache::readyForLoad, this, &ResourceCache::loadModelResources);

    // Connect post-construction routine to resource cache and emit signal
    // Queued connection will ensure slot is executed in receiver's thread
    QObject::connect(this,
        &ResourceCache::doneLoadingResource,
        this,
        &ResourceCache::runPostConstruction,
        Qt::QueuedConnection);

    //// Connect signal to reload a resource to resource cache load routine
    //QObject::connect(this,
    //    &ResourceCache::doneLoadingResource,
    //    this,
    //    (void(ResourceCache::*)(std::shared_ptr<ResourceHandle>))&ResourceCache::reloadResource,
    //    Qt::QueuedConnection);

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
/////////////////////////////////////////////////////////////////////////////////////////////s
bool ResourceCache::removeShaderPreset(const QString & name)
{
    std::unordered_map<Uuid, std::shared_ptr<ShaderPreset>>::iterator iter;
    if(hasShaderPreset(name, iter)) {
        m_shaderPresets.erase(iter);
        return true;
    }

    throw("Error, shader material with the specified name not found");
    return false;
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
    QMutexLocker lock(&m_resourceMutex);

    // Add resourceHandle
    const Uuid& uuid = resourceHandle->getUuid();
    if (m_resources.find(uuid) != m_resources.end()) {
#ifdef DEBUG_MODE
        const std::shared_ptr<ResourceHandle>& oldHandle = m_resources[uuid];
        logInfo("Reloading resource: " + oldHandle->getName());
#endif
        //m_resources[uuid] = resourceHandle;
        resourceHandle->touch();
    }
    else {
        Map::Emplace(m_resources, uuid, resourceHandle);

        // Only add to top level list if resource handle is not a child
        if (!resourceHandle->isChild()) {
            m_topLevelResources.push_front(resourceHandle);
        }

        // Add to type-specific maps if applicable
        switch (resourceHandle->getResourceType()) {
        case Resource::kModel:
            m_models[resourceHandle->getUuid()] = resourceHandle;
            break;
        case Resource::kMaterial:
            m_materials[resourceHandle->getUuid()] = resourceHandle;
            break;
        case Resource::kShaderProgram:
            m_shaderPrograms[resourceHandle->getUuid()] = resourceHandle;
            break;
        case Resource::kPythonScript:
            m_pythonScripts[resourceHandle->getUuid()] = resourceHandle;
            break;
        case Resource::kSkeleton:
            m_skeletons[resourceHandle->getUuid()] = resourceHandle;
            break;
        case Resource::kCubeTexture:
        case Resource::kAnimation:
        case Resource::kTexture:
        case Resource::kImage:
        case Resource::kMesh:
            break;
        default:
            throw("Error, texture type not recognized");
            break;
        }
    }

    // Return if resource not yet loaded
    if (!resourceHandle->resource(false)) {
        return true;
    }

    // Increment total cost of resources
    m_currentCost += resourceHandle->resource(false)->getCost();

#ifdef DEBUG_MODE
    logInfo("Inserted resource");
    logCurrentCost();
#endif

    // Remove resources if over max cost
    bool removedResources = false;
    bool succeeded = true;
    while (m_currentCost > m_maxCost) {
        remove(oldestResource());
        if (clearedRemovable()) {
#ifdef DEBUG_MODE
            logError("Could not clear resource cache sufficiently to store new resource");
            logError("Error, failed to load resource at" + resourceHandle->getPath()
                + ", resource too large at " + QString::number(resourceHandle->resource(false)->getCost()));
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
std::shared_ptr<ResourceHandle> ResourceCache::getTopLevelHandleWithPath(const QString & filepath) const
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
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ResourceCache::guaranteeHandleWithPath(const QString & filepath,
    Resource::ResourceType type, ResourceHandle::BehaviorFlags flags)
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
std::shared_ptr<ResourceHandle> ResourceCache::guaranteeHandleWithPath(const std::vector<QString>& filepaths,
    Resource::ResourceType type, ResourceHandle::BehaviorFlags flags)
{
    if (type != Resource::kShaderProgram)
        throw("Only shader programs are supported with multiple-file construction");

    // Check that a resource associated with any of the input files does not exist
    std::shared_ptr<ResourceHandle> handle;
    for (const QString& path : filepaths) {
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
 std::shared_ptr<ResourceHandle> ResourceCache::getHandleWithName(const QString & name, Resource::ResourceType type) const
 {
     auto resourceIter = std::find_if(m_resources.begin(), m_resources.end(),
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
 bool ResourceCache::hasShaderPreset(const QString & name, 
     std::unordered_map<Uuid, std::shared_ptr<ShaderPreset>>::const_iterator & iter) const
 {
     iter = std::find_if(m_shaderPresets.begin(), m_shaderPresets.end(),
         [&](const auto& matPair) {
         return matPair.second->getName() == name;
     });
     return m_shaderPresets.end() != iter;
 }
 /////////////////////////////////////////////////////////////////////////////////////////////
void ResourceCache::clear()
{
    QMutexLocker lock(&m_resourceMutex);

    // Start with a blank slate
    m_currentCost = 0;
    clearTopLevelResources();
    clearResources(m_resources);
    clearResources(m_models);
    clearResources(m_materials);
    clearResources(m_pythonScripts);
    clearResources(m_shaderPrograms);
    m_shaderPresets.clear();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ShaderPreset> ResourceCache::getShaderPreset(const QString & name, bool & created)
{
    // Check if shader material is in the map 
    std::unordered_map<Uuid, std::shared_ptr<ShaderPreset>>::const_iterator iM;
    if (hasShaderPreset(name, iM)) {
        created = false;
        return iM->second;
    }

    // Create new shader material if not in map
    created = true;
    auto preset = std::make_shared<ShaderPreset>(m_engine, name);
    Map::Emplace(m_shaderPresets, preset->getUuid(), preset);
    return preset;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ShaderPreset> ResourceCache::getShaderPreset(const Uuid & uuid)
{
    return m_shaderPresets[uuid];
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ResourceCache::getHandle(const Uuid & uuid) const
{
    auto iter = std::find_if(m_resources.begin(), m_resources.end(),
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
        auto handle = ResourceHandle::create(m_engine, Resource::kNullType);
        handle->loadFromJson(handleJson);
        handle->loadResource();
        return handle;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceCache::clearResources(ResourceMap& map)
{
    //m_resources.clear();
    auto endIter = map.end();
    for (auto it = map.begin(); it != endIter;) {
        if (!it->second->isCore())
        {
            // Erase resource if not a core resource
            map.erase(it++);    // or "it = m.erase(it)" since C++11
        }
        else {
            // Skip if is a core resource
            switch (it->second->getResourceType()) {
            case Resource::kShaderProgram:
                // Clear uniforms for shader program on reset
                it->second->resourceAs<ShaderProgram>()->clearUniforms();
                break;
            }
            it++;
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceCache::clearTopLevelResources()
{
    auto endIter = m_topLevelResources.end();
    for (auto it = m_topLevelResources.begin(); it != endIter;) {
        if (!(*it)->isCore())
        {
            // Erase resource if not a core resource
            m_topLevelResources.erase(it++);    // or "it = m.erase(it)" since C++11
        }
        else {
            // Skip if is a core resource
            it++;
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceCache::reloadResource(const Uuid& uuid)
{
    std::shared_ptr<ResourceHandle> handle = getHandle(uuid);
    handle->loadResource();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceCache::reloadResource(std::shared_ptr<ResourceHandle> handle)
{
    if (handle->resource(false)) {
#ifdef DEBUG_MODE
        throw("Error, tried to reload an already loaded resource");
#endif
        return;
    }

    handle->loadResource();
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ResourceCache::remove(std::shared_ptr<ResourceHandle> resourceHandle, ResourceHandle::DeleteFlags deleteFlags)
{
    // Lock in case adding from another thread
    QMutexLocker lock(&m_resourceMutex);

    // Move resource to the front of the queue, since it will either be removed, in which case
    // we don't want it in the back of the queue for another attempted removal, or it is permanent,
    // in which case we also don't want to reattempt removal
    resourceHandle->touch();

    bool forceDeletion = deleteFlags.testFlag(ResourceHandle::kForce) && !resourceHandle->isCore();
    if (!resourceHandle->isRemovable() && !forceDeletion) {
        return false;
    }

    if(resourceHandle->isChild())
        throw("Error, only top-level resources can be removed by resource cache");

    // Reduce cost
    m_currentCost -= resourceHandle->resource(false)->getCost();

    // Removes resource and child resources
    resourceHandle->removeResources(false);

    // Remove from resource cache structs if this is a hard delete
    if (deleteFlags.testFlag(ResourceHandle::kDeleteHandle)) {
        m_resources.erase(resourceHandle->getUuid());

        auto iter = std::find_if(m_topLevelResources.begin(), m_topLevelResources.end(),
            [&](const auto& handle) {
            return handle->getUuid() == resourceHandle->getUuid();
        }
        );
        if (iter == m_topLevelResources.end()) {
            throw("Error, only top-level resources can be forcefully deleted");
        }
        m_topLevelResources.erase(iter);

        // Erase from type-specific maps if applicable
        switch (resourceHandle->getResourceType()) {
        case Resource::kModel:
            m_models.erase(resourceHandle->getUuid());
            break;
        case Resource::kMaterial:
            m_materials.erase(resourceHandle->getUuid());
            break;
        case Resource::kShaderProgram:
            m_shaderPrograms.erase(resourceHandle->getUuid());
            break;
        case Resource::kPythonScript:
            m_pythonScripts.erase(resourceHandle->getUuid());
            break;
        case Resource::kSkeleton:
            m_skeletons.erase(resourceHandle->getUuid());
            break;
        default:
            break;
        }
    }

    if (m_currentCost < 0) {
        throw("Error, current cost cannot be less than zero");
    }

#ifdef DEBUG_MODE
    logInfo("Removed resource");
    logCurrentCost();
#endif

    // Emit signal for widgets to know that the handle was deleted
    emit m_engine->resourceCache()->resourceDeleted(resourceHandle);

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
    logInfo("Current cost: " + QString::number(m_currentCost));
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceCache::initializeCoreResources()
{
    // Create base behavior script
#ifndef DEBUG_MODE
    QString baseBehaviorPath = ":scripts/base_behavior.py";
#else
    QString baseBehaviorPath = QFileInfo(QFile("py_scripts:base_behavior.py")).absoluteFilePath();
#endif
    auto baseScriptHandle = ResourceHandle::create(m_engine, baseBehaviorPath, Resource::kPythonScript);
    baseScriptHandle->setCore(true);
    baseScriptHandle->loadResource();

    // Create base listener script
    QString baseListenerPath;
#ifndef DEBUG_MODE
    baseListenerPath = ":scripts/base_listener.py";
#else
    baseListenerPath = QFileInfo(QFile("py_scripts:base_listener.py")).absoluteFilePath();
#endif
    auto baseListenerHandle = ResourceHandle::create(m_engine, baseListenerPath, Resource::kPythonScript);
    baseListenerHandle->setCore(true);
    baseListenerHandle->loadResource();

    // Initialize shader resources
    for (const QString& shaderName : Shader::Builtins) {
        QString shaderPath = ":shaders/" + shaderName;
        auto shaderHandle = ResourceHandle::create(m_engine, shaderPath + ".vert", Resource::kShaderProgram);
        shaderHandle->setName(shaderName);
        shaderHandle->setCore(true);
        shaderHandle->additionalPaths().push_back(shaderPath + ".frag");
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
void ResourceCache::runPostConstruction(std::shared_ptr<ResourceHandle> resourceHandle) {

    QMutexLocker(&resourceHandle->mutex());

    // Set OpenGL context to main context
    m_engine->setGLContext();

    if (resourceHandle->isConstructed()) {
        throw("Error, resource already constructed");
    }
    resourceHandle->postConstruct();

#ifdef DEBUG_MODE
    logInfo("Running post-construction of " + resourceHandle->getName() +
        " on thread: " + Process::getThreadID());
#endif

}

/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue ResourceCache::asJson() const
{
    QJsonObject object;

    size_t loadProcessCount = m_processManager->loadProcessCount();
    if (m_loadCount > 0 || loadProcessCount > 0) {
        throw("Error, not all resources are done loading");
    }

    // Save resources to json
    QJsonArray resources;
    for (const std::shared_ptr<ResourceHandle>& handle : m_topLevelResources) {
        if (!handle->isCore()) {
            resources.append(handle->asJson());
        }
    }
    object.insert("resources", resources);

    // Save shader materials to json
    QJsonArray shaderPresets;
    for (const auto& smPair : m_shaderPresets) {
        shaderPresets.append(smPair.second->asJson());
    }
    object.insert("shaderPresets", shaderPresets);

    // Save max cost to json
    object.insert("maxCost", int(m_maxCost));

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceCache::loadFromJson(const QJsonValue & json)
{
#ifdef DEBUG_MODE    
    GL::OpenGLFunctions functions;
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
        Resource::ResourceType type = Resource::ResourceType(handleObject["type"].toInt());

        // Error checking for duplicate resources
        std::shared_ptr<ResourceHandle> handle = ResourceHandle::create(m_engine, type);
        handle->loadFromJson(handleObject);

#ifdef DEBUG_MODE    
        functions.printGLError("Error during resource creation");
#endif
    }

    // Load shader presets from json
    if (object.contains("shaderPresets")) { // legacy check
        const QJsonArray& shaderPresets = object.value("shaderPresets").toArray();
        for (const auto& shaderJson : shaderPresets) {
            auto shaderPreset = std::make_shared<ShaderPreset>(m_engine, shaderJson);
            m_shaderPresets.emplace(shaderPreset->getUuid(), shaderPreset);
        }
    }

    // Save max cost to json
    m_maxCost = object.value("maxCost").toInt();
}




/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces