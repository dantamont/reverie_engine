/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_RESOURCE_H
#define GB_RESOURCE_H
// standard
#include <tchar.h>
#include <type_traits>
#include <array>

// QT
#include <QFile>
#include <QObject>
#include <QMutex>

// Internal
#include "../mixins/GLoadable.h"
#include "../mixins/GNameable.h"
#include "../readers/GFileReader.h"
#include "../containers/GContainerExtensions.h"
#include "../containers/GFlags.h"

/////////////////////////////////////////////////////////////////////////////////////////////
// Macros
/////////////////////////////////////////////////////////////////////////////////////////////
#if defined(DEBUG)
#	define GB_NEW new(_NORMAL_BLOCK,__FILE__, __LINE__)
#else
#	define GB_NEW new
#endif

#ifndef SAFE_DELETE
    #define SAFE_DELETE(p)  { if(p) { delete (p); (p) = nullptr; } }
#endif

#ifndef SAFE_DELETE_ARRAY
    #define SAFE_DELETE_ARRAY(p)    { if(p) { delete [] (p); (p) = nullptr; } }
#endif 

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
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
class Serializable;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Type of resource
enum class ResourceType {
    kNullType = -1,
    kImage,
    kTexture,
    kMaterial,
    kMesh,
    kCubeTexture,
    kAnimation,
    kModel,
    kShaderProgram,
    kPythonScript,
    kSkeleton,
    kAudio,
    kUserType // Implement
};

/// @class Resource
class Resource: public Object {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Get folder name that will house the specified resource type
    const GString& ResourceTypeDirName(ResourceType type);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    Resource();
    virtual ~Resource();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    ResourceHandle* handle() const { return m_handle; }

    /// @brief Get the type of resource stored by this handle
    virtual ResourceType getResourceType() const = 0;

    /// @brief Return the cost of the resource
    virtual size_t getCost() const { return m_cost; }

    /// @}
    //--------------------------------------------------------------------------------------------
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

    //--------------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    /// @brief the handle for this resource
    ResourceHandle* m_handle = nullptr;

    /// @brief The cost of the resource
    size_t m_cost;

    /// @brief Folder names for each resource type
    static std::array<GString, (size_t)ResourceType::kUserType + 1> s_resourceDirNames;

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
    kUsesJson = (1 << 6) // Whether or not the resource uses JSON for initialization
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


/////////////////////////////////////////////////////////////////////////////////////////////
/// @class ResourceHandle
/// @brief Class representing a resource
class ResourceHandle: public Object, public Identifiable, public Nameable, public DistributedLoadable{
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    
    /// @brief Get identifying name of resource from JSON representation
    static GString getNameFromJson(const QJsonObject& json);

    /// @brief Create a resource handle
    static std::shared_ptr<ResourceHandle> create(CoreEngine* engine, const QJsonObject& json);
    static std::shared_ptr<ResourceHandle> create(CoreEngine* engine, ResourceType type);
    static std::shared_ptr<ResourceHandle> create(CoreEngine* engine, ResourceType type,
        ResourceBehaviorFlags flags);
    static std::shared_ptr<ResourceHandle> create(CoreEngine* engine, const GString& filepath, ResourceType type);
    static std::shared_ptr<ResourceHandle> create(CoreEngine* engine, const GString& filepath, ResourceType type,
        ResourceBehaviorFlags flags);

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Destructor
	/// @{
    ~ResourceHandle();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    ResourceBehaviorFlags& behaviorFlags() { return m_behaviorFlags; }
    void setBehaviorFlags(ResourceBehaviorFlags behaviorFlags) { m_behaviorFlags = behaviorFlags; }

    /// @brief Flags relating to the load/delete behavior of the resource
    bool isChild() const {
        return m_behaviorFlags.testFlag(ResourceBehaviorFlag::kChild);
    }
    void setChild(bool isChild) {
        m_behaviorFlags.setFlag(ResourceBehaviorFlag::kChild, isChild);
    }
    bool isRuntimeGenerated() const {
        return m_behaviorFlags.testFlag(ResourceBehaviorFlag::kRuntimeGenerated);
    }
    void setRuntimeGenerated(bool pathless) {
        m_behaviorFlags.setFlag(ResourceBehaviorFlag::kRuntimeGenerated, pathless);
    }

    /// @brief Will use JSON if manually flagged to do so, or if user generated
    bool usesJson() const {
        return m_behaviorFlags.testFlag(ResourceBehaviorFlag::kUsesJson) || isRuntimeGenerated();
    }
    void setUsesJson(bool uses) {
        m_behaviorFlags.setFlag(ResourceBehaviorFlag::kUsesJson, uses);
    }

    bool isCore() const {
        return m_behaviorFlags.testFlag(ResourceBehaviorFlag::kCore);
    }
    void setCore(bool isCore) {
        m_behaviorFlags.setFlag(ResourceBehaviorFlag::kCore, isCore);
    }
    bool isPermanent() const {
        return !m_behaviorFlags.testFlag(ResourceBehaviorFlag::kRemovable);
    }
    bool isRemovable() const {
        return m_behaviorFlags.testFlag(ResourceBehaviorFlag::kRemovable) && !m_behaviorFlags.testFlag(ResourceBehaviorFlag::kCore);
    }
    void setRemovable(bool toggle) {
        m_behaviorFlags.setFlag(ResourceBehaviorFlag::kRemovable, toggle);
    }
    bool isUnsaved() const {
        // Won't be saved if core or if explicitly flagged unsaved
        return m_behaviorFlags.testFlag(ResourceBehaviorFlag::kUnsaved) || m_behaviorFlags.testFlag(ResourceBehaviorFlag::kCore);
    }
    void setUnsaved(bool toggle) {
        m_behaviorFlags.setFlag(ResourceBehaviorFlag::kUnsaved, toggle);
    }

    /// @property IsLoading
    bool isLoading() const {
        return m_statusFlags.testFlag(ResourceStatusFlag::kIsLoading);
    }
    void setIsLoading(bool isLoading) { m_statusFlags.setFlag(ResourceStatusFlag::kIsLoading, isLoading); }

    bool isConstructed() const {
        return m_statusFlags.testFlag(ResourceStatusFlag::kConstructed);
    }

    void setConstructed(bool constructed) {
#ifdef DEBUG_MODE
        if (constructed) {
            if (!isLoading()) throw("Error, set constructed flag before loading flag");
        }
#endif
        m_statusFlags.setFlag(ResourceStatusFlag::kConstructed, constructed);
        m_statusFlags.setFlag(ResourceStatusFlag::kIsLoading, false);
    }

    /// @brief mutex
    QMutex& mutex() { return m_resourceMutex; }

    /// @brief Pointer to core engine
    CoreEngine* engine() const { return m_engine; }

    /// @brief The resource needs a reload
    bool needsReload() const { return !m_resource && !isLoading(); }

    /// @property Resource Json
    const QJsonObject& resourceJson() const { return m_resourceJson; };
    //QJsonObject& resourceJson() { return m_resourceJson; };
    void setResourceJson(const QJsonObject& object) { m_resourceJson = object; }

    /// @brief Get the type of resource stored by this handle
    ResourceType getResourceType() const { return m_type; }
    void setResourceType(ResourceType type) { m_type = type; }

    /// @brief Obtain resource
    inline Resource* resource() const {
#ifdef DEBUG_MODE
        if (m_resource) {
            if (m_resource->getResourceType() != m_type) {
                throw("Error, resource somehow has the incorrect type");
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

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    friend bool operator==(const ResourceHandle& r1, const ResourceHandle& r2);

    /// @}


	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    std::shared_ptr<ResourceHandle> sharedPtr();

    /// @brief Set child paths
    /// @details Recursively set filepaths of all children to the specified path
    void setChildPaths(const GString& filepath);

    /// @brief Get the child with the given UUID
    const std::shared_ptr<ResourceHandle>& getChild(const Uuid& uuid);
    const std::shared_ptr<ResourceHandle>& getChild(const GString& name, ResourceType type);

    /// @brief Get the resource children of a specified type
    void getChildren(ResourceType type, std::vector<std::shared_ptr<ResourceHandle>>& outChildren);

    /// @brief Add a child resource to this resource
    void addChild(const std::shared_ptr<ResourceHandle>& child);

    /// @brief Move this resource to the front of the most-recently used list in the resource cache
    void touch();

    /// @brief Load resource
    void loadResource();

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

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "ResourceHandle"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "rev::ResourceHandle"; }
    /// @}


protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends

    friend class ModelReader;
    friend class LoadProcess;

    /// @}


    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    ResourceHandle(CoreEngine* engine);
    ResourceHandle(CoreEngine* engine, ResourceType type);
    //ResourceHandle(CoreEngine* engine, const std::shared_ptr<Resource>& resource);
    ResourceHandle(CoreEngine* engine, const GString& filepath, ResourceType type);
    ResourceHandle(CoreEngine* engine, const GString& filepath, const GString& name, ResourceType type);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    virtual void notifyForReload() const;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Behavior flags
    ResourceBehaviorFlags m_behaviorFlags;

    /// @brief Status flags
    ResourceStatusFlags m_statusFlags;

    /// @brief Resource type
    ResourceType m_type;

    /// @brief Mutex to protect resource
    mutable QMutex m_resourceMutex;

    /// @brief Pointer to the core engine
    CoreEngine* m_engine;

    /// @brief Pointer to the resource
    mutable std::unique_ptr<Resource> m_resource;

    /// @brief Child resources
    std::vector<std::shared_ptr<ResourceHandle>> m_children;

    /// @brief Parent resource handle
    ResourceHandle* m_parent = nullptr;

    /// @brief JSON representing additional resource info
    QJsonObject m_resourceJson;

    /// @brief Pointer to a null handle to pass children by reference
    static std::shared_ptr<ResourceHandle> s_nullHandle;

    /// @}

};
//Q_DECLARE_METATYPE(std::shared_ptr<ResourceHandle>)



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif