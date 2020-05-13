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

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Class for a model 
class Model: public Object, 
    public Renderable,
    public std::enable_shared_from_this<Model>{
public:
	//-----------------------------------------------------------------------------------------------------------------
	/// @name Static
	/// @{

    enum ModelType {
        kStaticMesh,
        kCubeMap,
        kAnimatedMesh
    }; 

    /// @brief Create model (including subclasses) from JSON
    static std::shared_ptr<Model> create(CoreEngine* engine, const QJsonValue& json);

	/// @}

	//-----------------------------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    Model(CoreEngine* engine, const QString& uniqueName, ModelType type = kStaticMesh);
    Model(CoreEngine* engine, const QJsonValue& json);
    Model(const QString& uniqueName, const std::shared_ptr<Gb::ResourceHandle>& meshHandle);
	~Model();
	/// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief Type of model
    ModelType getModelType() const { return m_modelType; }

    /// @brief Get the handle for the mesh resource
    std::shared_ptr<ResourceHandle> meshHandle() { return m_meshHandle; }

    /// @brief Return filepath to geometry if there is one
    const QString& getFilePath() const;

    /// @brief Whether the model is from a file or not
    bool isFromFile();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    //friend bool operator==(const Model& m1, const Model& m2);

    /// @}

	//-----------------------------------------------------------------------------------------------------------------
	/// @name Public methods
	/// @{

    /// @property Set the name of the model and change location in resource map
    void rename(const QString& name);

    /// @brief Whether or not the model is still in the resourceCache
    bool inResourceCache() const;

    /// @brief Returns the Buffer group associated with this geometry
    /// @note Not thread safe, need to lock mutex for resource handle to use
    inline std::shared_ptr<Mesh> mesh() const {
        if (!m_meshHandle) {
            // No mesh handle at all
            return nullptr;
        }
        else if (m_meshHandle->resource(false)) {
            // Has mesh, return
            return std::static_pointer_cast<Mesh>(m_meshHandle->resource(false));
        }
        else {
            // Mesh is not yet loaded, will be set to resize from attempted access
#ifdef DEBUG_MODE
            logWarning("Warning, mesh not yet loaded for this model.  ResourceHandle is out of memory scope");
#endif
            return nullptr;
        }
    }

	/// @brief Draw this model
    /// [in] shapeMode 
	void draw(const std::shared_ptr<ShaderProgram>& shaderProgram, 
        RenderSettings* settings = nullptr) override;

    virtual void reload() override {}

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
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    std::shared_ptr<Model> sharedPtr() { return shared_from_this(); }

    /// @brief Checks that geometry is loaded and valid to draw, and attempts to load if not loaded
    bool checkMesh();

    /// @brief Set uniforms for the given shader
    /// @details Uniforms are pulled from the internal list, m_uniforms
    void bindUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram) override;

    void bindTextures() override;
    void releaseTextures() override;

    void drawGeometry(const std::shared_ptr<ShaderProgram>& shader, 
        RenderSettings* settings = nullptr) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief core engine
    CoreEngine* m_engine;

    /// @brief Mesh associated with this model's geometry
    std::shared_ptr<ResourceHandle> m_meshHandle;

	/// @brief Material names to update the meshes for this model
    /// @details Index is the name of the corresponding mesh
    std::unordered_map<QString, QString> m_materialNames;

    /// @brief Type of model
    ModelType m_modelType;


    /// @}
};



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif