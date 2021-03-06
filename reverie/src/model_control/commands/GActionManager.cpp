#include "GActionManager.h"
#include "../../core/GCoreEngine.h"
#include "../../view/GWidgetManager.h"
#include "../../GMainWindow.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
rev::ActionManager::ActionManager(CoreEngine* core):
    Manager(core, "ActionManager")
{
    initializeUndoView();
}
/////////////////////////////////////////////////////////////////////////////////////////////
rev::ActionManager::~ActionManager()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ActionManager::showUndoView()
{
    m_undoView->show();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ActionManager::initializeUndoView()
{
    // Create undo view
    m_undoStack = new QUndoStack(m_engine->widgetManager()->mainWindow());
    m_undoStack->setUndoLimit(0);

    m_undoView = new QUndoView(m_undoStack);
    m_undoView->setWindowTitle(tr("Command List"));

    // Do not quit application on widget close
    m_undoView->setAttribute(Qt::WA_QuitOnClose, false);
}


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces