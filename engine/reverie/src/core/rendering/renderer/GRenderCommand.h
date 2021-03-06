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
#include "../../mixins/GLoadable.h"
#include "../../containers/GString.h"
#include "../../containers/GContainerExtensions.h"
#include "../../rendering/renderer/GSortKey.h"
#include "GRenderSettings.h"
#include "../../containers/GFlags.h"
#include "../../geometry/GCollisions.h"

namespace rev {

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
enum class RenderablePassFlag;
typedef Flags<RenderablePassFlag> RenderablePassFlags;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Reserved IDs used for identifying renderables that are not part of a scene object
enum class RenderObjectId {
    kDebug = -1
};

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
    virtual void perform(MainRenderer& renderer, size_t commandIndex) = 0;

    virtual void onAddToQueue() = 0;

    /// @brief Performed directly prior to sort of all commands
    virtual void preSort() = 0;

	/// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    ///// @brief Outputs this data as a valid json string
    //QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    ///// @brief Populates this data using a valid json string
    //virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:

    /// @brief The sort key for this command
    SortKey m_sortKey;

};


/////////////////////////////////////////////////////////////////////////////////////////////
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
    DrawCommand(Renderable& renderable, ShaderProgram& program, AbstractCamera& camera, int sceneObjectId, ShaderProgram* prepassShaderProgram = nullptr);
    DrawCommand(Renderable& renderable, ShaderProgram& program, ShadowMap& shadowMap, int sceneObjectId);
    virtual ~DrawCommand() {}
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    int sceneObjectId() const { return m_sceneObjectId; }

    /// @brief Flags related to the render pass behavior of the renderable
    RenderablePassFlags passFlags() const { return m_passFlags; }
    void setPassFlags(RenderablePassFlags flags) { m_passFlags = flags; }

    AbstractCamera* camera() { return m_camera; }

    ShaderProgram* shaderProgram() { return m_shaderProgram; }

    Renderable* renderable() { return m_renderable; }

    virtual CommandType commandType() const {
        return CommandType::kDraw;
    }

    const AABB& worldBounds() const {
        return m_worldBounds;
    }
    AABB& worldBounds() { return m_worldBounds; }
    void setWorldBounds(const AABB& bounds);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Write to depth buffer
    virtual void depthPrePass(MainRenderer& renderer) override;

    /// @brief Perform shadow map pass (write into shadow map)
    virtual void shadowPass(MainRenderer& renderer) override;

    /// @brief Perform the render command. Could be a draw, or a settings change
    virtual void perform(MainRenderer& renderer, size_t commandIndex) override;

    /// @brief Set a uniform in the command
    void addUniform(const Uniform& uniform);

    template<typename T>
    void addUniform(const GStringView& uniformName, const T& uniformValue) {
        Vec::EmplaceBack(m_uniforms, uniformName, uniformValue);
    }

    const Uniform* getUniform(const GStringView& uniformName) const;

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
    //QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

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

    /// @param[in] commandIndex if negative, don't set uniform, otherwise sets g_colorId uniform in shader
    void updateShaderUniforms(MainRenderer& renderer, ShaderProgram& shaderProgram, bool ignoreMismatch, int commandIndex = -1);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Depth for this render command
    float m_depth;

    /// @brief Flags related to the render pass behavior of the renderable
    RenderablePassFlags m_passFlags;

    /// @brief Renderable
    Renderable* m_renderable = nullptr;

    /// @brief Shader stored in key
    ShaderProgram* m_shaderProgram = nullptr;
    ShaderProgram* m_prepassShaderProgram = nullptr;

    /// @brief Camera stored in key
    AbstractCamera* m_camera = nullptr;

    /// @brief Additional uniforms to use for rendering
    std::vector<Uniform> m_uniforms;

    /// @brief GL Render settings to override those of the renderable
    RenderSettings m_renderSettings;

    /// @brief Render layer
    SortingLayer* m_renderLayer = nullptr;

    /// @brief Scene object ID is negative if there is no scene object associated with the renderable
    int m_sceneObjectId;

    /// @brief Bounding geometry transformed into world-space
    AABB m_worldBounds;

    /// @brief Depth caching for normalizing depth for sort
    // TODO: Move to main renderer so this isn't static, will better enable multiple main renderers
    static float s_farthestDepth; // most negative depth
    static float s_nearestDepth; // most positive depth

    /// @}

};



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif