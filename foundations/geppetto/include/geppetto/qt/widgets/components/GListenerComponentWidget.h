#pragma once

// Qt
#include <QtWidgets>

// Internal
#include "GComponentWidget.h"

#include "ripple/network/messages/GModifyListenerScriptMessage.h"

namespace rev {

class FileLoadWidget;

/// @class ListenerWidget
/// @brief Widget representing a script component
class ListenerWidget : public SceneObjectComponentWidget{
    Q_OBJECT
public:
    /// @name Constructors/Destructor
    /// @{

    ListenerWidget(WidgetManager* wm, const json& componentJson, Uint32_t sceneObjectId, QWidget *parent = 0);
    ~ListenerWidget();

    /// @}

public slots:
signals:
private slots:
private:
    /// @name Private Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    void requestScriptUpdate(const char* cStr);

    /// @brief Get the event types currently set in the event types widget
    void getEventTypes(std::vector<int>& outVec);

    /// @}

    /// @name Private Members
    /// @{

    GModifyListenerScriptMessage m_listenerScriptMessage;

    /// @brief Path to the listener script used by this component
    FileLoadWidget* m_fileWidget{ nullptr };
    QPushButton* m_confirmButton{ nullptr };

    /// @brief The event types that the listener responds to
    /// @details Event types are comma-delimited
    // TODO: Create widget for custom event types
    QLineEdit* m_eventTypes{ nullptr };

    /// @}
};


// End namespaces
}