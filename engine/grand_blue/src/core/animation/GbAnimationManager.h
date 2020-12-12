/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_ANIMATION_MANAGER_H
#define GB_ANIMATION_MANAGER_H

#include <vector>

// QT

// Internal
#include "../../core/GbManager.h"
#include "../../core/mixins/GbLoadable.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class AnimationStateMachine;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief AnimationManager class
class AnimationManager: public Manager, public Serializable {
    Q_OBJECT
public:
	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    AnimationManager(CoreEngine* engine);
	~AnimationManager();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    const std::vector<AnimationStateMachine*>& stateMachines() const { return m_stateMachines; }

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Return the state machine with the specified name
    AnimationStateMachine* getStateMachine(const GString& name);
    AnimationStateMachine* getStateMachine(const Uuid& uuid);

    /// @brief Create and add a new state machine to the manager
    AnimationStateMachine* addStateMachine();

    void clear();

    /// @brief Called after construction
    virtual void postConstruction() override final;

	/// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "AnimationManager"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::AnimationManager"; }
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

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

    /// @brief State machines
    std::vector<AnimationStateMachine*> m_stateMachines;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif