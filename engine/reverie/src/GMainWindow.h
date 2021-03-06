// See:
// https://doc.qt.io/qtvstools/qtvstools-managing-projects.html
// https://doc.qt.io/qtvstools/qtvstools-getting-started.html

#ifndef GB_MAIN_WINDOW_H
#define GB_MAIN_WINDOW_H

// QT
#include <QLabel>
#include <QAction>
#include <QMenu>

#include <QMainWindow>

// Internal
#include "ui_GbBlankWindow.h"
#include "core/GObject.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations  
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {
class CoreEngine; 

namespace View { 
    class GLWidget; 
	class WidgetManager;
    class TextEdit;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Classes  
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Class defining the main application window
/// @details The main window class is used when there are docked windows, toolbars, and status bars
/// @note To avoid a border around internal widgets, a basic QWidget will need to be used
class MainWindow : public QMainWindow, public rev::Object
{
	Q_OBJECT

public:
	MainWindow(int argc, char *argv[], QWidget *parent = Q_NULLPTR);
	~MainWindow();

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @property DefaultTitle
    const QString& getDefaultTitle() const { return m_defaultTitle; }

    /// @}


    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "MainWindow"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "rev::MainWindow"; }
    /// @}

protected:

    void closeEvent(QCloseEvent* event);
    void resizeEvent(QResizeEvent* event);
    void showEvent(QShowEvent* event);

private slots:
    /// @brief Slots to respond to the user activating menu entries
    void newScenario();
    void openScenario();
    void saveScenario();
    void saveScenarioAs();
    void showUndoView();
    void showScriptOrderWidget();
    void showRenderLayers();
    void showShaderPresetsWidget();
    void scenarioPreferences();
    void aboutGrandBlue();

protected:

#ifndef QT_NO_CONTEXTMENU
    /// @brief Generates a context menu, overriding default implementation
    /// @note Context menus can be executed either asynchronously using the popup() function or 
    ///       synchronously using the exec() function
    void contextMenuEvent(QContextMenuEvent *event) override;
#endif // QT_NO_CONTEXTMENU

protected:
	//--------------------------------------------------------------------------------------------
	/// @name Friends
	/// @{
	friend class View::WidgetManager;

	/// @}

	//--------------------------------------------------------------------------------------------
	/// @name Protected Methods
	/// @{

    virtual void mouseMoveEvent(QMouseEvent* event) override;

    /// @brief checkValidity the menu bar
    /// @note See: https://doc.qt.io/qt-5/qtwidgets-mainwindows-menus-example.html
    void initializeMenus();

    /// @brief Initialize menu bar actions
    void initializeActions();

    /// @brief Display status bar
    void displayStatusBar();

    /// @brief Enable/Disable actions
    void enableScenarioActions();
    void disableScenarioActions();

	/// @}

	//--------------------------------------------------------------------------------------------
	/// @name Protected members
	/// @{
	// UI
	Ui::GbBlankWindow* m_ui;

    // Core engine
    rev::CoreEngine* m_engine;

    /// @brief Label for displaying info across GL display
    QLabel* m_infoLabel = nullptr;

    QString m_defaultTitle;

    QWidget* m_scriptOrderWidget = nullptr;
    QWidget* m_renderLayerWidget = nullptr;
    QWidget* m_shaderPresetsWidget = nullptr;

    /// @brief Widgets for file settings
    View::TextEdit* m_searchPathsWidget = nullptr;

    /// @brief Menu bar menus
    QMenu* m_fileMenu;
    QMenu* m_editMenu;
    QMenu* m_assetMenu;
    //QMenu* m_resourcesMenu;
    QMenu* m_settingsMenu;
    QMenu* m_helpMenu;

    /// @brief Menu bar actions
    /// @note a QActionGroup can be used to group actions such that only one can be active at a time
    /// See: https://doc.qt.io/qt-5/qtwidgets-mainwindows-menus-example.html
    QMenuBar* m_menuBar;
    QAction* m_newScenario;
    QAction* m_openScenario;
    QAction* m_saveScenario;
    QAction* m_saveScenarioAs;
    QAction* m_loadTexture;
    QAction* m_addMaterial;
    QAction* m_addModel;
    QAction* m_loadModel;
    QAction* m_addMesh;
    QAction* m_loadAudioFile;
    QAction* m_loadShaderProgram;
    QAction* m_serializeAssets;
    //QAction* m_addScript;
    QAction* m_undo;
    QAction* m_redo;
    QAction* m_showUndoView;
    QAction* m_showShaderPresets;
    QAction* m_showScriptOrderSettings;
    QAction* m_showRenderLayers;
    QAction* m_scenarioPreferences;
    QAction* m_aboutGrandBlue;


	/// @}
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespacing
}



#endif