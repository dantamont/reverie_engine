#include "GbAudioResource.h"

#include "GbSoundManager.h"
#include "../GbCoreEngine.h"
#include "../resource/GbResource.h"
#include "../resource/GbResourceCache.h"
#include "../utils/GbMemoryManager.h"

#include <soloud.h>
#include <soloud_wav.h>
#include <soloud_wavstream.h>
#include <soloud_thread.h>
#include "soloud_speech.h"

namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AudioSourceSettings
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue AudioSourceSettings::asJson() const
{
    QJsonObject object;
    object.insert("sourceFlags", (int)m_audioSourceFlags);
    object.insert("filterID", m_filterID);
    object.insert("minDistance", m_minDistance);
    object.insert("maxDistance", m_maxDistance);
    object.insert("attenModel", m_attenuationModel);
    object.insert("rolloff", m_rolloff);
    object.insert("doppler", m_dopplerFactor);
    object.insert("volume", m_defaultVolume);
    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AudioSourceSettings::loadFromJson(const QJsonValue & json, const SerializationContext & context)
{
    Q_UNUSED(context);
    QJsonObject object = json.toObject();
    m_audioSourceFlags = object["sourceFlags"].toInt(0);
    m_filterID = object["filterID"].toInt(-1);
    m_minDistance = object["minDistance"].toDouble(1);
    m_maxDistance = object["maxDistance"].toDouble(1e6);
    m_attenuationModel = object["attenModel"].toInt(0);
    m_rolloff = object["rolloff"].toDouble(1.0);
    m_dopplerFactor = object["doppler"].toDouble(1.0);
    m_defaultVolume = object["volume"].toDouble(1.0);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AudioResource
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> AudioResource::createHandle(CoreEngine * engine, const GString & audioPath, SourceType sourceType)
{
    // Create texture handle and add to cache
    auto soundHandle = prot_make_shared<ResourceHandle>(engine,
        audioPath, Resource::kAudio);
    soundHandle->setName(FileReader::PathToName(audioPath));
    soundHandle->setUsesJson(true);

    // Set handle attributes
    soundHandle->resourceJson().insert("sourceType", (int)sourceType);

    // Add handle to resource cache
    engine->resourceCache()->insertHandle(soundHandle);

    // Load texture
    soundHandle->loadResource();

    return soundHandle;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue AudioResource::asJson() const
{
    QJsonObject object;
    object.insert("sourceType", (int)m_sourceType);
    object.insert("sourceSettings", m_audioSourceSettings.asJson());
    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AudioResource::loadFromJson(const QJsonValue & json, const SerializationContext & context)
{
    QJsonObject object = json.toObject();
    m_sourceType = (SourceType)object["sourceType"].toInt();
    m_audioSourceSettings.loadFromJson(object, context);
    cacheSettings();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AudioResource::AudioResource(const GString & uniqueName, SourceType sourceType):
    Resource(uniqueName),
    m_sourceType(sourceType)
{
    switch (sourceType) {
    case AudioResource::SourceType::kWav:
        m_audioSource = new SoLoud::Wav();
        break;
    case AudioResource::SourceType::kWavStream:
        m_audioSource = new SoLoud::WavStream();
        break;
    default:
        throw("Error, source type unrecognized");
        break;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AudioResource::~AudioResource()
{
    delete m_audioSource;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AudioResource::cacheSettings()
{
    SoLoud::AudioSource* source = audioSource();
    source->setLooping(m_audioSourceSettings.testFlag(AudioSourceFlag::kLooping));
    source->setSingleInstance(m_audioSourceSettings.testFlag(AudioSourceFlag::kSingleInstance));
    source->set3dListenerRelative(m_audioSourceSettings.testFlag(AudioSourceFlag::kListenerRelative));
    source->set3dDistanceDelay(m_audioSourceSettings.testFlag(AudioSourceFlag::kUseDistanceDelay));
    source->setInaudibleBehavior(m_audioSourceSettings.testFlag(AudioSourceFlag::kTickOnInaudible),
        m_audioSourceSettings.testFlag(AudioSourceFlag::kKillOnInaudible));

    source->set3dMinMaxDistance(m_audioSourceSettings.m_minDistance, m_audioSourceSettings.m_maxDistance);
    source->set3dAttenuation(m_audioSourceSettings.m_attenuationModel, m_audioSourceSettings.m_rolloff);
    source->set3dDopplerFactor(m_audioSourceSettings.m_dopplerFactor);
    source->setVolume(m_audioSourceSettings.m_defaultVolume);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AudioResource::loadAudioSource(const GString & filepath)
{
    int error;
    switch (m_sourceType) {
    case AudioResource::SourceType::kWav:
        error = audioSource<SoLoud::Wav>()->load(filepath.c_str());
        break;
    case AudioResource::SourceType::kWavStream:
        error = audioSource<SoLoud::WavStream>()->load(filepath.c_str());
        break;
    default:
        throw("Error, source type unrecognized");
        break;
    }

    if (error) {
        logError("Error loading audio resource");
        throw("Error loading audio resource");
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AudioResource::onRemoval(ResourceCache* cache)
{
    Q_UNUSED(cache);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AudioResource::postConstruction()
{
    // Call parent class construction routine
    Resource::postConstruction();

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespacing