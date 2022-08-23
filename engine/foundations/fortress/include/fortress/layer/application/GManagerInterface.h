#pragma once

// Internal
#include "fortress/containers/GContainerExtensions.h"
#include "fortress/types/GNameable.h"

namespace rev {

/// @brief Generic manager class
/// @detail Manages some aspect of the application
class ManagerInterface: public NameableInterface {
public:

	ManagerInterface(const GString& name) :
		NameableInterface(name) {

	}
	virtual ~ManagerInterface() {}

    /// @brief Called after construction of the manager
	virtual void postConstruction() {}

};


} /// End rev namespace
