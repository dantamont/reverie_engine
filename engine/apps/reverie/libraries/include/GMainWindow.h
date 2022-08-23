#pragma once

/// @see https://doc.qt.io/qtvstools/qtvstools-managing-projects.html
/// @see https://doc.qt.io/qtvstools/qtvstools-getting-started.html

// QT
#include <QLabel>
#include <QAction>
#include <QMenu>

#include <QMainWindow>

// Internal
#include "ui_GbBlankWindow.h"
namespace rev {

class CoreEngine; 
class GLWidget; 
class WidgetManager;
class TextEdit;


/// @brief Class defining the main application window
/// @details The main window class is used when there are docked windows, toolbars, and status bars
/// @note To avoid a border around internal widgets, a basic QWidget will need to be used
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(int argc, char *argv[], QWidget *parent = Q_NULLPTR);
	~MainWindow();

    /// @name Properties
    /// @{

    /// @property DefaultTitle
    const QString& getDefaultTitle() const { return m_defaultTitle; }

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
    void showSpriteSheetWidget();
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
	/// @name Friends
	/// @{
	friend class WidgetManager;

	/// @}

	/// @name Protected Methods
	/// @{

    virtual void mouseMoveEvent(QMouseEvent* event) override;

    /// @brief Initialize widgets
    void initializeWidgets();

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
    QWidget* m_spriteSheetWidget = nullptr;

    /// @brief Widgets for file settings
    TextEdit* m_searchPathsWidget = nullptr;

    /// @brief Menu bar menus
    QMenu* m_fileMenu;
    QMenu* m_editMenu;
    QMenu* m_assetMenu;
    QMenu* m_toolsMenu;
    QMenu* m_settingsMenu;
    QMenu* m_helpMenu;

    /// @brief Menu bar actions
    /// @note a QActionGroup can be used to group actions such that only one can be active at a time
    /// See: https://doc.qt.io/qt-5/qtwidgets-mainwindows-menus-example.html
    // File
    QMenuBar* m_menuBar;
    QAction* m_newScenario;
    QAction* m_openScenario;
    QAction* m_saveScenario;
    QAction* m_saveScenarioAs;

    // Edit
    QAction* m_undo;
    QAction* m_redo;
    QAction* m_showUndoView;

    // Assets
    QAction* m_loadTexture;
    QAction* m_addMaterial;
    QAction* m_addModel;
    QAction* m_loadModel;
    QAction* m_addMesh;
    QAction* m_loadAudioFile;
    QAction* m_loadShaderProgram;
    QAction* m_serializeAssets;
    QAction* m_showShaderPresets;

    // Tools
    QAction* m_createSpriteMaterial;

    // Settings
    QAction* m_showScriptOrderSettings;
    QAction* m_showRenderLayers;
    QAction* m_scenarioPreferences;
    QAction* m_aboutGrandBlue;

#ifdef DEBUG_MODE

    QAction* m_test1;

#endif


	/// @}
};

// End namespacing
}
