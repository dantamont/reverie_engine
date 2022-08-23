#pragma once

#include <QObject>

// Internal
#include "fortress/layer/application/GManagerInterface.h"

namespace rev {

class CoreEngine;

/// @brief Generic manager class
/// @detail Manages some aspect of the application, and has access to the core engine
class Manager: public ManagerInterface, public QObject {
public:

	Manager(CoreEngine* core, const GString& name);
	virtual ~Manager();

    /// @brief Get engine from the manager
    CoreEngine* engine() const { return m_engine; }

protected:

    /// @brief Core engine
    CoreEngine* m_engine;
};


} /// End rev namespace
