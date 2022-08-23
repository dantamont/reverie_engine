// Standard
#include <shared_mutex>

// QT

// Internal
#include "fortress/process/GProcess.h"
#include "fortress/containers/math/GMatrix.h"

namespace rev {

class CoreEngine;
class ProcessManager;
class AnimationController;
class TransformInterface;

/// @class AnimationProcess
/// @brief Represents a skeletal animation process
class AnimationProcess : public Process {
public:

	/// @name Constructors/Destructor
	/// @{
    AnimationProcess(CoreEngine* engine, 
        AnimationController* controller,
        const TransformInterface* transform);
	~AnimationProcess();
	/// @}

    /// @name Properties
    /// @{

    std::shared_mutex& mutex() { return m_mutex; }

    const std::vector<Matrix4x4>& transforms() const { return m_transforms; }

    /// @}

	/// @name Public Methods
	/// @{

    /// @brief Functions to be overriden by process subclasses as needed
    virtual void onInit() override;
    virtual void onUpdate(double deltaSec) override;
    virtual void onLateUpdate(double deltaSec) override {}
    virtual void onFixedUpdate(double deltaSec) override;
    virtual void onPostUpdate(double deltaSec) override {}
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

    AnimationController* m_controller;
    const TransformInterface* m_transform;

    /// @brief The set of transforms for the current animation pose
    std::vector<Matrix4x4> m_transforms;

    /// @brief The elapsed time (in seconds) since begining playback
    double m_elapsedTimeSec = 0;

    /// @}
};


} // End namespaces
