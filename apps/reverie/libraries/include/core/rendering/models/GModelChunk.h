/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_MODEL_CHUNK_H
#define GB_MODEL_CHUNK_H

// QT
#include <memory>

// Internal
#include "core/mixins/GRenderable.h"

namespace rev {  

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class ResourceCache;
class CoreEngine;
class Material;
class ShaderProgram;
class SortingLayer;
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
    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const ModelChunk& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, ModelChunk& orObject);


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