#pragma once

// Standard
#include <shared_mutex>

// QT

// Internal
#include "fortress/process/GProcess.h"
#include "fortress/containers/math/GMatrix.h"

namespace rev {

class CoreEngine;
class ProcessManager;
class Sprite;

/// @class SpriteAnimationProcess
/// @brief Represents a 2D animation process
class SpriteAnimationProcess : public Process {
public:
    /// @name Constructors/Destructor
	/// @{
    SpriteAnimationProcess(CoreEngine* engine, Sprite* sprite);
	~SpriteAnimationProcess();

	/// @}

    /// @name Properties
    /// @{

    std::shared_mutex& mutex() { return m_mutex; }

    /// @}

	/// @name Public Methods
	/// @{

    /// @brief Functions to be overriden by process subclasses as needed
    virtual void onInit() override;
    virtual void onUpdate(double deltaSec) override;
    virtual void onLateUpdate(double deltaSec) override {}
    virtual void onPostUpdate(double deltaSec) override {}
    virtual void onFixedUpdate(double deltaSec) override;
    virtual void onSuccess() override;
    virtual void onFail() override;
    virtual void onAbort() override;
	
    /// @}


protected:
    /// @name Protected Members
    /// @{

    /// @brief The core engine, since processqueue doesn't hold onto it
    CoreEngine* m_engine;

    /// @brief Contol access to the process, since running on separate animation thread
    std::shared_mutex m_mutex;

    /// @brief The animated sprite
    Sprite* m_sprite;

    /// @brief The elapsed time (in seconds) since beginning playback
    double m_elapsedTimeSec = 0;

    /// @}
};


} // End namespaces
