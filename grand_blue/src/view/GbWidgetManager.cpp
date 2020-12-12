#include "GbWidgetManager.h"

#include <QMenu>


#include "../GbMainWindow.h"

#include "GL/GbGLWidget.h"
#include "../core/rendering/renderer/GbMainRenderer.h"
#include "tree/GbSceneTreeWidget.h"
#include "tree/GbResourceWidgets.h"
#include "tree/GbComponentWidget.h"
#include "players/GbPlayer.h"
#include "parameters/GbParameterWidgets.h"

#include "../core/GbCoreEngine.h"

namespace Gb {
namespace View {

//////////////////////////////////////////////////////////////////////////////////
WidgetManager::WidgetManager(Gb::CoreEngine* core, Gb::MainWindow* window) :
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

    for (const std::pair<QString, Gb::View::GLWidget*>& widgetPair : m_glWidgets) {
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