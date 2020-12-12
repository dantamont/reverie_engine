#ifndef GB_AUDIO_LISTENER_COMPONENT
#define GB_AUDIO_LISTENER_COMPONENT
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes
#include <shared_mutex>

// Qt
#include <QString>
#include <QJsonValue>

// Project
#include "GbComponent.h"
#include "../sound/GbAudioResource.h"

namespace SoLoud {
class AudioSource;
class Wav;
class WavStream;
class Bus;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SoundManager;
class ResourceHandle;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class AudioListenerComponent
class AudioListenerComponent: public Component {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    AudioListenerComponent();
    AudioListenerComponent(const std::shared_ptr<SceneObject>& object);
    ~AudioListenerComponent();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Set velocity in settings/SoLoud
    void setVelocity(const Vector3& velocity);
    
    /// @brief Set speed of sound in settings/SoLoud
    /// @note default is 343 (assuming m/s)
    void setSpeedOfSound(float sos);

    /// @brief Queue a move in the sound manager (updates all 3D sound settings)
    void queueUpdate3d();

    /// @brief Enable the behavior of this script component
    virtual void enable() override;

    /// @brief Disable the behavior of this script component
    virtual void disable() override;

    /// @brief Max number of allowed components per scene object
    virtual int maxAllowed() const override { return 1; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief the velocity of the object, in world units
    const Vector3& velocity() const { return m_velocity; }

    /// @brief The speed of sound, in world units
    float speedOfSound() const { return m_speedOfSound; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name GB object Properties
    /// @{
    /// @property className
    const char* className() const override { return "AudioListenerComponent"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::AudioListenerComponent"; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:
    friend class SoundManager;
    friend class UpdateListener3dCommand;

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Get sound manager
    SoundManager* soundManager() const;

    /// @brief Set listener position in SoLoud
    void update3dAttributes();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    // TODO: Implement set3dListenerParameters
    // Settings: location, speed, speed of sound

    /// @brief the velocity of the object, in world units
    Vector3 m_velocity;

    /// @brief The speed of sound, in world units
    float m_speedOfSound;

    /// @}


};
Q_DECLARE_METATYPE(AudioListenerComponent)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 
