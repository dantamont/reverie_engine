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

namespace View {
    class GLWidget;
}

namespace GL {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/// @class Main Renderer
/// @brief Driver for rendering all GL geometry
/// @details Stores positional data of a model in a VAO
class MainRenderer: Object, protected OpenGLFunctions {
     
public:    
	//---------------------------------------------------------------------------------------
	/// @name Static
	/// @{
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


    /// @}
     
	//---------------------------------------------------------------------------------------
	/// @name Public methods
	/// @{

    /// @brief Reset and prepare to render geometry
    void initialize();

	/// @brief Render geometry
	void render();

    /// @brief Clear the renderer
    void clear();

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

	/// @}

	//--------------------------------------------------------------------------------------------
	/// @name Protected methods
	/// @{

    /// @brief Set global GL settings
    void initializeGlobalSettings();

    /// @brief Clear models from static map to avoid crash on application close
    void clearModels();

	/// @}

	//---------------------------------------------------------------------------------------
	/// @name Protected members
	/// @{

    ///// @brief List of renderers to iterate over
    //std::multimap<SortingLayer, std::shared_ptr<Renderer>> m_renderers;

    /// @brief core engine
    Gb::CoreEngine* m_engine;

    /// @brief Widget for this renderer
    View::GLWidget* m_widget;
    
    /// @brief Determines elapsed time since the creation of the renderer
    QElapsedTimer m_timer;

	/// @}
};

       

///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
} 

#endif