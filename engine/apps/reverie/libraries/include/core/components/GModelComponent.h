#pragma once

// Project
#include "GComponent.h"
#include "core/rendering/GGLFunctions.h"
#include "core/mixins/GRenderable.h"

namespace rev {

class ShadowMap;
class Model;
class ResourceHandle;
class ShaderProgram;
class Uniform;
class DrawCommand;
class AbstractCamera;
class CollidingGeometry;
class SortingLayer;
class OpenGlRenderer;
class ShaderPreset;

/// @class ModelComponent
class ModelComponent: public Component{
public:
    /// @name Constructors/Destructor
    /// @{
    ModelComponent();
    ModelComponent(const std::shared_ptr<SceneObject>& object);
    ~ModelComponent();

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Get draw commands for the given camera and render layer
    std::vector<std::shared_ptr<DrawCommand>> getMeshCommands(AbstractCamera* camera, Uint32_t renderLayerId) const;

    /// @brief Get draw commands for the given shadow map
    std::vector<std::shared_ptr<DrawCommand>> getShadowMeshCommands(ShadowMap* shadowMap) const;

    /// @brief Clear the draw commands for the model
    void clearDrawCommands();
    void clearShadowDrawCommands();

    /// @brief Update/retrieve the draw commands for the model
    void retrieveDrawCommands(std::vector<std::shared_ptr<DrawCommand>>& outCommands, AbstractCamera* cam, const SortingLayer& currentLayer);
    void retrieveShadowDrawCommands(std::vector<std::shared_ptr<DrawCommand>>& outCommands, ShadowMap* shadowMap);

    /// @brief Called when a render layer is added to this model
    void onRenderLayerAdded(Uint32_t layerId);

    /// @brief Called when a render layer is removed from this model
    void onRenderLayerRemoved(Uint32_t layerId);

    /// @todo Make private
    /// @brief Create the draw commands for the model component
    void createDrawCommands(std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands, AbstractCamera& camera, const ShaderPreset* preset, Int32_t layerId = -1);

    /// @brief Create the draw commands for the model component
    void createShadowDrawCommands(std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands, ShadowMap& sm);

    /// @brief Enable the behavior of this script component
    virtual void enable() override;

    /// @brief Disable the behavior of this script component
    virtual void disable() override;

    /// @brief Max number of allowed components per scene object
    virtual int maxAllowed() const override { return 1; }

    /// @brief Update scene object's bounds given a transform
    void updateBounds(const TransformInterface& transform);

    /// @brief Used to manually update the mesh commands
    std::vector<std::shared_ptr<DrawCommand>>& drawCommands() {
        return m_meshDrawCommands;
    }
    std::vector<std::shared_ptr<DrawCommand>>& shadowDrawCommands() {
        return m_shadowDrawCommands;
    }

    bool hasDrawCommands() const;

    /// @}

    /// @name Properties
    /// @{

    /// @brief The render settings used for this model
    RenderSettings& renderSettings() { return m_renderSettings; }

    /// @property Model
    Model* model() const;
    const std::shared_ptr<ResourceHandle>& modelHandle() const {
        return m_modelHandle;
    }

    /// @brief Set the model used by this component
    void setModelHandle(const std::shared_ptr<ResourceHandle>& handle);

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const ModelComponent& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, ModelComponent& orObject);


    /// @}

protected:

    /// @name Protected Members
    /// @{

    struct ModelUniforms {
        UniformData m_false; ///< A false-values uniform
    };
    ModelUniforms m_uniforms; ///< The uniforms for the model component

    std::shared_ptr<ResourceHandle> m_modelHandle; ///< The model used by this model instance
    RenderSettings m_renderSettings; ///< The render settings associated with this model
    std::vector<std::shared_ptr<DrawCommand>> m_meshDrawCommands; ///< Commands used to draw this model
    std::vector<std::shared_ptr<DrawCommand>> m_shadowDrawCommands; ///< Commands used to draw this model's shadows

    /// @}


};


} // end namespacing
