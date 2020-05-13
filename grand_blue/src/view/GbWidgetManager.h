#ifndef GB_WIDGET_MANAGER_H
#define GB_WIDGET_MANAGER_H

//////////////////////////////////////////////////////////////////////////////////
// Includes  
//////////////////////////////////////////////////////////////////////////////////
// QT
#include <QtWidgets>
#include <QDockWidget>
#include <QMenuBar>
#include <QTimer>

// Internal
#include "../core/GbManager.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class MainWindow;
class CoreEngine;

namespace View {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class GLWidget;
class GraphWidget;
class ResourceTreeWidget;
class SceneTreeWidget;
class ComponentTreeWidget;
class PlayerControls;

//////////////////////////////////////////////////////////////////////////////////
// Class Definitions
//////////////////////////////////////////////////////////////////////////////////

/// @class Widget Manager
/// @brief Manager for controlling widgets and their layout
/// @note Multiple inheritance requires QObject flag be first
class WidgetManager: public Manager{
    Q_OBJECT
public:

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
	WidgetManager(Gb::CoreEngine* core, Gb::MainWindow* window);
	~WidgetManager();

	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief GL Widgets
    std::map<QString, Gb::View::GLWidget*>& glWidgets() { return m_glWidgets; }

    /// @brief Returns the main window for the application
    MainWindow* mainWindow() { return m_mainWindow; }

    /// @brief Returns the scene tree widget
    Gb::View::SceneTreeWidget* sceneTreeWidget() { return m_sceneTreeWidget; }

    Gb::View::ResourceTreeWidget* resourceTreeWidget() { return m_resourceTreeWidget; }

    /// @brief Returns the component widget
    Gb::View::ComponentTreeWidget* componentWidget() { return m_componentWidget; }


    /// @brief The widget for controlling scene playback
    Gb::View::PlayerControls* playbackWidget() { return m_playbackWidget; }

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Called on main window resize
    void resize(int w, int h);

    /// @brief Get pointer to main GL widget
    Gb::View::GLWidget* mainGLWidget() { return m_glWidgets["main"]; }

    /// @brief Clear the widget manager
    void clear();

	/// @}


protected:
	//--------------------------------------------------------------------------------------------
	/// @name Friends
	/// @{

	friend class Gb::MainWindow;

	/// @}

	//--------------------------------------------------------------------------------------------
	/// @name Protected Methods
	/// @{

	///@brief Initializes the OpenGL widget
	void initializeDefault();

	/// @}

	//--------------------------------------------------------------------------------------------
	/// @name Protected members
	/// @{

    /// @brief Timer for synchronizing widget values with data in corresponding objects
    //QTimer m_timer;

	/// @brief The main window for the application
	Gb::MainWindow* m_mainWindow;

	/// @brief Map of GL widgets
	std::map<QString, Gb::View::GLWidget*> m_glWidgets;

    /// @brief graph widget
    // TODO: Replace with subclass
    Gb::View::GraphWidget* m_graphWidget;

    /// @brief scene object tree widget
    Gb::View::ResourceTreeWidget* m_resourceTreeWidget;

    /// @brief Resource widget
    Gb::View::SceneTreeWidget* m_sceneTreeWidget;

    /// @brief scene object inspector widget
    Gb::View::ComponentTreeWidget* m_componentWidget;

    /// @brief scene playback widget
    Gb::View::PlayerControls* m_playbackWidget;

    /// @brief Docks in the main window
    QDockWidget* m_sceneDock;
    QDockWidget* m_resourceDock;
    QDockWidget* m_rightDock;
    QDockWidget* m_topDock;

	/// @}

};


///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespacing
}
}

#endif