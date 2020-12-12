/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_RENDER_COMMAND_H
#define GB_RENDER_COMMAND_H

// Standard
#include <unordered_map>

// QT
#include <QString>

// Internal
#include "../../mixins/GbLoadable.h"
#include "../../containers/GbString.h"
#include "../../containers/GbContainerExtensions.h"
#include "../../rendering/renderer/GbSortKey.h"
#include "GbRenderSettings.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class Renderable;
class ShaderProgram;
class AbstractCamera;
class RenderSettings;
class Material;
class MainRenderer;
struct Uniform;
struct SortingLayer;
class FrameBuffer;
class GLBuffer;
class ShadowMap;
/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class RenderCommand
/// @brief Class representing an abstract render command
class RenderCommand{
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    
    enum class CommandType {
        kNone = -1,
        kDraw = 0
    };

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    RenderCommand();
    virtual ~RenderCommand() {}
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    SortKey& sortKey() { return m_sortKey; }
    const SortKey& sortKey() const { return m_sortKey; }

    virtual CommandType commandType() const{
        return CommandType::kNone;
    }

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Perform depth prepass for the render command
    virtual void depthPrePass(MainRenderer& renderer) = 0;

    /// @brief Perform shadow map pass
    virtual void shadowPass(MainRenderer& renderer) = 0;

    /// @brief Perform the render command. Could be a draw, or a settings change
    virtual void perform(MainRenderer& renderer) = 0;

    virtual void onAddToQueue() = 0;

    /// @brief Performed directly prior to sort of all commands
    virtual void preSort() = 0;

	/// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    ///// @brief Outputs this data as a valid json string
    //QJsonValue asJson() const override;

    ///// @brief Populates this data using a valid json string
    //virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:

    /// @brief The sort key for this command
    SortKey m_sortKey;

};


/////////////////////////////////////////////////////////////////////////////////////////////
/// @class DrawCommand
/// @brief Class representing an item to be drawn
class DrawCommand : public RenderCommand {
public:
    //--------------------------------------------------------------------------------------------
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

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    DrawCommand();
    DrawCommand(Renderable& renderable, ShaderProgram& program, AbstractCamera& camera, ShaderProgram* prepassShaderProgram = nullptr);
    DrawCommand(Renderable& renderable, ShaderProgram& program, ShadowMap& shadowMap);
    virtual ~DrawCommand() {}
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    AbstractCamera* camera() { return m_camera; }

    ShaderProgram* shaderProgram() { return m_shaderProgram; }

    Renderable* renderable() { return m_renderable; }

    virtual CommandType commandType() const {
        return CommandType::kDraw;
    }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Write to depth buffer
    virtual void depthPrePass(MainRenderer& renderer) override;

    /// @brief Perform shadow map pass (write into shadow map)
    virtual void shadowPass(MainRenderer& renderer) override;

    /// @brief Perform the render command. Could be a draw, or a settings change
    virtual void perform(MainRenderer& renderer) override;

    /// @brief Set a uniform in the command
    void setUniform(const Uniform& uniform);

    template<typename T>
    void setUniform(const GStringView& uniformName, const T& uniformValue) {
        //auto iter = std::find_if(m_uniforms.begin(), m_uniforms.end(),
        //    [&](const Uniform& u) {
        //    return u.getName() == uniformName;
        //});

        //if (iter == m_uniforms.end()) {
            // Append uniform if not added
            Vec::EmplaceBack(m_uniforms, uniformName, uniformValue);
        //}
        //else {
        //    // Replace uniform if already set
        //    int idx = iter - m_uniforms.begin();
        //    m_uniforms[idx] = uniform;
        //}
    }

    /// @brief Add multiple uniforms to this command
    void addUniforms(const std::vector<Uniform>& uniforms);

    /// @brief Set render settings for the command, overriding previous ones if a new setting is specified
    RenderSettings& renderSettings() { return m_renderSettings; }

    /// @brief Obtain render layer
    SortingLayer* renderLayer() const {
        return m_renderLayer;
    }
    void setRenderLayer(SortingLayer* layer) {
        m_renderLayer = layer;
    }

    /// @brief Initialize the draw command
    void onAddToQueue() override;

    /// @brief Performed prior to sort
    void preSort() override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    ///// @brief Outputs this data as a valid json string
    //QJsonValue asJson() const override;

    ///// @brief Populates this data using a valid json string
    //virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Whether or not the command has the specified uniform
    bool hasUniform(const GString& uniformName, int* outIndex = nullptr);
    bool hasUniform(const Uniform& uniform, int* outIndex = nullptr);

    /// @brief Get the depth value used for the draw command
    float getDepth();

    /// @brief Update camera settings
    void updateCameraSettings(MainRenderer& renderer);

    void switchCameras(MainRenderer& renderer);

    /// @brief On shader switch
    //void switchShaders(MainRenderer& renderer);

    void updateShaderUniforms(MainRenderer& renderer, ShaderProgram& shaderProgram, bool ignoreMismatch);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Depth for this render command
    float m_depth;

    /// @brief Renderable
    Renderable* m_renderable = nullptr;

    /// @brief Shader stored in key
    ShaderProgram* m_shaderProgram = nullptr;
    ShaderProgram* m_prepassShaderProgram = nullptr;

    // Removed in favor of a material ID return method of renderable
    /// @brief Material to be stored in key
    //Material* m_material = nullptr;

    /// @brief Camera stored in key
    AbstractCamera* m_camera = nullptr;

    /// @brief Additional uniforms to use for rendering
    std::vector<Uniform> m_uniforms;

    /// @brief GL Render settings to override those of the renderable
    RenderSettings m_renderSettings;

    /// @brief Render layer
    SortingLayer* m_renderLayer = nullptr;

    /// @brief Depth caching for normalizing depth for sort
    // TODO: Move to main renderer so this isn't static, will better enable multiple main renderers
    static float s_farthestDepth; // most negative depth
    static float s_nearestDepth; // most positive depth

    /// @}

};


///////////////////////////////////////////////////////////////////////////////////////////////
///// @class DrawShadowMapCommand
///// @brief Class representing a command to draw to a shadow map
//class DrawShadowMapCommand : public DrawCommand {
//public:
//    //--------------------------------------------------------------------------------------------
//    /// @name Static
//    /// @{
//    /// @}
//
//    //--------------------------------------------------------------------------------------------
//    /// @name Constructors/Destructor
//    /// @{
//
//    DrawShadowMapCommand(Renderable& renderable, ShaderProgram& program, ShadowMap& shadowMap);
//    virtual ~DrawShadowMapCommand() {}
//    /// @}
//
//    //--------------------------------------------------------------------------------------------
//    /// @name Properties
//    /// @{
//
//    /// @}
//
//    //--------------------------------------------------------------------------------------------
//    /// @name Public Methods
//    /// @{
//
//    /// @brief Unused
//    virtual void depthPrePass(MainRenderer& renderer) override;
//
//    /// @brief Write to shadow map
//    virtual void perform(MainRenderer& renderer) override;
//
//
//    /// @}
//
//    //-----------------------------------------------------------------------------------------------------------------
//    /// @name Serializable Overrides
//    /// @{
//
//    /// @}
//
//protected:
//    //--------------------------------------------------------------------------------------------
//    /// @name Protected Methods
//    /// @{
//
//    /// @}
//
//    //--------------------------------------------------------------------------------------------
//    /// @name Protected Members
//    /// @{
//
//    /// @brief The shadow map being rendered to by this draw command
//    ShadowMap* m_shadowMap;
//
//    /// @}
//
//};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif