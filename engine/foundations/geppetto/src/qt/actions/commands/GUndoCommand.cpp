#include "geppetto/qt/actions/commands/GUndoCommand.h"
#include "geppetto/qt/actions/GActionManager.h"
#include "geppetto/qt/widgets/GWidgetManager.h"

namespace rev {


UndoCommand::UndoCommand(WidgetManager* wm, const QString & text, QUndoCommand * parent):
    QUndoCommand(text, parent),
    m_widgetManager(wm),
    m_id(s_undoCommandCount++)
{

}

UndoCommand::UndoCommand(WidgetManager* wm, QUndoCommand * parent):
    QUndoCommand(parent),
    m_widgetManager(wm),
    m_id(s_undoCommandCount++)
{
}

rev::UndoCommand::~UndoCommand()
{
}

void UndoCommand::perform()
{
    m_widgetManager->actionManager()->performAction(this);
}

Uint32_t UndoCommand::s_undoCommandCount = 0;

} // End namespaces