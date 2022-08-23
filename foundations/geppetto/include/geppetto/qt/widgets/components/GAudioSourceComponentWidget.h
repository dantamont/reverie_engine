#pragma once

// Qt
#include <QtWidgets>

// Internal
#include "GComponentWidget.h"

#include "ripple/network/messages/GModifyAudioComponentResourceMessage.h"
#include "ripple/network/messages/GModifyAudioComponentSourceFlagsMessage.h"
#include "ripple/network/messages/GModifyAudioComponentSourceAttenuationMessage.h"
#include "ripple/network/messages/GModifyAudioComponentSourceVolumeMessage.h"
#include "ripple/network/messages/GModifyAudioComponentSettingsMessage.h"
#include "ripple/network/messages/GRequestAudioResourceDataMessage.h"
#include "ripple/network/messages/GAudioResourceDataMessage.h"
#include "ripple/network/messages/GRequestAudioResourcesDataMessage.h"
#include "ripple/network/messages/GAudioResourcesDataMessage.h"
#include "ripple/network/messages/GRequestAudioResourcesDataMessage.h"
#include "ripple/network/messages/GResourceAddedMessage.h"

namespace rev{

/// @class AudioComponentWidget
/// @brief Widget representing a script component
class AudioComponentWidget : public SceneObjectComponentWidget{
    Q_OBJECT
public:
    /// @name Constructors/Destructor
    /// @{

    AudioComponentWidget(WidgetManager* wm, const json& componentJson, Uint32_t sceneObjectId, QWidget *parent = 0);
    ~AudioComponentWidget();

    /// @}

private:
    /// @name Private Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @brief Returns current index
    size_t populateAudioSources(const GAudioResourcesDataMessage& message);

    /// @brief Populate values based on audio source
    void updateSourceWidgets(const GAudioResourceDataMessage& message);

    void requestResourceUpdate();
    void requestSourceFlagsUpdate();
    void requestSourceAttenuationUpdate();
    void requestSourceVolumeUpdate();
    void requestComponentSettingsUpdate();
    void requestResourceData();
    void requestResourcesData();

    /// @}

    /// @name Private Members
    /// @{


    GModifyAudioComponentResourceMessage m_resourceMessage;
    GModifyAudioComponentSourceFlagsMessage m_sourceFlagsMessage;
    GModifyAudioComponentSourceAttenuationMessage m_sourceAttenuationMessage;
    GModifyAudioComponentSourceVolumeMessage m_sourceVolumeMessage;
    GModifyAudioComponentSettingsMessage m_componentSettingsMessage;
    GRequestAudioResourceDataMessage m_requestResourceMessage;
    GRequestAudioResourcesDataMessage m_requestResourcesMessage;

    /// @brief Audio source to use for the component
    QComboBox* m_audioSources{ nullptr };

    // "Global" settings, relating to audio resource and shared between audio components
    QCheckBox* m_isLooping{ nullptr };
    QCheckBox* m_singleInstance{ nullptr };
    QCheckBox* m_tickOnInaudible{ nullptr };
    QCheckBox* m_killOnInaudible{ nullptr };
    QCheckBox* m_listenerRelative{ nullptr };
    QCheckBox* m_useDistanceDelay{ nullptr };

    QLineEdit* m_minAttenDistance{ nullptr };
    QLineEdit* m_maxAttenDistance{ nullptr };
    QComboBox* m_attenuationModel{ nullptr };
    QLineEdit* m_attenuationRolloff{ nullptr };
    QLineEdit* m_sourceVolume{ nullptr };

    // "Local" settings, applying only for this component
    QCheckBox* m_isProtected{ nullptr };
    QCheckBox* m_isBackground{ nullptr };
    QCheckBox* m_is3d{ nullptr };

    QLineEdit* m_volume{ nullptr };
    QLineEdit* m_pan{ nullptr };
    QLineEdit* m_relativePlaySpeed{ nullptr };

    /// @}
};


// End namespaces        
}
