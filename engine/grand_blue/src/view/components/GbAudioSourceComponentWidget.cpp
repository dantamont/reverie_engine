#include "GbAudioSourceComponentWidget.h"

#include "../../core/components/GbAudioSourceComponent.h"
#include "../../core/GbCoreEngine.h"
#include "../../core/loop/GbSimLoop.h"
#include "../tree/GbComponentWidget.h"
#include "../../core/resource/GbResourceCache.h"

#include "../../core/scene/GbSceneObject.h"
#include "../../core/readers/GbJsonReader.h"
#include "../../core/components/GbComponent.h"


namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// AudioComponentWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
AudioComponentWidget::AudioComponentWidget(CoreEngine* core, Component* component, QWidget *parent) :
    ComponentWidget(core, component, parent){
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
AudioSourceComponent* AudioComponentWidget::audioComponent() const {
    return static_cast<AudioSourceComponent*>(m_component);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
AudioComponentWidget::~AudioComponentWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AudioComponentWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    // Audio sources widget
    m_audioSources = new QComboBox();
    m_audioSources->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    populateAudioSources();

    std::shared_ptr<ResourceHandle> audioHandle = audioComponent()->audioHandle();

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
    updateSourceWidgets();

    // Component widgets-----------------------------
    AudioComponentSettings& voiceProperties = audioComponent()->voiceProperties();
    m_isProtected = new QCheckBox("Protected");
    m_isProtected->setToolTip(QStringLiteral("Whether or not voices can be deleted when inaudible"));
    m_isProtected->setChecked(voiceProperties.testFlag(VoiceFlag::kProtected));

    // Disable if 3D settings are enabled
    m_isBackground = new QCheckBox("Background");
    m_isBackground->setToolTip(QStringLiteral("Whether or not voices are background (e.g., music)"));
    m_isBackground->setChecked(voiceProperties.testFlag(VoiceFlag::kBackground));
    m_isBackground->setDisabled(voiceProperties.testFlag(VoiceFlag::k3D));

    // Disable if background flag is enabled
    m_is3d = new QCheckBox("3D");
    m_is3d->setToolTip(QStringLiteral("Whether or not sounds use 3D spatial effects"));
    m_is3d->setChecked(voiceProperties.testFlag(VoiceFlag::k3D));
    m_is3d->setDisabled(m_isBackground->isChecked());

    m_volume = new QLineEdit(QString::number(voiceProperties.m_volume));
    m_volume->setValidator(new QDoubleValidator(0.0, 1.0, 4));
    m_volume->setToolTip(QStringLiteral("Volume for voices created using this component. Is applied on top of source volume"));

    m_pan = new QLineEdit(QString::number(voiceProperties.m_pan));
    m_pan->setValidator(new QDoubleValidator(-1.0, 1.0, 4));
    m_pan->setToolTip(QStringLiteral("Panning of voices. -1.0 is left-panned, 1.0 is right-panned"));
    m_pan->setDisabled(voiceProperties.testFlag(VoiceFlag::k3D));

    m_relativePlaySpeed = new QLineEdit(QString::number(voiceProperties.m_relativePlaySpeed));
    m_relativePlaySpeed->setValidator(new QDoubleValidator(1e-6, 1e6, 4));
    m_relativePlaySpeed->setToolTip(QStringLiteral("Relative playback speed of voices. 1.0 is normal speed"));

}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AudioComponentWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();

    // Make connection to set audio source
    connect(m_audioSources, static_cast<void(QComboBox::*)(int)>(
        &QComboBox::currentIndexChanged), this,
        [this](int index) {
        Q_UNUSED(index);
        QString path = m_audioSources->currentData().toString();
        std::shared_ptr<ResourceHandle> handle = nullptr;
        if (!path.isEmpty()) {
            handle = m_engine->resourceCache()->getTopLevelHandleWithPath(path);
            if (!handle) {
                throw("Error, handle with path " + path + "not found");
            }
        }
        audioComponent()->setAudioHandle(handle);

        // Update source widget values
        updateSourceWidgets();
    });

    // Source widgets --------------------------------------------------------------
    connect(m_isLooping, static_cast<void(QCheckBox::*)(int)>(
        &QCheckBox::stateChanged), this,
        [this](int state) {
        bool isChecked = state != 0;

        // Modify settings in audio handle
        const auto& handle = audioComponent()->audioHandle();
        if (!handle) {
            return;
        }
        AudioResource& audioResource = *handle->resourceAs<AudioResource>(false);

        audioResource.audioSourceSettings().setFlag(
            AudioSourceFlag::kLooping, isChecked);

        audioResource.cacheSettings();
    });
    connect(m_singleInstance, static_cast<void(QCheckBox::*)(int)>(
        &QCheckBox::stateChanged), this,
        [this](int state) {
        bool isChecked = state != 0;

        // Modify settings in audio handle
        const auto& handle = audioComponent()->audioHandle();
        if (!handle) {
            return;
        }
        AudioResource& audioResource = *handle->resourceAs<AudioResource>(false);

        audioResource.audioSourceSettings().setFlag(
            AudioSourceFlag::kSingleInstance, isChecked);

        audioResource.cacheSettings();
    });
    connect(m_tickOnInaudible, static_cast<void(QCheckBox::*)(int)>(
        &QCheckBox::stateChanged), this,
        [this](int state) {
        bool isChecked = state != 0;

        // Modify settings in audio handle
        const auto& handle = audioComponent()->audioHandle();
        if (!handle) {
            return;
        }
        AudioResource& audioResource = *handle->resourceAs<AudioResource>(false);

        audioResource.audioSourceSettings().setFlag(
            AudioSourceFlag::kTickOnInaudible, isChecked);

        audioResource.cacheSettings();
    });
    connect(m_killOnInaudible, static_cast<void(QCheckBox::*)(int)>(
        &QCheckBox::stateChanged), this,
        [this](int state) {
        bool isChecked = state != 0;

        // Modify settings in audio handle
        const auto& handle = audioComponent()->audioHandle();
        if (!handle) {
            return;
        }
        AudioResource& audioResource = *handle->resourceAs<AudioResource>(false);

        audioResource.audioSourceSettings().setFlag(
            AudioSourceFlag::kKillOnInaudible, isChecked);

        audioResource.cacheSettings();
    });
    connect(m_listenerRelative, static_cast<void(QCheckBox::*)(int)>(
        &QCheckBox::stateChanged), this,
        [this](int state) {
        bool isChecked = state != 0;

        // Modify settings in audio handle
        const auto& handle = audioComponent()->audioHandle();
        if (!handle) {
            return;
        }
        AudioResource& audioResource = *handle->resourceAs<AudioResource>(false);

        audioResource.audioSourceSettings().setFlag(
            AudioSourceFlag::kListenerRelative, isChecked);

        audioResource.cacheSettings();
    });
    connect(m_useDistanceDelay, static_cast<void(QCheckBox::*)(int)>(
        &QCheckBox::stateChanged), this,
        [this](int state) {
        bool isChecked = state != 0;

        // Modify settings in audio handle
        const auto& handle = audioComponent()->audioHandle();
        if (!handle) {
            return;
        }
        AudioResource& audioResource = *handle->resourceAs<AudioResource>(false);

        audioResource.audioSourceSettings().setFlag(
            AudioSourceFlag::kUseDistanceDelay, isChecked);

        audioResource.cacheSettings();
    });

    connect(m_minAttenDistance, static_cast<void(QLineEdit::*)(void)>(
        &QLineEdit::editingFinished), this,
        [this]() {
        // Modify settings in audio handle
        const auto& handle = audioComponent()->audioHandle();
        if (!handle) {
            return;
        }
        AudioResource& audioResource = *handle->resourceAs<AudioResource>(false);
        const QString& text = m_minAttenDistance->text();
        double minDistance = text.toDouble();

        audioResource.audioSourceSettings().m_minDistance = minDistance;

        audioResource.cacheSettings();
    });
    connect(m_maxAttenDistance, static_cast<void(QLineEdit::*)(void)>(
        &QLineEdit::editingFinished), this,
        [this]() {
        // Modify settings in audio handle
        const auto& handle = audioComponent()->audioHandle();
        if (!handle) {
            return;
        }
        AudioResource& audioResource = *handle->resourceAs<AudioResource>(false);
        const QString& text = m_minAttenDistance->text();
        double maxDistance = text.toDouble();

        audioResource.audioSourceSettings().m_maxDistance = maxDistance;

        audioResource.cacheSettings();
    });
    connect(m_attenuationModel, static_cast<void(QComboBox::*)(int)>(
        &QComboBox::currentIndexChanged), this,
        [this](int index) {
        // Modify settings in audio handle
        const auto& handle = audioComponent()->audioHandle();
        if (!handle) {
            return;
        }

        AudioResource& audioResource = *handle->resourceAs<AudioResource>(false);
        audioResource.audioSourceSettings().m_attenuationModel = index;

        audioResource.cacheSettings();
    });
    connect(m_attenuationRolloff, static_cast<void(QLineEdit::*)(void)>(
        &QLineEdit::editingFinished), this,
        [this]() {
        // Modify settings in audio handle
        const auto& handle = audioComponent()->audioHandle();
        if (!handle) {
            return;
        }
        AudioResource& audioResource = *handle->resourceAs<AudioResource>(false);
        const QString& text = m_attenuationRolloff->text();
        double rolloff = text.toDouble();

        audioResource.audioSourceSettings().m_rolloff = rolloff;

        audioResource.cacheSettings();
    });

    connect(m_sourceVolume, static_cast<void(QLineEdit::*)(void)>(
        &QLineEdit::editingFinished), this,
        [this]() {
        // Modify settings in audio handle
        const auto& handle = audioComponent()->audioHandle();
        if (!handle) {
            return;
        }
        AudioResource& audioResource = *handle->resourceAs<AudioResource>(false);
        const QString& text = m_sourceVolume->text();
        double volume = text.toDouble();

        audioResource.audioSourceSettings().m_defaultVolume = volume;

        audioResource.cacheSettings();
    });
    // End source widgets ----------------------------------------------------

    // Component-specific widgets --------------------------------------------
    connect(m_isProtected, static_cast<void(QCheckBox::*)(int)>(
        &QCheckBox::stateChanged), this,
        [this](int state) {
        bool isChecked = state != 0;
        AudioComponentSettings& settings = audioComponent()->voiceProperties();
        settings.setFlag(VoiceFlag::kProtected, isChecked);
    });
    connect(m_isBackground, static_cast<void(QCheckBox::*)(int)>(
        &QCheckBox::stateChanged), this,
        [this](int state) {
        bool isChecked = state != 0;
        AudioComponentSettings& settings = audioComponent()->voiceProperties();
        settings.setFlag(VoiceFlag::kBackground, isChecked);
        m_is3d->setDisabled(isChecked);
    });
    connect(m_is3d, static_cast<void(QCheckBox::*)(int)>(
        &QCheckBox::stateChanged), this,
        [this](int state) {
        bool isChecked = state != 0;
        AudioComponentSettings& settings = audioComponent()->voiceProperties();
        settings.setFlag(VoiceFlag::k3D, isChecked);
        m_isBackground->setDisabled(isChecked);
        m_pan->setDisabled(isChecked);
    });

    connect(m_volume, static_cast<void(QLineEdit::*)(void)>(
        &QLineEdit::editingFinished), this,
        [this]() {
        AudioComponentSettings& settings = audioComponent()->voiceProperties();
        const QString& text = m_volume->text();
        settings.m_volume = text.toDouble();
    });
    connect(m_pan, static_cast<void(QLineEdit::*)(void)>(
        &QLineEdit::editingFinished), this,
        [this]() {
        AudioComponentSettings& settings = audioComponent()->voiceProperties();
        const QString& text = m_pan->text();
        settings.m_pan = text.toDouble();
    });
    connect(m_relativePlaySpeed, static_cast<void(QLineEdit::*)(void)>(
        &QLineEdit::editingFinished), this,
        [this]() {
        AudioComponentSettings& settings = audioComponent()->voiceProperties();
        const QString& text = m_relativePlaySpeed->text();
        settings.m_relativePlaySpeed = text.toDouble();
    });

    // Connect to resource cache to reinitialize audio source list on load
    connect(m_engine->resourceCache(), &ResourceCache::resourceAdded,
        this, [this](std::shared_ptr<ResourceHandle> handle) {
        if (handle->getResourceType() != Resource::kAudio) {
            return;
        }

        // New audio is resource, so repopulate widget
        populateAudioSources();
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////////////////////////
size_t AudioComponentWidget::populateAudioSources()
{
    m_audioSources->clear();

    // Need to add a blank option!
    m_audioSources->addItem(QStringLiteral("None"), QStringLiteral(""));

    size_t currentIndex = 0;
    size_t count = 0;
    for (const auto& audioPair : m_engine->resourceCache()->audio()) {
        count++; // Start by incrementing, since 0 index is null resource

        QString path = QString(audioPair.second->getPath().c_str());
        m_audioSources->addItem(audioPair.second->getName(), path);
        if (audioComponent()->audioHandle()) {
            if (audioPair.second->getPath() == audioComponent()->audioHandle()->getPath()) {
                currentIndex = count;
            }
        }
    }

    return currentIndex;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AudioComponentWidget::updateSourceWidgets()
{
    std::shared_ptr<ResourceHandle> audioHandle = audioComponent()->audioHandle();
    if (!audioHandle) {
        return;
    }
    const AudioResource& audioResource = *audioHandle->resourceAs<AudioResource>();
    const AudioSourceSettings& settings = audioResource.audioSourceSettings();

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
    m_isLooping->setChecked(settings.testFlag(AudioSourceFlag::kLooping));
    m_singleInstance->setChecked(settings.testFlag(AudioSourceFlag::kSingleInstance));
    m_tickOnInaudible->setChecked(settings.testFlag(AudioSourceFlag::kTickOnInaudible));
    m_killOnInaudible->setChecked(settings.testFlag(AudioSourceFlag::kKillOnInaudible));
    m_listenerRelative->setChecked(settings.testFlag(AudioSourceFlag::kListenerRelative));
    m_useDistanceDelay->setChecked(settings.testFlag(AudioSourceFlag::kUseDistanceDelay));

    m_minAttenDistance->setText(QString::number(settings.m_minDistance));
    m_maxAttenDistance->setText(QString::number(settings.m_maxDistance));
    m_attenuationModel->setCurrentIndex(settings.m_attenuationModel);
    m_attenuationRolloff->setText(QString::number(settings.m_rolloff));

    m_sourceVolume->setText(QString::number(settings.m_defaultVolume));

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


///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
} // View
} // Gb