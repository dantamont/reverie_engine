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
#include "../GbGLFunctions.h"
#include "GbRenderers.h"
#include "GbSortKey.h"
#include "GbRenderContext.h"

namespace Gb {  

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class Scenario;
class Scene;
class SceneObject;
class UndoCommand;

struct SortingLayer;
class Texture;
class Material;
class Camera;
class ShaderProgram;
class RenderCommand;
class DrawCommand;

namespace View {
    class GLWidget;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

struct RenderFrameState {
    enum RenderStage {
        kDepthPrepass,
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
    Camera* m_camera = nullptr;
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
        kForwardDepthPass, // Forward rendering, with a depth pre-pass to avoid rendering occluded geometry
        kDeferred, // TODO: More complicated, deferred rendering!
        kForwardPlus // TODO: An alternative approach (may need compute shaders) 
    };

	/// @}

	//---------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{

    MainRenderer(Gb::CoreEngine* engine, View::GLWidget* widget);
    ~MainRenderer();

	/// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

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

    /// @brief Return prepass shader
    std::shared_ptr<ShaderProgram> prepassShader() const;

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

	/// @}

protected:
	//--------------------------------------------------------------------------------------------
	/// @name Friends
	/// @{

	friend class View::GLWidget;
    friend class Viewport;
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

    /// @brief Set global GL settings
    void initializeGlobalSettings();

    /// @brief Preprocess commands before queuing to render
    void preprocessCommands();

	/// @}

	//---------------------------------------------------------------------------------------
	/// @name Protected members
	/// @{

    LightingMode m_lightingMode = kForwardDepthPass;

    /// @brief Vector of current render commands to iterate over
    std::vector<std::shared_ptr<RenderCommand>> m_renderCommands;

    /// @brief Cache of render commands being updated
    std::vector<std::shared_ptr<RenderCommand>> m_receivedCommands;

    /// @brief Struct containing information about current rendering state
    RenderFrameState m_renderState;

    /// @brief core engine
    Gb::CoreEngine* m_engine;

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