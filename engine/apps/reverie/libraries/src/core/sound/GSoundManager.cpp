#include "core/sound/GSoundManager.h"

#include "core/GCoreEngine.h"
#include "core/events/GEventManager.h"
#include "core/components/GAudioSourceComponent.h"
#include "core/components/GAudioListenerComponent.h"

#include <soloud.h>
#include <soloud_wav.h>
#include <soloud_thread.h>
#include <soloud_speech.h>

namespace rev {


// Sound Manager

SoundManager::SoundManager(CoreEngine* engine) :
    Manager(engine, "ProcessManager"),
    m_running(true)
{
    m_soLoud = new SoLoud::Soloud();
    SoLoud::result result = m_soLoud->init((unsigned int)SoLoud::Soloud::CLIP_ROUNDOFF, SoLoud::Soloud::SDL2);
    if (result) {
        SoLoud::SOLOUD_ERRORS err = SoLoud::SOLOUD_ERRORS(result);
        Logger::Throw("Error initializing SoLoud");
    }
    m_audioThread = std::thread(&SoundManager::audioThreadCallback, this);
}

SoundManager::~SoundManager()
{
    // Clear the sound manager
    clear();

    // Atomic bool is thread-safe, quite convenient
    m_running = false;

    // Wait for audio thread to finish before cleanup
    m_audioThread.join();

    // Cleanup SoLoud
    m_soLoud->deinit();
    delete m_soLoud;
}

void SoundManager::setListener(AudioListenerComponent * listener)
{
    if (m_listener && listener) {
        Logger::Throw("Error, only one listener may exist at a time");
    }

    m_listener = listener;
    if (listener) {
        listener->queueUpdate3d();
    }
}

void SoundManager::addCommand(AudioCommand * command)
{
    std::unique_lock lock(m_commandMutex);
    m_commandQueue.push_back(command);
}

void SoundManager::addSource(AudioSourceComponent * source)
{
    std::unique_lock lock(m_sourceMutex);
    m_sourceComponents.push_back(source);
}

void SoundManager::removeSource(AudioSourceComponent * source)
{
    std::unique_lock lock(m_sourceMutex);
    auto iter = std::find_if(m_sourceComponents.begin(), m_sourceComponents.end(),
        [source](AudioSourceComponent* s) {
        return s->getUuid() == source->getUuid();
    });

    if (iter != m_sourceComponents.end()) {
        m_sourceComponents.erase(iter);
    }

    // Don't actually want to throw an error, since on scenario clear, sound manager
    // clears all sources before scene object may be deleted (which then attempts to delete
    // its audio component from the sound manager)
    //else {
    //    Logger::LogError("Error, source component not found for deletion");
    //    Logger::Throw("oopsie poopsie");
    //}
}

void SoundManager::clear()
{
    // Clear commands
    m_commandQueue.clear();
    m_audioCommands.clear();

    // Clear buses
    for (SoLoud::Bus* bus : m_buses) {
        delete bus;
    }
    m_buses.clear();
}

void SoundManager::audioThreadCallback() {
    while (m_running) {
        m_commandMutex.lock_shared();
        size_t numCommands = m_commandQueue.size();
        m_commandMutex.unlock_shared();

        // No commands to queue or sounds playing, sleep for a bit
        if (!numCommands && !m_soLoud->getActiveVoiceCount()) {
            SoLoud::Thread::sleep(10);
        }

        if (!numCommands) {
            continue; 
        }

        // Swap command queue with commands to perform
        m_commandMutex.lock();
        m_commandQueue.swap(m_audioCommands);
        m_commandQueue.clear();
        m_commandMutex.unlock();

        // Update active voice handles
        m_sourceMutex.lock();
        for (AudioSourceComponent* audioComp : m_sourceComponents) {
            audioComp->updateVoices();
        }
        m_sourceMutex.unlock();

        // Perform, delete, and clear audio commands
        for (AudioCommand* command : m_audioCommands) {
            command->perform();
            delete command;
        }
        m_audioCommands.clear();

        // Update 3D audio settings
        m_soLoud->update3dAudio();
    }
}

void to_json(json& orJson, const SoundManager& korObject)
{
}

void from_json(const json& korJson, SoundManager& orObject)
{
}



} // End namespaces