#ifndef GB_ACTION_MANAGER_H
#define GB_ACTION_MANAGER_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QUndoStack>
#include <QUndoView>
#include <QMutex>
#include <QMutexLocker>

// Internal
#include "GbUndoCommand.h"
#include "../../core/GbManager.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class ActionManager
/// @brief Manager for handling all undo and redo actions
class ActionManager : public Manager {
    Q_OBJECT
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    ActionManager(CoreEngine* core);
    ~ActionManager();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief Returns the undo stack for the action manager
    QUndoStack* undoStack() { return m_undoStack; }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the last undone action
    inline void redo() { 
        QMutexLocker lock(&m_mutex);
        m_undoStack->redo(); 
    }

    /// @brief Undoes the last action
    inline void undo() {
        QMutexLocker lock(&m_mutex);
        m_undoStack->undo(); 
    }

    /// @brief Performs the given command (by adding to undo stack)
    void performAction(UndoCommand* command) {
        QMutexLocker lock(&m_mutex);
        m_undoStack->push(command);
    }

    /// @brief Toggles on undo view
    void showUndoView();

    /// @}
protected:

    //--------------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @brief Create undo view
    void initializeUndoView();

    /// @}
    
    //--------------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @brief Mutex for actions
    QMutex m_mutex;

    /// @brief Undo stack
    QUndoStack* m_undoStack;

    /// @brief widget for viewing undo stack, inherits from QListView
    QUndoView* m_undoView;


    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif