#include "GMainWindow.h"

#include <QFileDialog>

#include "core/GCoreEngine.h"
#include "geppetto/qt/layer/types/GQtConverter.h"
#include "core/resource/GResourceCache.h"
#include "core/debugging/GDebugManager.h"
#include "core/resource/GFileManager.h"
#include "core/layer/view/widgets/graphics/GGLWidget.h"
#include "geppetto/qt/actions/GActionManager.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/general/GJsonWidget.h"
#include "geppetto/qt/widgets/tree/GResourceWidgets.h"
#include "geppetto/qt/widgets/tree/GShaderTreeWidget.h"
#include "geppetto/qt/widgets/tree/GScriptOrder.h"
#include "geppetto/qt/widgets/tree/GRenderLayerWidget.h"
#include "geppetto/qt/style/GFontIcon.h"
#include "geppetto/qt/widgets/general/GTextEdit.h"
#include "geppetto/qt/widgets/graphics/GSpriteSheetWidget.h"
#include "core/loop/GSimLoop.h"
#include "core/scene/GScenario.h"
#include "core/processes/GProcessManager.h"

#ifdef DEBUG_MODE
#include <core/scene/GSceneObject.h>
#include <core/components/GLightComponent.h>
#endif

namespace rev {


MainWindow::MainWindow(int argc, char *argv[], QWidget *parent)
	: QMainWindow(parent),
	m_ui(new Ui::GbBlankWindow),
    m_defaultTitle("Reverie Engine")
{
    // Set window title
    setWindowTitle(m_defaultTitle);

    // Initialize engine
    m_engine = new CoreEngine(argc, argv);
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
    initializeWidgets();
    m_engine->simulationLoop()->initializeConnections();

    // Initialize engine connections
    m_engine->initializeConnections();

#ifdef DEVELOP_MODE
    // Initialize menu actions
    initializeActions();

    // Initialize menus
    initializeMenus();

    // Display status message about context menu 
    displayStatusBar();
#endif

    // Set main window to track mouse
    //setMouseTracking(true);
}


MainWindow::~MainWindow()
{
	delete m_ui;
    delete m_shaderPresetsWidget;
    delete m_scriptOrderWidget;
    delete m_spriteSheetWidget;
    delete m_engine;
}


void MainWindow::closeEvent(QCloseEvent * event)
{
    event;
    QCoreApplication::quit();
}


void MainWindow::resizeEvent(QResizeEvent * event)
{
    event;
    int w = width();
    int h = height();
    
    m_engine->widgetManager()->resize(w, h);
}


void MainWindow::showEvent(QShowEvent * event)
{
    event;
}

void MainWindow::newScenario()
{
    // Disable all actions that would interfere
    disableScenarioActions();

#ifdef DEBUG_MODE
    if (m_infoLabel) {
        m_infoLabel->setText(tr("Invoked <b>File|New Scenario</b>"));
    }
#endif

    // Clear the engine
    m_engine->clearEngine(true);

    // Set the scenario in the main renderer
    m_engine->setNewScenario();

    // Renable actions
    enableScenarioActions();
}

void MainWindow::openScenario()
{
    // Disable all actions that would interfere
    disableScenarioActions();

#ifdef DEBUG_MODE
    if (m_infoLabel) {
        m_infoLabel->setText(tr("Invoked <b>File|Open Scenario</b>"));
    }
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
        m_engine->clearEngine(true, true);

        Scenario::LoadFromFile(filepath.toStdString(), m_engine);
        GApplicationSettings& settings = GApplicationSettings::Instance();
        settings.setRecentProject(QConverter::FromQt(filepath));

        // Set window title
        setWindowTitle(m_defaultTitle + " " + filepath);
    }

    // Renable actions that would interfere
    enableScenarioActions();
}

void MainWindow::saveScenario()
{
    // Disable all actions that would interfere
    disableScenarioActions();

#ifdef DEBUG_MODE
    if (m_infoLabel) {
        m_infoLabel->setText(tr("Invoked <b>File|Save Scenario</b>"));
    }
#endif
    // Save scenario to a file
    QString filepath = (QString)m_engine->scenario()->getPath();
    if (!filepath.isEmpty()) {
        // Save
        bool saved = m_engine->scenario()->save();

        // Save recent project in settings
        if (saved) {
            GApplicationSettings& settings = GApplicationSettings::Instance();
            settings.setRecentProject(QConverter::FromQt(filepath));
        }
        else {
            Logger::Throw("Error, failed to save scenario file to " + m_engine->scenario()->getPath());
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

void MainWindow::saveScenarioAs()
{
    // Disable all actions that would interfere
    disableScenarioActions();

#ifdef DEBUG_MODE
    if (m_infoLabel) {
        m_infoLabel->setText(tr("Invoked <b>File|Save Scenario As...</b>"));
    }
#endif
    // Launch file dialog
    QString filter = "Scenarios (*.scn *.json)";
    QString filepath = QFileDialog::getSaveFileName(this,
        tr("Save Scenario"),
        "",
        filter);

    // Save scenario as...
    if (!filepath.isEmpty()) {
        GString gFilePath(filepath.toStdString());
        bool saved = m_engine->scenario()->save(gFilePath);

        if (!saved) {
            Logger::Throw("Error, failed to save scenario file to " + gFilePath);
        }

        // Save recent project in settings
        GApplicationSettings& settings = GApplicationSettings::Instance();
        settings.setRecentProject(QConverter::FromQt(filepath));

        // Set window title
        setWindowTitle(m_defaultTitle + " " + filepath);

        if (m_engine->scenario()->getPath() != gFilePath) {
            Logger::Throw("Error, scenario filepath " + m_engine->scenario()->getPath() + " is not equal to " + gFilePath);
        }
    }

    // Reenable actions
    enableScenarioActions();
}

void MainWindow::showUndoView()
{
#ifdef DEBUG_MODE
    if (m_infoLabel) {
        m_infoLabel->setText(tr("Invoked <b>Edit|Show Undo View</b>"));
    }
#endif

    m_engine->actionManager()->showUndoView();
}

void MainWindow::showScriptOrderWidget()
{
    if (!m_scriptOrderWidget) {
        m_scriptOrderWidget = new QWidget();
        ScriptOrderTreeWidget* child = new ScriptOrderTreeWidget(m_engine->widgetManager());
        auto* layout = new QVBoxLayout();;
        layout->addWidget(child);

        m_scriptOrderWidget->setLayout(layout);
        //child->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        m_scriptOrderWidget->setMinimumHeight(800);
        m_scriptOrderWidget->setWindowTitle("Script Execution Order");
    }
    m_scriptOrderWidget->show();
}

void MainWindow::showRenderLayers()
{
    if (!m_renderLayerWidget) {
        m_renderLayerWidget = new QWidget();
        RenderLayerTreeWidget* child = new RenderLayerTreeWidget(m_engine->widgetManager());
        auto* layout = new QVBoxLayout();;
        layout->addWidget(child);

        m_renderLayerWidget->setLayout(layout);
        //child->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        m_renderLayerWidget->setMinimumHeight(800);
        m_renderLayerWidget->setWindowTitle("Render Layers");
    }
    m_renderLayerWidget->show();
}

void MainWindow::showShaderPresetsWidget()
{
    if (!m_shaderPresetsWidget) {
        m_shaderPresetsWidget = new QWidget();
        ShaderTreeWidget* child = new ShaderTreeWidget(m_engine->widgetManager());
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

void MainWindow::showSpriteSheetWidget()
{
    if (!m_spriteSheetWidget) {
        // Create a new sprite sheet widget, ensuring it is destroyed on close
        m_spriteSheetWidget = new SpriteSheetWidget(m_engine->widgetManager());
        m_spriteSheetWidget->setMinimumHeight(400);
        m_spriteSheetWidget->setWindowTitle("Sprite Sheet Generator");
        m_spriteSheetWidget->setAttribute(Qt::WA_DeleteOnClose, true);
        connect(m_spriteSheetWidget,
            &QWidget::destroyed, 
            this, 
            [this]() {m_spriteSheetWidget = nullptr; });
    }
    m_spriteSheetWidget->show();
    m_spriteSheetWidget->activateWindow();
    m_spriteSheetWidget->raise();
}

void MainWindow::scenarioPreferences()
{
#ifdef DEBUG_MODE
    if (m_infoLabel) {
        m_infoLabel->setText(tr("Invoked <b>Settings|Preferences</b>"));
    }
#endif

    // Create window and tab widgets
    QWidget* settingsWidget = new QWidget(this);
    QTabWidget* tabWidget = new QTabWidget();

    // Create file tab widget
    QWidget* fileWidget = new QWidget(settingsWidget);
    QVBoxLayout* fileLayout = new QVBoxLayout();
    m_searchPathsWidget = new TextEdit(fileWidget);
    m_searchPathsWidget->setText((QString)m_engine->fileManager()->searchPathString());
    fileLayout->addLayout(ParameterWidget::LabeledLayout("Search Paths:", m_searchPathsWidget));
    connect(m_searchPathsWidget, &TextEdit::editingFinished, this,
        [this]() {
        std::string str = m_searchPathsWidget->toPlainText().toStdString();
        m_engine->fileManager()->clear(false);
        m_engine->fileManager()->addSearchPath(str.c_str());
    });
    fileWidget->setLayout(fileLayout);

    tabWidget->addTab(fileWidget, SAIcon("folder-open"), "Directories");

    // Create debug tab widgets
    JsonWidget* debugWidget = new JsonWidget(
        m_engine->widgetManager(), 
        m_engine->debugManager()->debugSettings(), 
        { {"isDebugSettings", true} },
        settingsWidget,
        true);
    tabWidget->addTab(debugWidget, SAIcon("stethoscope"), "Debug");

    // Layout all tabs
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(tabWidget);

    // Make widget a window
    settingsWidget->setLayout(layout);
    settingsWidget->setWindowFlags(Qt::Window);
    settingsWidget->show();
}

void MainWindow::aboutGrandBlue()
{
#ifdef DEBUG_MODE
    if (m_infoLabel) {
        m_infoLabel->setText(tr("Invoked <b>Help|About Reverie</b>"));
    }
#endif
    QMessageBox::about(this, tr("About Reverie"), 
        tr("The <b>Reverie Engine</b> is a QT-based engine for modeling and simulation."));
}

#ifndef QT_NO_CONTEXTMENU
void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(m_scenarioPreferences);
    menu.addAction(m_aboutGrandBlue);
    menu.exec(event->globalPos());
}
#endif // QT_NO_CONTEXTMENU

void MainWindow::mouseMoveEvent(QMouseEvent * event)
{
    //Object().logInfo("Moved mouse main window");
    return QMainWindow::mouseMoveEvent(event);
}

void MainWindow::initializeWidgets()
{
    m_engine->widgetManager()->initializeGLWidget(new GLWidget("main", m_engine, this));
    m_engine->widgetManager()->initializeDefault();
}

void MainWindow::initializeMenus()
{
   // Toggle menu bar on
   m_menuBar = menuBar();
   m_menuBar->show();

   // Create connections to disable menu bar
   // Could have used setEnabled here as well
   connect(&ResourceCache::Instance(), 
       &ResourceCache::startedLoadingResources, // disable on resource load
       m_menuBar,
       [this]() {
       m_menuBar->setDisabled(true);
   });

   connect(&ResourceCache::Instance(),
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

   // Create an asset menu
   m_assetMenu = m_menuBar->addMenu("&Assets");
   QMenu* addMenu = m_assetMenu->addMenu("&Add");
   addMenu->addAction(m_addMesh);
   addMenu->addAction(m_addMaterial);
   addMenu->addAction(m_addModel);

   QMenu* loadMenu = m_assetMenu->addMenu("&Load");
   loadMenu->addAction(m_loadTexture);
   loadMenu->addAction(m_loadModel);
   loadMenu->addAction(m_loadShaderProgram);
   loadMenu->addAction(m_loadAudioFile);

   m_assetMenu->addSeparator();
   m_assetMenu->addAction(m_serializeAssets);

   m_assetMenu->addSeparator();
   m_assetMenu->addAction(m_showShaderPresets);

   // Create a Tools menu
   m_toolsMenu = m_menuBar->addMenu("&Tools");
   m_toolsMenu->addAction(m_createSpriteMaterial);

   // Create a settings menu
   m_settingsMenu = m_menuBar->addMenu("&Settings");
   m_settingsMenu->addAction(m_showScriptOrderSettings);
   m_settingsMenu->addAction(m_showRenderLayers);
   m_settingsMenu->addAction(m_scenarioPreferences);

   // Create a help menu
   m_helpMenu = m_menuBar->addMenu("&Help");
   m_helpMenu->addAction(m_aboutGrandBlue);

#ifdef DEBUG_MODE
   QMenu* testMenu = m_menuBar->addMenu("&Testing");
   testMenu->addAction(m_test1);
#endif

}

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
    m_loadTexture = new QAction(tr("&Load Texture"), this);
    m_loadTexture->setStatusTip("Load a texture into the scenario");
    connect(m_loadTexture,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new LoadTextureCommand(m_engine->widgetManager(), "Load Texture"));
        }
    );

    m_addMaterial = new QAction(tr("&Add Material"), this);
    m_addMaterial->setStatusTip("Add an empty material to the scenario");
    connect(m_addMaterial,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddMaterialCommand(m_engine->widgetManager(), "Add Material"));
        }
    );

    m_addModel = new QAction(tr("&Add Model"), this);
    m_addModel->setStatusTip("Create a new model in the current scenario");
    connect(m_addModel,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddModelCommand(m_engine->widgetManager(), "Add Model"));
        }
    );

    m_loadModel = new QAction(tr("&Load Model"), this);
    m_loadModel->setStatusTip("Load a model into the scenario");
    connect(m_loadModel,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new LoadModelCommand(m_engine->widgetManager(), "Load Model"));
        }
    );


    m_addMesh = new QAction(tr("&Add Mesh"), this);
    m_addMesh->setStatusTip("Create a mesh in the current scenario");
    connect(m_addMesh,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddMeshCommand(m_engine->widgetManager(), "Add Mesh"));
        }
    );

    m_loadShaderProgram = new QAction(tr("&Load Shader Program"), this);
    m_loadShaderProgram->setStatusTip("Load a shader program into the scenario");
    connect(m_loadShaderProgram,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddShaderCommand(m_engine->widgetManager(), "Load Shader Program"));
        }
    );

    m_loadAudioFile = new QAction(tr("&Load Audio File"), this);
    m_loadAudioFile->setStatusTip("Load an audio file into the scenario");
    connect(m_loadAudioFile,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new LoadAudioCommand(m_engine->widgetManager(), "Load Audio File"));
        }
    );

    m_serializeAssets = new QAction(tr("&Serialize Assets"), this);
    m_serializeAssets->setStatusTip("Convert all assets to binary files, and reassign paths.");
    connect(m_serializeAssets,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {
            // TODO: Implement
            Logger::LogWarning("Unimplemented");
        }
    );

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
    m_showShaderPresets = new QAction(tr("&Edit Shader Presets"), this);
    m_showShaderPresets->setStatusTip("Modify the shader presets in this scenario.");
    connect(m_showShaderPresets, &QAction::triggered, this, &MainWindow::showShaderPresetsWidget);

    // Display a window for creating sprite materials
    m_createSpriteMaterial = new QAction(tr("&Create a Sprite Material"), this);
    m_createSpriteMaterial->setStatusTip("Create a material representing a sprite sheet.");
    connect(m_createSpriteMaterial, &QAction::triggered, this, &MainWindow::showSpriteSheetWidget);


    // Display window for script execution settings
    m_showScriptOrderSettings = new QAction(tr("&Script Execution Order"), this);
    m_showScriptOrderSettings->setStatusTip("Modify the sorting layers that affect script execution order.");
    connect(m_showScriptOrderSettings, &QAction::triggered, this, &MainWindow::showScriptOrderWidget);

    // Display window for render layers
    m_showRenderLayers = new QAction(tr("&Rendering Layers"), this);
    m_showRenderLayers->setStatusTip("Modify the rendering layers that match cameras to renderable objects.");
    connect(m_showRenderLayers, &QAction::triggered, this, &MainWindow::showRenderLayers);

    // Create preferences action
    m_scenarioPreferences = new QAction(tr("&Scenario Settings"), this);
    m_scenarioPreferences->setShortcuts(QKeySequence::Preferences);
    m_scenarioPreferences->setStatusTip("View settings for the current scenario");
    connect(m_scenarioPreferences, &QAction::triggered, this, &MainWindow::scenarioPreferences);

    // Create about action
    m_aboutGrandBlue = new QAction(tr("&About"), this);
    m_aboutGrandBlue->setStatusTip("About Reverie");
    connect(m_aboutGrandBlue, &QAction::triggered, this, &MainWindow::aboutGrandBlue);

#ifdef DEBUG_MODE
    m_test1 = new QAction(tr("&Test 1"), this);
    m_test1->setStatusTip("Perform test 1.");
    connect(m_test1, &QAction::triggered, this, 
        [&]() {
            for (const std::shared_ptr<SceneObject>& so : m_engine->scenario()->scene().topLevelSceneObjects())
            {
                if (LightComponent* light = so->getComponent<LightComponent>(ComponentType::kLight)) {
                    //light->updateLight();
                    if (/*light->castsShadows()*/ light->sceneObject()->getName().contains("cyan")) {
                        if (light->isEnabled()) {
                            //light->disableShadowCasting(true);
                            //light->updateShadowMap();
                            //light->updateShadow();
                            //light->enableShadowCasting(true);

                            //int startIndex = NUM_SHADOWS_PER_LIGHT_TYPE * light->m_cachedLight.getType();
                            //int mapIndex = light->m_cachedShadow.m_mapIndexBiasesFarClip[0] + startIndex;
                            //light->lightingSettings().removeShadow(light->m_shadowMap.get(), mapIndex);
                        
                            // Reset shadow map
                            //light->m_shadowMap = nullptr;

                            // Set shadow value in SSBO
                            //light->m_cachedShadow.m_mapIndexBiasesFarClip[0] = -1;
                        
                        }
                    //light->disable();
                    }
                    //light->enable();
                }
            }
        }
        );
#endif
}

void MainWindow::displayStatusBar()
{
    QString message = tr("A context menu is available by right-clicking");
    statusBar()->showMessage(message);
}

void MainWindow::enableScenarioActions()
{
    m_newScenario->setEnabled(true);
    m_openScenario->setEnabled(true);
    m_saveScenario->setEnabled(true);
    m_saveScenarioAs->setEnabled(true);
}

void MainWindow::disableScenarioActions()
{
    m_newScenario->setEnabled(false);
    m_openScenario->setEnabled(false);
    m_saveScenario->setEnabled(false);
    m_saveScenarioAs->setEnabled(false);
}



// End namespacing
}
