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
#include "../core/GManager.h"

namespace rev {

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
class ResourceTreeWidget;
class SceneTreeWidget;
class ComponentTreeWidget;
class PlayerControls;
class ParameterWidget;
class ResourceListView;
class BlueprintListView;

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
	WidgetManager(rev::CoreEngine* core, rev::MainWindow* window);
	~WidgetManager();

	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    rev::View::BlueprintListView* blueprintWidget() const { return m_blueprintView; }

    QMutex& updateMutex() { return m_updateLock; }

    /// @brief GL Widgets
    tsl::robin_map<QString, rev::View::GLWidget*>& glWidgets() { return m_glWidgets; }

    /// @brief Returns the main window for the application
    MainWindow* mainWindow() { return m_mainWindow; }

    /// @brief Returns the scene tree widget
    rev::View::SceneTreeWidget* sceneTreeWidget() { return m_sceneTreeWidget; }

    rev::View::ResourceTreeWidget* resourceTreeWidget() { return m_resourceTreeWidget; }

    /// @brief Returns the component widget
    rev::View::ComponentTreeWidget* componentWidget() { return m_componentWidget; }


    /// @brief The widget for controlling scene playback
    rev::View::PlayerControls* playbackWidget() { return m_playbackWidget; }

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    void addParameterWidget(ParameterWidget* widget);
    void removeParameterWidget(ParameterWidget* widget);

    /// @brief Called on main window resize
    void resize(int w, int h);

    /// @brief Get pointer to main GL widget
    rev::View::GLWidget* mainGLWidget() { return m_glWidgets["main"]; }

    /// @brief Clear the widget manager
    void clear();

    /// @brief Update the values displayed in widgets
    void update();

    /// @brief Called after construction of the manager
    virtual void postConstruction() override;


	/// @}


protected:
	//--------------------------------------------------------------------------------------------
	/// @name Friends
	/// @{

	friend class rev::MainWindow;
    friend class rev::View::ParameterWidget;

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
	rev::MainWindow* m_mainWindow;

	/// @brief Map of GL widgets
	tsl::robin_map<QString, rev::View::GLWidget*> m_glWidgets;

    /// @brief Map of all parameter widgets
    tsl::robin_map<Uuid, View::ParameterWidget*> m_parameterWidgets;

    /// @brief Resource widgets
    rev::View::ResourceTreeWidget* m_resourceTreeWidget;
    rev::View::ResourceListView* m_resourceListView;
    rev::View::BlueprintListView* m_blueprintView;

    /// @brief scene object tree widget
    rev::View::SceneTreeWidget* m_sceneTreeWidget;

    /// @brief scene object inspector widget
    rev::View::ComponentTreeWidget* m_componentWidget;

    /// @brief scene playback widget
    rev::View::PlayerControls* m_playbackWidget;

    /// @brief Docks in the main window
    QDockWidget* m_sceneDock;
    QDockWidget* m_resourceTreeDock;
    QDockWidget* m_bottomDock;
    QDockWidget* m_rightDock;
    QDockWidget* m_topDock;

    QMutex m_updateLock;

	/// @}

};


///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespacing
}
}

#endif