#ifndef GB_MAIN_RENDERER_H
#define GB_MAIN_RENDERER_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Standard
#include <map>
#include <set>
#include <algorithm>

// QT
#include <QString>
#include <QElapsedTimer>
#include <QMutex>

// Internal
#include "../GGLFunctions.h"
#include "GSortKey.h"
#include "GRenderContext.h"

namespace rev {  

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class Scenario;
class Scene;
class SceneObject;
class UndoCommand;
class WorldRay;

struct SortingLayer;
class Texture;
class Material;
class AbstractCamera;
class ShaderProgram;
class RenderCommand;
class DrawCommand;
class SceneCamera;
//class DrawShadowMapCommand;

namespace View {
    class GLWidget;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

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
    int m_previousPlayMode = -1;
    int m_playMode = -2;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class Main Renderer
/// @brief Driver for rendering all GL geometry
/// @details Stores positional data of a model in a VAO
class MainRenderer: Object, protected GL::OpenGLFunctions {
public:    
	//---------------------------------------------------------------------------------------
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

	//---------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{

    MainRenderer(rev::CoreEngine* engine, View::GLWidget* widget);
    ~MainRenderer();

	/// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    QFlags<LightingFlag>& lightingFlags() { return m_lightingFlags; }

    View::GLWidget* widget() { return m_widget; }
    const View::GLWidget* widget() const { return m_widget; }

    QMutex& drawMutex() {
        return m_drawMutex;
    }

    RenderContext& renderContext() { return m_renderContext; }

    /// @}
     
	//---------------------------------------------------------------------------------------
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

    /// @brief What to do before draw
    void preDraw();

	/// @brief Render draw items
	void render();

    /// @brief Generate draw items
    void processScenes();

    /// @brief Add a render command to the renderer
    void addRenderCommand(const std::shared_ptr<RenderCommand>& command);
    void addShadowMapCommand(const std::shared_ptr<DrawCommand>& command);


	/// @}

protected:
	//--------------------------------------------------------------------------------------------
	/// @name Friends
	/// @{

	friend class View::GLWidget;
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

	//--------------------------------------------------------------------------------------------
	/// @name Protected methods
	/// @{

    /// @brief SSAO pass
    void ssaoPass(SceneCamera& camera);

    /// @brief Set global GL settings
    void initializeGlobalSettings();

    /// @brief Preprocess commands before queuing to render
    void preprocessCommands();

	/// @}

	//---------------------------------------------------------------------------------------
	/// @name Protected members
	/// @{

    LightingMode m_lightingMode = kForward;
    QFlags<LightingFlag> m_lightingFlags = kDepthPrePass | kClustered | kDynamicShadows | kSSAO;

    /// @brief Vector of current render commands to iterate over
    std::vector<std::shared_ptr<RenderCommand>> m_renderCommands;

    /// @brief Vector of render commands from the previous frame
    std::vector<std::shared_ptr<RenderCommand>> m_readRenderCommands;

    /// @brief Vector of current commands to draw to shadow map
    std::vector<std::shared_ptr<DrawCommand>> m_shadowMapCommands;

    /// @brief Cache of render commands being updated
    std::vector<std::shared_ptr<RenderCommand>> m_receivedCommands;

    /// @brief Cache of commands to draw to shadow map
    std::vector<std::shared_ptr<DrawCommand>> m_receivedShadowMapCommands;

    /// @brief Struct containing information about current rendering state
    RenderFrameState m_renderState;

    /// @brief core engine
    rev::CoreEngine* m_engine;

    /// @brief Widget for this renderer
    View::GLWidget* m_widget;
    
    /// @brief Determines elapsed time since the creation of the renderer
    QElapsedTimer m_timer;

    /// @brief Mutex lock for draw command containers
    QMutex m_drawMutex;

    /// @brief Render context
    RenderContext m_renderContext;

	/// @}
};

       

///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
} 

#endif