#pragma once

// QT
#include <memory>

// Internal
#include "GModelChunk.h"
#include "fortress/layer/framework/GFlags.h"

namespace rev {  

class Skeleton;

/// @brief Class for a model 
class Model: public Resource {
public:
	/// @name Static
	/// @{

    enum ModelFlag {
        kIsAnimated = 1 << 0 // Whether or not the model has animations associated with it
    }; 
    typedef Flags<ModelFlag> ModelFlags;

    /// @brief Create handle to a model
    static std::shared_ptr<ResourceHandle> CreateHandle(CoreEngine* engine);
    static std::shared_ptr<ResourceHandle> CreateHandle(CoreEngine* engine, 
        const GString& uniqueName, 
        ModelFlags flags = {});

	/// @}

	/// @name Constructors/Destructor
	/// @{
    Model();
    Model(ModelFlags flags);
    Model(ResourceHandle& handle, const nlohmann::json& json);
    Model(ResourceHandle& handle);
	~Model();
	/// @}

    /// @name Properties
    /// @{

    /// @brief Get the type of resource stored by this handle
    virtual GResourceType getResourceType() const override {
        return EResourceType::eModel;
    }

    /// @brief Skeleton for the model
    Skeleton* skeleton() const;

    const std::shared_ptr<ResourceHandle>& skeletonHandle() const { return m_skeletonHandle; }
    std::shared_ptr<ResourceHandle>& skeletonHandle() { return m_skeletonHandle; }

    /// @brief Whether or not the model is animated
    bool isAnimated() const;
    bool isStatic() const;

    void setAnimated(bool animated);

    /// @}

	/// @name Public methods
	/// @{

    /// @brief Get material with the given name
    const std::shared_ptr<ResourceHandle>& getMaterial(const GString& name);

    /// @brief Return the materials, meshes, and animations used by this model
    void getChildren(std::vector<std::shared_ptr<ResourceHandle>>& outMaterials,
        std::vector<std::shared_ptr<ResourceHandle>>& outMeshes,
        std::vector<std::shared_ptr<ResourceHandle>>& outAnimations) const;

    /// @brief Create a new mesh with the given name and add it to the model
    Mesh* addMesh(CoreEngine* core, const GString& meshName);

    /// @brief Create a new material with the given name and add it to the model
    Material* addMaterial(CoreEngine* core, const GString& mtlName);

    /// @brief Create a new animation with the given name and add it to the model
    Animation* addAnimation(CoreEngine* core, const GString& animName);

    /// @brief Add a skeleton to the model
    void addSkeleton(CoreEngine* core);

    const std::vector<ModelChunk>& chunks() const { return m_chunks; }
    std::vector<ModelChunk>& chunks() { return m_chunks; }

    /// @brief What action to perform on removal of the resource
    virtual void onRemoval(ResourceCache* cache = nullptr) override;

    /// @brief What action to perform post-construction of the model
    /// @details For performing any actions that need to be done on the main thread
    virtual void postConstruction(const ResourcePostConstructionData& postConstructData) override;

    /// @todo Move this to another application or library entirely.
    ///// @brief Load the resource from its binary representation
    ///// @return False if unimplemented, or failed to load
    //virtual bool loadBinary(const GString& filepath) override;

    ///// @brief Save the resource to its binary representation
    ///// @return False if unimplemented, or failed to save
    //virtual bool saveBinary(const GString& filepath) const override;


	/// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const Model& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, Model& orObject);


    /// @}

protected:

    /// @name Friends
    /// @{
    friend class Renderer;
    friend class ModelReader;
    /// @}

    /// @name Protected methods
    /// @{

    /// @brief Checks that resource is loaded and valid to draw, and attempts to load if not loaded
    bool checkMesh(const std::shared_ptr<ResourceHandle>& handle);
    bool checkMaterial(const std::shared_ptr<ResourceHandle>& handle);

    // TODO: Remove, deprecated
    void loadChunksFromJson(const json & object);

    /// @}

    /// @name Protected Members
    /// @{

    /// @brief Flags governing the behavior of the model
    ModelFlags m_modelFlags;

    /// @brief Vectors of meshes and associated materials
    std::vector<ModelChunk> m_chunks;

    /// @brief Skeleton for the model
    std::shared_ptr<ResourceHandle> m_skeletonHandle = nullptr;

    /// @}
};


} // End namespaces
