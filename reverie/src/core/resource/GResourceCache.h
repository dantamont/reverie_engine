#ifndef GB_RESOURCE_CACHE_H
#define GB_RESOURCE_CACHE_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Standard
//#include <list>
#include <deque>
#include <memory>
#include <map>
#include <functional>

// QT
#include <QString>
#include <QCache>
#include <QImage>
#include <QMutex>
#include <QMutexLocker>

// Internal
#include "../GManager.h"
#include "../rendering/geometry/GPolygon.h"
#include "GResource.h"

#include "../containers/GThreadedMap.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class Image;
class PythonClassScript;
class Texture;
class Material;
enum ShaderProgramType : int; 
class ShaderProgram;
class Mesh;
class Model;
class Shape;
class CubeMap;
class Animation;
class CoreEngine;
class ProcessManager;

/////////////////////////////////////////////////////////////////////////////////////////////
// Type Definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class ResourceCache
/// @brief Class representing a resource cache
/// @details Total cost represents available RAM (in MB)
class ResourceCache: public Manager, public Serializable{
    Q_OBJECT
public:
    typedef std::deque<std::shared_ptr<ResourceHandle>> ResourceList;
    typedef ThreadedMap<Uuid, std::shared_ptr<ResourceHandle>> ResourceMap;

    //--------------------------------------------------------------------------------------------
    /// @name Static/Enums
    /// @{

    //enum class CacheFlag {
    //    kIsLoading = 1 << 0 // Flag that resource cache is loading resources
    //};

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    ResourceCache(CoreEngine* core, ProcessManager* processManager, const size_t sizeInMb);
    ~ResourceCache();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    const ResourceList& topLevelResources() const { return m_topLevelResources; }

    QMutex& resourceMapMutex() { return s_resourceMutex; }

    /// @brief Process manager for pushing load requests
    ProcessManager* processManager() { return m_processManager; }

    /// @property Resources in the cache
    /// @note To ensure thread safety, manually lock the mutex for the map when accessing
    const ResourceMap& resources() const {
        return m_resources;
    }

    /// @property PolyconCache
    const std::shared_ptr<PolygonCache>& polygonCache() {
        return m_polygonCache;
    }

    /// @brief The max cost of the cache
    size_t getMaxCost() const { return m_maxCost; }
    void setMaxCost(size_t cost) { m_maxCost = cost; }

    /// @}
	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Whether or not the resource cache is currently loading any resources
    bool isLoadingResources() const { return m_loadCount > 0; }

    /// @brief Increment count of loading resources
    void incrementLoadCount();
    void decrementLoadCount();

    /// @brief Return true if all removable resources have been removed
    bool clearedRemovable() const;

    /// @brief Insert a resource into the cache
    /// @details Returns true if successful insert
    bool insertHandle(const std::shared_ptr<ResourceHandle>& resource);
    bool insertHandle(const std::shared_ptr<ResourceHandle>& resource, bool* clearedResources);

    /// @brief Clears all resources
    void clear();

    /// @brief Return resource handle by UUID
    std::shared_ptr<ResourceHandle> getHandle(const Uuid& uuid) const;

    /// @brief Obtain handle corresponding to the given JSON, or create if it does not exist
    std::shared_ptr<ResourceHandle> getHandle(const QJsonValue& handleJson) const;

    /// @brief Return a handle given it's name and type
    std::shared_ptr<ResourceHandle> getHandleWithName(const GString& name, ResourceType type) const;

    /// @brief This routine will only return top-level handles
    std::shared_ptr<ResourceHandle> getTopLevelHandleWithPath(const GString& filepath) const;

    /// @brief Retrieve or create a top-level handle
    std::shared_ptr<ResourceHandle> guaranteeHandleWithPath(const GString& filepath,
        ResourceType type,
        ResourceBehaviorFlags flags=0);
    std::shared_ptr<ResourceHandle> guaranteeHandleWithPath(const std::vector<GString>& filepaths,
        ResourceType type,
        ResourceBehaviorFlags flags=0);

    virtual void postConstruction() override;

    /// @brief Delete the given resource
    bool remove(const std::shared_ptr<ResourceHandle>& resourceHandle, ResourceDeleteFlags deleteFlags = 0);
    bool remove(ResourceHandle* resourceHandle, ResourceDeleteFlags deleteFlags = 0);

	/// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "ResourceCache"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "rev::ResourceCache"; }

    /// @}

signals:
    void resourceChanged(const Uuid& handle);
    void resourceAdded(const Uuid&  handle);
    void resourceDeleted(const Uuid&  handle);

    void doneLoadingResources(); // Done loading all resources
    void startedLoadingResources(); // Started loading resource when none were loading
    void doneLoadingResource(const Uuid&  resourceHandle);
    void resourceNeedsReload(const Uuid&  resourceHandle);

public slots:

    /// @brief Run the post-construction method of the given resource handle
    void runPostConstruction(const Uuid& handleId);

    /// @brief Reload a resource handle's resource
    //void reloadResource(const Uuid& handleUuid);
    //void reloadResource(std::shared_ptr<ResourceHandle> handle);

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class ResourceHandle;

    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Clear all resources that aren't core resources
    //void clearResources(ResourceMap& map);
    void clearResources();

    /// @brief Returns the oldest object
    const std::shared_ptr<ResourceHandle>& oldestResource() const {
        return m_topLevelResources.back();
    }

    /// @brief Return max memory allowed by the system, in megabytes
    static qint64 getMaxMemoryMb();

    /// @brief Log current cost
    void logCurrentCost() const;

    void initializeCoreResources();

    /// @brief Recursive routine for constructing a resource
    //void postConstructResource(const std::shared_ptr<ResourceHandle>& handle);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Mutex for managing resource map and list
    static QMutex s_resourceMutex;

    /// @brief Mutex for managing count of loaded objects
    QMutex m_loadCountMutex;

    /// @brief Top-level resources, sorted by most recently used
    ResourceList m_topLevelResources;

    /// @brief Map of all resources for easy access
    /// @details Indexed by UUID
    /// @note Contains all resources
    ResourceMap m_resources;

    /// @brief Cache dedicated to storing generated polygons
    std::shared_ptr<PolygonCache> m_polygonCache;

    /// @brief Max allowed cost in the cache
    size_t m_maxCost;

    /// @brief Current cost in the cache
    size_t m_currentCost;

    ///// @brief whether or not the cache is currently loading an object
    //QFlags<CacheFlag> m_cacheFlags;

    /// @brief count of currently loading objects
    size_t m_loadCount = 0;

    /// @brief Process manager for pushing load requests
    ProcessManager* m_processManager;

    /// @}

};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif