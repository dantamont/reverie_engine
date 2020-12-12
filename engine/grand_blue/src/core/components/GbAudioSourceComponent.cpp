#include "GbAudioSourceComponent.h"

#include "../GbCoreEngine.h"
#include "../sound/GbSoundManager.h"
#include "../resource/GbResourceCache.h"

#include "../scene/GbScene.h"
#include "../scene/GbScenario.h"
#include "../scene/GbSceneObject.h"

#include "../components/GbTransformComponent.h"

#include <soloud.h>
#include <soloud_wav.h>
#include <soloud_wavstream.h>

namespace Gb {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Audio Component Settings
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue AudioComponentSettings::asJson() const
{
    QJsonObject object;
    object.insert("voiceFlags", (int)m_voiceFlags);
    object.insert("volume", m_volume);
    object.insert("pan", m_pan);
    object.insert("relPlaySpeed", m_relativePlaySpeed);

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AudioComponentSettings::loadFromJson(const QJsonValue & json, const SerializationContext & context)
{
    Q_UNUSED(context)
    QJsonObject object = json.toObject();
    m_voiceFlags = object["voiceFlags"].toInt(0);
    m_volume = object["volume"].toDouble();
    m_pan = object["pan"].toDouble();
    m_relativePlaySpeed = object["relPlaySpeed"].toDouble();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Audio Source Component
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AudioSourceComponent::AudioSourceComponent() :
    Component(ComponentType::kAudioSource)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AudioSourceComponent::AudioSourceComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, ComponentType::kAudioSource)
{
    setSceneObject(sceneObject());
    sceneObject()->addComponent(this);
    sceneObject()->engine()->soundManager()->addSource(this);
}
/////////////////////////////////////////////////////////////////////////////////////////////
AudioSourceComponent::AudioSourceComponent(const AudioSourceComponent & other):
    Component(other.sceneObject(), ComponentType::kAudioSource)
{
    throw("Should never be called, only defined to avoid problems with QMetatype registration");
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AudioSourceComponent::~AudioSourceComponent()
{
    clear();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AudioSourceComponent::addVoice(int handle)
{
    std::unique_lock lock(m_voiceMutex);
    m_activeVoiceHandles.push_back(handle);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AudioSourceComponent::queuePlay()
{
    m_engine->soundManager()->addCommand(new PlayAudioCommand(this));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AudioSourceComponent::queueMove()
{
    //std::unique_lock lock(m_voiceMutex);
    //size_t numVoices = m_activeVoiceHandles.size();
    Vector3 pos = sceneObject()->transform()->getPosition(); // Get by value to avoid worrying about thread-safety
    //for (size_t i = 0; i < numVoices; i++) {
    //    int voiceHandle = m_activeVoiceHandles[i];
    //    //SoLoud::Soloud& soLoud = *m_engine->soundManager()->soLoud();
    //    //soLoud.set3dSourcePosition();
    //}
    m_engine->soundManager()->addCommand(new MoveAudioCommand(this, pos));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AudioSourceComponent::play()
{
    std::shared_ptr<AudioResource> audio = audioResource();
    if (!audio) {
        logWarning("Warning, audio not loaded, cannot play.");
        return;
    }

    // TODO: 3D, pause on start flag, pan, bus, background
    AudioSourceSettings& sourceSettings = audio->audioSourceSettings();
    SoLoud::Soloud& soLoud = *m_engine->soundManager()->soLoud();
    int handle;
    float volume = m_voiceProperties.m_volume * sourceSettings.m_defaultVolume;
    if (m_voiceProperties.testFlag(VoiceFlag::kBackground)) {
        handle = soLoud.playBackground(*audio->audioSource(),
            volume,
            (int)m_voiceProperties.testFlag(VoiceFlag::kPaused));
    }
    else if (m_voiceProperties.testFlag(VoiceFlag::k3D)) {
        // TODO: Implement sound velocity
        Vector3 pos = sceneObject()->transform()->getPosition(); // Get by value to avoid worrying about thread-safety
        handle = soLoud.play3d(*audio->audioSource(),
            pos.x(), pos.y(), pos.z(),
            0, 0, 0,
            (int)m_voiceProperties.testFlag(VoiceFlag::kPaused));
    }
    else {
        handle = soLoud.play(*audio->audioSource(),
            volume,
            m_voiceProperties.m_pan,
            (int)m_voiceProperties.testFlag(VoiceFlag::kPaused)
        );
    }
    
    if (m_voiceProperties.m_relativePlaySpeed != 1.0f) {
        soLoud.setRelativePlaySpeed(handle, m_voiceProperties.m_relativePlaySpeed);
    }

    addVoice(handle);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AudioSourceComponent::move(const Vector3& pos)
{
    std::unique_lock lock(m_voiceMutex);
    SoLoud::Soloud& soLoud = *m_engine->soundManager()->soLoud();
    size_t numVoices = m_activeVoiceHandles.size();
    for (size_t i = 0; i < numVoices; i++) {
        int voiceHandle = m_activeVoiceHandles[i];
        soLoud.set3dSourcePosition(voiceHandle, pos.x(), pos.y(), pos.z());
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AudioSourceComponent::enable()
{
    Component::enable();
}
//////////////// ///////////////////////////////////////////////////////////////////////////////////////////////////////
void AudioSourceComponent::disable()
{
    Component::disable();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue AudioSourceComponent::asJson() const
{
    QJsonObject object = Component::asJson().toObject();
    //object.insert("sourceType", (int)m_sourceType);
    if (m_audioHandle) {
        object.insert("audioSource", m_audioHandle->getPath().c_str());
    }

    object.insert("voiceProperties", m_voiceProperties.asJson());

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AudioSourceComponent::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    // Clear previous audio source
    clear();

    // Call parent class load
    Component::loadFromJson(json, context);
    const QJsonObject& object = json.toObject();

    if (object.contains("audioSource")) {
        m_audioHandle = m_engine->resourceCache()->getTopLevelHandleWithPath(object["audioSource"].toString());
        if (!m_audioHandle) {
            throw("Error, no audio handle created");
        }
    }

    if (object.contains("voiceProperties")) {
        m_voiceProperties.loadFromJson(object["voiceProperties"]);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AudioSourceComponent::clear()
{
    // TODO: Clean up voice groups

    // Remove from sound manager
    sceneObject()->engine()->soundManager()->removeSource(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SoundManager * AudioSourceComponent::soundManager() const
{
    return sceneObject()->engine()->soundManager();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AudioSourceComponent::updateVoices()
{
    // Manage list of active voice handles
    m_voiceMutex.lock();

    // Update vector of active voice handles
    static std::vector<int> activeVoices;
    size_t numVoices = m_activeVoiceHandles.size();
    activeVoices.reserve(numVoices);
    for (size_t i = 0; i < numVoices; i++) {
        int handle = m_activeVoiceHandles[i];
        if (m_engine->soundManager()->soLoud()->isValidVoiceHandle(handle)) {
            // Add voice to temporary active voice vector
            activeVoices.emplace_back(handle);
        }
    }
    activeVoices.swap(m_activeVoiceHandles);
    m_voiceMutex.unlock();
    activeVoices.clear();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing