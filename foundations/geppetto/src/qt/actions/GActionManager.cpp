#include "geppetto/qt/actions/GActionManager.h"
#include "geppetto/qt/widgets/GWidgetManager.h"

namespace rev {


rev::ActionManager::ActionManager(const GString& name):
    ManagerInterface(name)
{
    initializeUndoView();
}

rev::ActionManager::~ActionManager()
{
}

UndoCommand* ActionManager::getAction(Uint32_t id) const
{
    return *std::find_if(m_undoCommands.begin(), m_undoCommands.end(), 
        [&](const UndoCommand* command) {
            return command->getId() == id;
        });
}

void ActionManager::performAction(UndoCommand* command)
{
    QMutexLocker lock(&m_mutex);
    m_undoStack->push(command);

    if (m_undoCommands.size() >= s_undoLimit) {
        m_undoCommands.pop_front();
    }
    
    m_undoCommands.push_back(command);
}

void ActionManager::showUndoView()
{
    m_undoView->show();
}

void ActionManager::initializeUndoView()
{
    // Create undo view
    m_undoStack = new QUndoStack();
    m_undoStack->setUndoLimit(s_undoLimit);

    m_undoView = new QUndoView(m_undoStack);
    m_undoView->setWindowTitle(tr("Command List"));

    // Do not quit application on widget close
    m_undoView->setAttribute(Qt::WA_QuitOnClose, false);
}



} // End namespaces