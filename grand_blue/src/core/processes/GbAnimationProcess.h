/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_ANIMATION_PROCESS_H
#define GB_ANIMATION_PROCESS_H
// Standard
#include <shared_mutex>

// QT

// Internal
#include "GbProcess.h"
#include "../geometry/GbMatrix.h"

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class ProcessManager;
class AnimationController;
class Transform;

/////////////////////////////////////////////////////////////////////////////////////////////
// Typedefs
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class AnimationProcess
/// @brief Represents a python-driven, scripted process
class AnimationProcess : public Process {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    AnimationProcess(CoreEngine* engine, 
        AnimationController* controller,
        const Transform* transform);
	~AnimationProcess();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    std::shared_mutex& mutex() { return m_mutex; }

    const std::vector<Matrix4x4>& transforms() const { return m_transforms; }

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Functions to be overriden by process subclasses as needed
    virtual void onInit() override;
    virtual void onUpdate(unsigned long deltaMs) override;
    virtual void onFixedUpdate(unsigned long deltaMs) override;
    virtual void onSuccess() override;
    virtual void onFail() override;
    virtual void onAbort() override;

    /// @brief Whether the process is threaded as well
    virtual bool isThreaded() const override { return false; }
	
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "AnimationProcess"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::AnimationProcess"; }
    /// @}


protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Contol access to the process, since running on separate animation thread
    std::shared_mutex m_mutex;

    AnimationController* m_controller;
    const Transform* m_transform;

    /// @brief The set of transforms for the current animation pose
    std::vector<Matrix4x4> m_transforms;

    /// @brief The elapsed time (in seconds) since begining playback
    double m_elapsedTimeSec = 0;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif