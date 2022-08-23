#include "core/components/GAudioSourceComponent.h"

#include "core/GCoreEngine.h"
#include "core/sound/GSoundManager.h"
#include "core/resource/GResourceCache.h"

#include "core/scene/GScene.h"
#include "core/scene/GScenario.h"
#include "core/scene/GSceneObject.h"

#include "core/components/GTransformComponent.h"

#include <soloud.h>
#include <soloud_wav.h>
#include <soloud_wavstream.h>

namespace rev {


// Audio Component Settings

void to_json(json& orJson, const AudioComponentSettings& korObject)
{
    orJson["voiceFlags"] = (int)korObject.m_voiceFlags;
    orJson["volume"] = korObject.m_volume;
    orJson["pan"] = korObject.m_pan;
    orJson["relPlaySpeed"] = korObject.m_relativePlaySpeed;
}

void from_json(const json& korJson, AudioComponentSettings& orObject)
{    
    orObject.m_voiceFlags = korJson.value("voiceFlags", 0);
    korJson["volume"].get_to(orObject.m_volume);
    korJson["pan"].get_to(orObject.m_pan);
    korJson["relPlaySpeed"].get_to(orObject.m_relativePlaySpeed);
}



// Audio Source Component

AudioSourceComponent::AudioSourceComponent() :
    Component(ComponentType::kAudioSource)
{
}

AudioSourceComponent::AudioSourceComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, ComponentType::kAudioSource)
{
    setSceneObject(sceneObject());
    sceneObject()->setComponent(this);
    sceneObject()->scene()->engine()->soundManager()->addSource(this);
}

AudioSourceComponent::AudioSourceComponent(const AudioSourceComponent & other):
    Component(other.sceneObject(), ComponentType::kAudioSource)
{
    Logger::Throw("Should never be called, only defined to avoid problems with QMetatype registration");
}

AudioSourceComponent::~AudioSourceComponent()
{
}

void AudioSourceComponent::addVoice(int handle)
{
    std::unique_lock lock(m_voiceMutex);
    m_activeVoiceHandles.push_back(handle);
}

void AudioSourceComponent::queuePlay()
{
    sceneObject()->scene()->engine()->soundManager()->addCommand(new PlayAudioCommand(this));
}

void AudioSourceComponent::queueMove()
{
    //std::unique_lock lock(m_voiceMutex);
    //size_t numVoices = m_activeVoiceHandles.size();
    Vector3 pos = sceneObject()->transform().getPosition(); // Get by value to avoid worrying about thread-safety
    //for (size_t i = 0; i < numVoices; i++) {
    //    int voiceHandle = m_activeVoiceHandles[i];
    //    //SoLoud::Soloud& soLoud = *sceneObject()->scene()->engine()->soundManager()->soLoud();
    //    //soLoud.set3dSourcePosition();
    //}
    sceneObject()->scene()->engine()->soundManager()->addCommand(new MoveAudioCommand(this, pos));
}

void AudioSourceComponent::play()
{
    AudioResource* audio = audioResource();
    if (!audio) {
        Logger::LogWarning("Warning, audio not loaded, cannot play.");
        return;
    }

    // TODO: 3D, pause on start flag, pan, bus, background
    AudioSourceSettings& sourceSettings = audio->audioSourceSettings();
    SoLoud::Soloud& soLoud = *sceneObject()->scene()->engine()->soundManager()->soLoud();
    int handle;
    float volume = m_voiceProperties.m_volume * sourceSettings.m_defaultVolume;
    if (m_voiceProperties.testFlag(VoiceFlag::kBackground)) {
        handle = soLoud.playBackground(*audio->audioSource(),
            volume,
            (int)m_voiceProperties.testFlag(VoiceFlag::kPaused));
    }
    else if (m_voiceProperties.testFlag(VoiceFlag::k3D)) {
        // TODO: Implement sound velocity
        Vector3 pos = sceneObject()->transform().getPosition(); // Get by value to avoid worrying about thread-safety
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

void AudioSourceComponent::move(const Vector3& pos)
{
    std::unique_lock lock(m_voiceMutex);
    SoLoud::Soloud& soLoud = *sceneObject()->scene()->engine()->soundManager()->soLoud();
    size_t numVoices = m_activeVoiceHandles.size();
    for (size_t i = 0; i < numVoices; i++) {
        int voiceHandle = m_activeVoiceHandles[i];
        soLoud.set3dSourcePosition(voiceHandle, pos.x(), pos.y(), pos.z());
    }
}

void AudioSourceComponent::enable()
{
    Component::enable();
}
 
void AudioSourceComponent::disable()
{
    Component::disable();
}

void AudioSourceComponent::preDestruction(CoreEngine * core)
{
    clear(core);
}

void to_json(json& orJson, const AudioSourceComponent& korObject)
{
    // Parent class serialization
    ToJson<Component>(orJson, korObject);

    if (korObject.m_audioHandle) {
        orJson["audioSource"] = korObject.m_audioHandle->getPath().c_str();
    }

    orJson["voiceProperties"] = korObject.m_voiceProperties;
}

void from_json(const json& korJson, AudioSourceComponent& orObject)
{
    // Clear previous audio source
    orObject.clear(nullptr);

    // Call parent class load
    FromJson<Component>(korJson, orObject);

    if (korJson.contains("audioSource")) {
        orObject.m_audioHandle = ResourceCache::Instance().getTopLevelHandleWithPath(
            korJson.at("audioSource").get_ref<const std::string&>().c_str());
        if (!orObject.m_audioHandle) {
            Logger::Throw("Error, no audio handle created");
        }
    }

    if (korJson.contains("voiceProperties")) {
        korJson.at("voiceProperties").get_to(orObject.m_voiceProperties);
    }
}

void AudioSourceComponent::clear(CoreEngine* core)
{
    // TODO: Clean up voice groups

    // Remove from sound manager
    if (!core) {
        if (!sceneObject()) {
            Logger::Throw("Error, no way to retrieve core engine");
        }
        core = sceneObject()->scene()->engine();
    }
    core->soundManager()->removeSource(this);
}

SoundManager * AudioSourceComponent::soundManager() const
{
    return sceneObject()->scene()->engine()->soundManager();
}

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
        if (sceneObject()->scene()->engine()->soundManager()->soLoud()->isValidVoiceHandle(handle)) {
            // Add voice to temporary active voice vector
            activeVoices.emplace_back(handle);
        }
    }
    activeVoices.swap(m_activeVoiceHandles);
    m_voiceMutex.unlock();
    activeVoices.clear();
}



} // end namespacing