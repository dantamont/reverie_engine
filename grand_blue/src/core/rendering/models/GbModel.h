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
class Camera;
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
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

protected:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    virtual void preDraw() override;

    /// @brief Set uniforms for the given shader program
    /// @details Uniforms are pulled from the internal list, m_uniforms
    void bindUniforms(ShaderProgram& shaderProgram) override;

    void bindTextures(ShaderProgram* shaderProgram) override;
    void releaseTextures(ShaderProgram* shaderProgram) override;

    /// @brief Draw geometry associated with this renderable
    virtual void drawGeometry(ShaderProgram& shaderProgram, RenderSettings* settings = nullptr);

    std::shared_ptr<Mesh> meshResource() const;
    std::shared_ptr<Material> materialResource() const;

    /// #}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    //std::shared_ptr<Mesh> m_mesh;
    //std::shared_ptr<Material> m_material;

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

    enum ModelType {
        kUndefined = -1,
        kStaticMesh,
        kAnimatedMesh
    }; 

    /// @brief Create handle to a model
    static std::shared_ptr<ResourceHandle> createHandle(CoreEngine* engine);
    static std::shared_ptr<ResourceHandle> createHandle(CoreEngine* engine, 
        const QString& uniqueName, 
        ModelType type = kStaticMesh);

	/// @}

	//-----------------------------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    Model(CoreEngine* engine);
    Model(CoreEngine* engine, const QString& uniqueName, ModelType type = kStaticMesh);
    Model(CoreEngine* engine, const QString& uniqueName, Resource::ResourceType resourceType, ModelType type = kStaticMesh);
    Model(ResourceHandle& handle, const QJsonValue& json);
    Model(ResourceHandle& handle);
	~Model();
	/// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief Skeleton for the model
    std::shared_ptr<Skeleton> skeleton() const;

    const std::shared_ptr<ResourceHandle>& skeletonHandle() const { return m_skeletonHandle; }
    std::shared_ptr<ResourceHandle>& skeletonHandle() { return m_skeletonHandle; }

    /// @brief Type of model
    ModelType getModelType() const { return m_modelType; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    //friend bool operator==(const Model& m1, const Model& m2);

    /// @}

	//-----------------------------------------------------------------------------------------------------------------
	/// @name Public methods
	/// @{

    ///// @brief Generate the draw commands for the model
    //void createDrawCommands(std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands,
    //    Camera& camera,
    //    ShaderProgram& shader);

	///// @brief Draw this model
 //   /// [in] shapeMode 
	//void draw(ShaderProgram& shaderProgram, RenderSettings* settings = nullptr) override;

    /// @brief Get material with the given name
    std::shared_ptr<ResourceHandle> getMaterial(const QString& name);

    /// @brief Create a new mesh with the given name and add it to the model
    std::shared_ptr<Mesh> addMesh(const QString& meshName);

    /// @brief Create a new material with the given name and add it to the model
    std::shared_ptr<Material> addMaterial(const QString& mtlName);

    /// @brief Create a new animatino with the given name and add it to the model
    std::shared_ptr<Animation> addAnimation(const QString& animName);

    //std::vector<std::shared_ptr<Mesh>>& meshes() { return m_meshes; }
    //std::vector<std::shared_ptr<Material>>& materials() { return m_materials; }
    std::vector<std::shared_ptr<ModelChunk>>& chunks() { return m_chunks; }

    /// @brief What action to perform on removal of the resource
    virtual void onRemoval(ResourceCache* cache = nullptr) override;

    void addSkeleton();

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
    virtual void loadFromJson(const QJsonValue& json) override;

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

    ///// @brief Set uniforms for the given draw command
    ///// @details Uniforms are pulled from the internal list, m_uniforms
    //void bindUniforms(DrawCommand& drawCommand);

    
    // TODO: Remove, deprecated
    void loadChunksFromJson(const QJsonObject & object);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief core engine
    CoreEngine* m_engine;

    /// @brief Vectors of meshes and associated materials
    std::vector<std::shared_ptr<ModelChunk>> m_chunks;

    /// @brief Type of model
    ModelType m_modelType;

    /// @brief Skeleton for the model
    std::shared_ptr<ResourceHandle> m_skeletonHandle = nullptr;

    /// @}
};



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif