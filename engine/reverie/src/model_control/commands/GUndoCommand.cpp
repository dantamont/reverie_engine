#include "GUndoCommand.h"

#include "GActionManager.h"
#include "../../core/GCoreEngine.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
UndoCommand::UndoCommand(CoreEngine* core, const QString & text, QUndoCommand * parent):
    QUndoCommand(text, parent),
    m_engine(core)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
UndoCommand::UndoCommand(CoreEngine* core, QUndoCommand * parent):
    QUndoCommand(parent),
    m_engine(core)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
rev::UndoCommand::~UndoCommand()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void UndoCommand::perform()
{
    m_engine->actionManager()->performAction(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces