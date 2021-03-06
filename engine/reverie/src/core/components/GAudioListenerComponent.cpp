#include "GAudioListenerComponent.h"

#include "../GCoreEngine.h"
#include "../sound/GSoundManager.h"
#include "../resource/GResourceCache.h"

#include "../scene/GScene.h"
#include "../scene/GScenario.h"
#include "../scene/GSceneObject.h"

#include "../components/GTransformComponent.h"

#include <soloud.h>
#include <soloud_wav.h>
#include <soloud_wavstream.h>

namespace rev {

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
    sceneObject()->scene()->engine()->soundManager()->setListener(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AudioListenerComponent::~AudioListenerComponent()
{
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
void AudioListenerComponent::preDestruction(CoreEngine * core)
{
    core->soundManager()->setListener(nullptr);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue AudioListenerComponent::asJson(const SerializationContext& context) const
{
    QJsonObject object = Component::asJson(context).toObject();
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
    return sceneObject()->scene()->engine()->soundManager();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AudioListenerComponent::update3dAttributes()
{
    // Update position, look direction, velocity
    Transform& transform = sceneObject()->transform();
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