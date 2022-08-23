#pragma once

// Qt
#include <QtWidgets>

// External
#include "ripple/network/messages/GResetPythonScriptMessage.h"

// Internal
#include "geppetto/qt/widgets/components/GComponentWidget.h"

namespace rev {

class FileLoadWidget;

/// @class ScriptWidget
/// @brief Widget representing a script component
class ScriptWidget : public SceneObjectComponentWidget{
public:
    /// @name Constructors/Destructor
    /// @{

    ScriptWidget(WidgetManager* wm, const json& componentJson, Int32_t sId, QWidget *parent = 0);
    ~ScriptWidget();

    /// @}

private:
    /// @name Private Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}

    /// @name Private Members
    /// @{

    /// @brief Path to the python script used by this component
    FileLoadWidget* m_fileWidget;
    QPushButton* m_confirmButton;

    GResetPythonScriptMessage m_resetScriptMessage;

    /// @}
};

// End namespaces        
}