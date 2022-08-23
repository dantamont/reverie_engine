#include "geppetto/qt/widgets/components/GAudioSourceComponentWidget.h"

#include "geppetto/qt/widgets/components/GComponentWidget.h"
#include "fortress/json/GJson.h"

#include "enums/GAudioSourceFlagEnum.h"
#include "enums/GVoiceFlagEnum.h"
#include "enums/GResourceTypeEnum.h"

namespace rev {

AudioComponentWidget::AudioComponentWidget(WidgetManager* wm, const json& componentJson, Uint32_t sceneObjectId, QWidget *parent) :
    SceneObjectComponentWidget(wm, componentJson, sceneObjectId, parent){
    m_resourceMessage.setSceneObjectId(sceneObjectId);
    m_sourceFlagsMessage.setSceneObjectId(sceneObjectId);
    m_sourceAttenuationMessage.setSceneObjectId(sceneObjectId);
    m_sourceVolumeMessage.setSceneObjectId(sceneObjectId);
    m_componentSettingsMessage.setSceneObjectId(sceneObjectId);
    m_requestResourceMessage.setSceneObjectId(sceneObjectId);
    m_requestResourcesMessage.setSceneObjectId(sceneObjectId);

    initialize();
}

AudioComponentWidget::~AudioComponentWidget()
{
}

void AudioComponentWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    // Audio sources widget
    m_audioSources = new QComboBox();
    m_audioSources->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_requestResourcesMessage);

    // Source widgets -------------------------------
    QGroupBox* sourceBox = new QGroupBox(QStringLiteral("Audio Source"));
    sourceBox->setToolTip(QStringLiteral("Global settings for the audio resource."));

    // Checkboxes
    m_isLooping = new QCheckBox("Loop");
    m_isLooping->setChecked(false);
    m_isLooping->setToolTip(QStringLiteral("Whether or not sounds will loop when played"));

    m_singleInstance = new QCheckBox("Single Instance");
    m_singleInstance->setChecked(false);
    m_singleInstance->setToolTip(QStringLiteral("Whether or not more than one sound can play at a time"));

    m_tickOnInaudible = new QCheckBox("Tick");
    m_tickOnInaudible->setToolTip(QStringLiteral("Whether or not to play sounds when inaudible"));

    m_killOnInaudible = new QCheckBox("Kill");
    m_killOnInaudible->setToolTip(QStringLiteral("Whether or not to delete sounds when inaudible"));

    m_listenerRelative = new QCheckBox("Listener Relative");
    m_listenerRelative->setToolTip(QStringLiteral("Whether or not 3D coordinates are specified relative to a listener at (0, 0, 0)"));

    m_useDistanceDelay = new QCheckBox("Distance Delay");
    m_useDistanceDelay->setToolTip(QStringLiteral("Whether or not playback is delayed based on sound distance from listener"));

    // Attenuation model
    m_minAttenDistance = new QLineEdit("1.0");
    m_minAttenDistance->setValidator(new QDoubleValidator(1e-6, 1e12, 6));
    m_maxAttenDistance = new QLineEdit("1e6");
    m_maxAttenDistance->setValidator(new QDoubleValidator(1e-6, 1e12, 6));

    m_attenuationModel = new QComboBox();
    m_attenuationModel->addItem("No Attenuation", 0);
    m_attenuationModel->addItem("Inverse Distance", 1);
    m_attenuationModel->addItem("Linear Distance", 2);
    m_attenuationModel->addItem("Exponential Distance", 3);

    m_attenuationRolloff = new QLineEdit("1.0");
    m_attenuationRolloff->setValidator(new QDoubleValidator(1.0, 1e12, 6));
    m_attenuationRolloff->setToolTip("Scaling factor for how steeply volume drops off due to attenuation");

    // Source Volume
    m_sourceVolume = new QLineEdit("1.0");
    m_sourceVolume->setValidator(new QDoubleValidator(0.0, 1.0, 4));
    m_sourceVolume->setToolTip(QStringLiteral("Global volume for the sound file used by this component"));

    // Update all source-related widget values
    requestResourceData();

    // Component widgets-----------------------------
    const json& voiceProperties = m_componentJson["voiceProperties"];
    EVoiceFlags voiceFlags = voiceProperties["voiceFlags"].get<Int32_t>();
    m_isProtected = new QCheckBox("Protected");
    m_isProtected->setToolTip(QStringLiteral("Whether or not voices can be deleted when inaudible"));
    m_isProtected->setChecked(voiceFlags.testFlag(EVoiceFlag::eProtected));

    // Disable if 3D settings are enabled
    m_isBackground = new QCheckBox("Background");
    m_isBackground->setToolTip(QStringLiteral("Whether or not voices are background (e.g., music)"));
    m_isBackground->setChecked(voiceFlags.testFlag(EVoiceFlag::eBackground));
    m_isBackground->setDisabled(voiceFlags.testFlag(EVoiceFlag::e3d));

    // Disable if background flag is enabled
    m_is3d = new QCheckBox("3D");
    m_is3d->setToolTip(QStringLiteral("Whether or not sounds use 3D spatial effects"));
    m_is3d->setChecked(voiceFlags.testFlag(EVoiceFlag::e3d));
    m_is3d->setDisabled(m_isBackground->isChecked());

    m_volume = new QLineEdit(QString::number(voiceProperties["volume"].get<Float32_t>()));
    m_volume->setValidator(new QDoubleValidator(0.0, 1.0, 4));
    m_volume->setToolTip(QStringLiteral("Volume for voices created using this component. Is applied on top of source volume"));

    m_pan = new QLineEdit(QString::number(voiceProperties["pan"].get<Float32_t>()));
    m_pan->setValidator(new QDoubleValidator(-1.0, 1.0, 4));
    m_pan->setToolTip(QStringLiteral("Panning of voices. -1.0 is left-panned, 1.0 is right-panned"));
    m_pan->setDisabled(voiceFlags.testFlag(EVoiceFlag::e3d));

    m_relativePlaySpeed = new QLineEdit(QString::number(voiceProperties["relPlaySpeed"].get<Float32_t>()));
    m_relativePlaySpeed->setValidator(new QDoubleValidator(1e-6, 1e6, 4));
    m_relativePlaySpeed->setToolTip(QStringLiteral("Relative playback speed of voices. 1.0 is normal speed"));

}

void AudioComponentWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();

    // Make connection to set audio source
    connect(m_audioSources, static_cast<void(QComboBox::*)(int)>(
        &QComboBox::currentIndexChanged), this,
        [this](int index) {
            Q_UNUSED(index);
            requestResourceData();
        }
    );

    // Source widgets
    connect(m_isLooping, static_cast<void(QCheckBox::*)(int)>(
        &QCheckBox::stateChanged), this,
        [this](int state) {
            Q_UNUSED(state);
            requestSourceFlagsUpdate();
        }
    );
    connect(m_singleInstance, static_cast<void(QCheckBox::*)(int)>(
        &QCheckBox::stateChanged), this,
        [this](int state) {
            Q_UNUSED(state);
            requestSourceFlagsUpdate();
        }
    );
    connect(m_tickOnInaudible, static_cast<void(QCheckBox::*)(int)>(
        &QCheckBox::stateChanged), this,
        [this](int state) {
            Q_UNUSED(state);
            requestSourceFlagsUpdate();
        }
    );
    connect(m_killOnInaudible, static_cast<void(QCheckBox::*)(int)>(
        &QCheckBox::stateChanged), this,
        [this](int state) {
            Q_UNUSED(state);
            requestSourceFlagsUpdate();
        }
    );
    connect(m_listenerRelative, static_cast<void(QCheckBox::*)(int)>(
        &QCheckBox::stateChanged), this,
        [this](int state) {
            Q_UNUSED(state);
            requestSourceFlagsUpdate();
        }
    );
    connect(m_useDistanceDelay, static_cast<void(QCheckBox::*)(int)>(
        &QCheckBox::stateChanged), this,
        [this](int state) {
            Q_UNUSED(state);
            requestSourceFlagsUpdate();
        }
    );

    connect(m_minAttenDistance, static_cast<void(QLineEdit::*)(void)>(
        &QLineEdit::editingFinished), this,
        [this]() {
            requestSourceAttenuationUpdate();
        }
    );
    connect(m_maxAttenDistance, static_cast<void(QLineEdit::*)(void)>(
        &QLineEdit::editingFinished), this,
        [this]() {
            requestSourceAttenuationUpdate();
        }
    );
    connect(m_attenuationModel, static_cast<void(QComboBox::*)(int)>(
        &QComboBox::currentIndexChanged), this,
        [this](int index) {
            Q_UNUSED(index);
            requestSourceAttenuationUpdate();
        }
    );
    connect(m_attenuationRolloff, static_cast<void(QLineEdit::*)(void)>(
        &QLineEdit::editingFinished), this,
        [this]() {
            requestSourceAttenuationUpdate();
        }
    );

    connect(m_sourceVolume, static_cast<void(QLineEdit::*)(void)>(
        &QLineEdit::editingFinished), this,
        [this]() {
            requestSourceVolumeUpdate();
        }   
    );


    // Component-specific widgets --------------------------------------------
    connect(m_isProtected, static_cast<void(QCheckBox::*)(int)>(
        &QCheckBox::stateChanged), this,
        [this](int state) {
            Q_UNUSED(state);
            requestComponentSettingsUpdate();
        }
    );
    connect(m_isBackground, static_cast<void(QCheckBox::*)(int)>(
        &QCheckBox::stateChanged), this,
        [this](int state) {
            bool isChecked = state != 0;
            requestComponentSettingsUpdate();
            m_is3d->setDisabled(isChecked);
        }
    );
    connect(m_is3d, static_cast<void(QCheckBox::*)(int)>(
        &QCheckBox::stateChanged), this,
        [this](int state) {
            bool isChecked = state != 0;
            requestComponentSettingsUpdate();
            m_isBackground->setDisabled(isChecked);
            m_pan->setDisabled(isChecked);
        }
    );

    connect(m_volume, static_cast<void(QLineEdit::*)(void)>(
        &QLineEdit::editingFinished), this,
        [this]() {
            requestComponentSettingsUpdate();
        }
    );
    connect(m_pan, static_cast<void(QLineEdit::*)(void)>(
        &QLineEdit::editingFinished), this,
        [this]() {
            requestComponentSettingsUpdate();
        }
    );
    connect(m_relativePlaySpeed, static_cast<void(QLineEdit::*)(void)>(
        &QLineEdit::editingFinished), this,
        [this]() {
            requestComponentSettingsUpdate();
        }
    );

    connect(m_widgetManager, 
        &WidgetManager::receivedAudioResourceDataMessage, 
        this, 
        &AudioComponentWidget::updateSourceWidgets);

    // Connect to resource cache to reinitialize audio source list on load
    connect(m_widgetManager, &WidgetManager::receivedResourceAddedMessage,
        this, [this](const GResourceAddedMessage& message) {

        if (message.getResourceType() != (size_t)EResourceType::eAudio) {
            return;
        }

        // New audio is resource, so request repopulation
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_requestResourcesMessage);
    });

    connect(m_widgetManager,
        &WidgetManager::receivedAudioResourcesDataMessage,
        this,
        &AudioComponentWidget::populateAudioSources);

}

void AudioComponentWidget::layoutWidgets()
{
    // Create base widget layout
    ComponentWidget::layoutWidgets();

    // Create new widgets
    QBoxLayout* layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addWidget(new QLabel("Audio Source:"));
    layout->addWidget(m_audioSources);

    // Source widgets -------------------------------
    QGroupBox* sourceWidgets = new QGroupBox("Audio Source");
    sourceWidgets->setToolTip(QStringLiteral("Global settings for the audio file, shared between components"));
    QGridLayout* sourceGrid = new QGridLayout();
    sourceGrid->addWidget(m_isLooping, 0, 0, 1, 2); // row, column, row span, column span
    sourceGrid->addWidget(m_singleInstance, 0, 2, 1, 2);
    sourceGrid->addWidget(m_tickOnInaudible, 1, 0, 1, 2);
    sourceGrid->addWidget(m_killOnInaudible, 1, 2, 1, 2);
    sourceGrid->addWidget(m_listenerRelative, 2, 0, 1, 2);
    sourceGrid->addWidget(m_useDistanceDelay, 2, 2, 1, 2);
    sourceGrid->addWidget(new QLabel("Fade Start:"), 3, 0, 1, 1);
    sourceGrid->addWidget(m_minAttenDistance, 3, 1, 1, 1);
    sourceGrid->addWidget(new QLabel("Fade End:"), 3, 2, 1, 1);
    sourceGrid->addWidget(m_maxAttenDistance, 3, 3, 1, 1);
    sourceGrid->addWidget(m_attenuationModel, 4, 0, 1, 2);
    sourceGrid->addWidget(new QLabel("Rolloff:"), 4, 2, 1, 1);
    sourceGrid->addWidget(m_attenuationRolloff, 4, 3, 1, 1);
    sourceGrid->addWidget(new QLabel("Volume:"), 5, 0, 1, 2);
    sourceGrid->addWidget(m_sourceVolume, 5, 1, 1, 3);

    sourceGrid->setSpacing(5);
    sourceWidgets->setLayout(sourceGrid);

    // Component widgets-----------------------------
    QGroupBox* voiceWidgets = new QGroupBox("Voice Settings");
    voiceWidgets->setToolTip(QStringLiteral("Settings for the voices generated by this component."));
    QGridLayout* voiceGrid = new QGridLayout();
    voiceGrid->addWidget(m_isProtected, 0, 0, 1, 2); // row, column, row span, column span
    voiceGrid->addWidget(m_isBackground, 0, 2, 1, 2);
    voiceGrid->addWidget(m_is3d, 1, 0, 1, 2);
    voiceGrid->addWidget(new QLabel("Volume:"), 2, 0, 1, 1);
    voiceGrid->addWidget(m_volume, 2, 1, 1, 1);
    voiceGrid->addWidget(new QLabel("Pan:"), 2, 2, 1, 1);
    voiceGrid->addWidget(m_pan, 2, 3, 1, 2);
    voiceGrid->addWidget(new QLabel("Play Speed:"), 3, 0, 1, 1);
    voiceGrid->addWidget(m_relativePlaySpeed, 3, 1, 1, 1);

    voiceGrid->setSpacing(5);
    voiceWidgets->setLayout(voiceGrid);


    // Add widgets to main layout -------------------
    m_mainLayout->addLayout(layout);
    m_mainLayout->addWidget(sourceWidgets);
    m_mainLayout->addWidget(voiceWidgets);
}


size_t AudioComponentWidget::populateAudioSources(const GAudioResourcesDataMessage& message)
{
    m_audioSources->clear();

    // Need to add a blank option!
    m_audioSources->addItem(QStringLiteral("None"), QStringLiteral(""));

    json resourceJson = GJson::FromBytes(message.getJsonBytes());

    // Audio can only be a top-level resource
    size_t currentIndex = 0;
    size_t len = resourceJson["names"].size();
    const json& names = resourceJson["names"];
    const json& paths = resourceJson["paths"];
    for (size_t i = 0; i < len; i++) {
        // Get path to audio file and add as item
        m_audioSources->addItem(
            names[i].get_ref<const std::string&>().c_str(), 
            paths[i].get_ref<const std::string&>().c_str()
        );
        if (m_componentJson.contains("audioSource")) {
            // If path matches that of the component's current audio handle, set as index
            const std::string& path = m_componentJson["audioSource"];
            if (paths[i].get_ref<const std::string&>() == path) {
                currentIndex = i;
            }
        }
    }

    return currentIndex;
}

void AudioComponentWidget::updateSourceWidgets(const GAudioResourceDataMessage& message)
{
    if (!m_componentJson.contains("audioSource")) {
        return;
    }

    json resourcejson = GJson::FromBytes(message.getJsonBytes());
    const json& settings = resourcejson["audioSettings"];

    // Block all signals to prevent callbacks from being used
    m_isLooping->blockSignals(true);
    m_singleInstance->blockSignals(true);
    m_tickOnInaudible->blockSignals(true);
    m_killOnInaudible->blockSignals(true);
    m_listenerRelative->blockSignals(true);
    m_useDistanceDelay->blockSignals(true);
    m_minAttenDistance->blockSignals(true);
    m_maxAttenDistance->blockSignals(true);
    m_attenuationModel->blockSignals(true);
    m_attenuationRolloff->blockSignals(true);
    m_sourceVolume->blockSignals(true);

    // Set widget values
    EAudioSourceFlags sourceFlags = settings["sourceFlags"].get<Uint32_t>();

    m_isLooping->setChecked(sourceFlags.testFlag(EAudioSourceFlag::eLooping));
    m_singleInstance->setChecked(sourceFlags.testFlag(EAudioSourceFlag::eSingleInstance));
    m_tickOnInaudible->setChecked(sourceFlags.testFlag(EAudioSourceFlag::eTickOnInaudible));
    m_killOnInaudible->setChecked(sourceFlags.testFlag(EAudioSourceFlag::eKillOnInaudible));
    m_listenerRelative->setChecked(sourceFlags.testFlag(EAudioSourceFlag::eListenerRelative));
    m_useDistanceDelay->setChecked(sourceFlags.testFlag(EAudioSourceFlag::eUseDistanceDelay));

    Float64_t minDistance = settings["minDistance"];
    Float64_t maxDistance = settings["maxDistance"];
    Float64_t rolloff = settings["rolloff"];
    Float64_t attenModel = settings["attenModel"];
    Float64_t volume = settings["volume"];
    m_minAttenDistance->setText(QString::number(minDistance));
    m_maxAttenDistance->setText(QString::number(maxDistance));
    m_attenuationModel->setCurrentIndex(attenModel);
    m_attenuationRolloff->setText(QString::number(rolloff));

    m_sourceVolume->setText(QString::number(volume));

    // Unblock all signals
    m_isLooping->blockSignals(false);
    m_singleInstance->blockSignals(false);
    m_tickOnInaudible->blockSignals(false);
    m_killOnInaudible->blockSignals(false);
    m_listenerRelative->blockSignals(false);
    m_useDistanceDelay->blockSignals(false);
    m_minAttenDistance->blockSignals(false);
    m_maxAttenDistance->blockSignals(false);
    m_attenuationModel->blockSignals(false);
    m_attenuationRolloff->blockSignals(false);
    m_sourceVolume->blockSignals(false);
}

void AudioComponentWidget::requestResourceUpdate()
{
    GString path = m_audioSources->currentData().toString().toStdString();
    m_resourceMessage.setResourcePath(path.c_str());
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_resourceMessage);
}

void AudioComponentWidget::requestSourceFlagsUpdate()
{
    EAudioSourceFlags sourceFlags = 0;
    sourceFlags.setFlag(EAudioSourceFlag::eLooping, m_isLooping->isChecked());
    sourceFlags.setFlag(EAudioSourceFlag::eSingleInstance, m_singleInstance->isChecked());
    sourceFlags.setFlag(EAudioSourceFlag::eTickOnInaudible, m_tickOnInaudible->isChecked());
    sourceFlags.setFlag(EAudioSourceFlag::eKillOnInaudible, m_killOnInaudible->isChecked());
    sourceFlags.setFlag(EAudioSourceFlag::eListenerRelative, m_listenerRelative->isChecked());
    sourceFlags.setFlag(EAudioSourceFlag::eUseDistanceDelay, m_useDistanceDelay->isChecked());
    m_sourceFlagsMessage.setAudioSourceFlags(sourceFlags);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_sourceFlagsMessage);
}

void AudioComponentWidget::requestSourceAttenuationUpdate()
{
    Float64_t minAttenuation = m_minAttenDistance->text().toDouble();
    Float64_t maxAttenuation = m_maxAttenDistance->text().toDouble();
    Int32_t attenModel = m_attenuationModel->currentIndex();
    Float64_t rolloff = m_attenuationRolloff->text().toDouble();
    m_sourceAttenuationMessage.setMinDistance(minAttenuation);
    m_sourceAttenuationMessage.setMaxDistance(maxAttenuation);
    m_sourceAttenuationMessage.setAttenuationModel(attenModel);
    m_sourceAttenuationMessage.setAttenuationRolloff(rolloff);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_sourceAttenuationMessage);
}

void AudioComponentWidget::requestSourceVolumeUpdate()
{
    Float64_t sourceVolume = m_sourceVolume->text().toDouble();
    m_sourceVolumeMessage.setVolume(sourceVolume);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_sourceVolumeMessage);
}

void AudioComponentWidget::requestComponentSettingsUpdate()
{
    EVoiceFlags voiceFlags = 0;
    voiceFlags.setFlag(EVoiceFlag::eProtected, m_isProtected->isChecked());
    voiceFlags.setFlag(EVoiceFlag::eBackground, m_isBackground->isChecked());
    voiceFlags.setFlag(EVoiceFlag::e3d, m_is3d->isChecked());

    m_componentSettingsMessage.setVoiceFlags(voiceFlags);
    m_componentSettingsMessage.setVolume(m_volume->text().toDouble());
    m_componentSettingsMessage.setPan(m_pan->text().toDouble());
    m_componentSettingsMessage.setRelativePlaySpeed(m_relativePlaySpeed->text().toDouble());

    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_componentSettingsMessage);

}

void AudioComponentWidget::requestResourceData()
{
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_requestResourceMessage);
}

void AudioComponentWidget::requestResourcesData()
{
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_requestResourcesMessage);
}


} // rev