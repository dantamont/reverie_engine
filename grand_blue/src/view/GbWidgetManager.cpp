#include "GbWidgetManager.h"

#include <QMenu>

//#include "../third_party/PythonQt/src/gui/PythonQtScriptingConsole.h"
#include "../../third_party/PythonQt/src/gui/PythonQtScriptingConsole.h"

#include "../GbMainWindow.h"

#include "GL/GbGLWidget.h"
#include "../core/rendering/renderer/GbMainRenderer.h"
#include "nodes/GbGraphWidget.h"
#include "tree/GbSceneTreeWidget.h"
#include "tree/GbResourceWidgets.h"
#include "tree/GbComponentWidget.h"
#include "players/GbPlayer.h"

#include "../core/GbCoreEngine.h"

namespace Gb {
namespace View {

//////////////////////////////////////////////////////////////////////////////////
WidgetManager::WidgetManager(Gb::CoreEngine* core, Gb::MainWindow* window) :
	Manager(core, "WidgetManager"),
	m_mainWindow(window),
    m_graphWidget(nullptr),
    m_sceneTreeWidget(nullptr),
    m_componentWidget(nullptr)
{
    //m_timer.setInterval(500);
    //auto widgetUpdateSlot = [&]() {
    //    m_engine->resourcesChanged();
    //};
    //connect(&m_timer, &QTimer::timeout, this, widgetUpdateSlot);
    //m_timer.start();
}
//////////////////////////////////////////////////////////////////////////////////
WidgetManager::~WidgetManager()
{
    delete mainGLWidget();
    delete m_graphWidget;
    delete m_sceneTreeWidget;
    delete m_componentWidget;
}
//////////////////////////////////////////////////////////////////////////////////
void WidgetManager::resize(int w, int h)
{
    w; h;
    //m_mainWindow->resizeDocks({ m_glDock }, { int(w) }, Qt::Horizontal);
    //m_mainWindow->resizeDocks({ m_glDock }, { int(h) }, Qt::Vertical);
}
//////////////////////////////////////////////////////////////////////////////////
void WidgetManager::clear()
{
    for (const std::pair<QString, Gb::View::GLWidget*>& widgetPair : m_glWidgets) {
        widgetPair.second->clear();
    }
}
//////////////////////////////////////////////////////////////////////////////////
void WidgetManager::initializeDefault()
{
    // Initialize GL widget
    m_glWidgets[QString::fromStdString("main")] = new GLWidget("main", m_engine, m_mainWindow);

    m_mainWindow->setCentralWidget(m_glWidgets["main"]);

#ifdef DEVELOP_MODE
    // Initialize scene tree widget
    m_sceneTreeWidget = new SceneTreeWidget(m_engine, "Scenario Widget");
    m_sceneDock = new QDockWidget("Scenario Tree View", m_mainWindow);
    m_sceneDock->setWidget(m_sceneTreeWidget);

    m_resourceTreeWidget = new ResourceTreeWidget(m_engine, "Resource Widget");
    m_resourceDock = new QDockWidget("Resource View", m_mainWindow);
    m_resourceDock->setWidget(m_resourceTreeWidget);

    m_mainWindow->addDockWidget(Qt::LeftDockWidgetArea, m_sceneDock);
    m_mainWindow->addDockWidget(Qt::LeftDockWidgetArea, m_resourceDock);
    //m_mainWindow->splitDockWidget(m_sceneDock, m_resourceDock);
    m_sceneDock->setFloating(false);
    m_resourceDock->setFloating(false);

    // Initialize scene inspector widget
    m_componentWidget = new ComponentTreeWidget(m_engine, "Component Widget");
    m_rightDock = new QDockWidget("Component View", m_mainWindow);
    m_rightDock->setWidget(m_componentWidget);

    m_mainWindow->addDockWidget(Qt::RightDockWidgetArea, m_rightDock);
    m_rightDock->setFloating(false);

    // Initialize playback widget
    m_playbackWidget = new PlayerControls(m_engine);
    m_topDock = new QDockWidget("Player", m_mainWindow); 
    m_topDock->setWidget(m_playbackWidget);

    m_mainWindow->addDockWidget(Qt::TopDockWidgetArea, m_topDock);
    m_topDock->setFloating(false);

#endif

#ifdef DEBUG_MODE
    // Create layout to house menu and GL widget
    QWidget *topFiller = new QWidget;
    topFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_mainWindow->m_infoLabel = new QLabel(tr("<i>Running in debug mode</i>"));
    m_mainWindow->m_infoLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    m_mainWindow->m_infoLabel->setAlignment(Qt::AlignCenter);

    // Need to ignore mouse events
    //m_mainWindow->m_infoLabel->setFocusPolicy(Qt::NoFocus);
    m_mainWindow->m_infoLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    QWidget *bottomFiller = new QWidget;
    bottomFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(5, 5, 5, 5);
    layout->addWidget(topFiller);
    layout->addWidget(m_mainWindow->m_infoLabel);
    layout->addWidget(bottomFiller);
    mainGLWidget()->setLayout(layout);
#endif
}


//////////////////////////////////////////////////////////////////////////////////
// End namespacing
}
}