/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_MODEL_CHUNK_H
#define GB_MODEL_CHUNK_H

// QT
#include <memory>

// Internal
#include "../../mixins/GRenderable.h"

namespace rev {  

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

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

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

    Material* materialResource() const;

    /// #}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{
    /// @}
};



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif