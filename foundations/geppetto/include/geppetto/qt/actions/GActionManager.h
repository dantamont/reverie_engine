#pragma once

// Qt
#include <QUndoStack>
#include <QUndoView>
#include <QMutex>
#include <QMutexLocker>

#include <deque>

// Internal
#include "fortress/layer/application/GManagerInterface.h"
#include "geppetto/qt/actions/commands/GUndoCommand.h"

namespace rev {

/// @class ActionManager
/// @brief Manager for handling all undo and redo actions
class ActionManager : public QObject, public ManagerInterface {
    Q_OBJECT
public:

    ActionManager(const GString& name);
    ~ActionManager();

    /// @brief Returns the undo stack for the action manager
    QUndoStack* undoStack() { return m_undoStack; }

    /// @brief Return the action with the given ID
    UndoCommand* getAction(Uint32_t id) const;

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
    void performAction(UndoCommand* command);

    /// @brief Toggles on undo view
    void showUndoView();

protected:

    static constexpr Uint32_t s_undoLimit = 100;

    /// @brief Create undo view
    void initializeUndoView();

    QMutex m_mutex; ///< Mutex for actions
    QUndoStack* m_undoStack; ///< Undo stack
    QUndoView* m_undoView; ///< Widget for viewing undo stack, inherits from QListView

    std::deque<UndoCommand*> m_undoCommands; ///< The undo commands, accessible since Qt is a nightmare
};

} // End namespaces
