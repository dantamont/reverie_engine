#pragma once

// std
#include <mutex>

// Qt
#include <QTimer>

// Internal
#include <core/GManager.h>
#include "fortress/layer/framework/GSignalSlot.h"
#include "fortress/time/GStopwatchTimer.h"
#include "enums/GSimulationPlayModeEnum.h"
#include "enums/GSimulationPlayStateEnum.h"

namespace rev {

class ProcessManager;

/// @class SimulationLoop
/// @brief Manager for handling all simulation loop logic
class SimulationLoop : public Manager {
    Q_OBJECT
public:

    /// @name Constructors/Destructor
    /// @{
    SimulationLoop(CoreEngine* core, double updateInterval=0, double fixedUpdateInterval = 0.016);
    ~SimulationLoop();
    /// @}

    /// @name Properties
    /// @{

    double elapsedTime() const { return m_updateTimer.getElapsed<double>(); }

    double fixedStep() const { return m_fixedUpdateInterval; }

    /// @property UserInterfaceMutex
    std::mutex& updateMutex() {
        return m_updateMutex;
    }

    Signal<GSimulationPlayMode>& changedPlayModeSignal() { return m_changedPlayMode; }

    /// @property Play state
    bool isPlaying() const { return m_playState == ESimulationPlayState::ePlayedState; }
    bool isPaused() const { return m_playState == ESimulationPlayState::ePausedState; }
    bool isStopped() const { return m_playState == ESimulationPlayState::eStoppedState; }
    GSimulationPlayState getPlayState() const { return m_playState; }
    void setPlayState(GSimulationPlayState playState) { m_playState = playState; }

    /// @property Play Mode
    GSimulationPlayMode getPlayMode() const { return m_playMode; }
    void setPlayMode(GSimulationPlayMode playMode);

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Start the simulation loop
    void play();

    /// @brief Pause the simulation loop
    void pause();

    /// @brief Initialize connections
    void initializeConnections();

    /// @}

protected slots:

    /// @name Private Methods
    /// @{
    /// @brief Update a frame of the simulation loop
    /// @note Main loop should remain serialized, see:
    // https://stackoverflow.com/questions/565137/multithreaded-job-queue-manager
    // https://gamedev.stackexchange.com/questions/58442/how-should-i-invoke-a-physics-engine
    void update();

    /// @}
protected:
    /// @name Friends
    /// @{

    friend class CoreEngine;

    /// @}

    /// @name Private Methods
    /// @{
    /// @}
    
    /// @name Private Members
    /// @{

    /// @details How much time (s) must pass before a fixed update event is called
    double m_fixedUpdateInterval; ///< Fixed update interval

    /// @details How much time (s) must pass before an update event is called
    double m_minimumUpdateInterval; ///< Minimum update interval

    std::mutex m_updateMutex; ///< Mutex to protect update loop from GUI modifications
    QTimer m_timer; ///< Send time signal to drive simulation loop
    StopwatchTimer m_updateTimer; ///< Time-keeping for updates
    double m_accumulatedFixedTime; ///< Time-keeping for fixed updates, in seconds
    GSimulationPlayState m_playState; ///< Play state
    GSimulationPlayMode m_playMode; ///< Play mode
    Signal<GSimulationPlayMode> m_changedPlayMode; ///< Signal that the play mode was changed

    /// @}
};

} // End namespaces
