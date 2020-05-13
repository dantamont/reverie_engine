#include "GbResource.h"
#include "GbResourceCache.h"

#include <QDir>
#include <QList>
#include <QDirIterator>
#include <QMutexLocker>

#include "../readers/GbJsonReader.h"

#include "../GbCoreEngine.h"
#include "../events/GbEventManager.h"

#include "../rendering/geometry/GbMesh.h"

#include "../processes/GbLoadProcess.h"
#include "../processes/GbProcessManager.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Resource Attributes
/////////////////////////////////////////////////////////////////////////////////////////////
ResourceAttributes::ResourceAttributes(const QJsonValue & json)
{
    loadFromJson(json);
}
const GVariant & ResourceAttributes::at(const QString & name) const
{
    if (m_attributes.find(name) == m_attributes.end()) {
        return GVariant();
    }
    return m_attributes.at(name);
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue ResourceAttributes::asJson() const
{
    QVariantMap map = GVariant::toQVariantMap(m_attributes);
    QJsonObject object = QJsonObject::fromVariantMap(map);
    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceAttributes::loadFromJson(const QJsonValue & json)
{
    QString jsonStr = JsonReader::getJsonValueAsQString(json);
    QVariantMap map = json.toObject().toVariantMap();
    m_attributes = GVariant::toGVariantMap(map);
}


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Resource
/////////////////////////////////////////////////////////////////////////////////////////////
Resource::Resource(const QString & name, ResourceType type) :
    Object(name),
    m_type(type),
    m_cost(1), // default cost
    m_isConstructed(false)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Resource::Resource(ResourceType type) :
    m_type(type),
    m_cost(1), // default cost
    m_isConstructed(false)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Resource::postConstruction()
{
#ifdef DEBUG_MODE
    logInfo("Running post-construction routine");
#endif

    m_isConstructed = true;
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
ResourceHandle::ResourceHandle(CoreEngine* engine):
    Object(),
    Loadable(""),
    m_resource(nullptr),
    m_engine(engine),
    m_isLoading(false)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
ResourceHandle::ResourceHandle(CoreEngine* engine, const std::shared_ptr<Resource>& resource, Priority priority) :
    Object(resource->getName(), kCaseInsensitive),
    Loadable(""),
    m_priority(priority),
    m_type(resource->getType()),
    m_engine(engine),
    m_isLoading(false)
{
    setResource(resource, false);
}
/////////////////////////////////////////////////////////////////////////////////////////////
ResourceHandle::ResourceHandle(CoreEngine* engine, const QString & filepath, Resource::ResourceType type, Priority priority) :
    Object(FileReader::pathToName(filepath), kCaseInsensitive),
    Loadable(filepath),
    m_priority(priority),
    m_type(type),
    m_resource(nullptr),
    m_engine(engine),
    m_isLoading(false)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
ResourceHandle::ResourceHandle(CoreEngine* engine, const QString& filepath, const QString& name, Resource::ResourceType type, Priority priority):
    Object(name),
    Loadable(filepath),
    m_priority(priority),
    m_type(type),
    m_resource(nullptr),
    m_engine(engine),
    m_isLoading(false)
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
        if (m_resource->getType() != m_type) {
            throw("Error, resource somehow has the incorrect type");
        }
    }
#endif

    bool needsReload = !m_resource && !m_isLoading;
    if (needsReload) {
        emit m_engine->eventManager()->resourceNeedsReload(sharedPtr());
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

    if (resource->getType() != m_type) {
        throw("Error, resource added with the incorrect type");
    }

    m_resource = resource;

    if (lockMutex) {
        m_resourceMutex.unlock();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
void ResourceHandle::removeResource(bool lockMutex)
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
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue ResourceHandle::asJson() const
{
    // TODO: Generalize resource loading with a "resize" routine, so that
    // resources without paths can be reloaded
    QJsonObject object;
    object.insert("name", m_name);
    object.insert("type", int(m_type));
    object.insert("priority", int(m_priority));
    object.insert("path", m_path);
    object.insert("attributes", m_attributes.asJson());

    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////
void ResourceHandle::loadFromJson(const QJsonValue & json)
{
    // Parse out JSON for resource
    QJsonObject resourceObject = json.toObject();
    Resource::ResourceType type = Resource::ResourceType(resourceObject["type"].toInt());
    QString filepath = resourceObject["path"].toString();
    ResourceHandle::Priority priority = ResourceHandle::Priority(resourceObject["priority"].toInt());
    QString name;
    if (filepath.isEmpty() || resourceObject.contains("name")) {
        name = resourceObject["name"].toString();
    }
    else {
        name = FileReader::pathToName(filepath);
    }

    // Load attributes
    m_name = name;
    m_path = filepath;
    m_type = type;
    m_priority = priority;
    m_attributes = ResourceAttributes(resourceObject["attributes"]);

    // Clean up resource
    if (m_resource) {
        removeResource(true);
    }

    // Reload resource
    m_engine->resourceCache()->reloadResource(m_uuid);
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ResourceHandle::sharedPtr()
{
    return m_engine->resourceCache()->getResourceHandle(getUuid());
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool operator==(const ResourceHandle & r1, const ResourceHandle & r2)
{
    bool samePath = r1.getPath() == r2.getPath();
    bool sameName = r1.getName() == r2.getName();
    bool sameType = r1.getType() == r2.getType();
    bool sameResource = r1.m_resource == r2.m_resource;

    return samePath && sameType && sameName && sameResource;
}


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces