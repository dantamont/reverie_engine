#pragma once

// Standard
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
#include "core/GManager.h"
#include "core/rendering/geometry/GPolygon.h"
#include "GResourceHandle.h"

#include "fortress/layer/framework/GSignalSlot.h"
#include "fortress/containers/concurrent/GThreadedMap.h"
#include "fortress/layer/framework/GSingleton.h"

namespace rev {

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

namespace gl {
    class VertexArrayObject;
};

/// @class ResourceCache
/// @brief Class representing a resource cache
/// @details Total cost represents available RAM (in MB)
class ResourceCache: public Manager, public SingletonInterface<ResourceCache, false/*CreateOnInstance*/>{
    Q_OBJECT
public:
    typedef std::deque<std::shared_ptr<ResourceHandle>> ResourceList;
    typedef ThreadedMap<Uuid, std::shared_ptr<ResourceHandle>> ResourceMap;

	/// @name Constructors/Destructor
	/// @{

    ~ResourceCache();

	/// @}

    /// @name Public Methods
    /// @{

    const ResourceList& topLevelResources() const { return m_topLevelResources; }

    std::mutex& resourceMapMutex() { return m_resourceMutex; }

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

    /// @brief Add data to post construct a resource
    void addPostConstructionData(const Uuid& handleId, const ResourcePostConstructionData& data);

    /// @brief Update resource cache with post-construction calls
    void postConstructResources();

    /// @brief Whether or not the resource cache is currently loading any resources
    bool isLoadingResources() const { return m_loadCount.load() > 0; }

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
    std::shared_ptr<ResourceHandle> getHandle(const json& handleJson) const;

    /// @brief Return a handle given it's name and type
    std::shared_ptr<ResourceHandle> getHandleWithName(const GString& name, GResourceType type) const;

    /// @brief This routine will only return top-level handles
    std::shared_ptr<ResourceHandle> getTopLevelHandleWithPath(const GString& filepath) const;

    /// @brief Retrieve or create a top-level handle
    std::shared_ptr<ResourceHandle> guaranteeHandleWithPath(const GString& filepath,
        GResourceType type,
        ResourceBehaviorFlags flags=0);
    std::shared_ptr<ResourceHandle> guaranteeHandleWithPath(const std::vector<GString>& filepaths,
        GResourceType type,
        ResourceBehaviorFlags flags=0);

    virtual void postConstruction() override;

    /// @brief Delete the given resource
    bool remove(const std::shared_ptr<ResourceHandle>& resourceHandle, ResourceDeleteFlags deleteFlags = 0);
    bool remove(ResourceHandle* resourceHandle, ResourceDeleteFlags deleteFlags = 0);

	/// @}

    /// @name Operators
    /// @{

    ResourceCache& operator=(const ResourceCache& other) = delete;

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const ResourceCache& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, ResourceCache& orObject);


    /// @}

    Signal<Uuid> m_resourceAdded; ///< Signal that a resource was added
    Signal<Uuid> m_resourceChanged; ///< Signal that a resource was modified
    Signal<Uuid> m_resourceDeleted; ///< Signal that a resource was deleted

signals:
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
    /// @name Friends
    /// @{

    friend class ResourceHandle;

    /// @}

    /// @name Protected Methods
    /// @{

    /// @brief Protected destructor since this is a singleton
    ResourceCache(CoreEngine* core, ProcessManager* processManager, const size_t sizeInMb);
    ResourceCache(const ResourceCache& cache) = delete;

    /// @brief Clear all resources that aren't core resources
    //void clearResources(ResourceMap& map);
    void clearResources();

    /// @brief Returns the oldest object
    const std::shared_ptr<ResourceHandle>& oldestResource() const {
        return m_topLevelResources.back();
    }

    /// @brief Log current cost
    void logCurrentCost() const;

    void initializeCoreResources();

    /// @brief Queue post construction of a resource
    void queuePostConstruction(const Uuid& uuid);

    /// @}

    /// @name Protected Members
    /// @{

    std::vector<Uuid> m_resourcesForPostConstruction; ///< Resources to post-construct next frame
    std::mutex m_postConstructMutex;
    std::mutex m_resourceMutex; ///< Mutex for managing resource map and list
    ResourceList m_topLevelResources; ///< Top-level resources, sorted by most recently used
    ResourceMap m_resources; ///< Map of all resources for easy access, indexed by UUID
    std::shared_ptr<PolygonCache> m_polygonCache; ///< Cache dedicated to storing generated polygons
    tsl::robin_map<Uuid, ResourcePostConstructionData> m_resourcePostConstructionData; ///< Data to post-construct resources on the main thread
    size_t m_maxCost; ///< Max allowed cost in the cache
    size_t m_currentCost; ///< Current cost in the cache
    std::atomic<uint32_t> m_loadCount = 0; ///< count of currently loading objects
    ProcessManager* m_processManager; ///< Process manager for pushing load requests

    //gl::VertexArrayObject* m_globalMeshVao; ///< VAO storing the indices and vertex data for ALL meshes in the scene
    //MeshVertexAttributes m_meshVertexData; ///< Vertex data for all loaded meshes
    /// @}
};

} // End namespaces
