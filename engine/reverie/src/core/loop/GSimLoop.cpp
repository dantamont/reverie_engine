#include "GSimLoop.h"
#include "../GCoreEngine.h"

#include "../processes/GProcessManager.h"
#include "../events/GEventManager.h"
#include "../physics/GPhysicsManager.h"
#include "../../view/GWidgetManager.h"
#include "../../view/players/GPlayer.h"
#include "../../view/GL/GGLWidget.h"
#include "../input/GInputHandler.h"
#include "../../GMainWindow.h"
#include "../debugging/GDebugManager.h"
#include <core/scene/GScenario.h>

namespace rev {


/////////////////////////////////////////////////////////////////////////////////////////////
SimulationLoop::SimulationLoop(CoreEngine* core, size_t updateInterval, size_t fixedUpdateInterval):
    Manager(core, "SimulationLoop"),
    m_playState(kPausedState),
    m_processManager(std::make_unique<ProcessManager>(core)),
    m_fixedDeltaTime(0),
    m_minimumUpdateInterval(updateInterval), // in ms
    m_fixedUpdateInterval(fixedUpdateInterval), // in ms
    m_elapsedTime(0),
    m_playMode(kDebug)
{
    // Assert that fixed update interval be nonzero
    if(fixedUpdateInterval < 0) throw("Fixed update interval must be greater than zero");

    // Set timer precision (may be overkill)
    m_timer.setInterval(m_minimumUpdateInterval);
    m_timer.setTimerType(Qt::PreciseTimer);

    // Initialize update timer, which just keep track of elapsed time
    m_updateTimer.restart();

    // Play by default
    play();
}
/////////////////////////////////////////////////////////////////////////////////////////////
SimulationLoop::~SimulationLoop()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void SimulationLoop::play()
{
    m_updateTimer.restart();
    m_timer.start();
    m_playState = kPlayedState;
    emit unpaused();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void SimulationLoop::pause()
{
    m_timer.stop();
    m_playState = kPausedState;
    emit paused();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void SimulationLoop::update()
{
    // Mutex lock rendering to avoid conflicts with GUI interactions
    QMutexLocker lock(&m_updateMutex);

    // Get delta time
    // TODO: Support nSecs if necessary, times less than 1ms are not counted
    unsigned long deltaMs = m_updateTimer.restart();
    m_fixedDeltaTime += deltaMs;
    m_elapsedTime += deltaMs;

    // Update loop

    // Process Events
    m_engine->eventManager()->processEvents();

    // Update Input (polling)
    m_engine->widgetManager()->mainGLWidget()->inputHandler().update(deltaMs);

    // Update debug manager
    if(m_playMode == kDebug) m_engine->debugManager()->step(deltaMs);

    // Fixed update loop (update physics)
    size_t fiveSecs = 5000;
    if (m_fixedDeltaTime < fiveSecs) {
        while (m_fixedDeltaTime > m_fixedUpdateInterval) {
            // Update physics
            float pStep = m_fixedUpdateInterval / 1000.0;
            m_engine->physicsManager()->step(pStep);

            // Perform fixed updates
            m_processManager->fixedUpdateProcesses(m_fixedUpdateInterval);
            m_fixedDeltaTime -= m_fixedUpdateInterval;
        }
    }
    else {
        m_fixedDeltaTime = 0;
        logWarning("Warning, loop took more than five seconds, skipping fixed update");
    }

    // Run processes, user scripts, e.g. update AI
    m_processManager->updateProcesses(deltaMs);

    // Perform late update 
    // Scripted processes know to perform lateUpdate the second time around
    m_processManager->updateProcesses(deltaMs);

#ifdef DEVELOP_MODE
    // Update widgets
    m_engine->widgetManager()->update();
#endif

    // Render
    m_engine->widgetManager()->mainGLWidget()->update();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void SimulationLoop::initializeConnections()
{
    // Connect pause and play to controlling widget
    connect(m_engine->widgetManager()->playbackWidget(),
        &View::PlayerControls::play,
        this,
        &SimulationLoop::play);
    connect(m_engine->widgetManager()->playbackWidget(),
        &View::PlayerControls::pause,
        this,
        &SimulationLoop::pause);

    // Connect timer timeout to the render slot
    connect(&m_timer, 
        &QTimer::timeout,
        this, 
        static_cast<void(SimulationLoop::*)(void)>(&SimulationLoop::update));
}

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces