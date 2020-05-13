/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_RESOURCE_H
#define GB_RESOURCE_H
// standard
#include <tchar.h>

// QT
#include <QFile>
#include <QObject>
#include <QMutex>

// Internal
#include "../containers/GbGVariant.h"
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
class ShaderProgram;
class Image;
class CoreEngine;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class ResourceAttributes
/// @brief Struct for storing attributes of a resource
/// @details For passing into processResource function of LoadProcess
class ResourceAttributes: public Serializable {
public:
    ResourceAttributes(){}
    ResourceAttributes(const QJsonValue& json);
    ~ResourceAttributes() {}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{
    /// @brief Check whether resource attributes has the given attribute
    bool hasAttribute(const QString& name) const{
        return m_attributes.find(name) != m_attributes.end();
    }

    /// @brief Check if empty or not
    bool isEmpty() const { return m_attributes.empty(); }

    /// @brief Return attribute
    const GVariant& at(const QString& name) const;
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

    std::map<QString, GVariant> m_attributes;
};


/////////////////////////////////////////////////////////////////////////////////////////////
/// @classs Resource
class Resource : public Gb::Object{
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Type of resource
    enum ResourceType {
        kImage,
        kTexture,
        kMaterial,
        kMesh,
        kCubeTexture,
        kAnimation
    };

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    Resource(const QString& name, ResourceType type);
    Resource(ResourceType type);
    virtual ~Resource() {}
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief Whether the resource is constructed or not
    bool isConstructed() const { return m_isConstructed; }

    /// @brief Get the type of resource stored by this handle
    Resource::ResourceType getType() const { return m_type; }

    /// @brief Return the cost of the resource
    virtual size_t getCost() const { return m_cost; }

    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

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
    //--------------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    void logConstructedWarning() {
        if (m_isConstructed) {
            logWarning("Resource is already constructed, returning");
        }
    }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    /// @brief The cost of the resource
    size_t m_cost;

    /// @brief Resource type
    ResourceType m_type;

    /// @brief Whether resource is done being constructed or not
    bool m_isConstructed;

    /// @}

};

/////////////////////////////////////////////////////////////////////////////////////////////
/// @class ResourceHandle
/// @brief Class representing a resource
class ResourceHandle: public Object, public Loadable{
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    
    enum Priority {
        kRemovable, // Resource is deleted if low on queue and cache needs space
        kPermanent, // resource is never automatically deleted
    };

    enum LoadFlags {
        kCore = 1, // Resource is never deleted, as it is required by the engine
    };

    /// @brief Get identifying name of resource from JSON representation
    static QString getNameFromJson(const QJsonObject& json);

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    ResourceHandle(CoreEngine* engine);
    ResourceHandle(CoreEngine* engine, const std::shared_ptr<Resource>& resource, Priority = kRemovable);
    ResourceHandle(CoreEngine* engine, const QString& filepath, Resource::ResourceType type, Priority = kRemovable);
    ResourceHandle(CoreEngine* engine, const QString& filepath, const QString& name, Resource::ResourceType type, Priority = kRemovable);
    ~ResourceHandle();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief Flags relating to the load/delete behavior of the resource
    bool isCoreResource() const {
        return m_loadFlags.testFlag(kCore);
    }
    void setCore(bool toggle) {
        m_loadFlags.setFlag(kCore, toggle);
    }

    /// @brief mutex
    QMutex& mutex() {
        return m_resourceMutex;
    }

    /// @brief Pointer to core engine
    CoreEngine* engine() { return m_engine; }

    /// @property IsLoading
    bool getIsLoading() const { return m_isLoading; }
    void setIsLoading(bool isLoading) { m_isLoading = isLoading; }

    /// @property ResourceAttributes
    const ResourceAttributes& getAttributes() const { return m_attributes; };
    void setAttributes(const ResourceAttributes& attributes) { m_attributes = attributes; }

    /// @brief Get the type of resource stored by this handle
    Resource::ResourceType getType() const { return m_type; }

    /// @brief Obtain resource
    const std::shared_ptr<Resource>& resource(bool lockMutex);

    /// @brief Set the resource for this handle
    void setResource(const std::shared_ptr<Resource>& resource, bool lockMutex);

    /// @property Priority
    Priority getPriority() const { return m_priority; }
    void setPriority(Priority priority) { m_priority = priority; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    friend bool operator==(const ResourceHandle& r1, const ResourceHandle& r2);

    /// @}


	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Remove resource
    void removeResource(bool lockMutex);

	/// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    void loadFromJson(const QJsonValue& json) override;

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

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    std::shared_ptr<ResourceHandle> sharedPtr();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Whether or not the resource is loading
    bool m_isLoading;

    /// @brief Priority type of the resource
    Priority m_priority;

    /// @brief Load flags
    QFlags<LoadFlags> m_loadFlags;

    /// @brief Resource type
    Resource::ResourceType m_type;

    /// @brief Mutex to protect resource
    QMutex m_resourceMutex;

    /// @brief Pointer to the core engine
    CoreEngine* m_engine;

    /// @brief Pointer to the resource
    std::shared_ptr<Resource> m_resource;

    /// @brief Attributes for regenerating the resource
    ResourceAttributes m_attributes;


    /// @}

};
Q_DECLARE_METATYPE(std::shared_ptr<ResourceHandle>)



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif