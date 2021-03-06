#ifndef GB_LISTENER_COMPONENT_WIDGET_H
#define GB_LISTENER_COMPONENT_WIDGET_H


///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QtWidgets>

// Internal
#include "GComponentWidgets.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

class ListenerComponent;

namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ListenerWidget
/// @brief Widget representing a script component
class ListenerWidget : public ComponentWidget{
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    ListenerWidget(CoreEngine* core, Component* component, QWidget *parent = 0);
    ~ListenerWidget();

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

    /// @brief Return component as a listener component
    ListenerComponent* listenerComponent() const;

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @brief Get the event types currently set in the event types widget
    void getEventTypes(std::vector<int>& outVec);

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    /// @brief Path to the listener script used by this component
    FileLoadWidget* m_fileWidget;
    QPushButton* m_confirmButton;

    /// @brief The event types that the listener responds to
    /// @details Event types are comma-delimited
    // TODO: Create widget for custom event types
    QLineEdit* m_eventTypes;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // COMPONENT_WIDGETS_H