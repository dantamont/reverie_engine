/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_ANIMATION_MANAGER_H
#define GB_ANIMATION_MANAGER_H

#include <vector>

// QT

// Internal
#include "core/GManager.h"
#include "fortress/types/GLoadable.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class AnimationStateMachine;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief AnimationManager class
class AnimationManager: public Manager {
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

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const AnimationManager& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, AnimationManager& orObject);


    /// @}


protected:

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    std::vector<AnimationStateMachine*> m_stateMachines; ///< All of the animation state machines

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif