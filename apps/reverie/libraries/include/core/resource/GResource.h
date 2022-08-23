#pragma once

// standard
#include <tchar.h>
#include <type_traits>
#include <array>
#include <atomic>

// QT
#include <QFile>
#include <QObject>
#include <QMutex>

// External
#include "enums/GResourceTypeEnum.h"
#include "fortress/json/GJson.h"
#include "fortress/types/GLoadable.h"
#include "fortress/types/GIdentifiable.h"
#include "fortress/types/GNameable.h"
#include "fortress/containers/GContainerExtensions.h"
#include "fortress/layer/framework/GFlags.h"
#include "logging/GLogger.h"

// Internal
#include "core/readers/GFileReader.h"

namespace rev {

class ResourceCache;
class Image;
class Texture;
class Material;
class Mesh;
class CubeTexture;
class Animation;
class Model;
class ShaderProgram;
class PythonScript;

class CoreEngine;
class ResourceHandle;

/// @class Resource
class Resource {
public:
    /// @name Static
    /// @{

    /// @brief Get folder name that will house the specified resource type
    const GString& ResourceTypeDirName(GResourceType type);

    /// @}

    /// @name Constructors/Destructor
    /// @{
    Resource();
    virtual ~Resource();
    /// @}

    /// @name Properties
    /// @{

    ResourceHandle* handle() const { return m_handle; }

    /// @brief Get the type of resource stored by this handle
    virtual GResourceType getResourceType() const = 0;

    /// @brief Return the cost of the resource
    virtual size_t getCost() const { return m_cost; }

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Load the resource from its binary representation
    /// @return False if unimplemented, or failed to load
    virtual bool loadBinary(const GString& filepath);

    /// @brief Save the resource to its binary representation
    /// @return False if unimplemented, or failed to save
    virtual bool saveBinary(const GString& filepath) const;

    /// @brief What action to perform on removal of the resource
    virtual void onRemoval(ResourceCache* cache = nullptr) = 0;

    /// @brief What action to perform post-construction of the resource
    /// @details For performing any actions that need to be done on the main thread
    virtual void postConstruction();

    /// @}

protected:
    friend class ResourceHandle;

    /// @name Private Members
    /// @{

    /// @brief the handle for this resource
    ResourceHandle* m_handle = nullptr;

    /// @brief The cost of the resource
    size_t m_cost;

    /// @brief Folder names for each resource type
    static std::array<GString, (size_t)EResourceType::eUserType + 1> s_resourceDirNames;

    /// @}

private:
};


enum class ResourceBehaviorFlag {
    kRemovable = (1 << 0), // By default, resource is never deleted. If flagged, can be removed to save memory
    kChild = (1 << 1), // Is a child resource, i.e., is loaded in along with a parent resource
    kParent = (1 << 2), // Is a parent resource, i.e., is loaded along with child resources
    kRuntimeGenerated = (1 << 3), // The resource has no path associated with it, i.e., it is not loaded from a file
    kCore = (1 << 4), // Is a core resource, will not be remove, no matter what, and won't save to file. Overrides removable flag
    kUnsaved = (1 << 5), // If unsaved, will not be saved to file
    kUsesJson = (1 << 6), // Whether or not the resource uses JSON for initialization
    kHidden = (1 << 7), // If hidden, is not exposed to user at all, e.g., through widgets
};
MAKE_BITWISE(ResourceBehaviorFlag);
MAKE_FLAGS(ResourceBehaviorFlag, ResourceBehaviorFlags)

enum class ResourceStatusFlag {
    kConstructed = (1 << 0), // Resource was successfully constructed
    //kDelete = (1 << 1), // Resource on queue for deletion
    kIsLoading = (1 << 2) // Whether or not the source is loading
};
MAKE_BITWISE(ResourceStatusFlag);
MAKE_FLAGS(ResourceStatusFlag, ResourceStatusFlags)

enum class ResourceDeleteFlag {
    kDeleteHandle = (1 << 0), // Removes handle from resource cache entirely
    kForce = (1 << 1) // Force deletion of a resource, even if it is flagged as permanent
};
MAKE_BITWISE(ResourceDeleteFlag);
MAKE_FLAGS(ResourceDeleteFlag, ResourceDeleteFlags)




/// @class ResourceHandle
/// @brief Class representing a resource
class ResourceHandle: public IdentifiableInterface, public NameableInterface, public DistributedLoadableInterface{
public:
    /// @name Static
    /// @{
    
    /// @brief Get identifying name of resource from JSON representation
    static GString getNameFromJson(const json& json);

    /// @brief Create a resource handle
    static std::shared_ptr<ResourceHandle> create(CoreEngine* engine, const json& json);
    static std::shared_ptr<ResourceHandle> create(CoreEngine* engine, GResourceType type);
    static std::shared_ptr<ResourceHandle> create(CoreEngine* engine, GResourceType type,
        ResourceBehaviorFlags flags);
    static std::shared_ptr<ResourceHandle> create(CoreEngine* engine, const GString& filepath, GResourceType type);
    static std::shared_ptr<ResourceHandle> create(CoreEngine* engine, const GString& filepath, GResourceType type,
        ResourceBehaviorFlags flags);

    /// @}

	/// @name Destructor
	/// @{
    ~ResourceHandle();
	/// @}

    /// @name Properties
    /// @{

    const ResourceStatusFlags statusFlags() const {
        return (ResourceStatusFlags)m_statusFlags.load();
    }
    void setStatusFlags(ResourceStatusFlags statusFlags) {
        m_statusFlags.store((uint32_t)statusFlags);
    }

    const ResourceBehaviorFlags behaviorFlags() const { 
        return (ResourceBehaviorFlags)m_behaviorFlags.load();
    }
    void setBehaviorFlags(ResourceBehaviorFlags behaviorFlags) { 
        m_behaviorFlags.store((uint32_t)behaviorFlags); 
    }

    /// @brief Flags relating to the load/delete behavior of the resource
    bool isChild() const {
        return behaviorFlags().testFlag(ResourceBehaviorFlag::kChild);
    }
    void setChild(bool isChild) {
        ResourceBehaviorFlags flags = behaviorFlags();
        flags.setFlag(ResourceBehaviorFlag::kChild, isChild);
        setBehaviorFlags(flags);
    }
    bool isRuntimeGenerated() const {
        return behaviorFlags().testFlag(ResourceBehaviorFlag::kRuntimeGenerated);
    }
    void setRuntimeGenerated(bool pathless) {
        ResourceBehaviorFlags flags = behaviorFlags();
        flags.setFlag(ResourceBehaviorFlag::kRuntimeGenerated, pathless);
        setBehaviorFlags(flags);
    }

    /// @brief Will use JSON if manually flagged to do so, or if user generated
    bool usesJson() const {
        return behaviorFlags().testFlag(ResourceBehaviorFlag::kUsesJson) || isRuntimeGenerated();
    }
    void setUsesJson(bool uses) {
        ResourceBehaviorFlags flags = behaviorFlags();
        flags.setFlag(ResourceBehaviorFlag::kUsesJson, uses);
        setBehaviorFlags(flags);
    }

    bool isHidden() const {
        return behaviorFlags().testFlag(ResourceBehaviorFlag::kHidden);
    }
    void setHidden(bool isHidden) {
        ResourceBehaviorFlags flags = behaviorFlags();
        flags.setFlag(ResourceBehaviorFlag::kHidden, isHidden);
        setBehaviorFlags(flags);
    }

    bool isCore() const {
        return behaviorFlags().testFlag(ResourceBehaviorFlag::kCore);
    }
    void setCore(bool isCore) {
        ResourceBehaviorFlags flags = behaviorFlags();
        flags.setFlag(ResourceBehaviorFlag::kCore, isCore);
        setBehaviorFlags(flags);
    }
    bool isPermanent() const {
        return !behaviorFlags().testFlag(ResourceBehaviorFlag::kRemovable);
    }
    bool isRemovable() const {
        ResourceBehaviorFlags flags = behaviorFlags();
        return flags.testFlag(ResourceBehaviorFlag::kRemovable) && !flags.testFlag(ResourceBehaviorFlag::kCore);
    }
    void setRemovable(bool toggle) {
        ResourceBehaviorFlags flags = behaviorFlags();
        flags.setFlag(ResourceBehaviorFlag::kRemovable, toggle);
        setBehaviorFlags(flags);
    }
    bool isUnsaved() const {
        // Won't be saved if core or if explicitly flagged unsaved
        ResourceBehaviorFlags flags = behaviorFlags();
        return flags.testFlag(ResourceBehaviorFlag::kUnsaved) || flags.testFlag(ResourceBehaviorFlag::kCore);
    }
    void setUnsaved(bool toggle) {
        ResourceBehaviorFlags flags = behaviorFlags();
        flags.setFlag(ResourceBehaviorFlag::kUnsaved, toggle);
        setBehaviorFlags(flags);
    }

    /// @property IsLoading
    bool isLoading() const {
        return statusFlags().testFlag(ResourceStatusFlag::kIsLoading);
    }
    void setIsLoading(bool isLoading) { 
        ResourceStatusFlags flags = statusFlags();
        flags.setFlag(ResourceStatusFlag::kIsLoading, isLoading);
        setStatusFlags(flags);
    }

    bool isConstructed() const {
        return statusFlags().testFlag(ResourceStatusFlag::kConstructed);
    }

    void setConstructed(bool constructed) {
#ifdef DEBUG_MODE
        if (constructed) {
            if (!isLoading())
            {
                Logger::Throw("Error, set constructed flag before loading flag");
            }
        }
#endif
        ResourceStatusFlags flags = statusFlags();
        flags.setFlag(ResourceStatusFlag::kConstructed, constructed);
        flags.setFlag(ResourceStatusFlag::kIsLoading, false);
        setStatusFlags(flags);
    }

    /// @brief mutex
    QMutex& mutex() { return m_resourceMutex; }

    /// @brief Pointer to core engine
    CoreEngine* engine() const { return m_engine; }

    /// @brief The resource needs a reload
    bool needsReload() const { return !m_resource && !isLoading(); }

    /// @property Resource Json
    const json& cachedResourceJson() const { return m_cachedResourceJson; };
    void setCachedResourceJson(const json& object) { m_cachedResourceJson = object; }

    /// @brief Get the type of resource stored by this handle
    GResourceType getResourceType() const { return m_type; }
    void setResourceType(GResourceType type) { m_type = type; }

    /// @brief Obtain resource
    inline Resource* resource() const {
#ifdef DEBUG_MODE
        if (m_resource) {
            if (m_resource->getResourceType() != m_type) {
                Logger::Throw("Error, resource somehow has the incorrect type");
            }
        }
#endif

        if (needsReload()) {
            notifyForReload();
        }

        //if (lockMutex) {
        //    m_resourceMutex.unlock();
        //}
        return m_resource.get();
    }

    /// @brief Set the resource for this handle
    void setResource(std::unique_ptr<Resource> resource, bool lockMutex);

    /// @brief Child resource handles
    std::vector<std::shared_ptr<ResourceHandle>>& children() { return m_children; }
    const std::vector<std::shared_ptr<ResourceHandle>>& children() const { return m_children; }

    /// @brief Parent resource handle
    ResourceHandle* parent() { return m_parent; }

    /// @}

    /// @name Operators
    /// @{

    friend bool operator==(const ResourceHandle& r1, const ResourceHandle& r2);

    /// @}

	/// @name Public Methods
	/// @{

    std::shared_ptr<ResourceHandle> sharedPtr();

    /// @brief Return as JSON
    json asJson() const;

    /// @brief Load resource from json
    /// @note Correct JSON serialization requires a cast to the resource's exact class
    void loadResourceJson(const json& json);

    /// @brief Set child paths
    /// @details Recursively set filepaths of all children to the specified path
    void setChildPaths(const GString& filepath);

    /// @brief Get the child with the given UUID
    const std::shared_ptr<ResourceHandle>& getChild(const Uuid& uuid);
    const std::shared_ptr<ResourceHandle>& getChild(const GString& name, GResourceType type);

    /// @brief Get the resource children of a specified type
    void getChildren(GResourceType type, std::vector<std::shared_ptr<ResourceHandle>>& outChildren);

    /// @brief Add a child resource to this resource
    void addChild(const std::shared_ptr<ResourceHandle>& child);

    /// @brief Move this resource to the front of the most-recently used list in the resource cache
    void touch();

    /// @brief Load resource
    /// @param[in] serialLoad if true, load on the main thread
    void loadResource(bool serialLoad = false);

    /// @brief Unload resource and those of all children from memory
    void unloadResource(bool lockMutex);

    /// @brief Remove from resource cache structs
    /// @details Must make sure to lock resource cache's resource mutex before calling this
    void removeFromCache(bool removeFromTopLevel);

    /// @brief Return resource as the specified casted type
    // TODO: Remove mutex check here, I'm 90% sure it's useless
    template<typename T>
    T* resourceAs() const {
        //static_assert(std::is_base_of_v<Resource, T>, "Can only convert to resource types");
        //if constexpr (std::is_base_of_v<Resource, T>) {
        //    // Perform compile-time conversion if possible
        //    return static_cast<T*>(resource(lockMutex));
        //}
        //else {
            // Dynamic cast is necessary for properly verifying polymorphism (see how a reinterpret_cast breaks things)
            return dynamic_cast<T*>(resource());
        //}
    }

    /// @brief Recursively post-construct the resource and all children
    void postConstruct(ResourceHandle* handle = nullptr, int level =  0);

	/// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const ResourceHandle& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, ResourceHandle& orObject);


    /// @}


protected:
    /// @name Friends

    friend class ModelReader;
    friend class LoadProcess;

    /// @}

    /// @name Constructors/Destructor
    /// @{
    ResourceHandle(CoreEngine* engine);
    ResourceHandle(CoreEngine* engine, GResourceType type);
    //ResourceHandle(CoreEngine* engine, const std::shared_ptr<Resource>& resource);
    ResourceHandle(CoreEngine* engine, const GString& filepath, GResourceType type);
    ResourceHandle(CoreEngine* engine, const GString& filepath, const GString& name, GResourceType type);

    /// @}

    /// @name Protected Methods
    /// @{

    virtual void notifyForReload() const;

    /// @brief Get json representation of resource
    /// @note Correct JSON serialization requires a cast to the resource's exact class
    json getResourceJson() const;

    /// @}

    /// @name Protected Members
    /// @{

    /// @brief Behavior flags
    //ResourceBehaviorFlags m_behaviorFlags;
    std::atomic<uint32_t> m_behaviorFlags{ 0 };

    /// @brief Status flags
    //ResourceStatusFlags m_statusFlags;
    std::atomic<uint32_t> m_statusFlags{ 0 };

    /// @brief Resource type
    GResourceType m_type;

    /// @brief Mutex to protect resource
    mutable QMutex m_resourceMutex;

    /// @brief Pointer to the core engine
    CoreEngine* m_engine{ nullptr };

    /// @brief Pointer to the resource
    mutable std::unique_ptr<Resource> m_resource;

    /// @brief Child resources
    std::vector<std::shared_ptr<ResourceHandle>> m_children;

    /// @brief Parent resource handle
    ResourceHandle* m_parent = nullptr;

    /// @brief JSON representing additional resource info
    json m_cachedResourceJson;

    /// @brief Pointer to a null handle to pass children by reference
    static std::shared_ptr<ResourceHandle> s_nullHandle;

    /// @}

};
//Q_DECLARE_METATYPE(std::shared_ptr<ResourceHandle>)



} // End namespaces
