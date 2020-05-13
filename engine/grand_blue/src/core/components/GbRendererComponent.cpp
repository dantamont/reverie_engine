#include "GbRendererComponent.h"

#include "../GbCoreEngine.h"
#include "../rendering/renderer/GbRenderers.h"

#include "../scene/GbScene.h"
#include "../scene/GbScenario.h"
#include "../scene/GbSceneObject.h"

namespace Gb {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RendererComponent::RendererComponent() :
    Component(kRenderer)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RendererComponent::RendererComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, kRenderer)
{
    setSceneObject(sceneObject());
    sceneObject()->addComponent(this);
    m_renderer = std::make_shared<Renderer>(sceneObject()->engine());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Gb::RendererComponent::RendererComponent(const std::shared_ptr<SceneObject>& object, const std::shared_ptr<Renderer>& renderer):
    Component(object, kRenderer)
{
    setSceneObject(sceneObject());
    sceneObject()->addComponent(this);
    setRenderer(renderer);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Gb::RendererComponent::~RendererComponent()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RendererComponent::enable()
{
    Component::enable();
}
//////////////// ///////////////////////////////////////////////////////////////////////////////////////////////////////
void RendererComponent::disable()
{
    Component::disable();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue RendererComponent::asJson() const
{
    QJsonObject object = Component::asJson().toObject();

    if (m_renderer) {
        object.insert("renderer", m_renderer->asJson());
    }
    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RendererComponent::loadFromJson(const QJsonValue & json)
{
    Component::loadFromJson(json);
    const QJsonObject& object = json.toObject();
    m_renderer = std::make_shared<Renderer>(sceneObject()->engine());
    if (object.contains("renderer")) {
        m_renderer->loadFromJson(object["renderer"]);
    }
    else {
        m_renderer->loadFromJson(object);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing