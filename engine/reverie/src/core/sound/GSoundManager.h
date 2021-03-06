#ifndef GB_SOUND_MANAGER_H
#define GB_SOUND_MANAGER_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Standard includes
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <atomic>

// Internal
#include "GAudioCommands.h"
#include "../GManager.h"
#include "../mixins/GLoadable.h"

namespace SoLoud {
class Soloud;
class Bus;
}

namespace rev {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class AudioResource;
class AudioSourceComponent;
class AudioListenerComponent;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief SoundManager class
class SoundManager: public Manager, public Serializable {
    Q_OBJECT
public:
	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    SoundManager(CoreEngine* engine);
	~SoundManager();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief The SoLoud main engine
    SoLoud::Soloud* soLoud() { return m_soLoud; }

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Set the listener for the scenario
    void setListener(AudioListenerComponent* listener);

    /// @brief Add audio command
    void addCommand(AudioCommand* command);

    /// @brief Add an audio source component
    void addSource(AudioSourceComponent* source);

    /// @brief remove an audio source component
    void removeSource(AudioSourceComponent* source);

    /// @brief Clear of all sounds
    void clear();

	/// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "SoundManager"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "rev::SoundManager"; }
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

public slots:

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Callback controlling the audio thread
    void audioThreadCallback();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief SoLoud engine
    SoLoud::Soloud* m_soLoud;

    /// @brief The audio listener
    AudioListenerComponent* m_listener = nullptr;

    /// @brief All audio sources in all scenes
    std::vector<AudioSourceComponent*> m_sourceComponents;

    /// @brief Audio buses
    std::vector<SoLoud::Bus*> m_buses;

    /// @brief The mutex used for the audio command queue
    std::shared_mutex m_commandMutex;

    /// @brief The mutex used to manage audio sources
    std::shared_mutex m_sourceMutex;

    /// @brief Queue of received audio commands
    std::vector<AudioCommand*> m_commandQueue;

    /// @brief Audio commands to perform
    std::vector<AudioCommand*> m_audioCommands;

    /// @brief The audio thread
    std::thread m_audioThread;

    /// @brief Used to shut down the thread
    std::atomic_bool m_running = true;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif