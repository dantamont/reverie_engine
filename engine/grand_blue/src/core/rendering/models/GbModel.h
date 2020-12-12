/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_MODEL_H
#define GB_MODEL_H

// QT
#include <QOpenGLBuffer>
#include <memory>

// Internal
#include "../../GbObject.h"
#include "../GbGLFunctions.h"
#include "../shaders/GbUniform.h"
#include "../../mixins/GbNameable.h"
#include "../../mixins/GbRenderable.h"
#include "../../resource/GbResource.h"
#include "../../rendering/geometry/GbMesh.h"

namespace Gb {  

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class ResourceCache;
class CoreEngine;
class Material;
class ShaderProgram;
struct SortingLayer;
class SceneCamera;
class DrawCommand;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class representing a renderable chunk of geometry
class ModelChunk: public Renderable {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    ModelChunk(const std::shared_ptr<ResourceHandle>& mesh, const std::shared_ptr<ResourceHandle>& mtl);
    ~ModelChunk();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief Whether mesh and material are loaded
    bool isLoaded() const;

    virtual size_t getSortID() override;

    /// @brief Create bounding box for this model chunk
    void generateBounds();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    virtual void preDraw() override;

    /// @brief Set uniforms for the given shader program
    /// @details Uniforms are pulled from the internal list, m_uniforms
    void bindUniforms(ShaderProgram& shaderProgram) override;

    void bindTextures(ShaderProgram* shaderProgram, RenderContext* context) override;
    void releaseTextures(ShaderProgram* shaderProgram, RenderContext* context) override;

    /// @brief Draw geometry associated with this renderable
    virtual void drawGeometry(ShaderProgram& shaderProgram, RenderSettings* settings = nullptr);

    std::shared_ptr<Mesh> meshResource() const;
    std::shared_ptr<Material> materialResource() const;

    /// #}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    std::shared_ptr<ResourceHandle> m_mesh;
    std::shared_ptr<ResourceHandle> m_material;

    /// @}
};



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class for a model 
class Model: public Resource, public Serializable {
public:
	//-----------------------------------------------------------------------------------------------------------------
	/// @name Static
	/// @{

    enum ModelFlag {
        kIsAnimated = 1 << 0 // Whether or not the model has animations associated with it
    }; 
    typedef QFlags<ModelFlag> ModelFlags;

    /// @brief Create handle to a model
    static std::shared_ptr<ResourceHandle> createHandle(CoreEngine* engine);
    static std::shared_ptr<ResourceHandle> createHandle(CoreEngine* engine, 
        const GString& uniqueName, 
        ModelFlags flags = {});

	/// @}

	//-----------------------------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    Model();
    Model(const GString& uniqueName, ModelFlags flags = {});
    Model(ResourceHandle& handle, const QJsonValue& json);
    Model(ResourceHandle& handle);
	~Model();
	/// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief Get the type of resource stored by this handle
    virtual Resource::ResourceType getResourceType() const override {
        return Resource::kModel;
    }

    /// @brief Skeleton for the model
    std::shared_ptr<Skeleton> skeleton() const;

    const std::shared_ptr<ResourceHandle>& skeletonHandle() const { return m_skeletonHandle; }
    std::shared_ptr<ResourceHandle>& skeletonHandle() { return m_skeletonHandle; }

    /// @brief Whether or not the model is animated
    bool isAnimated() const;
    bool isStatic() const;

    void setAnimated(bool animated);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    //friend bool operator==(const Model& m1, const Model& m2);

    /// @}

	//-----------------------------------------------------------------------------------------------------------------
	/// @name Public methods
	/// @{

    /// @brief Get material with the given name
    const std::shared_ptr<ResourceHandle>& getMaterial(const GString& name);

    /// @brief Create a new mesh with the given name and add it to the model
    std::shared_ptr<Mesh> addMesh(CoreEngine* core, const GString& meshName);

    /// @brief Create a new material with the given name and add it to the model
    std::shared_ptr<Material> addMaterial(CoreEngine* core, const GString& mtlName);

    /// @brief Create a new animation with the given name and add it to the model
    std::shared_ptr<Animation> addAnimation(CoreEngine* core, const GString& animName);

    /// @brief Add a skeleton to the model
    void addSkeleton(CoreEngine* core);

    const std::vector<ModelChunk>& chunks() const { return m_chunks; }

    /// @brief What action to perform on removal of the resource
    virtual void onRemoval(ResourceCache* cache = nullptr) override;

    /// @brief What action to perform post-construction of the model
    /// @details For performing any actions that need to be done on the main thread
    virtual void postConstruction() override;

	/// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name GB object Properties
    /// @{
    /// @property className
    const char* className() const override { return "Model"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::Model"; }
    /// @}

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{
    friend class Renderer;
    friend class ModelReader;
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief Checks that resource is loaded and valid to draw, and attempts to load if not loaded
    bool checkMesh(const std::shared_ptr<ResourceHandle>& handle);
    bool checkMaterial(const std::shared_ptr<ResourceHandle>& handle);

    // TODO: Remove, deprecated
    void loadChunksFromJson(const QJsonObject & object);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Flags governing the behavior of the model
    size_t m_modelFlags;

    /// @brief Vectors of meshes and associated materials
    std::vector<ModelChunk> m_chunks;

    /// @brief Skeleton for the model
    std::shared_ptr<ResourceHandle> m_skeletonHandle = nullptr;

    /// @}
};



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif