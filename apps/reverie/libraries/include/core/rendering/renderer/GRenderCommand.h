#pragma once

// Internal
#include "fortress/containers/GSortingLayer.h"
#include "fortress/types/GLoadable.h"
#include "fortress/types/GString.h"
#include "fortress/containers/GContainerExtensions.h"
#include "core/rendering/renderer/GSortKey.h"
#include "GRenderSettings.h"
#include "fortress/layer/framework/GFlags.h"
#include "core/geometry/GCollisions.h"

namespace rev {

class UniformContainer;
class Renderable;
class ShaderProgram;
class AbstractCamera;
class RenderSettings;
class Material;
class OpenGlRenderer;
class Uniform;
class UniformData;
class FrameBuffer;
class GlBuffer;
class ShadowMap;
enum class RenderablePassFlag;
typedef Flags<RenderablePassFlag> RenderablePassFlags;

/// @brief Reserved IDs used for identifying renderables that are not part of a scene object
enum class RenderObjectId {
    kDebug = -1
};

/// @class RenderCommand
/// @brief Class representing an abstract render command
class RenderCommand{
public:
    /// @name Static
    /// @{
    
    enum class CommandType {
        kNone = -1,
        kDraw = 0
    };

    /// @}

	/// @name Constructors/Destructor
	/// @{
    RenderCommand();
    virtual ~RenderCommand() {}
	/// @}

    /// @name Properties
    /// @{

    SortKey& sortKey() { return m_sortKey; }
    const SortKey& sortKey() const { return m_sortKey; }

    virtual CommandType commandType() const{
        return CommandType::kNone;
    }

    /// @}

	/// @name Public Methods
	/// @{

    /// @brief Perform depth prepass for the render command
    virtual void depthPrePass(OpenGlRenderer& renderer) = 0;

    /// @brief Perform shadow map pass
    virtual void shadowPass(OpenGlRenderer& renderer) = 0;

    /// @brief Perform the render command. Could be a draw, or a settings change
    virtual void perform(OpenGlRenderer& renderer, uint32_t commandIndex) = 0;

    virtual void onAddToQueue() = 0;

    /// @brief Performed directly prior to sort of all commands
    virtual void preSort() = 0;

	/// @}

protected:

    SortKey m_sortKey; ///< The sort key for this command

};



/// @class DrawCommand
/// @brief Class representing an item to be drawn
class DrawCommand : public RenderCommand {
public:
    /// @name Static
    /// @{

    /// @brief Obtain depth range values
    static std::pair<float, float> DepthRange() {
        return std::make_pair(s_nearestDepth, s_farthestDepth);
    }

    /// @brief Reset depths for a new round of rendering
    // TODO: Move depths to main renderer so that they aren't static
    static void ResetDepths();

    /// @}

    /// @name Constructors/Destructor
    /// @{
    DrawCommand();
    DrawCommand(Renderable& renderable, UniformContainer& uniformContainer, AbstractCamera& camera, int sceneObjectId);
    DrawCommand(Renderable& renderable, ShaderProgram& program, UniformContainer& uniformContainer, AbstractCamera& camera, int sceneObjectId, ShaderProgram* prepassShaderProgram = nullptr);
    virtual ~DrawCommand() {}
    /// @}

    /// @name Properties
    /// @{

    int sceneObjectId() const { return m_sceneObjectId; }

    void setShaderPrograms(ShaderProgram* prepassShader, ShaderProgram* mainShader);

    /// @brief Flags related to the render pass behavior of the renderable
    RenderablePassFlags passFlags() const { return m_passFlags; }
    void setPassFlags(RenderablePassFlags flags) { m_passFlags = flags; }

    AbstractCamera* camera() { return m_camera; }

    ShaderProgram* shaderProgram() { return m_shaderProgram; }
    ShaderProgram* prepassShaderProgram() { return m_prepassShaderProgram; }

    Renderable* renderable() { return m_renderable; }

    virtual CommandType commandType() const {
        return CommandType::kDraw;
    }

    const AABB& renderableWorldBounds() const {
        return m_renderableWorldBounds;
    }
    void setRenderableWorldBounds(const AABB& bounds);

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Write to depth buffer
    virtual void depthPrePass(OpenGlRenderer& renderer) override;

    /// @brief Perform shadow map pass (write into shadow map)
    virtual void shadowPass(OpenGlRenderer& renderer) override;

    /// @brief Perform the render command. Could be a draw, or a settings change
    virtual void perform(OpenGlRenderer& renderer, uint32_t commandIndex) override;

    /// @brief Set a uniform in the command
    void addUniform(const UniformData& uniformData, Int32_t mainUniformId, Int32_t prepassUniformId);

    /// @brief Replace the uniform if set, otherwise add
    void setUniform(const UniformData& uniformData, Int32_t mainUniformId, Int32_t prepassUniformId);

    /// @brief Add multiple uniforms to this command
    void addUniforms(const std::vector<Uniform>& uniforms, const std::vector<Int32_t>& prepassUniformIds);

    /// @brief Set render settings for the command, overriding previous ones if a new setting is specified
    RenderSettings& renderSettings() { return m_renderSettings; }

    /// @brief Obtain render layer
    const SortingLayer& renderLayer() const {
        return m_renderLayer;
    }
    void setRenderLayer(const SortingLayer& layer) {
        m_renderLayer = layer;
    }

    /// @brief Initialize the draw command
    void onAddToQueue() override;

    /// @brief Performed prior to sort
    void preSort() override;

    /// @brief Whether or not the command has the specified uniform
    bool hasUniform(Uint32_t uniformId, int* outIndex);

    /// @deprecated Only used for debug mode
    bool hasUniform(const GStringView& name, int* outIndex);

    /// @}

protected:
    /// @name Protected Methods
    /// @{

    /// @brief Whether or not the command has the specified uniform
    bool hasUniform(const Uniform& uniform, int* outIndex);

    /// @brief Get the depth value used for the draw command
    float getDepth();

    /// @brief Update camera settings
    void updateCameraSettings(OpenGlRenderer& renderer);

    void switchCameras(OpenGlRenderer& renderer);

    /// @param[in] commandIndex if negative, don't set uniform, otherwise sets g_colorId uniform in shader
    void updateShaderUniforms(OpenGlRenderer& renderer, ShaderProgram& shaderProgram, bool ignoreMismatch, int commandIndex = -1);

    /// @}

    /// @name Protected Members
    /// @{

    float m_depth; ///< Depth for this render command
    RenderablePassFlags m_passFlags; ///< Flags about the current render pass
    Renderable* m_renderable = nullptr; ///< The thing to draw
    ShaderProgram* m_shaderProgram = nullptr; ///< Shader stored in key
    ShaderProgram* m_prepassShaderProgram = nullptr;
    AbstractCamera* m_camera = nullptr; ///< Camera stored in key
    UniformContainer* m_uniformContainer{ nullptr }; ///< The container housing uniform values
    std::vector<Uniform> m_prepassUniforms; ///<  Additional uniforms to use for rendering
    std::vector<Uniform> m_mainUniforms; ///<  Additional uniforms to use for rendering
    RenderSettings m_renderSettings; ///< GL Render settings to override those of the renderable 
    SortingLayer m_renderLayer; ///< Render layer
    int m_sceneObjectId; ///< Scene object ID is negative if there is no scene object associated with the renderable
    AABB m_renderableWorldBounds; ///< Bounding geometry transformed into world-space

    /// @brief Depth caching for normalizing depth for sort
    /// @todo Move to main renderer so this isn't static, will better enable multiple main renderers
    static float s_farthestDepth; ///< The most negative depth
    static float s_nearestDepth; ///< The most positive depth

    /// @}

};


} // End namespaces
