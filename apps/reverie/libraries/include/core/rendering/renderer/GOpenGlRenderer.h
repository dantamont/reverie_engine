#pragma once

// Standard
#include <vector>
#include <set>
#include <algorithm>
#include <mutex>

// External
#include "fortress/containers/GStrictGrowContainer.h"
#include "fortress/time/GStopwatchTimer.h"
#include "enums/GSimulationPlayModeEnum.h"

// Internal
#include "core/rendering/GGLFunctions.h"
#include "core/rendering/shaders/GUniform.h"
#include "GSortKey.h"
#include "GRenderContext.h"

namespace rev {  

class CoreEngine;
class Scenario;
class Scene;
class SceneObject;
class UndoCommand;
class WorldRay;

class SortingLayer;
class Texture;
class Material;
class AbstractCamera;
class ShaderProgram;
class RenderCommand;
class DrawCommand;
class SceneCamera;
class GLWidget;


struct RenderFrameState {
    enum RenderStage {
        kDepthPrepass,
        kShadowMapping,
        kRender
    };

    enum RenderActionFlag {
        kResize = 1 << 0 // Window was resized during this frame
    };

    bool playModeChanged() const {
        return m_playMode != m_previousPlayMode;
    }

    bool resized() const {
        return m_actionFlags.testFlag(kResize);
    }

    SortKey m_lastRenderedKey;
    AbstractCamera* m_camera = nullptr;
    ShaderProgram* m_shaderProgram = nullptr;
    RenderStage m_stage = kDepthPrepass;
    QFlags<RenderActionFlag> m_actionFlags;
    GSimulationPlayMode m_previousPlayMode{ -1 };
    GSimulationPlayMode m_playMode{ -2 };
};


/// @class Main Renderer
/// @brief Driver for rendering all GL geometry
/// @details Stores positional data of a model in a VAO
class OpenGlRenderer: protected gl::OpenGLFunctions {
public:    
	/// @name Static
	/// @{

    enum LightingMode {
        kForward, // Most basic
        kDeferred // TODO: More complicated, deferred rendering!
    };

    // Tiled forward shading is sometimes also referred to as Forward+
    // See: http://www.aortiz.me/2018/12/21/CG.html#tiled-shading--forward

    // For forward depth pass, need to be able to preserve glPosition as well as discards in frag shader,
    // while minimizing performance impact of the shader. Right now, the same shader is used
    // for both passes, which is not great
    enum LightingFlag {
        kClustered = 1 << 0, // Perform light-culling using clustered approach
        kDepthPrePass = 1 << 1, // FIXME: TODO: Simplify pre-pass so that actually helps performance
        kDynamicShadows = 1 << 2, // Whether or not to allow shadow casting
        kSSAO = 1 << 3 // Whether or not to perform SSAO
    };

	/// @}

	/// @name Constructors/Destructor
	/// @{

    OpenGlRenderer(rev::CoreEngine* engine, GLWidget* widget);
    ~OpenGlRenderer();

	/// @}

    /// @name Properties
    /// @{

    QFlags<LightingFlag>& lightingFlags() { return m_lightingFlags; }

    GLWidget* widget() { return m_widget; }
    const GLWidget* widget() const { return m_widget; }

    std::mutex& drawMutex() const {
        return m_drawMutex;
    }

    RenderContext& renderContext() { return m_renderContext; }

    /// @}
     
	/// @name Public methods
	/// @{

    /// @brief Raycast deferred for hit scene objects. Return IDs of scene objects
    bool raycastDeferredGeometry(const WorldRay& raycast, std::vector<int>& outSceneObjectIds);

    /// @brief Return default prepass shader
    /// @details Unused, since text doesn't like it
    //ShaderProgram* getDefaultPrepassShader() const;

    /// @brief Return SSAO shader
    ShaderProgram* getSSAOShader() const;
    ShaderProgram* getSSAOBlurShader() const;

    /// @brief Return cluster grid generation compute shader
    ShaderProgram* getClusterGridShader() const;

    /// @brief Return light culling compute shader
    ShaderProgram* getLightCullingShader() const;

    /// @brief Return active cluster compute shader
    ShaderProgram* getActiveClusterShader() const;

    /// @brief Return active cluster compression compute shader
    ShaderProgram* getActiveClusterCompressShader() const;

    /// @brief Render commands from previous frame
    const std::vector<std::shared_ptr<RenderCommand>>& readRenderCommands() const { return m_readRenderCommands; }

    /// @brief Render commands for the current frame
    std::vector<std::shared_ptr<RenderCommand>>& renderCommands() { return m_renderCommands; }

    /// @brief Incoming render commands
    std::vector<std::shared_ptr<RenderCommand>>& receivedCommands() { return m_receivedCommands; }

    /// @brief Reset and prepare to render geometry
    void initialize();

    /// @brief Request a resize of framebuffers in the renderer
    void requestResize();

	/// @brief Render draw items
	void render();

    /// @brief Generate draw items
    void processScenes();

    /// @brief Add a render command to the renderer
    void addRenderCommand(const std::shared_ptr<RenderCommand>& command);
    void addShadowMapCommand(const std::shared_ptr<DrawCommand>& command);

    template<typename CommandType>
    void addRenderCommands(const std::vector<std::shared_ptr<CommandType>>& commands) {
        Uint32_t numCommands = m_receivedCommands.size();
        m_receivedCommands.reserve(numCommands + commands.size());
        for (const auto& command : commands) {
            command->onAddToQueue();
            m_receivedCommands.push_back(command);
        }
    }

    template<typename CommandType>
    void addShadowMapCommands(const std::vector<std::shared_ptr<CommandType>>& commands) {
        Uint32_t numCommands = m_receivedShadowMapCommands.size();
        m_receivedShadowMapCommands.reserve(numCommands + commands.size());
        for (const auto& command : commands) {
            command->onAddToQueue();
            m_receivedShadowMapCommands.push_back(command);
        }
    }

    void clearPersistentCommands() {
        m_persistentRenderCommands.clear();
    }

    template<typename CommandType>
    void addPersistentRenderCommands(const std::vector<std::shared_ptr<CommandType>>& commands) {
        Uint32_t numCommands = m_persistentRenderCommands.size();
        m_persistentRenderCommands.reserve(numCommands + commands.size());
        for (const auto& command : commands) {
            command->onAddToQueue();
            m_persistentRenderCommands.push_back(command);
        }
    }

	/// @}

protected:
	/// @name Friends
	/// @{

	friend class GLWidget;
    friend class Viewport;
    friend class Camera;
    friend class SceneCommand;
    friend class AddScenarioCommand;
    friend class AddSceneCommand;
    friend class AddSceneObjectCommand;
    friend class AddSceneObjectComponent;
    friend class RenderCommand;
    friend class DrawCommand;

	/// @}

	/// @name Protected methods
	/// @{

    /// @brief What to do before draw
    void preDraw();

    /// @brief The depth pre-pass
    /// @detail Multi-sampled depth pre-pass (can be toggled on and off)
    /// @todo TODO: Shorter far plane, could make more performant, and it's not worth drawing farther objects anyway
    void depthPrePass();

    /// @brief The dynamic lighting (shadow mapping) prepass
    void shadowMappingPass();

    /// @brief The clustered lighting pass
    /// @detail Determine the light culling for each camera from render commands
    void clusteredLightingPass(ESimulationPlayMode mode);

    /// @brief SSAO pass
    /// @detail Renders the SSAO texture
    void ssaoPass(ESimulationPlayMode mode);
    void ssaoCameraPass(SceneCamera& camera);

    /// @brief Render pass
    /// @detail Draw the actual scene
    void renderPass(ESimulationPlayMode mode);

    /// @brief Post-processing pass
    /// @detail Render quad with scene and any post-processing affects applied
    void postProcessingPass(ESimulationPlayMode mode);

    /// @brief Set global GL settings
    void initializeGlobalSettings();

    /// @brief Preprocess commands before queuing to render
    void preprocessCommands();

	/// @}

	/// @name Protected members
	/// @{

    /// @todo Create an SSAO settings object, maybe move to render context
    /// @brief Uniforms used for implementing SSAO
    struct SsaoUniforms {
        UniformData m_offsets;
        UniformData m_scale;
        UniformData m_noiseSize;
        UniformData m_kernelSize;
        UniformData m_bias;
        UniformData m_radius;
    };
    SsaoUniforms m_ssaoUniforms; ///< The SSAO uniforms

    LightingMode m_lightingMode = kForward;
    QFlags<LightingFlag> m_lightingFlags = kDepthPrePass | kClustered | kDynamicShadows | kSSAO;

    std::vector<std::shared_ptr<RenderCommand>> m_renderCommands; ///< Vector of current render commands to iterate over
    std::vector<std::shared_ptr<RenderCommand>> m_readRenderCommands; ///< Vector of render commands from the previous frame
    std::vector<std::shared_ptr<DrawCommand>> m_shadowMapCommands; ///< Vector of current commands to draw to shadow map
    std::vector<std::shared_ptr<RenderCommand>> m_receivedCommands; ///< Cache of render commands being updated
    std::vector<std::shared_ptr<DrawCommand>> m_receivedShadowMapCommands; ///< Cache of commands to draw to shadow map

    std::vector<std::shared_ptr<RenderCommand>> m_persistentRenderCommands; ///< Vector of persistent commands to draw

    StrictGrowContainer<std::vector<UniformData>> m_commandIndexUniforms; ///< Set this only when renderable is created

    RenderFrameState m_renderState; ///< Struct containing information about current rendering state
    rev::CoreEngine* m_engine{ nullptr }; ///< Core engine
    GLWidget* m_widget{ nullptr }; ///< Widget for this renderer
    StopwatchTimer m_timer; ///< Determines elapsed time since the creation of the renderer
    mutable std::mutex m_drawMutex; ///< Mutex lock for draw command containers
    RenderContext m_renderContext; ///< Render context

	/// @}
};

       
// End namespaces        
} 
