#ifndef GB_SCRIPT_COMPONENT_WIDGET_H
#define GB_SCRIPT_COMPONENT_WIDGET_H


///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QtWidgets>

// Internal
#include "GbComponentWidgets.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ScriptWidget
/// @brief Widget representing a script component
class ScriptWidget : public ComponentWidget{
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    ScriptWidget(CoreEngine* core, Component* component, QWidget *parent = 0);
    ~ScriptWidget();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{
    /// @}

public slots:
signals:
private slots:
private:
    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @brief Return component as a script component
    ScriptComponent* scriptComponent() const;

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    /// @brief Path to the python script used by this component
    FileLoadWidget* m_fileWidget;
    QPushButton* m_confirmButton;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // COMPONENT_WIDGETS_H