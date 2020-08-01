#include "GbMainWindow.h"

#include <QFileDialog>

#include "core/GbCoreEngine.h"
#include "core/resource/GbResourceCache.h"
#include "model_control/commands/GbActionManager.h"
#include "view/GbWidgetManager.h"
#include "view/GL/GbGLWidget.h"
#include "view/tree/GbResourceWidgets.h"
#include "view/tree/GbShaderTreeWidget.h"
#include "view/tree/GbScriptOrder.h"
#include "view/tree/GbRenderLayerWidget.h"
#include "core/loop/GbSimLoop.h"
#include "core/scene/GbScenario.h"
#include "core/processes/GbProcess.h"
#include "core/processes/GbProcessManager.h"

namespace Gb {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent),
	m_ui(new Ui::GbBlankWindow),
    m_defaultTitle("Reverie Engine")
{
    // Set window title
    setWindowTitle(m_defaultTitle);

    // Initialize engine
    m_engine = new Gb::CoreEngine();
    m_engine->initialize(this);

    // Configure UI
	m_ui->setupUi(this);

    // Configure the main window
    setCorner(Qt::TopLeftCorner, Qt::TopDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::TopDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::BottomDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);
    setMinimumSize(160, 160);

    // Initialize widgets
    m_engine->widgetManager()->initializeDefault();
    m_engine->simulationLoop()->initializeConnections();

    // Initialize engine connections
    m_engine->initializeConnections();

#ifdef DEBUG_MODE
    // Initialize menu actions
    initializeActions();

    // Initialize menus
    initializeMenus();

    // Display status message about context menu 
    displayStatusBar();
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MainWindow::~MainWindow()
{
	delete m_ui;
    m_engine->deleteLater();
    delete m_shaderPresetsWidget;
    delete m_scriptOrderWidget;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::closeEvent(QCloseEvent * event)
{
    event;
    QCoreApplication::quit();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::resizeEvent(QResizeEvent * event)
{
    event;
    int w = width();
    int h = height();
    
    m_engine->widgetManager()->resize(w, h);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::showEvent(QShowEvent * event)
{
    event;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::newScenario()
{
    // Disable all actions that would interfere
    disableScenarioActions();

#ifdef DEBUG_MODE
    m_infoLabel->setText(tr("Invoked <b>File|New Scenario</b>"));
#endif

    // Clear the engine
    m_engine->clearEngine(true);

    // Set the scenario in the main renderer
    m_engine->setNewScenario();

    // Renable actions
    enableScenarioActions();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::openScenario()
{
    // Disable all actions that would interfere
    disableScenarioActions();

#ifdef DEBUG_MODE
    m_infoLabel->setText(tr("Invoked <b>File|Open Scenario</b>"));
#endif

    // Launch file dialog
    QString filter = "Scenarios (*.scn *.json)";
    QString filepath = QFileDialog::getOpenFileName(this, 
        tr("Open Scenario"),
        "",
        filter);

    // Open scenario from a file
    if (!filepath.isEmpty()) {
        // Clear the engine
        m_engine->clearEngine(true);

        Scenario::loadFromFile(filepath, m_engine);
        auto* settings = new Gb::Settings::INISettings();
        settings->setRecentProject(filepath);

        // Set window title
        setWindowTitle(m_defaultTitle + " " + filepath);
        delete settings;
    }

    // Renable actions that would interfere
    enableScenarioActions();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::saveScenario()
{
    // Disable all actions that would interfere
    disableScenarioActions();

#ifdef DEBUG_MODE
    m_infoLabel->setText(tr("Invoked <b>File|Save Scenario</b>"));
#endif
    // Save scenario to a file
    const QString& filepath = m_engine->scenario()->getPath();
    if (!filepath.isEmpty()) {
        // Save
        bool saved = m_engine->scenario()->save();

        // Save recent project in settings
        if (saved) {
            auto* settings = new Gb::Settings::INISettings();
            settings->setRecentProject(filepath);
            delete settings;
        }
        else {
            throw("Error, failed to save scenario file to " + m_engine->scenario()->getPath());
        }

        // Set window title
        setWindowTitle(m_defaultTitle + " " + filepath);
    }
    else {
        // Save as
        saveScenarioAs();
    }

    // Reenable actions
    enableScenarioActions();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::saveScenarioAs()
{
    // Disable all actions that would interfere
    disableScenarioActions();

#ifdef DEBUG_MODE
    m_infoLabel->setText(tr("Invoked <b>File|Save Scenario As...</b>"));
#endif
    // Launch file dialog
    QString filter = "Scenarios (*.scn *.json)";
    QString filepath = QFileDialog::getSaveFileName(this,
        tr("Save Scenario"),
        "",
        filter);

    // Save scenario as...
    if (!filepath.isEmpty()) {
        bool saved = m_engine->scenario()->save(filepath);

        if (!saved) {
            throw("Error, failed to save scenario file to " + filepath);
        }

        // Save recent project in settings
        auto* settings = new Gb::Settings::INISettings();
        settings->setRecentProject(filepath);
        delete settings;

        // Set window title
        setWindowTitle(m_defaultTitle + " " + filepath);

        if (m_engine->scenario()->getPath() != filepath) {
            throw("Error, scenario filepath " + m_engine->scenario()->getPath() + " is not equal to " + filepath);
        }
    }

    // Reenable actions
    enableScenarioActions();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::showUndoView()
{
#ifdef DEBUG_MODE
    m_infoLabel->setText(tr("Invoked <b>Edit|Show Undo View</b>"));
#endif

    m_engine->actionManager()->showUndoView();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::showScriptOrderWidget()
{
    if (!m_scriptOrderWidget) {
        m_scriptOrderWidget = new QWidget();
        View::ScriptOrderTreeWidget* child = new View::ScriptOrderTreeWidget(m_engine);
        auto* layout = new QVBoxLayout();;
        layout->addWidget(child);

        m_scriptOrderWidget->setLayout(layout);
        //child->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        m_scriptOrderWidget->setMinimumHeight(800);
        m_scriptOrderWidget->setWindowTitle("Script Execution Order");
    }
    m_scriptOrderWidget->show();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::showRenderLayers()
{
    if (!m_renderLayerWidget) {
        m_renderLayerWidget = new QWidget();
        View::RenderLayerTreeWidget* child = new View::RenderLayerTreeWidget(m_engine);
        auto* layout = new QVBoxLayout();;
        layout->addWidget(child);

        m_renderLayerWidget->setLayout(layout);
        //child->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        m_renderLayerWidget->setMinimumHeight(800);
        m_renderLayerWidget->setWindowTitle("Render Layers");
    }
    m_renderLayerWidget->show();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::showShaderPresetsWidget()
{
    if (!m_shaderPresetsWidget) {
        m_shaderPresetsWidget = new QWidget();
        View::ShaderTreeWidget* child = new View::ShaderTreeWidget(m_engine);
        auto* layout = new QVBoxLayout();;
        layout->addWidget(child);

        m_shaderPresetsWidget->setLayout(layout);
        //child->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        m_shaderPresetsWidget->setMinimumHeight(800);
        m_shaderPresetsWidget->setWindowTitle("Shader Presets");
    }
    m_shaderPresetsWidget->show();
    m_shaderPresetsWidget->activateWindow();
    m_shaderPresetsWidget->raise();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::viewPreferences()
{
#ifdef DEBUG_MODE
    m_infoLabel->setText(tr("Invoked <b>Settings|Preferences</b>"));
#endif
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::aboutGrandBlue()
{
#ifdef DEBUG_MODE
    m_infoLabel->setText(tr("Invoked <b>Help|About Grand Blue</b>"));
#endif
    QMessageBox::about(this, tr("About Grand Blue"), 
        tr("The <b>Grand Blue Engine</b> is a QT-based engine for modelling and simulation."));
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef QT_NO_CONTEXTMENU
void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(m_viewPreferences);
    menu.addAction(m_aboutGrandBlue);
    menu.exec(event->globalPos());
}
#endif // QT_NO_CONTEXTMENU

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::initializeMenus()
{
   // Toggle menu bar on
   m_menuBar = menuBar();
   m_menuBar->show();

   // Create connections to disable menu bar
   connect(m_engine->resourceCache(), 
       &ResourceCache::startedLoadingResources, // disable on resource load
       m_menuBar,
       [this]() {
       m_menuBar->setDisabled(true);
   });

   connect(m_engine->resourceCache(),
       &ResourceCache::doneLoadingResources, // reenable when done loading
       m_menuBar,
       [this]() {
       m_menuBar->setDisabled(false);
   });

   // Create a file menu
   // The ampersand in the menu item's text sets Alt+F as a shortcut for this menu
   m_fileMenu = m_menuBar->addMenu("&File");
   m_fileMenu->addAction(m_newScenario);
   m_fileMenu->addAction(m_openScenario);
   m_fileMenu->addSeparator();
   m_fileMenu->addAction(m_saveScenario);
   m_fileMenu->addAction(m_saveScenarioAs);
   //m_fileMenu->addAction(m_addScript);

   // Create an edit menu
   m_editMenu = m_menuBar->addMenu("&Edit");
   m_editMenu->addAction(m_undo);
   m_editMenu->addAction(m_redo);
   m_editMenu->addAction(m_showUndoView);

   // Create an insert menu
   m_insertMenu = m_menuBar->addMenu("&Insert");
   m_insertMenu->addAction(m_addMesh);
   m_insertMenu->addSeparator();
   m_insertMenu->addAction(m_addTexture);
   m_insertMenu->addAction(m_addMaterial);
   m_insertMenu->addSeparator();
   m_insertMenu->addAction(m_addModel);
   m_insertMenu->addAction(m_loadModel);
   m_insertMenu->addSeparator();
   m_insertMenu->addAction(m_addShaderProgram);

   // Create a resources menu
   m_resourcesMenu = m_menuBar->addMenu("&Resources");
   m_resourcesMenu->addAction(m_showShaderPresets);

   // Create a settings menu
   m_settingsMenu = m_menuBar->addMenu("&Settings");
   m_settingsMenu->addAction(m_showScriptOrderSettings);
   m_settingsMenu->addAction(m_showRenderLayers);
   m_settingsMenu->addAction(m_viewPreferences);

   // Create a help menu
   m_helpMenu = m_menuBar->addMenu("&Help");
   m_helpMenu->addAction(m_aboutGrandBlue);

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::initializeActions()
{
    // Create new scenario action
    m_newScenario = new QAction(tr("&New Scenario"), this);
    m_newScenario->setShortcuts(QKeySequence::New);
    m_newScenario->setStatusTip("Create a new scenario");
    connect(m_newScenario, &QAction::triggered, this, &MainWindow::newScenario);

    // Create open scenario action
    m_openScenario = new QAction(tr("&Open Scenario"), this);
    m_openScenario->setShortcuts(QKeySequence::Open);
    m_openScenario->setStatusTip("Open a scenario");
    connect(m_openScenario, &QAction::triggered, this, &MainWindow::openScenario);

    // Create save scenario action
    m_saveScenario = new QAction(tr("&Save Scenario"), this);
    m_saveScenario->setShortcuts(QKeySequence::Save);
    m_saveScenario->setStatusTip("Save the current scenario");
    connect(m_saveScenario, &QAction::triggered, this, &MainWindow::saveScenario);

    // Create save scenario as action
    m_saveScenarioAs = new QAction(tr("&Save Scenario As"), this);
    m_saveScenarioAs->setShortcuts(QKeySequence::SaveAs);
    m_saveScenarioAs->setStatusTip("Save a scenario under a new file m_name");
    connect(m_saveScenarioAs, &QAction::triggered, this, &MainWindow::saveScenarioAs);

    // Initialize add actions
    m_addTexture = new QAction(tr("&Load Texture"), this);
    m_addTexture->setStatusTip("Load a texture into the scenario");
    connect(m_addTexture,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new LoadTextureCommand(m_engine,
                "Load Texture"));
    });

    m_addMaterial = new QAction(tr("&Add Material"), this);
    m_addMaterial->setStatusTip("Add an empty material to the scenario");
    connect(m_addMaterial,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddMaterialCommand(m_engine,
                "Add Material"));
    });

    m_addModel = new QAction(tr("&Add Model"), this);
    m_addModel->setStatusTip("Create a new model in the current scenario");
    connect(m_addModel,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddModelCommand(m_engine,
                "Add Model"));
    });

    m_loadModel = new QAction(tr("&Load Model"), this);
    m_loadModel->setStatusTip("Load a model into the scenario");
    connect(m_loadModel,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new LoadModelCommand(m_engine,
                "Load Model"));
    });


    m_addMesh = new QAction(tr("&Add Mesh"), this);
    m_addMesh->setStatusTip("Create a mesh in the current scenario");
    connect(m_addMesh,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddMeshCommand(m_engine, "Add Mesh"));
    });

    m_addShaderProgram = new QAction(tr("&Load Shader Program"), this);
    m_addShaderProgram->setStatusTip("Load a shader program into the scenario");
    connect(m_addShaderProgram,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddShaderCommand(m_engine,
                "Load Shader Program"));
    });

    // Create undo action
    m_undo = m_engine->actionManager()->undoStack()->createUndoAction(this, (tr("&Undo")));
    m_undo->setShortcuts(QKeySequence::Undo);
    m_undo->setStatusTip("Undo the user's most recent action");

    // Create redo action
    m_redo = m_engine->actionManager()->undoStack()->createRedoAction(this, (tr("&Redo")));
    m_redo->setShortcuts(QKeySequence::Redo);
    m_redo->setStatusTip("Redo the user's most recent undone action");

    // Create show undo view action
    m_showUndoView = new QAction(tr("&Show Recent Actions"), this);
    m_showUndoView->setStatusTip("Show the user's most recent actions");
    connect(m_showUndoView, &QAction::triggered, this, &MainWindow::showUndoView);

    // Display a window for shader presets
    m_showShaderPresets = new QAction(tr("&Shader Presets"), this);
    m_showShaderPresets->setStatusTip("Modify the shader presets in this scenario.");
    connect(m_showShaderPresets, &QAction::triggered, this, &MainWindow::showShaderPresetsWidget);


    // Display window for script execution settings
    m_showScriptOrderSettings = new QAction(tr("&Script Execution Order"), this);
    m_showScriptOrderSettings->setStatusTip("Modify the sorting layers that affect script execution order.");
    connect(m_showScriptOrderSettings, &QAction::triggered, this, &MainWindow::showScriptOrderWidget);

    // Display window for render layers
    m_showRenderLayers = new QAction(tr("&Rendering Layers"), this);
    m_showRenderLayers->setStatusTip("Modify the rendering layers that match cameras to renderable objects.");
    connect(m_showRenderLayers, &QAction::triggered, this, &MainWindow::showRenderLayers);

    // Create preferences action
    m_viewPreferences = new QAction(tr("&Preferences..."), this);
    m_viewPreferences->setShortcuts(QKeySequence::Preferences);
    m_viewPreferences->setStatusTip("View Grand Blue preferences");
    connect(m_viewPreferences, &QAction::triggered, this, &MainWindow::viewPreferences);

    // Create about action
    m_aboutGrandBlue = new QAction(tr("&About"), this);
    m_aboutGrandBlue->setStatusTip("About Grand Blue");
    connect(m_aboutGrandBlue, &QAction::triggered, this, &MainWindow::aboutGrandBlue);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::displayStatusBar()
{
    QString message = tr("A context menu is available by right-clicking");
    statusBar()->showMessage(message);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::enableScenarioActions()
{
    m_newScenario->setEnabled(true);
    m_openScenario->setEnabled(true);
    m_saveScenario->setEnabled(true);
    m_saveScenarioAs->setEnabled(true);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::disableScenarioActions()
{
    m_newScenario->setEnabled(false);
    m_openScenario->setEnabled(false);
    m_saveScenario->setEnabled(false);
    m_saveScenarioAs->setEnabled(false);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespacing
}
