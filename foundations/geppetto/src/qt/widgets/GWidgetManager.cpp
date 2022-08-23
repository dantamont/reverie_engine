#include "geppetto/qt/widgets/GWidgetManager.h"

#include <QMenu>


#include <QMainWindow>

#include "ripple/network/gateway/GMessageGateway.h"

#include "geppetto/qt/widgets/graphics/GGLWidgetInterface.h"
#include "geppetto/qt/widgets/list/GResourceListView.h"
#include "geppetto/qt/widgets/list/GBlueprintView.h"
#include "geppetto/qt/widgets/tree/GSceneTreeWidget.h"
#include "geppetto/qt/widgets/tree/GResourceWidgets.h"
#include "geppetto/qt/widgets/tree/GComponentTreeWidget.h"
#include "geppetto/qt/widgets/playback/player/GPlayer.h"
#include "geppetto/qt/widgets/types/GParameterWidgets.h"

#include "geppetto/qt/style/GFontIcon.h"
#include "geppetto/qt/widgets/tree/GComponentTreeWidget.h"

namespace rev {


WidgetManager::WidgetManager(QMainWindow* window) :
	ManagerInterface("WidgetManager"),
	m_mainWindow(window),
    m_sceneTreeWidget(nullptr),
    m_componentWidget(nullptr)
{
}

WidgetManager::~WidgetManager()
{
    delete mainGLWidget();
    delete m_sceneTreeWidget;
    delete m_componentWidget;
    delete m_gateway;

    delete m_resourceTreeWidget;
    delete m_resourceListView;
    delete m_blueprintView;
    delete m_playbackWidget;
}

const json& WidgetManager::sceneObjectJson(Uint32_t id) const
{
    const json& sceneObjects = sceneJson()["sceneObjects"];
    const json* match{ nullptr };
    for (const json& sceneObject : sceneObjects) {
        match = getSceneObjectJson(id, sceneObject);
        if (match) {
            break;
        }
    }

    if (match == nullptr) {
        assert(false && "Scene object with ID not found");
    }
    return *match;
}

void WidgetManager::setMessageGateway(GMessageGateway* gateway)
{
#ifdef DEBUG_MODE
    assert(m_gateway == nullptr && "Gateway cannot be replaced if not null");
#endif

    m_gateway = gateway;
}

void WidgetManager::setActionManager(ActionManager* actionManager)
{
    m_actionManager = actionManager;
}

void WidgetManager::onUpdate(double)
{
    // Update my widget garbage collector
    WidgetCollector& collector = WidgetCollector::Instance();
    collector.update();
}

void WidgetManager::onLateUpdate(double deltaSec)
{
#ifdef DEVELOP_MODE
    QMutexLocker lock(&m_updateLock);
    for (const auto& widgetPair : m_parameterWidgets) {
        if (!widgetPair.second->isVisible()) {
            continue;
        }
        if (!widgetPair.second->isChild()) {
            widgetPair.second->update();
        }
    }

    m_gateway->processMessages();
#endif
}

void WidgetManager::onPostUpdate(double deltaSec)
{
    mainGLWidget()->update();
}

void WidgetManager::addParameterWidget(ParameterWidget * widget)
{
    // Don't lock update mutex if already locked, such as during a late update call
    bool locked = m_updateLock.tryLock(); 
#ifdef DEBUG_MODE
    assert(!m_parameterWidgets.count(widget->getUuid()) && "Error widget already added to parameter widget map");
#endif

    m_parameterWidgets[widget->getUuid()] = widget;

    if (locked) {
        m_updateLock.unlock();
    }
}

void WidgetManager::removeParameterWidget(ParameterWidget * widget)
{
    // Don't lock update mutex if already locked, such as during a late update call
    bool locked = m_updateLock.tryLock();
    m_parameterWidgets.erase(widget->getUuid());
    if (locked) {
        m_updateLock.unlock();
    }
}

void WidgetManager::resize(int w, int h)
{
    w; h;
    //m_mainWindow->resizeDocks({ m_glDock }, { int(w) }, Qt::Horizontal);
    //m_mainWindow->resizeDocks({ m_glDock }, { int(h) }, Qt::Vertical);
}

void WidgetManager::initializeGLWidget(rev::GLWidgetInterface* widget)
{
#ifdef DEBUG_MODE
    assert(nullptr == m_glWidget && "GL Widget already initialized");
#endif
    m_glWidget = widget;
    m_mainWindow->setCentralWidget(m_glWidget);

#ifdef DEBUG_MODE
    bool hasMouseTracking = m_mainWindow->centralWidget()->hasMouseTracking();
    assert(hasMouseTracking && "Error, GL Widget not tracking mouse");
#endif
}

void WidgetManager::clear()
{
    QMutexLocker lock(&m_updateLock);

    m_glWidget->clear();

    // Clear parameter widgets
    for (const auto& widgetPair : m_parameterWidgets) {
        widgetPair.second->clear();
    }
}

void WidgetManager::postConstruction()
{
    ManagerInterface::postConstruction();

    // Ensure that framebuffers are properly resized
    m_glWidget->requestResize();
}

const json* WidgetManager::getSceneObjectJson(Uint32_t id, const json& sceneObjectJson) const
{
    const json* match{ nullptr };
    bool isMatch = sceneObjectJson["id"].get<Uint32_t>() == id;
    if (!isMatch) {
        for (const json& child : sceneObjectJson["children"]) {
            match = getSceneObjectJson(id, child);
            if (nullptr != match) {
                break;
            }
        }
    }
    else {
        match = &sceneObjectJson;
    }
    return match;
}

void WidgetManager::initializeDefault()
{

#ifdef DEVELOP_MODE
    // Initialize scene tree widget
    m_sceneTreeWidget = new SceneTreeWidget(this, m_actionManager, "Scenario Widget");
    m_sceneDock = new QDockWidget("Scenario Tree View", m_mainWindow);
    m_sceneDock->setWidget(m_sceneTreeWidget);

    // Initialize resource dock widgets
    m_resourceTreeWidget = new ResourceTreeWidget(this, "Resource Widget");
    m_resourceTreeDock = new QDockWidget("Resource Tree View", m_mainWindow);
    m_resourceTreeDock->setWidget(m_resourceTreeWidget);

    m_resourceListView = new ResourceListView(this, "Resource List View");
    m_blueprintView = new BlueprintListView(this, "Blueprint List View");
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
    m_componentWidget = new ComponentTreeWidget(this, "Component Widget");
    m_rightDock = new QDockWidget("Component View", m_mainWindow);
    m_rightDock->setWidget(m_componentWidget);

    m_mainWindow->addDockWidget(Qt::RightDockWidgetArea, m_rightDock);
    m_rightDock->setFloating(false);

    // Initialize playback widget
    m_playbackWidget = new PlayerControls(this);
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



} // rev