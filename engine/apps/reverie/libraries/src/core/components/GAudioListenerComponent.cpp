#include "core/components/GAudioListenerComponent.h"

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



// Audio Listener Component

AudioListenerComponent::AudioListenerComponent() :
    Component(ComponentType::kAudioListener)
{
}

AudioListenerComponent::AudioListenerComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, ComponentType::kAudioListener),
    m_speedOfSound(343)
{
    setSceneObject(sceneObject());
    sceneObject()->setComponent(this);
    sceneObject()->scene()->engine()->soundManager()->setListener(this);
}

AudioListenerComponent::~AudioListenerComponent()
{
}

void AudioListenerComponent::setVelocity(const Vector3& velocity)
{
    m_velocity = velocity;
    queueUpdate3d();
}

void AudioListenerComponent::setSpeedOfSound(float sos)
{
    m_speedOfSound = sos;
    queueUpdate3d();
}

void AudioListenerComponent::queueUpdate3d()
{
    soundManager()->addCommand(new UpdateListener3dCommand(this));
}

void AudioListenerComponent::enable()
{
    Component::enable();
}
 
void AudioListenerComponent::disable()
{
    Component::disable();
}

void AudioListenerComponent::preDestruction(CoreEngine * core)
{
    core->soundManager()->setListener(nullptr);
}

void to_json(json& orJson, const AudioListenerComponent& korObject)
{
    ToJson<Component>(orJson, korObject);
    orJson["velocity"] = korObject.m_velocity;
    orJson["sos"] = korObject.m_speedOfSound;
}

void from_json(const json& korJson, AudioListenerComponent& orObject)
{
    // Call parent class load
    FromJson<Component>(korJson, orObject);
    orObject.setVelocity(Vector3(korJson["velocity"]));
    orObject.setSpeedOfSound(korJson["sos"].get<Float64_t>());
}

SoundManager * AudioListenerComponent::soundManager() const
{
    return sceneObject()->scene()->engine()->soundManager();
}

void AudioListenerComponent::update3dAttributes()
{
    // Update position, look direction, velocity
    const IndexedTransform& transform = sceneObject()->transform();
    const Vector3& pos = transform.getPosition();
    const Vector4& lookDir = transform.worldMatrix().getColumn(3).normalized(); // TODO: Untested, but rotation * forward vec (0, 0, 1) == third column
    soundManager()->soLoud()->set3dListenerParameters(
        pos.x(),
        pos.y(),
        pos.z(),
        lookDir.x(),
        lookDir.y(),
        lookDir.z(),
        m_velocity.x(),
        m_velocity.y(),
        m_velocity.z()
    );

    // Update speed of sound
    soundManager()->soLoud()->set3dSoundSpeed(m_speedOfSound);
}


} // end namespacing