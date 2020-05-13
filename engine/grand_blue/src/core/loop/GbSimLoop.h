#ifndef GB_SIM_LOOP_H
#define GB_SIM_LOOP_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QElapsedTimer>
#include <QTimer>

// Internal
#include "../../core/GbManager.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class ProcessManager;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class SimulationLoop
/// @brief Manager for handling all simulation loop logic
/** @details The simulation loop is as such:
while(running)
{
    processInput();

    while(elapsedTime > m_fixedUpdateInterval){
        fixedUpdate();
        elapsedTime -= m_fixedUpdateInterval;
    }

    while(isTimeForUpdate)
    {
        update();
    }

    render();
}
*/
class SimulationLoop : public Manager {
    Q_OBJECT
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum PlayState {
        kPlayedState,
        kPausedState,
        kStoppedState
    };

    enum PlayMode {
        kStandard,
        kDebug
    };

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    SimulationLoop(CoreEngine* core, size_t updateInterval=0, size_t fixedUpdateInterval=16);
    ~SimulationLoop();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    quint64 elapsedTime() const { return m_elapsedTime; }

    size_t fixedStep() const { return m_fixedUpdateInterval; }

    /// @property UserInterfaceMutex
    QMutex& userInterfaceMutex() {
        return m_uiMutex;
    }

    /// @property ProcessManager
    std::unique_ptr<ProcessManager>& processManager() { return m_processManager; }

    /// @property Play state
    bool isPlaying() const { return m_playState == kPlayedState; }
    bool isPaused() const { return m_playState == kPausedState; }
    bool isStopped() const { return m_playState == kStoppedState; }
    PlayState getPlayState() const { return m_playState; }
    void setPlayState(PlayState playState) { m_playState = playState; }

    /// @property Play Mode
    PlayMode getPlayMode() const { return m_playMode; }
    void setPlayMode(PlayMode playMode) { m_playMode = playMode; }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Start the simulation loop
    void play();

    /// @brief Pause the simulation loop
    void pause();

    /// @brief Initialize connections
    void initializeConnections();

    /// @}

signals:

    void paused();
    void unpaused();

protected slots:

    //--------------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{
    /// @brief Update a frame of the simulation loop
    /// @note Main loop should remain serialized, see:
    // https://stackoverflow.com/questions/565137/multithreaded-job-queue-manager
    // https://gamedev.stackexchange.com/questions/58442/how-should-i-invoke-a-physics-engine
    void update();

    /// @}
protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class CoreEngine;

    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{



    /// @}
    
    //--------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    /// @brief Mutex to protect update loop from GUI modifications
    QMutex m_uiMutex;

    /// @brief Manager for handling processes
    std::unique_ptr<ProcessManager> m_processManager;

    /// @brief Fixed update interval
    /// @details How much time (ms) must pass before a fixed update event is called
    size_t m_fixedUpdateInterval;

    /// @brief Minimum update interval
    /// @details How much time (ms) must pass before an update event is called
    size_t m_minimumUpdateInterval;

    /// @brief Send time signal to drive simulation loop
    QTimer m_timer;

    /// @brief Total elapsed time since start of simulation in msec (for querying)
    quint64 m_elapsedTime;

    /// @brief Time-keeping for updates
    QElapsedTimer m_updateTimer;

    /// @brief Time-keeping for updates
    unsigned long m_fixedDeltaTime;

    /// @brief Play state
    PlayState m_playState;

    /// @brief Play mode
    PlayMode m_playMode;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif