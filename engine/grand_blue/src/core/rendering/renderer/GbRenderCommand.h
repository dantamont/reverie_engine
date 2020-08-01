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
#include "../../containers/GbContainerExtensions.h"
#include "../../rendering/renderer/GbSortKey.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class Renderable;
class ShaderProgram;
class Camera;
class RenderSettings;
class Material;
class MainRenderer;
struct Uniform;
struct SortingLayer;
class FrameBuffer;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class RenderCommand
/// @brief Class representing an abstract render command
class RenderCommand : public Serializable{
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
    //virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

protected:

    /// @brief The sort key for this command
    SortKey m_sortKey;

    /// @brief the render target for this command
    //FrameBuffer* m_frameBuffer;

};


/////////////////////////////////////////////////////////////////////////////////////////////
/// @class DrawCommand
/// @brief Class representing an item to be drawn
class DrawCommand : public RenderCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Reset depths for a new round of rendering
    // TODO: Move depths to main renderer so that they aren't static
    static void resetDepths();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    DrawCommand();
    DrawCommand(Renderable& renderable, ShaderProgram& program, Camera& camera);
    virtual ~DrawCommand() {}
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    Renderable* renderable() { return m_renderable; }

    virtual CommandType commandType() const {
        return CommandType::kDraw;
    }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Perform the render command. Could be a draw, or a settings change
    virtual void perform(MainRenderer& renderer) override;

    /// @brief Set a uniform in the command
    void setUniform(const Uniform& uniform);

    /// @brief Set render settings for the command
    std::vector<RenderSettings*> renderSettings() { return m_renderSettings; }
    void addRenderSettings(RenderSettings* settings, bool addToFront = false) {
        if (addToFront) {
            m_renderSettings.insert(m_renderSettings.begin(), settings);
        }
        else {
            m_renderSettings.push_back(settings);
        }
    }

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
    //virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Get the depth value used for the draw command
    float getDepth();

    /// @brief Render the command
    void render(MainRenderer& renderer);

    /// @brief Write to depth buffer
    void depthPrePass(MainRenderer& renderer);

    /// @brief Update camera settings
    void updateCameraSettings(MainRenderer& renderer);

    void updateShaderUniforms(MainRenderer& renderer);

    void updateShaderUniforms(MainRenderer& renderer, ShaderProgram& shaderProgram);

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

    // Removed in favor of a material ID return method of renderable
    /// @brief Material to be stored in key
    //Material* m_material = nullptr;

    /// @brief Camera stored in key
    Camera* m_camera = nullptr;

    /// @brief Additional uniforms to use for rendering
    std::unordered_map<QString, Uniform> m_uniforms;

    /// @brief GL Render settings to override those of the renderable
    std::vector<RenderSettings*> m_renderSettings;

    /// @brief Render layer
    SortingLayer* m_renderLayer;

    /// @brief Depth caching for normalizing depth for sort
    // TODO: Move to main renderer so this isn't static, will better enable multiple main renderers
    static float s_farthestDepth; // most negative depth
    static float s_nearestDepth; // most positive depth

    /// @}

};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif