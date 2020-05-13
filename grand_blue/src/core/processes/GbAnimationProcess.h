/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_ANIMATION_PROCESS_H
#define GB_ANIMATION_PROCESS_H
// External
#include <PythonQt.h>

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
class SkeletonPose;

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
        ProcessManager* manager);
	~AnimationProcess();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    const std::vector<Matrix4x4f>& transforms() const { return m_transforms; }

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

    friend class ProcessManager;
    friend class AnimationController;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    AnimationController* m_controller;

    /// @brief The set of transforms for the current animation pose
    std::vector<Matrix4x4f> m_transforms;

    std::shared_ptr<SkeletonPose> m_pose;
    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif