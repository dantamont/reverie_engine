#pragma once

// Standard includes
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <atomic>

// Internal
#include "GAudioCommands.h"
#include "core/GManager.h"
#include "fortress/types/GLoadable.h"

namespace SoLoud {
class Soloud;
class Bus;
}

namespace rev {

class CoreEngine;
class AudioResource;
class AudioSourceComponent;
class AudioListenerComponent;

/// @brief SoundManager class
class SoundManager: public Manager {
    Q_OBJECT
public:
	/// @name Constructors/Destructor
	/// @{
    SoundManager(CoreEngine* engine);
	~SoundManager();
	/// @}

    /// @name Properties
    /// @{

    /// @brief The SoLoud main engine
    SoLoud::Soloud* soLoud() { return m_soLoud; }

    /// @}

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

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const SoundManager& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, SoundManager& orObject);


    /// @}

public slots:

protected:
    /// @name Protected Methods
    /// @{

    /// @brief Callback controlling the audio thread
    void audioThreadCallback();

    /// @}

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


} // End namespaces
