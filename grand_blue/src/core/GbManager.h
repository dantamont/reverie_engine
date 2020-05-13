/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_MANAGER_H
#define GB_MANAGER_H

// QT

// Internal
#include "service/GbService.h"
#include "GbObject.h"
#include "containers/GbContainerExtensions.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class MainWindow;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////


/// @brief Generic manager class
/// @detailed Manages some aspect of the application
class Manager : public Service{
    Q_OBJECT
public:
	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
	Manager(CoreEngine* core, const QString& name);
	virtual ~Manager();
	/// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Get engine from the manager
    CoreEngine* engine() const { return m_engine; }

    /// @brief Called after construction of the manager
    virtual void postConstruction() override { Service::postConstruction(); }

	/// @}
protected:
    //--------------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @brief Obtain the main window
    MainWindow* mainWindow();

    /// @brief Show a dialog with the given title and text
    void showMessageBox(const QString& title, const QString& text);

    /// @}

    /// @brief Core engine
    CoreEngine* m_engine;
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif