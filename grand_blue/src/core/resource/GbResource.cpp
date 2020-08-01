#include "GbResource.h"
#include "GbResourceCache.h"

#include <QDir>
#include <QList>
#include <QDirIterator>
#include <QMutexLocker>

#include "../containers/GbFlags.h"
#include "../GbCoreEngine.h"
#include "../rendering/geometry/GbMesh.h"
#include "../processes/GbLoadProcess.h"
#include "../processes/GbProcessManager.h"
#include "../utils/GbMemoryManager.h"

namespace Gb {


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Resource
/////////////////////////////////////////////////////////////////////////////////////////////
Resource::Resource(const QString & name, ResourceType type) :
    Object(name),
    m_type(type),
    m_cost(1) // default cost
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Resource::Resource(ResourceType type) :
    m_type(type),
    m_cost(1) // default cost
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Resource::isSerializable() const
{
    bool serializable = false;
    switch (m_type) {
    case kImage:
    case kTexture:
    case kMesh:
    case kCubeTexture:
    case kAnimation:
    case kSkeleton:
        break;
    case kMaterial:
    case kModel:
    case kShaderProgram:
    case kPythonScript:
        serializable = true;
        break;
    case kNullType:
    default:
        throw("Error, invalid resource type");
        break;
    }
    
    return serializable;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Resource::postConstruction()
{
#ifdef DEBUG_MODE
    logInfo("Running post-construction routine for resource: " + m_name);

    //if (m_name.isEmpty()) throw("Error, resource has no name");
#endif
}





/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// ResourceHandle
/////////////////////////////////////////////////////////////////////////////////////////////
QString ResourceHandle::getNameFromJson(const QJsonObject & json)
{
    QString filepath = json["path"].toString();
    Resource::ResourceType type = Resource::ResourceType(json["type"].toInt());
    QString name;
    switch (type) {
    case Resource::kAnimation:
        name = json["name"].toString();
        break;
    default:
        if (filepath.isEmpty()) {
            name = json["name"].toString();
        }
        else {
            name = FileReader::pathToName(filepath);
        }
    }
    return name;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ResourceHandle::create(CoreEngine * engine, Resource::ResourceType type)
{
    auto handle = prot_make_shared<ResourceHandle>(engine, type);
    engine->resourceCache()->insertHandle(handle);
    return handle;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ResourceHandle::create(CoreEngine * engine, Resource::ResourceType type, BehaviorFlags flags)
{
    auto handle = prot_make_shared<ResourceHandle>(engine, type);
    handle->setBehaviorFlags(flags);
    engine->resourceCache()->insertHandle(handle);
    return handle;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ResourceHandle::create(CoreEngine * engine, const QString & filepath, Resource::ResourceType type)
{
    auto handle = prot_make_shared<ResourceHandle>(engine, filepath, type);
    engine->resourceCache()->insertHandle(handle);
    return handle;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ResourceHandle::create(CoreEngine * engine, const QString & filepath, Resource::ResourceType type, BehaviorFlags flags)
{
    auto handle = prot_make_shared<ResourceHandle>(engine, filepath, type);
    handle->setBehaviorFlags(flags);
    engine->resourceCache()->insertHandle(handle);
    return handle;
}
/////////////////////////////////////////////////////////////////////////////////////////////
ResourceHandle::ResourceHandle(CoreEngine* engine):
    Object(),
    DistributedLoadable(""),
    m_resource(nullptr),
    m_engine(engine)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
ResourceHandle::ResourceHandle(CoreEngine * engine, Resource::ResourceType type) :
    Object(),
    m_type(type),
    m_resource(nullptr),
    m_engine(engine)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
ResourceHandle::ResourceHandle(CoreEngine* engine, const std::shared_ptr<Resource>& resource) :
    Object(resource->getName(), kCaseInsensitive),
    DistributedLoadable(""),
    m_type(resource->getResourceType()),
    m_engine(engine)
{
    setResource(resource, false);
}
/////////////////////////////////////////////////////////////////////////////////////////////
ResourceHandle::ResourceHandle(CoreEngine* engine, const QString & filepath, Resource::ResourceType type) :
    Object(FileReader::pathToName(filepath), kCaseInsensitive),
    DistributedLoadable(filepath),
    m_type(type),
    m_resource(nullptr),
    m_engine(engine)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
ResourceHandle::ResourceHandle(CoreEngine* engine, const QString& filepath, const QString& name, Resource::ResourceType type):
    Object(name),
    DistributedLoadable(filepath),
    m_type(type),
    m_resource(nullptr),
    m_engine(engine)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
ResourceHandle::~ResourceHandle()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
const std::shared_ptr<Resource>& ResourceHandle::resource(bool lockMutex)
{
    if (lockMutex) {
       m_resourceMutex.lock();
    }

#ifdef DEBUG_MODE
    if (m_resource) {
        if (m_resource->getResourceType() != m_type) {
            throw("Error, resource somehow has the incorrect type");
        }
    }
#endif

    if (needsReload()) {
        emit m_engine->resourceCache()->resourceNeedsReload(sharedPtr());
    }

    if (lockMutex) {
        m_resourceMutex.unlock();
    }
    return m_resource;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceHandle::setResource(const std::shared_ptr<Resource>& resource, bool lockMutex)
{
    if (lockMutex) {
        m_resourceMutex.lock();
    }

    if (resource->getResourceType() != m_type) {
        throw("Error, resource added with the incorrect type");
    }

    m_resource = resource;
    resource->m_handle = this;

    if (lockMutex) {
        m_resourceMutex.unlock();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ResourceHandle::getChild(const Uuid & uuid)
{
    auto iter = std::find_if(m_children.begin(), m_children.end(),
        [&](const auto& child) {
        return child->getUuid() == uuid;
    });
    if (iter != m_children.end()) {
        return *iter;
    }
    else {
        return nullptr;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceHandle::getChildren(Resource::ResourceType type, std::vector<std::shared_ptr<ResourceHandle>>& outChildren)
{
    outChildren.reserve(m_children.size());
    for (const auto& child : m_children) {
        if (child->getResourceType() == type)
            outChildren.push_back(child);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceHandle::addChild(const std::shared_ptr<ResourceHandle>& child)
{
    auto childIter = std::find_if(m_children.begin(), m_children.end(),
        [&](const std::shared_ptr<ResourceHandle>& c) {
        return c->getUuid() == child->getUuid();
    });
    if (childIter != m_children.end()) {
        throw("Error, child already added to resource");
    }

    m_children.push_back(child);
    
    // Only set parent if this object is dependent on parent
    if (child->isChild()) {
        child->m_parent = this;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceHandle::touch()
{
    // If resource is a child, not in LRU so no need to touch
    if (isChild()) return;

    std::list<std::shared_ptr<ResourceHandle>>& topLevelResources = m_engine->resourceCache()->m_topLevelResources;
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
        logError("Error, resource with UUID not found in lru");
#endif
        throw("Error, resource with UUID not found in lru");
    }
    topLevelResources.push_front(thisShared);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceHandle::loadResource()
{
    if (isLoading()) {
        throw("Error, attempting to load a resource that is currently loading");
        return;
    }

    if (isConstructed()) {
        throw("Error, resource is already constructed");
        return;
    }

    if (m_resource) {
        throw("Error, resource is already loaded, and construction flag failed to capture this");
    }

#ifdef DEBUG_MODE
    logInfo("Loading resource " + m_name);
#endif    

    // Increment count of loading resources and initialize load process
    if (!isChild()) {
        m_engine->resourceCache()->incrementLoadCount();
    }
    else {
#ifdef DEBUG_MODE
        logInfo("Not incrementing, is child");
#endif
    }

    auto loadProcess = std::make_shared<LoadProcess>(m_engine, sharedPtr());

    // Use type to determine whether or not to run on another thread
    switch (m_type) {
    case Resource::kModel:
    case Resource::kTexture:
    case Resource::kMaterial:
    case Resource::kImage:
    case Resource::kMesh:
    case Resource::kAnimation:
    case Resource::kCubeTexture:
        m_engine->resourceCache()->processManager()->attachProcess(loadProcess);
        break;
    case Resource::kShaderProgram:
    case Resource::kPythonScript:
        loadProcess->onInit(); // Force initialization on this thread
        m_engine->resourceCache()->processManager()->attachProcess(loadProcess); // attach process
        break;
    case Resource::kSkeleton:
    default:
        throw("Error, type not recognized");
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceHandle::removeResources(bool lockMutex)
{
    if (lockMutex) {
        m_resourceMutex.lock();
    }

    // Call resources pre-removal routine
    m_resource->onRemoval();

    // Removes resource
    m_resource = nullptr;

    if (lockMutex) {
        m_resourceMutex.unlock();
    }

    // Remove resources from children 
    if(!isUserGenerated()){ // If user-generated, children are linked manually, and therefore not dependent on parent resource
        for (const auto& child : m_children) {
            child->removeResources(lockMutex);

            // Emit signal for widgets to know that the handle was deleted
            emit m_engine->resourceCache()->resourceDeleted(child);
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceHandle::postConstruct(ResourceHandle* handle, int level)
{
    if (!handle) handle = this;

    if (handle->isConstructed()) {
        if(!handle->parent())
            throw("Error, reconstructing an alreaday constructed resource");
        else {
            logWarning("Skipped reconstruction of resource " + handle->getName());
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
    handle->resource(false)->postConstruction();
    handle->setConstructed(true);

    // Decrement count of loading objects, making sure not to double decrement
    if (!handle->isChild()) {
        m_engine->resourceCache()->decrementLoadCount();
    }

    // Emit signal that reloaded process (for widgets)
    std::shared_ptr<ResourceHandle> handlePtr = handle->sharedPtr();
    emit m_engine->resourceCache()->resourceAdded(handlePtr);
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue ResourceHandle::asJson() const
{
    QJsonObject object = DistributedLoadable::asJson().toObject();
    object.insert("name", m_name);
    object.insert("type", int(m_type));
    object.insert("behaviorFlags", int(m_behaviorFlags));
    object.insert("attributes", m_attributes.asJson());
    object.insert("uuid", m_uuid.asString());
    if (!m_resource) {
        throw("Error, resource not loaded at serialization time");
    }
    if (isUserGenerated()) {
        // Need to cache resource JSON if user-generated
        if (m_resource->isSerializable()) {
            // Polygons are not serializable, so don't cache
            object.insert("resourceJson", std::dynamic_pointer_cast<Serializable>(m_resource)->asJson());
        }
    }
    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////
void ResourceHandle::loadFromJson(const QJsonValue & json)
{
    // Parse out JSON for resource
    QJsonObject object = json.toObject();
    DistributedLoadable::loadFromJson(json);
    Resource::ResourceType type = Resource::ResourceType(object["type"].toInt());
    m_behaviorFlags = Flags::toFlags<BehaviorFlag>(object["behaviorFlags"].toInt());

    // Load attributes
    m_name = object["name"].toString();
    m_type = type;
    m_attributes = Dictionary(object["attributes"]);
    m_uuid = Uuid(object["uuid"].toString());
    if (m_uuid.isNull()) {
        throw("Error, UUID for resource handle is invalid");
    }
    if (object.contains("resourceJson")) {
        m_resourceJson = object["resourceJson"].toObject();
    }

    // Clean up resource
    if (m_resource) {
        removeResources(true);
    }

    // Reload resource
    loadResource();
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ResourceHandle::sharedPtr()
{
    if (!m_parent)
        return m_engine->resourceCache()->getHandle(getUuid());
    else
        return m_parent->getChild(m_uuid);
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool operator==(const ResourceHandle & r1, const ResourceHandle & r2)
{
    bool samePath = r1.getPath() == r2.getPath();
    bool sameName = r1.getName() == r2.getName();
    bool sameType = r1.getResourceType() == r2.getResourceType();
    bool sameResource = r1.m_resource == r2.m_resource;

    return samePath && sameType && sameName && sameResource;
}


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces