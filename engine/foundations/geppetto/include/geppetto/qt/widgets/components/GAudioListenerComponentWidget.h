#pragma once

// Qt
#include <QtWidgets>

// Internal
#include "GComponentWidget.h"

#include "ripple/network/messages/GModifyAudioListenerMessage.h"

namespace rev {

/// @class AudioListenerWidget
/// @brief Widget representing a script component
class AudioListenerWidget : public SceneObjectComponentWidget{
    Q_OBJECT
public:
    /// @name Constructors/Destructor
    /// @{

    AudioListenerWidget(WidgetManager* wm, const json& componentJson, Uint32_t sceneObjectId, QWidget *parent = 0);
    ~AudioListenerWidget();

    /// @}

private:
    /// @name Private Methods
    /// @{

    void requestAudioListenerUpdate();

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}

    /// @name Private Members
    /// @{

    GModifyAudioListenerMessage m_listenerMessage;

    QLineEdit* m_velocityX{ nullptr };
    QLineEdit* m_velocityY{ nullptr };
    QLineEdit* m_velocityZ{ nullptr };

    QLineEdit* m_speedOfSound{ nullptr };

    /// @}
};



// End namespaces
}
