#include "core/loop/GSimLoop.h"
#include "core/GCoreEngine.h"

#include "core/resource/GResourceCache.h"
#include "core/processes/GProcessManager.h"
#include "core/events/GEventManager.h"
#include "core/physics/GPhysicsManager.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/playback/player/GPlayer.h"
#include "geppetto/qt/widgets/graphics/GGLWidgetInterface.h"
#include "core/layer/view/widgets/graphics/GInputHandler.h"
#include "GMainWindow.h"
#include "core/debugging/GDebugManager.h"
#include "core/scene/GScenario.h"

#include "core/layer/gateway/GApplicationGateway.h"

namespace rev {



SimulationLoop::SimulationLoop(CoreEngine* core, double updateInterval, double fixedUpdateInterval):
    Manager(core, "SimulationLoop"),
    m_playState(ESimulationPlayState::ePausedState),
    m_accumulatedFixedTime(0),
    m_minimumUpdateInterval(updateInterval), // in sec
    m_fixedUpdateInterval(fixedUpdateInterval), // in sec
    m_playMode(ESimulationPlayMode::eDebug)
{
    // Assert that fixed update interval be nonzero
    if (fixedUpdateInterval <= 0) {
        Logger::Throw("Fixed update interval must be greater than zero");
    }

    // Set timer precision (may be overkill)
    m_timer.setInterval(m_minimumUpdateInterval);
    m_timer.setTimerType(Qt::PreciseTimer);

    // Initialize update timer, which just keep track of elapsed time
    m_updateTimer.restart();

    // Play by default
    play();
}

SimulationLoop::~SimulationLoop()
{
}

void SimulationLoop::setPlayMode(GSimulationPlayMode playMode)
{
    m_playMode = playMode;
    m_changedPlayMode.emitForAll(playMode);
}

void SimulationLoop::play()
{
    m_updateTimer.restart();
    m_timer.start();
    m_playState = ESimulationPlayState::ePlayedState;
}

void SimulationLoop::pause()
{
    m_timer.stop();
    m_playState = ESimulationPlayState::ePausedState;
}

void SimulationLoop::update()
{
    // Mutex lock rendering to avoid conflicts with GUI interactions
    std::unique_lock lock(m_updateMutex);

    // Get delta time
    double deltaSec = StopwatchTimer::ToSeconds<double>(m_updateTimer.restart());
    m_accumulatedFixedTime += deltaSec;

    // Update loop

    // Post-construct any resources
    ResourceCache::Instance().postConstructResources();

    // Process Events
    m_engine->eventManager()->processEvents();

    // Update Input (polling)
    m_engine->widgetManager()->mainGLWidget()->inputHandler().update(deltaSec);

    // Update debug manager
    if (m_playMode == ESimulationPlayMode::eDebug) {
        m_engine->debugManager()->step(deltaSec); 
    }

    // Fixed update loop (update physics)
    constexpr double fiveSecs = 5.0;
    if (m_accumulatedFixedTime < fiveSecs) {
        while (m_accumulatedFixedTime > m_fixedUpdateInterval) {
            // Update physics
            m_engine->physicsManager()->step(m_fixedUpdateInterval);

            // Perform fixed updates
            m_engine->processManager()->onFixedUpdate(m_fixedUpdateInterval);
            m_accumulatedFixedTime -= m_fixedUpdateInterval;
        }
    }
    else {
        m_accumulatedFixedTime = 0;
        Logger::LogWarning("Warning, loop took more than five seconds, skipping fixed update");
    }

    // Run processes, user scripts, e.g. update AI
    m_engine->processManager()->onUpdate(deltaSec);

    // Perform late update
    m_engine->processManager()->onLateUpdate(deltaSec); // Update processes
    m_engine->applicationGateway()->processMessages(); // Update application messages
    m_engine->widgetManager()->onLateUpdate(deltaSec); // Update widgets, and process widget messages

    // Perform post update
    m_engine->processManager()->onPostUpdate(deltaSec); // Update processes
    m_engine->widgetManager()->onPostUpdate(deltaSec); // Render OpenGL widget
}

void SimulationLoop::initializeConnections()
{
    // Connect pause and play to controlling widget
    //m_engine->widgetManager()->playbackWidget()->m_playSignal.connect(this, &SimulationLoop::play);
    //m_engine->widgetManager()->playbackWidget()->m_pauseSignal.connect(this, &SimulationLoop::pause);

    // Connect timer timeout to the render slot
    connect(&m_timer, 
        &QTimer::timeout,
        this, 
        static_cast<void(SimulationLoop::*)(void)>(&SimulationLoop::update));
}


} // End namespaces