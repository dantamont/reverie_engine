#ifndef GB_AUDIO_COMMANDS_H
#define GB_AUDIO_COMMANDS_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Internal
#include "../geometry/GbVector.h"

namespace SoLoud {
class Soloud;
class Bus;
}

namespace Gb {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class AudioSourceComponent;
class AudioListenerComponent;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class AudioCommand
/// @brief Audio command to be performed on audio thread
class AudioCommand {
public:
    AudioCommand() {}
    virtual ~AudioCommand() {}

    virtual void perform() = 0;
};
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class AudioListenerCommand
/// @brief Audio command to be performed on audio thread
class AudioListenerCommand: public AudioCommand {
public:
    AudioListenerCommand();
    AudioListenerCommand(AudioListenerComponent* component);
    virtual ~AudioListenerCommand();

    virtual void perform() = 0;

protected:

    /// @brief The audio component corresponding to the command
    AudioListenerComponent* m_audioListener = nullptr;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/// @class UpdateListener3dCommand
/// @brief Audio command to be performed on audio thread
class UpdateListener3dCommand : public AudioListenerCommand {
public:
    UpdateListener3dCommand(AudioListenerComponent* component);

    virtual void perform();
};

/////////////////////////////////////////////////////////////////////////////////////////////
/// @class AudioSourceCommand
/// @brief Audio command to be performed on audio thread
class AudioSourceCommand: public AudioCommand {
public:
    AudioSourceCommand();
    AudioSourceCommand(AudioSourceComponent* component);
    virtual ~AudioSourceCommand();

    virtual void perform() = 0;

protected:

    /// @brief The audio component corresponding to the command
    AudioSourceComponent* m_audioComponent = nullptr;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PlayCommand
/// @brief Audio command to be performed on audio thread
class PlayAudioCommand: public AudioSourceCommand{
public:
    PlayAudioCommand(AudioSourceComponent* component);

    virtual void perform();
};

///////////////////////////////////////////////////////////////////////////////////////////////
/// @class MoveAudioCommand
/// @brief Audio command to be performed on audio thread
class MoveAudioCommand : public AudioSourceCommand {
public:
    MoveAudioCommand(AudioSourceComponent* component, const Vector3& pos);

    virtual void perform();

protected:
    Vector3 m_position;
    Vector3 m_velocity;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class UpdateVoiceCommand
/// @brief Command to update settings of a voice being played
class UpdateVoiceCommand : public AudioSourceCommand {
public:

    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    UpdateVoiceCommand(AudioSourceComponent* component, int voiceHandle);

    virtual void perform() = 0;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
    /// @}

protected:

    int m_voiceHandle;
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif