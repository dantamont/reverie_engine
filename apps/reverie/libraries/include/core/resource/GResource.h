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

/// @brief Data to use on post-construction of a resource
struct ResourcePostConstructionData {
    bool m_deleteAfterConstruction{ true }; ///< Whether or not to delete data after post-construction
    void* m_data{ nullptr };
};

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
    /// @param[in] postConstructData data to be used to post-construct the resource
    virtual void postConstruction(const ResourcePostConstructionData& postConstructData);

    /// @}

protected:
    friend class ResourceHandle;

    /// @name Private Members
    /// @{

    ResourceHandle* m_handle = nullptr; ///< the handle for this resource
    size_t m_cost; ///<  The cost of the resource
    static std::array<GString, (size_t)EResourceType::eUserType + 1> s_resourceDirNames; ///< Folder names for each resource type

    /// @}

private:
};

} // End namespaces
