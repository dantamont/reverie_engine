#ifndef GB_RESOURCE_CACHE_H
#define GB_RESOURCE_CACHE_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Standard
#include <list>
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
#include "../GbManager.h"
#include "../rendering/geometry/GbPolygon.h"
#include "GbResource.h"

namespace Gb {

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
    typedef std::list<std::shared_ptr<ResourceHandle>> ResourceList;
    typedef std::unordered_map<QString, std::shared_ptr<ResourceHandle>> ResourceMap;

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    ResourceCache(CoreEngine* core, ProcessManager* processManager, const size_t sizeInMb);
    ~ResourceCache();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief Process manager for pushing load requests
    ProcessManager* processManager() { return m_processManager; }

    /// @brief Map of all shader programs, since shader programs are not resources
    std::unordered_map<QString, std::shared_ptr<ShaderProgram>>& shaderPrograms() { return m_shaderPrograms; }

    /// @brief Map of all materials, since materials are not resources
    std::unordered_map<QString, std::shared_ptr<Material>>& materials() { return m_materials; }

    /// @brief Map of all models, since models are not resources
    std::unordered_map<QString, std::shared_ptr<Model>>& models() { return m_models; }

    /// @property resourceCache
    const ResourceMap& resources() {
        return m_resources;
    }

    /// @property PolyconCache
    std::shared_ptr<PolygonCache> polygonCache() {
        return m_polygonCache;
    }

    /// @brief The max cost of the cache
    size_t getMaxCost() const { return m_maxCost; }
    void setMaxCost(size_t cost) { m_maxCost = cost; }

    /// @}
	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Remove shader program from resource cache and all renderers
    bool removeShaderProgram(const std::shared_ptr<ShaderProgram>& shader);

    /// @brief Remove material from resource cache
    bool removeMaterial(const QString& name);
    bool removeMaterial(const Uuid& uuid);
    bool removeMaterial(std::shared_ptr<Material> material);

    /// @brief Remove model from resource cache
    bool removeModel(const QString& name);
    bool removeModel(std::shared_ptr<Model> model);

    /// @brief Remove resurces with the given filepath
    bool removeResources(const QString& filepath);

    /// @brief Return true if all removable resources have been removed
    bool clearedRemovable() const;

    /// @brief Insert a resource into the cache
    /// @details Returns true if successful insert
    bool insert(std::shared_ptr<ResourceHandle> resource);
    bool insert(std::shared_ptr<ResourceHandle> resource, bool* clearedResources);

    /// @brief Whether the resource map has the specified resource
    std::shared_ptr<ResourceHandle> handleWithName(const QString& name, Resource::ResourceType type);
    std::shared_ptr<ResourceHandle> handleWithFilepath(const QString& filepath, Resource::ResourceType type);

    /// @brief Whether the resource map has the specified model
    inline bool hasShaderProgram(const QString& name) {
        return m_shaderPrograms.find(name.toLower()) != m_shaderPrograms.end();
    }

    /// @brief Whether the resource map has the specified model
    inline bool hasModel(const QString& name) {
        return m_models.find(name.toLower()) != m_models.end();
    }

    /// @brief Whether the resource map has the specified material
    inline bool hasMaterial(const QString& name) {
        return m_materials.find(name.toLower()) != m_materials.end();
    }

    /// @brief Clears all resources
    void clear();

    /// @brief Clear all models
    void clearModels();

    /// @brief Clear all shader programs
    void clearShaderPrograms();

    /// @brief Obtains shader given the name, creating if it doesn't exist
    std::shared_ptr<ShaderProgram> getShaderProgramByName(const QString& name);
    std::shared_ptr<ShaderProgram> getShaderProgramByFilePath(const QString& fragpath, const QString& vertPath);

    /// @brief Obtains a material to add to the renderer's list, creating if it doesn't exist
    std::shared_ptr<Material> getMaterial(const QString& name, bool create=true);
    std::shared_ptr<Material> createMaterial(const QJsonValue& json);
    //std::shared_ptr<Material> getMaterialByFilePath(const QString& filepath);

    /// @brief Add a material to the resource cache
    void addMaterial(std::shared_ptr<Material> material);

    /// @brief Return cubemap with given filepath and name
    std::shared_ptr<CubeMap> getCubemap(const QString& name);
    std::shared_ptr<CubeMap> createCubemap(const QString& name, const QString& filepath);

    /// @brief Return animation with the given name
    std::shared_ptr<ResourceHandle> getAnimationByName(const QString& name);

    /// @brief Add a model to the resource cache
    void addModel(std::shared_ptr<Model> model);

    /// @brief Return model with the given name
    /// @details If model not found in map, return new model
    std::shared_ptr<Model> getModel(const QString& name);
    std::shared_ptr<Model> getModelByFilePath(const QString& filepath);
    std::shared_ptr<Model> createModel(const QJsonValue& json);

    /// @brief Obtains a script, given a source filepath
    std::shared_ptr<PythonClassScript> getScript(const QString& filepath);

    /// @brief Obtains a mesh, given a source filepath
    std::shared_ptr<ResourceHandle> getMesh(const QString& nameOrFilepath);

    /// @brief Obtains a texture given the filepath, creating if it doesn't exist
    std::shared_ptr<ResourceHandle> getTexture(const QString& filePath, int textureType);

    /// @brief Obtains a cube texture given the filepaths, creating if it doesn't exist
    std::shared_ptr<ResourceHandle> getCubeTexture(const QString& filePath);

    /// @brief Obtains an image given the filepath, creating if it doesn't exist
    std::shared_ptr<ResourceHandle> getImage(const QString& filePath,
        QImage::Format format = QImage::Format_Invalid);

    /// @brief Return resource handle by UUID
    std::shared_ptr<ResourceHandle> getResourceHandle(const Uuid& uuid);

    /// @brief Obtain a resource handle corresponding to the given JSON
    std::shared_ptr<ResourceHandle> getResourceHandle(const QJsonValue& json);

	/// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "ResourceCache"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::ResourceCache"; }

    /// @}

signals:
    void resourceChanged(std::shared_ptr<Object> resourceLike);
    //void resourceChanged(const std::shared_ptr<ResourceHandle>& resource);
    //void resourceChanged(const std::shared_ptr<Material>& mtl);
    //void resourceChanged(const std::shared_ptr<Model>& model);
    //void resourceChanged(const std::shared_ptr<ShaderProgram>& shader);

    void resourceAdded(std::shared_ptr<Object> resourceLike);
    //void resourceAdded(const std::shared_ptr<ResourceHandle>& resourceLike);
    //void resourceAdded(const std::shared_ptr<Material>& mtl);
    //void resourceAdded(const std::shared_ptr<Model>& model);
    //void resourceAdded(const std::shared_ptr<ShaderProgram>& shader);
    //void resourceAdded(const std::shared_ptr<PythonClassScript>& script);

    void resourceDeleted(std::shared_ptr<Object> resourceLike);
    //void resourceDeleted(const std::shared_ptr<ResourceHandle>& resource);
    //void resourceDeleted(const std::shared_ptr<Material>& mtl);
    //void resourceDeleted(const std::shared_ptr<Model>& model);
    //void resourceDeleted(const std::shared_ptr<ShaderProgram>& shader);
    //void resourceDeleted(const std::shared_ptr<PythonClassScript>& script);

public slots:

    /// @brief Run the post-construction method of the given resource handle
    void runPostConstruction(std::shared_ptr<ResourceHandle> resourceHandle);

    /// @brief Reload a resource handle's resource
    void reloadResource(const Uuid& handleUuid);
    void reloadResource(std::shared_ptr<ResourceHandle> handle);

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
    void clearResources();

    /// @brief Obtain a resource of the given type from the resource cache
    /// @details Identifier string is a resource name if the resource is generated in memory,
    /// e.g. A polygonal mesh, such as a cube or rectangle 
    std::shared_ptr<ResourceHandle> getResourceHandleByFilePath(const QString& filePath,
        Resource::ResourceType type,
        ResourceHandle::Priority priority = ResourceHandle::kRemovable,
        const ResourceAttributes& resourceAttributes = ResourceAttributes());

    std::shared_ptr<ResourceHandle> getResourceHandleByName(const QString& name,
        Resource::ResourceType type,
        ResourceHandle::Priority priority = ResourceHandle::kRemovable,
        const ResourceAttributes& resourceAttributes = ResourceAttributes());

    /// @brief Update position of resource to front of queue
    void update(std::shared_ptr<ResourceHandle> resource);

    /// @brief Returns the oldest object
    std::shared_ptr<ResourceHandle> oldestResource() const {
        return m_lru.back();
    }

    /// @brief Delete the given resource
    bool remove(std::shared_ptr<ResourceHandle> resource);

    /// @brief Return max memory allowed by the system, in megabytes
    static qint64 getMaxMemoryMb();

    /// @brief Log current cost
    void logCurrentCost() const;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Mutex for managing resource map and list
    QMutex m_resourceMutex;

    /// @brief Most recently used resources
    ResourceList m_lru;

    /// @brief Map of resources for easy access
    /// @details Indexed by unique name, which is filename only 
    /// if filepath exists and filepath is unique
    /// Contains meshes, textures
    ResourceMap m_resources;

    /// @brief Cache dedicated to storing generated polygons
    std::shared_ptr<PolygonCache> m_polygonCache;

    /// @brief Map of all materials, since materials are not resources
    /// @details Materials are indexed by lowercase material name
    std::unordered_map<QString, std::shared_ptr<Material>> m_materials;

    /// @brief Map of all models, since models are not resources
    /// @details Map key is the unique name (lowercase) of the model
    std::unordered_map<QString, std::shared_ptr<Model>> m_models;

    /// @brief The shader programs used by all renderers
    /// @details Indexed by name
    std::unordered_map<QString, std::shared_ptr<ShaderProgram>> m_shaderPrograms;

    /// @brief Map of all python scripts, which are not resources due to small size, indexed by filepath
    /// @details indexed by file path
    std::unordered_map<QString, std::shared_ptr<PythonClassScript>> m_pythonScripts;

    /// @brief Max allowed cost in the cache
    size_t m_maxCost;

    /// @brief Current cost in the cache
    size_t m_currentCost;

    /// @brief Process manager for pushing load requests
    ProcessManager* m_processManager;

    /// @}

};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif