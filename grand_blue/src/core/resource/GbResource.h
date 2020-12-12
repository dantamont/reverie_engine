/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_RESOURCE_H
#define GB_RESOURCE_H
// standard
#include <tchar.h>
#include <type_traits>

// QT
#include <QFile>
#include <QObject>
#include <QMutex>

// Internal
#include "../containers/GbDictionary.h"
#include "../mixins/GbLoadable.h"
#include "../mixins/GbNameable.h"
#include "../readers/GbFileReader.h"
#include "../containers/GbContainerExtensions.h"

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

namespace Gb {

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

/// @class Resource
class Resource : public Gb::Object{
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Type of resource
    enum ResourceType {
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
        kAudio
    };

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    Resource(const GString& name);
    Resource();
    virtual ~Resource() {}
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    ResourceHandle* handle() { return m_handle; }

    /// @brief Get the type of resource stored by this handle
    virtual Resource::ResourceType getResourceType() const = 0;

    /// @brief Return the cost of the resource
    virtual size_t getCost() const { return m_cost; }

    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Whether or not the resource is serializable
    bool isSerializable() const;

    /// @brief What action to perform on removal of the resource
    virtual void onRemoval(ResourceCache* cache = nullptr) = 0;

    /// @brief What action to perform post-construction of the resource
    /// @details For performing any actions that need to be done on the main thread
    virtual void postConstruction();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "Resource"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::Resource"; }
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

    /// @}

private:
};

/////////////////////////////////////////////////////////////////////////////////////////////
/// @class ResourceHandle
/// @brief Class representing a resource
class ResourceHandle: public Object, public DistributedLoadable{
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    
    enum BehaviorFlag {
        kRemovable = (1 << 0), // By default, resource is never deleted. If flagged, can be removed to save memory
        kChild = (1 << 1), // Is a child resource, i.e., is loaded in along with a parent resource
        kParent = (1 << 2), // Is a parent resource, i.e., is loaded along with child resources
        kUserGenerated = (1 << 3), // The resource has no path associated with it, i.e., it is not loaded from a file
        kCore = (1 << 4), // Is a core resource, will not be remove, no matter what, and won't save to file. Overrides removable flag
        kUnsaved = (1 << 5), // If unsaved, will not be saved to file
        kUsesJson = (1 << 6) // Whether or not the resource uses JSON for initialization
    };
    typedef QFlags<BehaviorFlag> BehaviorFlags;

    enum StatusFlag {
        kConstructed = (1 << 0), // Resource was successfully constructed
        //kDelete = (1 << 1), // Resource on queue for deletion
        kIsLoading = (1 << 2) // Whether or not the source is loading
    };
    typedef QFlags<StatusFlag> StatusFlags;

    enum DeleteFlag {
        kDeleteHandle = (1 << 0), // Removes handle from resource cache entirely
        kForce = (1 << 1) // Force deletion of a resource, even if it is flagged as permanent
    };
    typedef QFlags<DeleteFlag> DeleteFlags;

    /// @brief Get identifying name of resource from JSON representation
    static GString getNameFromJson(const QJsonObject& json);

    /// @brief Create a resource handle
    static std::shared_ptr<ResourceHandle> create(CoreEngine* engine, const QJsonObject& json);
    static std::shared_ptr<ResourceHandle> create(CoreEngine* engine, Resource::ResourceType type);
    static std::shared_ptr<ResourceHandle> create(CoreEngine* engine, Resource::ResourceType type,
        BehaviorFlags flags);
    static std::shared_ptr<ResourceHandle> create(CoreEngine* engine, const GString& filepath, Resource::ResourceType type);
    static std::shared_ptr<ResourceHandle> create(CoreEngine* engine, const GString& filepath, Resource::ResourceType type,
        BehaviorFlags flags);

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Destructor
	/// @{
    ~ResourceHandle();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    BehaviorFlags& behaviorFlags() { return m_behaviorFlags; }
    void setBehaviorFlags(BehaviorFlags behaviorFlags) { m_behaviorFlags = behaviorFlags; }

    /// @brief Flags relating to the load/delete behavior of the resource
    bool isChild() const {
        return m_behaviorFlags.testFlag(kChild);
    }
    void setChild(bool isChild) {
        m_behaviorFlags.setFlag(kChild, isChild);
    }
    bool isUserGenerated() const {
        return m_behaviorFlags.testFlag(kUserGenerated);
    }
    void setUserGenerated(bool pathless) {
        m_behaviorFlags.setFlag(kUserGenerated, pathless);
    }

    /// @brief Will use JSON if manually flagged to do so, or if user generated
    bool usesJson() const {
        return m_behaviorFlags.testFlag(kUsesJson) || isUserGenerated();
    }
    void setUsesJson(bool uses) {
        m_behaviorFlags.setFlag(kUsesJson, uses);
    }

    bool isCore() const {
        return m_behaviorFlags.testFlag(kCore);
    }
    void setCore(bool isCore) {
        m_behaviorFlags.setFlag(kCore, isCore);
    }
    bool isPermanent() const {
        return !m_behaviorFlags.testFlag(kRemovable);
    }
    bool isRemovable() const {
        return m_behaviorFlags.testFlag(kRemovable) && !m_behaviorFlags.testFlag(kCore);
    }
    void setRemovable(bool toggle) {
        m_behaviorFlags.setFlag(kRemovable, toggle);
    }
    bool isUnsaved() const {
        // Won't be saved if core or if explicitly flagged unsaved
        return m_behaviorFlags.testFlag(kUnsaved) || m_behaviorFlags.testFlag(kCore);
    }
    void setUnsaved(bool toggle) {
        m_behaviorFlags.setFlag(kUnsaved, toggle);
    }

    /// @property IsLoading
    bool isLoading() const {
        return m_statusFlags.testFlag(kIsLoading);
    }
    void setIsLoading(bool isLoading) { m_statusFlags.setFlag(kIsLoading, isLoading); }

    bool isConstructed() const {
        return m_statusFlags.testFlag(kConstructed);
    }

    void setConstructed(bool constructed) {
#ifdef DEBUG_MODE
        if (constructed) {
            if (!isLoading()) throw("Error, set constructed flag before loading flag");
        }
#endif
        m_statusFlags.setFlag(kConstructed, constructed);
        m_statusFlags.setFlag(kIsLoading, false);
    }

    /// @brief mutex
    QMutex& mutex() { return m_resourceMutex; }

    /// @brief Pointer to core engine
    CoreEngine* engine() const { return m_engine; }

    /// @brief The resource needs a reload
    bool needsReload() const { return !m_resource && !isLoading(); }

    /// @property Resource Json
    // TODO: Deprecate, redundant with dictionary attributes
    const QJsonObject& resourceJson() const { return m_resourceJson; };
    QJsonObject& resourceJson() { return m_resourceJson; };
    void setResourceJson(const QJsonObject& object) { m_resourceJson = object; }

    /// @brief Get the type of resource stored by this handle
    Resource::ResourceType getResourceType() const { return m_type; }
    void setResourceType(Resource::ResourceType type) { m_type = type; }

    /// @brief Obtain resource
    const std::shared_ptr<Resource>& resource(bool lockMutex);

    /// @brief Set the resource for this handle
    void setResource(const std::shared_ptr<Resource>& resource, bool lockMutex);

    /// @brief Child resource handles
    std::vector<std::shared_ptr<ResourceHandle>>& children() { return m_children; }
    const std::vector<std::shared_ptr<ResourceHandle>>& children() const { return m_children; }

    /// @brief Parent resource handle
    ResourceHandle* parent() { return m_parent; }

    /// @brief The reader for this handle
    // TODO: Deprecate, this is clunky
    const std::shared_ptr<Object>& reader() const { return m_reader; }
    void setReader(const std::shared_ptr<Object>& reader) { m_reader = reader; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    friend bool operator==(const ResourceHandle& r1, const ResourceHandle& r2);

    /// @}


	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Set child paths
    /// @details Recursively set filepaths of all children to the specified path
    void setChildPaths(const GString& filepath);

    /// @brief Get the child with the given UUID
    const std::shared_ptr<ResourceHandle>& getChild(const Uuid& uuid);
    const std::shared_ptr<ResourceHandle>& getChild(const GString& name, Resource::ResourceType type);

    /// @brief Get the resource children of a specified type
    void getChildren(Resource::ResourceType type, std::vector<std::shared_ptr<ResourceHandle>>& outChildren);

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
    template<typename T>
    std::shared_ptr<T> resourceAs(bool lockMutex = false) {
        if constexpr (std::is_base_of_v<Resource, T>) {
            // Perform compile-time conversion if possible
            return std::static_pointer_cast<T>(resource(lockMutex));
        }
        else {
            // Dynamic cast is necessary for properly verifying polymorphism (see how a reinterpret_cast breaks things)
            return std::dynamic_pointer_cast<T>(resource(lockMutex));
        }
    }

    /// @brief Recursively post-construct the resource and all children
    void postConstruct(ResourceHandle* handle = nullptr, int level =  0);

	/// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "ResourceHandle"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::ResourceHandle"; }
    /// @}


protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends

    friend class ModelReader;

    /// @}


    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    ResourceHandle(CoreEngine* engine);
    ResourceHandle(CoreEngine* engine, Resource::ResourceType type);
    ResourceHandle(CoreEngine* engine, const std::shared_ptr<Resource>& resource);
    ResourceHandle(CoreEngine* engine, const GString& filepath, Resource::ResourceType type);
    ResourceHandle(CoreEngine* engine, const GString& filepath, const GString& name, Resource::ResourceType type);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    std::shared_ptr<ResourceHandle> sharedPtr();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Behavior flags
    BehaviorFlags m_behaviorFlags;

    /// @brief Status flags
    StatusFlags m_statusFlags;

    /// @brief Resource type
    Resource::ResourceType m_type;

    /// @brief Mutex to protect resource
    QMutex m_resourceMutex;

    /// @brief Pointer to the core engine
    CoreEngine* m_engine;

    /// @brief Pointer to the resource
    std::shared_ptr<Resource> m_resource;

    /// @brief Child resources
    std::vector<std::shared_ptr<ResourceHandle>> m_children;

    /// @brief Parent resource handle
    ResourceHandle* m_parent = nullptr;

    /// @brief JSON representing additional resource info
    QJsonObject m_resourceJson;

    /// @brief (Optional) Reader object for parsing resource
    /// @details Saved here to reference in model post-construction
    std::shared_ptr<Object> m_reader = nullptr;

    /// @brief Pointer to a null handle to pass children by reference
    static std::shared_ptr<ResourceHandle> s_nullHandle;

    /// @}

};
Q_DECLARE_METATYPE(std::shared_ptr<ResourceHandle>)



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif