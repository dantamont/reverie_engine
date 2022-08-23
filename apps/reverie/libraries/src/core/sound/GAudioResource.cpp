#include "core/sound/GAudioResource.h"

#include "fortress/system/memory/GPointerTypes.h"
#include "fortress/system/path/GFile.h"
#include "fortress/system/path/GPath.h"

#include "core/sound/GSoundManager.h"
#include "core/GCoreEngine.h"
#include "core/resource/GResource.h"
#include "core/resource/GResourceCache.h"

#include <soloud.h>
#include <soloud_wav.h>
#include <soloud_wavstream.h>
#include <soloud_thread.h>
#include "soloud_speech.h"

namespace rev {



// AudioSourceSettings

void to_json(json& orJson, const AudioSourceSettings& korObject)
{
    orJson["sourceFlags"] = (int)korObject.m_audioSourceFlags;
    orJson["filterID"] = korObject.m_filterID;
    orJson["minDistance"] = korObject.m_minDistance;
    orJson["maxDistance"] = korObject.m_maxDistance;
    orJson["attenModel"] = korObject.m_attenuationModel;
    orJson["rolloff"] = korObject.m_rolloff;
    orJson["doppler"] = korObject.m_dopplerFactor;
    orJson["volume"] = korObject.m_defaultVolume;
}

void from_json(const json& korJson, AudioSourceSettings& orObject)
{
    orObject.m_audioSourceFlags = korJson.value("sourceFlags", 0);
    orObject.m_filterID = korJson.value("filterID", -1);
    orObject.m_minDistance = korJson.value("minDistance", 1.0);
    orObject.m_maxDistance = korJson.value("maxDistance", 1.0e6);
    orObject.m_attenuationModel = korJson.value("attenModel", 0);
    orObject.m_rolloff = korJson.value("rolloff", 1.0);
    orObject.m_dopplerFactor = korJson.value("doppler", 1.0);
    orObject.m_defaultVolume = korJson.value("volume", 1.0);
}




// AudioResource

std::shared_ptr<ResourceHandle> AudioResource::CreateHandle(CoreEngine * engine, const GString & audioPath, SourceType sourceType)
{
    // Create texture handle and add to cache
    static constexpr bool s_keepExtension = true;
    static constexpr bool s_caseInsensitive = false;

    auto soundHandle = prot_make_shared<ResourceHandle>(engine,
        audioPath, EResourceType::eAudio);
    GFile audioFile(audioPath);
    soundHandle->setName(audioFile.getFileName(s_keepExtension, s_caseInsensitive));
    soundHandle->setUsesJson(true);

    // Set handle attributes
    json jsonObject;
    jsonObject["sourceType"] = (int)sourceType;
    soundHandle->setCachedResourceJson(jsonObject);

    // Add handle to resource cache
    ResourceCache::Instance().insertHandle(soundHandle);

    // Load texture
    soundHandle->loadResource();

    return soundHandle;
}

void to_json(json& orJson, const AudioResource& korObject)
{
    orJson["sourceType"] = (int)korObject.m_sourceType;
    orJson["sourceSettings"] = korObject.m_audioSourceSettings;
}

void from_json(const json& korJson, AudioResource& orObject)
{
    
    orObject.m_sourceType = (AudioResource::SourceType)korJson.at("sourceType").get<Int32_t>();
    korJson.get_to(orObject.m_audioSourceSettings);
    orObject.cacheSettings();
}

//AudioResource::AudioResource(const GString & uniqueName, SourceType sourceType):
    //Resource(uniqueName),
AudioResource::AudioResource(SourceType sourceType) :
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
        Logger::Throw("Error, source type unrecognized");
        break;
    }
}

AudioResource::~AudioResource()
{
    delete m_audioSource;
}

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
        Logger::Throw("Error, source type unrecognized");
        break;
    }

    if (error) {
        Logger::LogError("Error loading audio resource");
        Logger::Throw("Error loading audio resource");
    }
}

void AudioResource::onRemoval(ResourceCache* cache)
{
    Q_UNUSED(cache);
}

void AudioResource::postConstruction()
{
    // Call parent class construction routine
    Resource::postConstruction();

}



} // End namespacing