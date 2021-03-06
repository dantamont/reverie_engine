#include "GWidgetManager.h"

#include <QMenu>


#include "../GMainWindow.h"

#include "GL/GGLWidget.h"
#include "../core/rendering/renderer/GMainRenderer.h"
#include "list/GResourceListView.h"
#include "list/GBlueprintView.h"
#include "tree/GSceneTreeWidget.h"
#include "tree/GResourceWidgets.h"
#include "tree/GComponentWidget.h"
#include "players/GPlayer.h"
#include "parameters/GParameterWidgets.h"

#include "../core/GCoreEngine.h"
#include "style/GFontIcon.h"

namespace rev {
namespace View {

//////////////////////////////////////////////////////////////////////////////////
WidgetManager::WidgetManager(rev::CoreEngine* core, rev::MainWindow* window) :
	Manager(core, "WidgetManager"),
	m_mainWindow(window),
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
    delete m_sceneTreeWidget;
    delete m_componentWidget;

    delete m_resourceTreeWidget;
    delete m_resourceListView;
    delete m_blueprintView;
    delete m_playbackWidget;
}
//////////////////////////////////////////////////////////////////////////////////
void WidgetManager::addParameterWidget(ParameterWidget * widget)
{
    QMutexLocker lock(&m_updateLock);
    if (m_parameterWidgets.count(widget->getUuid())) {
        throw("Error widget already added to parameter widget map");
    }
    m_parameterWidgets[widget->getUuid()] = widget;
}
//////////////////////////////////////////////////////////////////////////////////
void WidgetManager::removeParameterWidget(ParameterWidget * widget)
{
    QMutexLocker lock(&m_updateLock);
    m_parameterWidgets.erase(widget->getUuid());
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
    QMutexLocker lock(&m_updateLock);

    for (const std::pair<QString, rev::View::GLWidget*>& widgetPair : m_glWidgets) {
        widgetPair.second->clear();
    }

    // Clear parameter widgets
    for (const auto& widgetPair : m_parameterWidgets) {
        widgetPair.second->clear();
    }
}
//////////////////////////////////////////////////////////////////////////////////
void WidgetManager::update()
{
    QMutexLocker lock(&m_updateLock);
    for (const auto& widgetPair : m_parameterWidgets) {
        if (!widgetPair.second->isVisible()) { 
            continue; 
        }
        widgetPair.second->update();
    }
}
//////////////////////////////////////////////////////////////////////////////////
void WidgetManager::postConstruction()
{
    Manager::postConstruction();

    for (const auto& widgetPair : m_glWidgets) {
        // Ensure that framebuffers are properly resized
        widgetPair.second->renderer()->requestResize();
    }
}
//////////////////////////////////////////////////////////////////////////////////
void WidgetManager::initializeDefault()
{
    // Initialize GL widget
    QString mainName = QStringLiteral("main");
    m_glWidgets[mainName] = new GLWidget(mainName, m_engine, m_mainWindow);

    m_mainWindow->setCentralWidget(m_glWidgets[mainName]);
#ifdef DEBUG_MODE
    bool hasMouseTracking = m_mainWindow->centralWidget()->hasMouseTracking();
    if (!hasMouseTracking) {
        throw("Error, GL Widget not tracking mouse");
    }
    //for (QWidget* child : m_mainWindow->findChildren<QWidget*>()) {
    //    child->setMouseTracking(true);
    //}
#endif

#ifdef DEVELOP_MODE
    // Initialize scene tree widget
    m_sceneTreeWidget = new SceneTreeWidget(m_engine, "Scenario Widget");
    m_sceneDock = new QDockWidget("Scenario Tree View", m_mainWindow);
    m_sceneDock->setWidget(m_sceneTreeWidget);

    // Initialize resource dock widgets
    m_resourceTreeWidget = new ResourceTreeWidget(m_engine, "Resource Widget");
    m_resourceTreeDock = new QDockWidget("Resource Tree View", m_mainWindow);
    m_resourceTreeDock->setWidget(m_resourceTreeWidget);

    m_resourceListView = new ResourceListView(m_engine, "Resource List View");
    m_blueprintView = new BlueprintListView(m_engine, "Blueprint List View");
    m_bottomDock = new QDockWidget("", m_mainWindow);

    QTabWidget* bottomTab = new QTabWidget();
    bottomTab->addTab(m_blueprintView, SAIcon("pencil-ruler"), "Blueprints");
    bottomTab->addTab(m_resourceListView, SAIcon("file-alt"), "Resources");
    m_bottomDock->setTitleBarWidget(new QWidget(m_bottomDock));
    m_bottomDock->setWidget(bottomTab);

    m_mainWindow->addDockWidget(Qt::BottomDockWidgetArea, m_bottomDock);
    m_mainWindow->addDockWidget(Qt::LeftDockWidgetArea, m_sceneDock);
    m_mainWindow->addDockWidget(Qt::LeftDockWidgetArea, m_resourceTreeDock);
    //m_mainWindow->splitDockWidget(m_sceneDock, m_resourceDock);
    m_sceneDock->setFloating(false);
    m_resourceTreeDock->setFloating(false);

    // Set corners for dock widgets so that bottom widget doesn't take up whole bottom
    m_mainWindow->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    m_mainWindow->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

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
    m_topDock->setTitleBarWidget(new QWidget(m_topDock));

    m_mainWindow->addDockWidget(Qt::TopDockWidgetArea, m_topDock);
    m_topDock->setFloating(false);

#endif

#ifdef DEBUG_MODE
    // Create layout to house menu and GL widget
    QWidget *topFiller = new QWidget;
    topFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    //m_mainWindow->m_infoLabel = new QLabel(tr("<i>Running in debug mode</i>"));
    //m_mainWindow->m_infoLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    //m_mainWindow->m_infoLabel->setAlignment(Qt::AlignCenter);

    // Need to ignore mouse events
    //m_mainWindow->m_infoLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    QWidget *bottomFiller = new QWidget;
    bottomFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(5, 5, 5, 5);
    layout->addWidget(topFiller);
    //layout->addWidget(m_mainWindow->m_infoLabel);
    layout->addWidget(bottomFiller);
    mainGLWidget()->setLayout(layout);
#endif
}


//////////////////////////////////////////////////////////////////////////////////
// End namespacing
}
}