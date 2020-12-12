#include "GbAudioListenerComponent.h"

#include "../GbCoreEngine.h"
#include "../sound/GbSoundManager.h"
#include "../resource/GbResourceCache.h"

#include "../scene/GbScene.h"
#include "../scene/GbScenario.h"
#include "../scene/GbSceneObject.h"

#include "../components/GbTransformComponent.h"

#include <soloud.h>
#include <soloud_wav.h>
#include <soloud_wavstream.h>

namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Audio Listener Component
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AudioListenerComponent::AudioListenerComponent() :
    Component(ComponentType::kAudioListener)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AudioListenerComponent::AudioListenerComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, ComponentType::kAudioListener),
    m_speedOfSound(343)
{
    setSceneObject(sceneObject());
    sceneObject()->addComponent(this);
    sceneObject()->engine()->soundManager()->setListener(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AudioListenerComponent::~AudioListenerComponent()
{
    sceneObject()->engine()->soundManager()->setListener(nullptr);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AudioListenerComponent::setVelocity(const Vector3& velocity)
{
    m_velocity = velocity;
    queueUpdate3d();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AudioListenerComponent::setSpeedOfSound(float sos)
{
    m_speedOfSound = sos;
    queueUpdate3d();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AudioListenerComponent::queueUpdate3d()
{
    soundManager()->addCommand(new UpdateListener3dCommand(this));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AudioListenerComponent::enable()
{
    Component::enable();
}
//////////////// ///////////////////////////////////////////////////////////////////////////////////////////////////////
void AudioListenerComponent::disable()
{
    Component::disable();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue AudioListenerComponent::asJson() const
{
    QJsonObject object = Component::asJson().toObject();
    object["velocity"] = m_velocity.asJson();
    object["sos"] = m_speedOfSound;
    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AudioListenerComponent::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    // Call parent class load
    Component::loadFromJson(json, context);
    const QJsonObject& object = json.toObject();
    setVelocity(Vector3(object["velocity"]));
    setSpeedOfSound(object["sos"].toDouble());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SoundManager * AudioListenerComponent::soundManager() const
{
    return sceneObject()->engine()->soundManager();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AudioListenerComponent::update3dAttributes()
{
    // Update position, look direction, velocity
    Transform& transform = *sceneObject()->transform();
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing