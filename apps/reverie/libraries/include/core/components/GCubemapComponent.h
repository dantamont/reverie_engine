#pragma once
// Standard Includes
#include <memory>

// Project
#include "GComponent.h"
#include "core/mixins/GRenderable.h"
#include "core/resource/GResourceHandle.h"
#include "fortress/containers/GColor.h"

namespace rev {

class CubeMap;
class CubeTexture;
class ShaderProgram;
class RenderSettings;

/// @class CubeMapComponent
/// @brief Class representing a CubeMap 
/// @details Sampled in shader as a 3D direction vector
/// @note See: https://learnopengl.com/Advanced-OpenGL/Cubemaps
class CubeMapComponent: public Component, public NameableInterface, public Renderable {
public:
    /// @name Constructors/Destructor
    /// @{
    CubeMapComponent();
    CubeMapComponent(const CubeMapComponent & component);
    CubeMapComponent(const std::shared_ptr<SceneObject>& object, const nlohmann::json& json);
    CubeMapComponent(const std::shared_ptr<SceneObject>& object);
    ~CubeMapComponent();

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Enable the behavior of this script component
    virtual void enable() override;

    /// @brief Disable the behavior of this script component
    virtual void disable() override;

    /// @brief Max number of allowed components per scene
    virtual int maxAllowed() const override { return 1; }

    /// @brief Set as default cubemap for the scene
    void setDefault();
    bool isDefault() const;

    /// @property geometry for this model
    void draw(ShaderProgram& shaderProgram, RenderContext* context, RenderSettings* settings = nullptr, size_t drawFlags = 0) override;

    /// @brief Set the cube texture at the given filepath, loading if required
    void setCubeTexture(const GString& filepath);

    virtual void reload() override {}

    CubeTexture* texture();

    virtual size_t getSortID() override { return 0; }

    /// @}

    /// @name Properties
    /// @{

    /// @brief Diffuse color
    Color& diffuseColor() { return m_color; }
    void setDiffuseColor(const Color& color) {
        m_color = color;
    }

    const std::shared_ptr<ResourceHandle>& textureHandle() const { return m_cubeTextureHandle; }

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const CubeMapComponent& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, CubeMapComponent& orObject);


    /// @}

protected:
    /// @name Protected Methods
    /// @{

    /// @brief checkValidity the cubemap
    void initialize();

    /// @brief Set uniforms in the shader being used for rendering
    void bindUniforms(ShaderProgram& shaderProgram, CubeTexture& cubeTexture);


    /// @}

    struct CubemapUniformData {
        UniformData m_cubeTextureIndex; ///< The index of the texture used on the cubemap
        UniformData m_diffuseColor; ///< The diffuse color of the cubemap
        UniformData m_worldMatrix; ///< The world matrix of the cubemap
    };

    /// @name Protected Members
    /// @{

    CubemapUniformData m_uniforms; ///< The uniforms for the cubemap
    std::shared_ptr<ResourceHandle> m_cubeTextureHandle{ nullptr }; ///< The texture corresponding to this cubemap
    Color m_color; ///< The diffuse color of the cubemap

    /// @}

};

} // end namespacing
