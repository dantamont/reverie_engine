#include "GbResourceCache.h"

#include "GbImage.h"

#include <QDirIterator>
#include <QRegExp>

#include "../processes/GbProcessManager.h"
#include "../processes/GbLoadProcess.h"

#include "../components/GbLight.h"
#include "../scripting/GbPythonScript.h"
#include "../rendering/geometry/GbPolygon.h"
#include "../GbCoreEngine.h"
#include "../utils/GbMemoryManager.h"
#include "../rendering/shaders/GbShaders.h"
#include "../rendering/materials/GbMaterial.h"
#include "../rendering/geometry/GbVertexData.h"
#include "../rendering/models/GbModel.h"
#include "../rendering/materials/GbCubeMap.h"
#include "../animation/GbAnimation.h"

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
bool ResourceCache::removeShaderProgram(const std::shared_ptr<ShaderProgram>& shader)
{
    auto iter = std::find_if(m_shaderPrograms.begin(), m_shaderPrograms.end(),
        [&](const auto& shaderPair) {
        return shaderPair.second->getUuid() == shader->getUuid();
    });

    if (iter != m_shaderPrograms.end()) {
        m_shaderPrograms.erase(iter);
        return true;
    }
    else {
        return false;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ResourceCache::removeMaterial(const QString & name)
{
    if (m_materials.find(name.toLower()) != m_materials.end()) {
        m_materials.erase(name);
        return true;
    }
    return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ResourceCache::removeMaterial(const Uuid & uuid)
{
    auto iter = std::find_if(m_materials.begin(), m_materials.end(),
        [&](const auto& matPair) {
        return matPair.second->getUuid() == uuid;
    });

    if (iter != m_materials.end()) {
        m_materials.erase(iter);
        return true;
    }
    else {
        return false;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ResourceCache::removeMaterial(std::shared_ptr<Material> material)
{
    return removeMaterial(material->getUuid());
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ResourceCache::removeModel(const QString & name)
{
    if (m_models.find(name) != m_models.end()) {
        m_models.erase(name);
        return true;
    }
    return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ResourceCache::removeModel(std::shared_ptr<Model> model)
{
    auto iter = std::find_if(m_models.begin(), m_models.end(), 
        [&](const auto& modelPair) {
        return modelPair.second->getUuid() == model->getUuid();
    });
    
    if (iter != m_models.end()) {
        // Remove model if iterator found
        m_models.erase(iter);

        // Remove all resources from the model filepath
        removeResources(model->getFilePath());

        return true;
    }
    else {
        return false;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ResourceCache::removeResources(const QString & filepath)
{
    bool removed = false;
    std::unordered_map<QString, std::shared_ptr<ResourceHandle>>::iterator iter = m_resources.begin();
    while (iter != m_resources.end()) {
        if (iter->second->getPath() == filepath) {
            if (iter->second->isCoreResource()) continue;
            // Remove resource and delete handle
            remove(iter->second);
            iter = m_resources.erase(iter);
            removed = true;
        }
        else {
            // Continue to next resource
            iter++;
        }
    }

    // Remove resources from list of recently used resources
    ResourceList::iterator litr = m_lru.begin();
    while (litr != m_lru.end()) {
        if ((*litr)->getPath() == filepath) {
            if ((*litr)->isCoreResource()) continue;
            litr = m_lru.erase(litr);
        }
        else {
            // Continue to next resource
            litr++;
        }
    }

    return removed;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ResourceCache::clearedRemovable() const
{
    return !std::any_of(m_lru.begin(), m_lru.end(), [](std::shared_ptr<ResourceHandle> r) {
        return r->getPriority() == ResourceHandle::kRemovable;
    });
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ResourceCache::insert(std::shared_ptr<ResourceHandle> resource)
{
    return insert(resource, nullptr);
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ResourceCache::insert(std::shared_ptr<ResourceHandle> resourceHandle, bool* clearedResources)
{
    // Lock in case adding from another thread
    QMutexLocker lock(&m_resourceMutex);

    // Add resourceHandle
    const QString& name = resourceHandle->getName();
    if (m_resources.find(name) != m_resources.end()) {
#ifdef DEBUG_MODE
        logInfo("Reloading resource: " + name);
#endif
        m_resources[name] = resourceHandle;
        update(resourceHandle);
    }
    else {
        Map::Emplace(m_resources, name, resourceHandle);
        m_lru.push_front(resourceHandle);
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
 std::shared_ptr<ResourceHandle> ResourceCache::handleWithName(const QString & name, Resource::ResourceType type)
 {
     auto resourceIter = std::find_if(m_resources.begin(), m_resources.end(),
         [&](const std::pair<QString, std::shared_ptr<ResourceHandle>>& r) {
         return r.first == name && r.second->getType() == type;
     });
     if (resourceIter != m_resources.end()) {
         return resourceIter->second;
     }
     else {
         return nullptr;
     }
 }
 /////////////////////////////////////////////////////////////////////////////////////////////
 std::shared_ptr<ResourceHandle> ResourceCache::handleWithFilepath(const QString & filepath, Resource::ResourceType type)
 {
     auto resourceIter = std::find_if(m_resources.begin(), m_resources.end(),
         [&](const std::pair<QString, std::shared_ptr<ResourceHandle>>& r) {
         return r.second->getPath() == filepath && r.second->getType() == type;
     });
     if (resourceIter != m_resources.end()) {
         return resourceIter->second;
     }
     else {
         return nullptr;
     }
 }
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceCache::clear()
{
    QMutexLocker lock(&m_resourceMutex);

    // Start with a blank slate
    m_currentCost = 0;
    m_lru.clear();
    clearResources();
    m_materials.clear();
    clearModels();
    clearShaderPrograms();
    m_pythonScripts.clear();

}
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceCache::clearModels()
{
    m_models.clear();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceCache::clearShaderPrograms()
{ 
    // TODO: Formalize this
    // Keep "built-in" shaders
    QStringList builtIn = {
        "simple", 
        "text", 
        "basic",
        "lines", 
        "cubemap",
        "axes",
        "points"
    };

    for (auto it = m_shaderPrograms.begin(); it != m_shaderPrograms.end(); /*no increment*/) {
        if (!builtIn.contains(it->first))
        {
            m_shaderPrograms.erase(it++);    // or "it = m.erase(it)" since C++11
        }
        else
        {
            ++it;
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ShaderProgram> ResourceCache::getShaderProgramByName(const QString& name)
{
    if (m_shaderPrograms.find(name) != m_shaderPrograms.end()) {
        return m_shaderPrograms.at(name);
    }

    if (name == "basic"){
        auto basicProgram = std::make_shared<BasicShaderProgram>();
        m_shaderPrograms[name] = basicProgram;
        return m_shaderPrograms[name];
    }
    else if (name == "cubemap"){
        auto cubemapProgram = std::make_shared<CubemapShaderProgram>();
        m_shaderPrograms[name] = cubemapProgram;
        return m_shaderPrograms[name];
    }
    else {
#ifdef DEBUG_MODE
        throw("Error, no shader program found with the name " + name);
#endif
    }

    return nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ShaderProgram> ResourceCache::getShaderProgramByFilePath(const QString & fragpath, const QString & vertPath)
{
    auto shaderIter = std::find_if(m_shaderPrograms.begin(), m_shaderPrograms.end(), 
        [&](const std::pair<QString, std::shared_ptr<ShaderProgram>>& shaderPair) {
        return shaderPair.second->getFragShader()->getPath() == fragpath && 
            shaderPair.second->getVertShader()->getPath() == vertPath;
    }
    );

    if (shaderIter != m_shaderPrograms.end()) {
        // Found shader with matching filepaths, return
        return shaderIter->second;
    }
    else {
        // Did not find matching shader, so create
        QString name = FileReader::pathToName(fragpath, false);
        if (name == "basic") {
            auto basicProgram = std::make_shared<BasicShaderProgram>();
            m_shaderPrograms[name] = basicProgram;
            return m_shaderPrograms[name];
        }
        else if (name == "cubemap") {
            auto cubemapProgram = std::make_shared<CubemapShaderProgram>();
            m_shaderPrograms[name] = cubemapProgram;
            return m_shaderPrograms[name];
        }
        else {
            auto customProgram = std::make_shared<ShaderProgram>(vertPath, fragpath);
            m_shaderPrograms[customProgram->getName()] = customProgram;
            return m_shaderPrograms[name];
        }
    }

    // Should never be reached
    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Material> ResourceCache::getMaterial(const QString & name, bool create)
{
    // Check if material is in the map 
    std::unordered_map<QString, std::shared_ptr<Material>>::iterator iM = m_materials.find(name.toLower());
    if (iM != m_materials.end()) {
        return iM->second;
    }

    if (create) {
        // Create new material if not in map
        auto mat = std::make_shared<Material>(m_engine, name);
        Map::Emplace(m_materials, name.toLower(), mat);
        return mat;
    }
    else {
        return nullptr;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Material> ResourceCache::createMaterial(const QJsonValue & json)
{
    auto material = std::make_shared<Material>(m_engine, json);
    QString mtlName = material->getName().toLower();
    m_materials.emplace(mtlName, material);
    return material;
}
/////////////////////////////////////////////////////////////////////////////////////////////
//std::shared_ptr<Material> ResourceCache::getMaterialByFilePath(const QString & filepath)
//{
//    auto materialIter = std::find_if(m_materials.begin(), m_materials.end(),
//        [&](const std::pair<QString, std::shared_ptr<Material>>& mtlPair) {
//            return mtlPair.second->getPath() == filepath;
//    });
//
//    if (materialIter != m_materials.end()) {
//        // Return material if it exists
//        return materialIter->second;
//    }
//
//    // TODO: Implement material retrieval
//
//    return nullptr;
//}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceCache::addMaterial(std::shared_ptr<Material> material)
{
    if (m_materials.find(material->getName().toLower()) != m_materials.end()) {
#ifdef DEBUG_MODE
        throw("Error, material already exists in resource cache" + material->getName());
#endif
        logError("Material already exists in resource cache: " + material->getName());
    }
    Map::Emplace(m_materials, material->getName().toLower(), material);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<CubeMap> ResourceCache::getCubemap(const QString & name)
{
    auto iter = m_models.find(name.toLower());
    if (iter != m_models.end()) {
        if (iter->second->getModelType() != Model::kCubeMap) {
            throw("Error, model found is not a cubemap");
        }
        else {
            return std::static_pointer_cast<CubeMap>(iter->second);
        }
    }

    return nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<CubeMap> ResourceCache::createCubemap(const QString & name, const QString & filepath)
{
    // Create cubemap
    auto cubeTexture = m_engine->resourceCache()->getCubeTexture(filepath);
    auto cubemap = std::make_shared<CubeMap>(m_engine, name, cubeTexture);
    m_models.emplace(name.toLower(), cubemap);

    return cubemap;
}
///////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ResourceCache::getAnimationByName(const QString & name)
{
    // Return animation if it exists
    return getResourceHandleByName(name.toLower(), Resource::kAnimation);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceCache::addModel(std::shared_ptr<Model> model)
{
    // Throw error if model exists with the same name
    QString key = model->getName().toLower();
    if (m_models.find(key) != m_models.end()) {
        throw("Error, model exists with the name " + model->getName());
    }
    m_models[key] = model;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Model> ResourceCache::getModel(const QString& name)
{
    // Return model if it exists
    if (m_models.find(name.toLower()) != m_models.end()) {
        return m_models.at(name.toLower());
    }

    // Create model if it is a polygon
    std::shared_ptr<ResourceHandle> meshHandle;
    if (PolygonCache::isPolygonName(name)) {
        meshHandle = m_polygonCache->getPolygon(name);
        auto model = std::make_shared<Model>(name, meshHandle);
        m_models.emplace(name.toLower(), model);
        return m_models[name.toLower()];
    }
    else {
        return nullptr;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Model> ResourceCache::getModelByFilePath(const QString & filepath)
{
    // Search for model by filepath
    auto modelIter = std::find_if(m_models.begin(), m_models.end(),
        [&](const std::pair<QString, std::shared_ptr<Model>>& modelPair) {
        if (!modelPair.second->isFromFile()) {
            return false;
        }
        else {
            return modelPair.second->getFilePath() == filepath;
        }
    });

    if (modelIter != m_models.end()) {
        // Return model if it exists
        return modelIter->second;
    }

    // Create model if it does not exist
    QString name = FileReader::pathToName(filepath);
    std::shared_ptr<ResourceHandle> meshHandle = getMesh(filepath);
    auto model = std::make_shared<Model>(name, meshHandle);
    m_models.emplace(name.toLower(), model);

    return model;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Model> ResourceCache::createModel(const QJsonValue & json)
{
    std::shared_ptr<Model> model = Model::create(m_engine, json);
    m_models.emplace(model->getName().toLower(), model);
    return model;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<PythonClassScript> ResourceCache::getScript(const QString & filepath)
{
    // Return if script already added
    if (m_pythonScripts.find(filepath) != m_pythonScripts.end()) {
        return m_pythonScripts.at(filepath);
    }

    // Check if filepath exists
    if (FileReader::fileExists(filepath)) {
        auto script = std::make_shared<PythonClassScript>(m_engine, filepath);
        m_pythonScripts.emplace(filepath, script);
        return script;
    }
    else {
#ifdef DEBUG_MODE
        logError("Failed to load file at path " + filepath);
        throw("Failed to load file");
#endif
    }
    return nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ResourceCache::getMesh(const QString & nameOrFilepath)
{
    // String can be name of resource file or name of resource
    if (PolygonCache::isPolygonName(nameOrFilepath)) {
        return getResourceHandleByName(nameOrFilepath, Resource::kMesh);
    }
    else if (handleWithName(nameOrFilepath, Resource::kMesh)) {
        // If there is a mesh with this string as a name, find
        return getResourceHandleByName(nameOrFilepath, Resource::kMesh);
    }
    else {
        // Default to find via filepath
        return getResourceHandleByFilePath(nameOrFilepath, Resource::kMesh);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ResourceCache::getTexture(const QString & filePath, int textureType)
{
    ResourceAttributes attr;
    attr.m_attributes.emplace("type", textureType);
    return getResourceHandleByFilePath(filePath,
        Resource::kTexture,
        ResourceHandle::kRemovable,
        attr);
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ResourceCache::getCubeTexture(const QString & filePath)
{
    return getResourceHandleByFilePath(filePath, Resource::kCubeTexture);
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ResourceCache::getImage(
    const QString & filePath, 
    QImage::Format format)
{
    // Create resource attributes
    ResourceAttributes attributes;
    if (format != QImage::Format_Invalid) {
        attributes.m_attributes["format"] = GVariant(format);
    }

    return getResourceHandleByFilePath(filePath,
        Resource::kImage,
        ResourceHandle::kRemovable,
        attributes);
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ResourceCache::getResourceHandle(const Uuid & uuid)
{
    auto iter = std::find_if(m_resources.begin(), m_resources.end(),
        [&](const std::pair<QString, std::shared_ptr<ResourceHandle>>& resourcePair) {
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
std::shared_ptr<ResourceHandle> ResourceCache::getResourceHandle(const QJsonValue & json)
{
    const QJsonObject& object = json.toObject();
    QString name = object["name"].toString();
    QString path = object["path"].toString();
    Resource::ResourceType type = Resource::ResourceType(object["type"].toInt());
    ResourceHandle::Priority priority = ResourceHandle::Priority(object["priority"].toInt());
    ResourceAttributes attributes = ResourceAttributes(object["attributes"]);

    std::shared_ptr<ResourceHandle> handle;
#ifdef DEBUG_MODE
    if (name == "" && path == "") {
        throw("Error, no name or filepath specified");
        return nullptr;
    }
#endif

    switch (type) {
    case Resource::kAnimation:
        handle = getResourceHandleByName(name, type, priority, attributes);
        break;
    default:
        if (path == "") {
            handle = getResourceHandleByName(name, type, priority, attributes);
        }
        else {
            handle = getResourceHandleByFilePath(path, type, priority, attributes);
        }
    }

    return handle;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceCache::clearResources()
{
    //m_resources.clear();
    auto endIter = m_resources.end();
    for (auto it = m_resources.begin(); it != endIter;) {
        if (!it->second->isCoreResource())
        {
            // Erase resource if not a core resource
            m_resources.erase(it++);    // or "it = m.erase(it)" since C++11
        }
        else {
            // Skip if is a core resource
            it++;
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ResourceCache::getResourceHandleByFilePath(const QString & filepath,
    Resource::ResourceType type,
    ResourceHandle::Priority priority,
    const ResourceAttributes& resourceAttributes)
{
    // Check if handle is in the cache 
    std::shared_ptr<ResourceHandle> handle;
    if (handleWithFilepath(filepath, type)) {
        handle = handleWithFilepath(filepath, type);
    }

    // Check if handle needs to be reloaded
    if (!resourceAttributes.isEmpty() || !handle) {

        // Create handle to add resource object to cache if not found
        if (!handle) {
            handle = std::make_shared<ResourceHandle>(m_engine, filepath, type, priority);

            // Insert handle into map so that a duplicate doesn't get generated
            insert(handle);
        }
        else {
            // Need to set priority if reloading resource, in case has changed
            handle->setPriority(priority);
        }
        auto loadResourceProcess = std::make_shared<LoadProcess>(m_engine,
            m_processManager,
            handle,
            resourceAttributes);
        m_processManager->attachProcess(loadResourceProcess);

    }

    return handle;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ResourceCache::getResourceHandleByName(const QString & name,
    Resource::ResourceType type,
    ResourceHandle::Priority priority,
    const ResourceAttributes& resourceAttributes)
{
    // Check if handle is in the cache 
    std::shared_ptr<ResourceHandle> handle;
    if (handleWithName(name, type)) {
        handle = handleWithName(name, type);
    }

    // Check if handle needs to be reloaded
    if (!resourceAttributes.isEmpty() || !handle) {
        // Special case for generated meshes without filepaths
        if (type == Resource::kMesh) {
            if (PolygonCache::isPolygonName(name)) {
                return m_polygonCache->getPolygon(name);
            }
        }


        // Create handle to add resource object to cache if not found
        if (!handle) {
            handle = std::make_shared<ResourceHandle>(m_engine, "", name, type, priority);

            // Insert handle into map so that a duplicate doesn't get generated
            insert(handle);
        }
        else {
            // Need to set priority if reloading resource, in case it has changed
            handle->setPriority(priority);
        }

    }

    return handle;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceCache::update(std::shared_ptr<ResourceHandle> resource)
{
    auto iter = std::find_if(m_lru.begin(), m_lru.end(),
        [&](const std::shared_ptr<ResourceHandle>& r) {
        return resource->getUuid() == r->getUuid();
    });
    if (iter != m_lru.end()) {
        m_lru.erase(iter);
    }
    else {
#ifdef DEBUG_MODE
        logError("Error, resource with UUID not found in lru");
#endif
    }
    m_lru.push_front(resource);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceCache::reloadResource(const Uuid& uuid)
{
    std::shared_ptr<ResourceHandle> handle = getResourceHandle(uuid);
    reloadResource(handle);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceCache::reloadResource(std::shared_ptr<ResourceHandle> handle)
{
    // TODO: Add a resize attempt count to avoid infinitely trying to resize resource on error
    if (handle->resource(false)) {
#ifdef DEBUG_MODE
        logWarning("Warning, tried to resize an already loaded resource");
#endif
        return;
    }

#ifdef DEBUG_MODE
        logInfo("Loading resource that went out of scope");
#endif    
    auto loadResourceProcess = std::make_shared<LoadProcess>(m_engine,
        m_processManager,
        handle,
        handle->getAttributes());
    m_processManager->attachProcess(loadResourceProcess);
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ResourceCache::remove(std::shared_ptr<ResourceHandle> resourceHandle)
{
    // Lock in case adding from another thread
    QMutexLocker lock(&m_resourceMutex);
    //QMutexLocker lock2(&resourceHandle->mutex());

    if (resourceHandle->getPriority() == ResourceHandle::kPermanent) {
        // Move resource to the front of the queue if high priority
        update(resourceHandle);
#ifdef DEBUG_MODE
        logInfo("Did not remove resource due to priority");
        logCurrentCost();
#endif
        return false;
    }

    // Don't remove from structures, just call removal method for resource handle
    // Remove resource from structures
    //m_lru.remove(resource);
    //m_resources.erase(resource->getPath());

    // Reduce cost
    m_currentCost -= resourceHandle->resource(false)->getCost();

    // Removes resource
    resourceHandle->removeResource(false);

    if (m_currentCost < 0) {
        throw("Error, current cost cannot be less than zero");
    }

#ifdef DEBUG_MODE
    logInfo("Removed resource");
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
    logInfo("Current cost: " + QString::number(m_currentCost));
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceCache::runPostConstruction(std::shared_ptr<ResourceHandle> resourceHandle) {

    QMutexLocker(&resourceHandle->mutex());

    // Set OpenGL context to main context
    m_engine->setGLContext();

    resourceHandle->resource(false)->postConstruction();
#ifdef DEBUG_MODE
    logInfo("Running post-construction on thread: " + Process::getThreadID());
#endif

    // Emit signal that reloaded process (for widgets)
    emit m_engine->resourceCache()->resourceAdded(resourceHandle);
}

/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue ResourceCache::asJson() const
{
    QJsonObject object;

    // Save resources to json
    QJsonArray resources;
    for (const auto& resourceHandlePair : m_resources) {
        switch (resourceHandlePair.second->getType()) {
        case Resource::kMesh:
            // If mesh is a polygon, no need to cache
            if (PolygonCache::isPolygonName(resourceHandlePair.second->getName()))
                continue;
            else
                resources.append(resourceHandlePair.second->asJson());
        default:
            resources.append(resourceHandlePair.second->asJson());
            break;
        }
    }
    object.insert("resources", resources);

    // Save shaders to json
    QJsonArray shaders;
    for (const auto& shaderPair : m_shaderPrograms) {
        shaders.append(shaderPair.second->asJson());
    }
    object.insert("shaderPrograms", shaders);

    // Save models to json
    QJsonArray models;
    for (const auto& modelPair : m_models) {
        models.append(modelPair.second->asJson());
    }
    object.insert("models", models);

    // Save materials to json
    QJsonArray materials;
    for (const auto& materialPair : m_materials) {
        materials.append(materialPair.second->asJson());
    }
    object.insert("materials", materials);

    // Save python scripts to json
    QJsonArray scripts;
    for (const std::pair<QString, std::shared_ptr<PythonClassScript>>& scriptPair : m_pythonScripts) {
        // If script is base behavior, don't save to JSON
        if (scriptPair.second->getPath().contains("base_behavior.py")) {
            continue;
        }
        scripts.append(scriptPair.second->asJson());
    }
    object.insert("scripts", scripts);

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

#ifdef DEBUG_MODE    
    functions.printGLError("Error after setting context");
#endif

    const QJsonObject& object = json.toObject();

    // Load resources from json
    QJsonArray resources = object.value("resources").toArray();
    for (const auto& resourceJson : resources) {
        // Parse out JSON for resource
        QJsonObject resourceObject = resourceJson.toObject();
        Resource::ResourceType type = Resource::ResourceType(resourceObject["type"].toInt());
        QString filepath = resourceObject["path"].toString();
        ResourceHandle::Priority priority = ResourceHandle::Priority(resourceObject["priority"].toInt());
        ResourceAttributes attributes = ResourceAttributes(resourceObject["attributes"]);
        QString name = ResourceHandle::getNameFromJson(resourceObject);

#ifdef DEBUG_MODE    
        functions.printGLError("Error before resource creation");
#endif

        // Skip animations, there is no process for loading these
        switch (type) {
        case Resource::kAnimation:
            continue;
        }

        // Error checking for duplicate resources
        std::shared_ptr<ResourceHandle> handle = handleWithName(name, type);
        if (handle) {
            // If resource is a material, texture, or animation, 
            // may already have loaded in with mesh
            // TODO: See if this needs to be reverted
            //if (handle->getType() != Resource::kMaterial &&
            //    handle->getType() != Resource::kTexture &&
            //    handle->getType() != Resource::kAnimation &&
            //    !name.contains("gridcube")
            //    ) {
            //    // Throw error if duplicate resource is loaded
            //    throw("Error, filepath already in resources");
            //}
            //else {
            //    continue;
            //}
            continue;
        }

        // Determine whether to use resource name or filepath when searching for resource
        if (filepath.isEmpty()) {
            // Create resource handle
            std::shared_ptr<ResourceHandle> resourceHandle = getResourceHandleByName(name,
                type,
                priority,
                attributes);
        }
        else {
            // Create resource handle
            std::shared_ptr<ResourceHandle> resourceHandle = getResourceHandleByFilePath(filepath,
                type,
                priority,
                attributes);
        }

#ifdef DEBUG_MODE    
        functions.printGLError("Error on resource creation");
#endif
    }

    // Load materials from json
    const QJsonArray& materials = object.value("materials").toArray();
    for (const auto& materialJson : materials) {
        createMaterial(materialJson);
    }

    // Load shaders from json
    const QJsonArray& shaders = object.value("shaderPrograms").toArray();
    for (const auto& shaderJson : shaders) {
        auto shader = ShaderProgram::create(shaderJson);
        if (shader) {
            // Only add if successfully loaded
            m_shaderPrograms.emplace(shader->getName(), shader);
        }
    }

    // Load models from json
    const QJsonArray& models = object.value("models").toArray();
    for (const auto& modelJson : models) {
        // Create models from the JSON
        createModel(modelJson);
    }

    // Load scripts from json
    const QJsonObject& jsonObject = json.toObject();
    QString scriptPath;
#ifndef DEBUG_MODE
    scriptPath = ":scripts/base_behavior.py"
#else
    scriptPath = QFileInfo(QFile("py_scripts:base_behavior.py")).absoluteFilePath();
#endif
    m_engine->resourceCache()->getScript(scriptPath);
    if (jsonObject.contains("scripts")) {
        for (const auto& scriptJson : jsonObject.value("scripts").toArray()) {
            auto script = std::make_shared<PythonClassScript>(m_engine, scriptJson);
            m_pythonScripts.emplace(script->getPath(), script);
        }
    }

    // Save max cost to json
    m_maxCost = object.value("maxCost").toInt();
}




/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces