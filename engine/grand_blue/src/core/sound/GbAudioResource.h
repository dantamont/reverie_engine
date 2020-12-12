#ifndef GB_AUDIO_RESOURCE_H
#define GB_AUDIO_RESOURCE_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Standard

// QT

// Internal
#include "../resource/GbResource.h"
#include "../containers/GbContainerExtensions.h"

namespace SoLoud {
class AudioSource;
}

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class ResourceCache;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Flags describing the audio source
enum class AudioSourceFlag {
    kLooping = 1 << 0, // Whether or not the sound plays on repeat
    kSingleInstance = 1 << 1, // Whether or not the more than one instance of the sound can be played at once
    kTickOnInaudible = 1 << 2, // Whether or not to keep ticking (playing) the sound when inaudible
    kKillOnInaudible = 1 << 3, // Whether or not to kill the sound once inaudible
    //kEnable3D = 1 << 4, // Whether or not 3D processing is enabled for the source (not required if play3d is used)
    kListenerRelative = 1 << 5, // Whether or not the sound coordinates are relative to a listener at (0, 0, 0)
    kUseDistanceDelay = 1 << 6 // Whether or not to delay playback based on sound's distance from listener, off by default
};
typedef QFlags<AudioSourceFlag> AudioSourceFlags;

/// @struct AudioSourceSettings
/// @brief The properties associated with an audio source
struct AudioSourceSettings {

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    bool testFlag(AudioSourceFlag flag) const {
        return m_audioSourceFlags & (size_t)flag;
    }

    void setFlag(AudioSourceFlag flag, bool value) {
        if (value) {
            // Toggle flag on
            m_audioSourceFlags |= (size_t)flag;
        }
        else {
            // Toggle flag off
            m_audioSourceFlags &= ~(size_t)flag;
        }
    }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{


    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const;

    /// @brief Populates this data using a valid json string
    void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty());

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Members
    /// @{

    size_t m_audioSourceFlags = 0;

    // TODO: Implement filters
    int m_filterID = -1;

    /// @brief min/max 3D distances for the audio source
    /// @details Designate where attenuation model begins and ends for the audio source
    float m_minDistance = 1;
    float m_maxDistance = 1e6;

    /// @brief The attenuation model used by the sound
    /// @brief See SoLoud::AudioSource::ATTENUATION_MODELS
    int m_attenuationModel = 0;

    /// @brief How strongly sound drops off based on attenuation model, larger means stronger drop-off
    float m_rolloff = 1.0f;

    /// @brief Doppler factor for the sound
    float m_dopplerFactor = 1.0f;

    /// @brief The default volume of the instances created from this source
    float m_defaultVolume = 1.0f;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class AudioResource
/// @brief Represents an audio file
class AudioResource: public Resource, public Serializable {
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief SoLoud source type of the resource
    enum class SourceType {
        kWav,
        kWavStream,
        kSpeech
    };

    static std::shared_ptr<ResourceHandle> AudioResource::createHandle(CoreEngine* engine,
        const GString& audioPath,
        SourceType sourceType);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    ~AudioResource();
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Cache settings into actual SoLoud audio source
    void cacheSettings();

    /// @brief Load audio source from file
    void loadAudioSource(const GString& filepath);

    /// @brief What to do on removal from resource cache
    void onRemoval(ResourceCache* cache = nullptr) override;

    /// @brief What action to perform post-construction of the resource
    /// @details For performing any actions that need to be done on the main thread
    virtual void postConstruction() override;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties 
    /// @{

    SourceType audioSourceType() const { return m_sourceType; }

    const AudioSourceSettings& audioSourceSettings() const { return m_audioSourceSettings; }
    AudioSourceSettings& audioSourceSettings() { return m_audioSourceSettings; }

    SoLoud::AudioSource* audioSource() {
        return m_audioSource;
    }

    template<typename T>
    T* audioSource() {
        return static_cast<T*>(m_audioSource);
    }

    template<typename T>
    const T* audioSource() const {
        return static_cast<const T*>(m_audioSource);
    }


    /// @brief Get the type of resource stored by this handle
    virtual Resource::ResourceType getResourceType() const override {
        return Resource::kAudio;
    }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "AudioResource"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::AudioResource"; }
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:
    //---------------------------------------------------------------------------------------
    /// @name Friends
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    AudioResource(const GString& uniqueName, SourceType sourceType);
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private methods 
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private members 
    /// @{

    /// @brief Type of audio source
    SourceType m_sourceType;

    AudioSourceSettings m_audioSourceSettings;

    /// @brief Audio source
    /// @details Audio sources contain all of the information pertaining to each sound.
    /// Shorter sounds that are used frequently should use Wav, longer should use WavStream,
    /// since WavStream consumes less memory (at the cost of processing power)
    SoLoud::AudioSource* m_audioSource = nullptr;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif