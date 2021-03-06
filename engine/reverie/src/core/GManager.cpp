#include "GManager.h"
#include "GCoreEngine.h"

#include "../GMainWindow.h"
#include "../view/GWidgetManager.h"

#include <QMessageBox>

namespace rev{

/////////////////////////////////////////////////////////////////////////////////////////////
Manager::Manager(CoreEngine* core, const QString& name):
    Service(name, nullptr, true),
    m_engine(core)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Manager::~Manager()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
MainWindow* Manager::mainWindow()
{
    if (m_engine->widgetManager()) {
        return m_engine->widgetManager()->mainWindow();
    }
    else {
        throw("Error, main engine does not contain a widget manager");
        return nullptr;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void Manager::showMessageBox(const QString & title, const QString & text)
{
    QMessageBox::about(mainWindow(),
        tr(title.toStdString().c_str()),
        tr(text.toStdString().c_str()));
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}