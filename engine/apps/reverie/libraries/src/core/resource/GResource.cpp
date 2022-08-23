#include "core/resource/GResource.h"
#include "core/resource/GResourceCache.h"

#include <QDir>
#include <QList>
#include <QDirIterator>
#include <QMutexLocker>

#include "fortress/system/memory/GPointerTypes.h"
#include "fortress/system/path/GFile.h"
#include "fortress/layer/framework/GFlags.h"

#include "core/GCoreEngine.h"
#include "core/rendering/geometry/GMesh.h"
#include "core/processes/GLoadProcess.h"
#include "core/processes/GProcessManager.h"

#include "core/rendering/models/GModel.h"
#include "core/rendering/geometry/GSkeleton.h"
#include "core/rendering/materials/GCubeTexture.h"
#include "core/rendering/materials/GMaterial.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/sound/GAudioResource.h"
#include "core/scripting/GPythonScript.h"

namespace rev {



// Resource

const GString & Resource::ResourceTypeDirName(GResourceType type)
{
    if ((int)type < 0) {
        Logger::Throw("Error, invalid type");
    }
    return s_resourceDirNames[(size_t)type];
}

Resource::Resource() :
    m_cost(sizeof(Resource)) // default cost
{
}

Resource::~Resource()
{
}

bool Resource::loadBinary(const GString& filepath)
{
    return false;
}

bool Resource::saveBinary(const GString& filepath) const
{
    return false;
}

void Resource::postConstruction()
{
#ifdef DEBUG_MODE
    //Logger::LogInfo("Running post-construction routine for resource: " + m_name);

    //if (m_name.isEmpty()) Logger::Throw("Error, resource has no name");
#endif
}


//const std::shared_ptr<Resource> Resource::s_nullResource = nullptr;


std::array<GString, (size_t)EResourceType::eUserType + 1> Resource::s_resourceDirNames =
{ {
    "images",
    "textures",
    "materials",
    "meshes",
    "cube_textures",
    "animations",
    "models",
    "shaders",
    "scripts",
    "skeletons",
    "audio"
} };





// ResourceHandle

GString ResourceHandle::getNameFromJson(const json & json)
{
    GString filepath;
    json["path"].get_to(filepath);
    GResourceType type = GResourceType(json.at("type").get<Int32_t>());
    GString name;
    switch ((EResourceType)type) {
    case EResourceType::eAnimation:
        json["name"].get_to(name);
        break;
    default:
        if (filepath.isEmpty()) {
            json["name"].get_to(name);
        }
        else {
            GFile myFile(filepath);
            static constexpr bool s_keepExtension = true;
            static constexpr bool s_caseInsensitive = false;
            name = myFile.getFileName(s_keepExtension, s_caseInsensitive);
        }
    }
    return name;
}

std::shared_ptr<ResourceHandle> ResourceHandle::create(CoreEngine * engine, const json& korJson)
{
    GResourceType type = GResourceType(korJson.at("type").get<Int32_t>());

    // MUST set uuid before inserting into research cache, or else it will be indexed with the wrong UUID
    Uuid uuid = korJson["uuid"];

    auto handle = prot_make_shared<ResourceHandle>(engine, type);
    handle->m_uuid = uuid;

    ResourceCache::Instance().insertHandle(handle);
    korJson.get_to(*handle);

    return handle;
}

std::shared_ptr<ResourceHandle> ResourceHandle::create(CoreEngine * engine, GResourceType type)
{
    auto handle = prot_make_shared<ResourceHandle>(engine, type);
    ResourceCache::Instance().insertHandle(handle);
    return handle;
}

std::shared_ptr<ResourceHandle> ResourceHandle::create(CoreEngine * engine, GResourceType type, ResourceBehaviorFlags flags)
{
    auto handle = prot_make_shared<ResourceHandle>(engine, type);
    handle->setBehaviorFlags(flags);
    ResourceCache::Instance().insertHandle(handle);
    return handle;
}

std::shared_ptr<ResourceHandle> ResourceHandle::create(CoreEngine * engine, const GString & filepath, GResourceType type)
{
    auto handle = prot_make_shared<ResourceHandle>(engine, filepath, type);
    ResourceCache::Instance().insertHandle(handle);
    return handle;
}

std::shared_ptr<ResourceHandle> ResourceHandle::create(CoreEngine * engine, const GString & filepath, GResourceType type, ResourceBehaviorFlags flags)
{
    auto handle = prot_make_shared<ResourceHandle>(engine, filepath, type);
    handle->setBehaviorFlags(flags);
    ResourceCache::Instance().insertHandle(handle);
    return handle;
}

ResourceHandle::ResourceHandle(CoreEngine* engine):
    DistributedLoadableInterface(""),
    m_resource(nullptr),
    m_engine(engine)
{
}

ResourceHandle::ResourceHandle(CoreEngine * engine, GResourceType type) :
    m_type(type),
    m_resource(nullptr),
    m_engine(engine)
{
}

//ResourceHandle::ResourceHandle(CoreEngine* engine, const std::shared_ptr<Resource>& resource) :
//    NameableInterface(resource->getName(), kCaseInsensitive),
//    DistributedLoadableInterface(""),
//    m_type(resource->getResourceType()),
//    m_engine(engine)
//{
//    setResource(resource, false);
//}

ResourceHandle::ResourceHandle(CoreEngine* engine, const GString & filepath, GResourceType type) :
    NameableInterface(GFile(filepath).getFileName(true, false), kCaseInsensitive),
    DistributedLoadableInterface(filepath),
    m_type(type),
    m_resource(nullptr),
    m_engine(engine)
{
}

ResourceHandle::ResourceHandle(CoreEngine* engine, const GString& filepath, const GString& name, GResourceType type):
    NameableInterface(name),
    DistributedLoadableInterface(filepath),
    m_type(type),
    m_resource(nullptr),
    m_engine(engine)
{
}
void ResourceHandle::notifyForReload() const
{
    emit ResourceCache::Instance().resourceNeedsReload(m_uuid);
}

ResourceHandle::~ResourceHandle()
{
    // Remove all resources on destruction
    //unloadResource(false);
}

void ResourceHandle::setResource(std::unique_ptr<Resource> resource, bool lockMutex)
{
    if (lockMutex) {
        m_resourceMutex.lock();
    }

    if (resource->getResourceType() != m_type) {
        Logger::Throw("Error, resource added with the incorrect type");
    }

    m_resource = std::move(resource);
    m_resource->m_handle = this;

    if (lockMutex) {
        m_resourceMutex.unlock();
    }
}

void ResourceHandle::setChildPaths(const GString & filepath)
{
    setPath(filepath);
    for (const std::shared_ptr<ResourceHandle>& child : m_children) {
        child->setChildPaths(filepath);
    }
}

const std::shared_ptr<ResourceHandle>& ResourceHandle::getChild(const Uuid & uuid)
{
    auto iter = std::find_if(m_children.begin(), m_children.end(),
        [&](const auto& child) {
        return child->getUuid() == uuid;
    });
    if (iter != m_children.end()) {
        return *iter;
    }
    else {
        return s_nullHandle;
    }
}

const std::shared_ptr<ResourceHandle>& ResourceHandle::getChild(const GString & name, GResourceType type)
{
    for (size_t i = 0; i < m_children.size(); i++) {
        const auto& childHandle = m_children[i];
        if (childHandle->getResourceType() != type) {
            continue;
        }

        if (childHandle->getName() == name) {
            return childHandle;
        }
    }
    
    // No matching child found
    return s_nullHandle;
}

void ResourceHandle::getChildren(GResourceType type, std::vector<std::shared_ptr<ResourceHandle>>& outChildren)
{
    outChildren.reserve(m_children.size());
    for (const auto& child : m_children) {
        if (child->getResourceType() == type)
            outChildren.push_back(child);
    }
}

void ResourceHandle::addChild(const std::shared_ptr<ResourceHandle>& child)
{
    auto childIter = std::find_if(m_children.begin(), m_children.end(),
        [&](const std::shared_ptr<ResourceHandle>& c) {
        return c->getUuid() == child->getUuid();
    });
    if (childIter != m_children.end()) {
        Logger::Throw("Error, child already added to resource");
    }

    m_children.push_back(child);
    
    // Only set parent if this object is dependent on parent
    if (child->isChild()) {
        child->m_parent = this;
    }
}

void ResourceHandle::touch()
{
    // If resource is a child, not in LRU so no need to touch
    if (isChild()) return;

    std::deque<std::shared_ptr<ResourceHandle>>& topLevelResources = ResourceCache::Instance().m_topLevelResources;
    auto iter = std::find_if(topLevelResources.begin(), topLevelResources.end(),
        [&](const std::shared_ptr<ResourceHandle>& r) {
        return m_uuid == r->getUuid();
    });
    std::shared_ptr<ResourceHandle> thisShared;
    if (iter != topLevelResources.end()) {
        thisShared = *iter;
        topLevelResources.erase(iter);
    }
    else {
#ifdef DEBUG_MODE
        Logger::LogError("Error, resource with UUID not found in lru");
#endif
        std::vector<GString> resourceNames;
        for (const auto& handle : topLevelResources) {
            resourceNames.push_back(handle->getName());
        }
        Logger::Throw("Error, resource with UUID not found in lru");
    }
    topLevelResources.push_front(thisShared);
}

void ResourceHandle::loadResource(bool serialLoad)
{
    if (isLoading()) {
        Logger::Throw("Error, attempting to load a resource that is currently loading");
        return;
    }

    if (isConstructed()) {
        Logger::Throw("Error, resource is already constructed");
        return;
    }

    if (m_resource) {
        Logger::Throw("Error, resource is already loaded, and construction flag failed to capture this");
    }

#ifdef DEBUG_MODE
    Logger::LogInfo("Loading resource " + m_name);
#endif    

    // Increment count of loading resources and initialize load process
    if (!isChild()) {
        ResourceCache::Instance().incrementLoadCount();
    }
    else {
#ifdef DEBUG_MODE
        Logger::LogInfo("Not incrementing, is child");
#endif
    }

    auto handlePtr = sharedPtr();
    if (!handlePtr) {
        // FIXME: This happens sometimes
        Logger::Throw("Error, threading issue, handle not yet added to cache, fix this");
    }

    auto loadProcess = std::make_shared<LoadProcess>(sharedPtr());

    // Use type to determine whether or not to run on another thread
    switch ((EResourceType)m_type) {
    // Types can be initialized on other threads
    case EResourceType::eModel:
    case EResourceType::eTexture:
    case EResourceType::eMaterial:
    case EResourceType::eMesh:
    case EResourceType::eAnimation:
    case EResourceType::eCubeTexture:
    case EResourceType::eAudio:
        if (!serialLoad) {
            // Only start another process if flagged to do so
            ResourceCache::Instance().processManager()->threadedProcessQueue().attachProcess(loadProcess);
            break;
        }
    // Types that need to be initialized on main GUI thread
    case EResourceType::eShaderProgram:
    case EResourceType::ePythonScript:
        loadProcess->onInit(); // Force initialization on this thread
        ResourceCache::Instance().processManager()->threadedProcessQueue().attachProcess(loadProcess); // attach process
        break;
    case EResourceType::eSkeleton:
    default:
        Logger::Throw("Error, type not recognized");
    }

    // Catch exceptions (if on this thread)
    // Added since process manager won't check for exceptions if one has been raised on this thread
    {
        std::unique_lock lock(loadProcess->exceptionMutex());
        const std::exception_ptr& ex = loadProcess->exception();
        if (ex) {
            GString oErrStr;
            try { 
                std::rethrow_exception(ex);
            }
            catch (const std::exception& e) { 
                oErrStr = e.what();
            }
            catch (const std::string& e) {
                oErrStr = e.c_str();
            }
            catch (const char* e) {
                oErrStr = e;
            }
            catch (...) { 
                std::rethrow_exception(ex);
            }
        }
    }
}


void ResourceHandle::unloadResource(bool lockMutex)
{
    if (lockMutex) {
        m_resourceMutex.lock();
    }

    // Call resources pre-removal routine
    m_resource->onRemoval();

    // Removes resource
    m_resource = nullptr;

    // Check if there was an OpenGL error from unloadiing the resource
#ifdef DEBUG_MODE
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    bool error = gl.printGLError("Error unloading resource");
    if (error) {
        Logger::Throw("Error unloading resource");
    }
#endif

    if (lockMutex) {
        m_resourceMutex.unlock();
    }

    // Remove resources from children 
    if(!isRuntimeGenerated()){ // If user-generated, children are linked manually, and therefore not dependent on parent resource
        for (const auto& child : m_children) {
            child->unloadResource(lockMutex);

            // Emit signal for widgets to know that the handle's resource was deleted
            ResourceCache::Instance().m_resourceDeleted.emitForAll(child->getUuid());
        }
    }
}

void ResourceHandle::removeFromCache(bool removeFromTopLevel)
{
    // Remove from map of all resources
    size_t numErased; // Count to check whether erase was successful
    ResourceCache& cache = ResourceCache::Instance();
    numErased = cache.m_resources.erase(m_uuid);
    if (!numErased) {
#ifdef DEBUG_MODE
        Logger::Throw("Error, failed to erase from map of all resources");
#else
        Logger::LogError("Error, failed to erase from map of all resources");
#endif
    }

    // Remove from top level resources if not a child
    if (!isChild() && removeFromTopLevel) {
        auto iter = std::find_if(cache.m_topLevelResources.begin(), cache.m_topLevelResources.end(),
            [&](const auto& handle) {
            return handle->getUuid() == m_uuid;
        }
        );
        if (iter == cache.m_topLevelResources.end()) {
            Logger::Throw("Error, only top-level resources can be forcefully deleted");
        }
        cache.m_topLevelResources.erase(iter);
    }

    // Remove child resources from cache
    if (!isRuntimeGenerated()) { 
        // If user-generated, children are linked manually, and therefore not dependent on parent resource
        for (const auto& child : m_children) {
            child->removeFromCache(removeFromTopLevel);
        }
    }
}

void ResourceHandle::postConstruct(ResourceHandle* handle, int level)
{
    if (!handle) handle = this;

    if (handle->isConstructed()) {
        if(!handle->parent())
            Logger::Throw("Error, reconstructing an alreaday constructed resource");
        else {
            Logger::LogWarning("Skipped reconstruction of resource " + handle->getName());
            return;
        }
    }

    // Perform post-construction on children first
    for (const std::shared_ptr<ResourceHandle>& child : handle->children()) {
        // Only post-construct if child is actually dependent on this resource
        if (child->isChild()) {
            int lvl = level + 1;
            postConstruct(child.get(), lvl);
        }
    }

    // Post-construct resource
    handle->resource()->postConstruction();
    handle->setConstructed(true);

    // Decrement count of loading objects, making sure not to double decrement
    if (!handle->isChild()) {
        ResourceCache::Instance().decrementLoadCount();
    }

    // Emit signal that reloaded process (for widgets)
    std::shared_ptr<ResourceHandle> handlePtr = handle->sharedPtr();
    if (!handlePtr) {
        Logger::Throw("Error, somehow no handle found");
    }
	
	/// @fixme Causing slow startup with ResourceTreeWidget
    if (!handlePtr->isHidden()) {
        ResourceCache::Instance().m_resourceAdded.emitForAll(handlePtr->getUuid());
    }
}

void to_json(json& orJson, const ResourceHandle& korObject)
{
    ToJson<DistributedLoadableInterface>(orJson, korObject);
    orJson["name"] = korObject.m_name.c_str();
    orJson["type"] = int(korObject.m_type);
    orJson["behaviorFlags"] = int(korObject.m_behaviorFlags);
    orJson["uuid"] = korObject.m_uuid;

    /// @todo Remove comment. This check was removed because handle gets serialized when talking to widgets on resource deletion
    //if (!korObject.m_resource) {
    //    Logger::Throw("Error, resource not loaded at serialization time");
    //}

    if (korObject.usesJson()) {
        // Need to cache resource JSON if either user-generated or flagged to use JSON

        // If resource is not serializable, use most recently cached resource JSON
        if (!korObject.m_cachedResourceJson.empty()) {
            orJson["resourceJson"] = korObject.m_cachedResourceJson;
        }
        else
        {
            orJson["resourceJson"] = korObject.getResourceJson();
        }
    }
}

void from_json(const json& korJson, ResourceHandle& orObject)
{
    // Parse out JSON for resource
    FromJson<DistributedLoadableInterface>(korJson, orObject);
    GResourceType type = GResourceType(korJson.at("type").get<Int32_t>());
    orObject.setBehaviorFlags(ResourceBehaviorFlags(korJson.at("behaviorFlags").get<Int32_t>()));

    // Load attributes
    korJson["name"].get_to(orObject.m_name);
    orObject.m_type = type;
    Uuid uuid = korJson["uuid"];
    if (orObject.m_uuid.isNull()) {
        Logger::Throw("Error, UUID for resource handle is invalid");
    }
    if (orObject.m_uuid != uuid) {
        Logger::Throw("Error, UUID should have been set before loading handle, see ResourceHandle::create");
    }
    if (korJson.contains("resourceJson")) {
        orObject.m_cachedResourceJson = korJson["resourceJson"];
    }

    // Clean up resource
    if (orObject.m_resource) {
        orObject.unloadResource(true);
    }

    // Reload resource
    orObject.loadResource();
}

std::shared_ptr<ResourceHandle> ResourceHandle::sharedPtr()
{
    if (!m_parent)
        return ResourceCache::Instance().getHandle(getUuid());
    else
        return m_parent->getChild(m_uuid);
}

json ResourceHandle::asJson() const
{
    json myJson;
    to_json(myJson, *this);
    return myJson;
}

json ResourceHandle::getResourceJson() const
{
    json oJson;

    if (!m_resource) {
        return oJson;
    }

    /// @todo Auto-generate this
    // Use type to cast to json
    switch ((EResourceType)m_type) {
    case EResourceType::eModel:
        ToJson<Model>(oJson, *m_resource);
        break;
    case EResourceType::eTexture:
        ToJson<Texture>(oJson, *m_resource);
        break;
    case EResourceType::eMaterial:
        ToJson<Material>(oJson, *m_resource);
        break;
    case EResourceType::eMesh:
        ToJson<Mesh>(oJson, *m_resource);
        break;
    case EResourceType::eAnimation:
        ToJson<Animation>(oJson, *m_resource);
        break;
    case EResourceType::eCubeTexture:
        ToJson<CubeTexture>(oJson, *m_resource);
        break;
    case EResourceType::eAudio:
        ToJson<AudioResource>(oJson, *m_resource);
        break;
    case EResourceType::eShaderProgram:
        ToJson<ShaderProgram>(oJson, *m_resource);
        break;
    case EResourceType::ePythonScript:
        ToJson<PythonClassScript>(oJson, *m_resource);
        break;
    case EResourceType::eSkeleton:
        ToJson<Skeleton>(oJson, *m_resource);
        break;
    default:
        Logger::Throw("Error, type not recognized");
    }

    return oJson;
}

void ResourceHandle::loadResourceJson(const json& oJson)
{
    /// @todo Auto-generate this
    // Use type to cast to json
    switch ((EResourceType)m_type) {
    case EResourceType::eModel:
        FromJson<Model>(oJson, *m_resource);
        break;
    case EResourceType::eTexture:
        FromJson<Texture>(oJson, *m_resource);
        break;
    case EResourceType::eMaterial:
        FromJson<Material>(oJson, *m_resource);
        break;
    case EResourceType::eMesh:
        FromJson<Mesh>(oJson, *m_resource);
        break;
    case EResourceType::eAnimation:
        FromJson<Animation>(oJson, *m_resource);
        break;
    case EResourceType::eCubeTexture:
        FromJson<CubeTexture>(oJson, *m_resource);
        break;
    case EResourceType::eAudio:
        FromJson<AudioResource>(oJson, *m_resource);
        break;
    case EResourceType::eShaderProgram:
        FromJson<ShaderProgram>(oJson, *m_resource);
        break;
    case EResourceType::ePythonScript:
        FromJson<PythonClassScript>(oJson, *m_resource);
        break;
    case EResourceType::eSkeleton:
        FromJson<Skeleton>(oJson, *m_resource);
        break;
    default:
        Logger::Throw("Error, type not recognized");
    }
}

bool operator==(const ResourceHandle & r1, const ResourceHandle & r2)
{
    bool samePath = r1.getPath() == r2.getPath();
    bool sameName = r1.getName() == r2.getName();
    bool sameType = r1.getResourceType() == r2.getResourceType();
    bool sameResource = r1.m_resource == r2.m_resource;

    return samePath && sameType && sameName && sameResource;
}

std::shared_ptr<ResourceHandle> ResourceHandle::s_nullHandle = nullptr;




} // End namespaces