#include "GbModelComponent.h"

#include "../GbCoreEngine.h"
#include "../resource/GbResourceCache.h"

#include "../scene/GbScene.h"
#include "../scene/GbScenario.h"
#include "../scene/GbSceneObject.h"

#include "../rendering/shaders/GbShaders.h"
#include "../rendering/shaders/GbUniform.h"
#include "../rendering/models/GbModel.h"

#include "GbAnimationComponent.h"

namespace Gb {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModelComponent::ModelComponent() :
    Component(kModel)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModelComponent::ModelComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, kModel)
{
    setSceneObject(sceneObject());
    sceneObject()->addComponent(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Gb::ModelComponent::~ModelComponent()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelComponent::bindUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram)
{
    if (!m_model) {
#ifdef DEBUG_MODE
        logWarning("Warning, no model is set for model component");
#endif
        return;
    }

    // Iterate through uniforms to update in shader program class
    Renderable::bindUniforms(shaderProgram);

    // If the shader allows animations
    if (shaderProgram->hasUniform("isAnimated")) {
        if (sceneObject()->hasComponent(Component::kBoneAnimation)) {
            // If there are animations, update the corresponding shader uniforms
            sceneObject()->boneAnimation()->bindUniforms(shaderProgram);
        }
        else {
            shaderProgram->setUniformValue("isAnimated", false);
        }
    }

    // Update the uniforms in GL
    shaderProgram->updateUniforms();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelComponent::releaseUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram)
{
    Q_UNUSED(shaderProgram)
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelComponent::drawGeometry(const std::shared_ptr<ShaderProgram>& shaderProgram, RenderSettings * settings)
{
    // TODO: Clean up
    Q_UNUSED(shaderProgram);
    Q_UNUSED(settings);
    throw("Unused");
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelComponent::draw(const std::shared_ptr<ShaderProgram>& shaderProgram, RenderSettings* settings)
{
    if (!m_model) {
        return;
    }

    // TODO: Reload model if no longer in resource cache
    if (!m_engine->resourceCache()->hasModel(m_model->getName())) {
        logWarning("Removing out-of-scope model for model component: " + m_model->getName());
        m_model = nullptr;
        return;
    }

    // Set the uniforms for the model component
    bindUniforms(shaderProgram);

    // Draw the model
    m_model->draw(shaderProgram, settings);

    // Release the uniforms for the model component
    releaseUniforms(shaderProgram);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelComponent::enable()
{
    Component::enable();
}
//////////////// ///////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelComponent::disable()
{
    Component::disable();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue ModelComponent::asJson() const
{
    QJsonObject object = Component::asJson().toObject();
    QJsonValue renderable = Renderable::asJson();
    object.insert("renderable", renderable);

    if (m_model) {
        object.insert("model", m_model->getName());
    }

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelComponent::loadFromJson(const QJsonValue & json)
{
    Component::loadFromJson(json);
    const QJsonObject& object = json.toObject();

    // Load renderable attributes
    if (object.contains("renderable")) {
        Renderable::loadFromJson(object["renderable"]);
    }

    // Load model
    if (object.contains("model")) {
        const QString& modelName = object["model"].toString();
        m_model = m_engine->resourceCache()->getModel(modelName);
        if (!m_model) {
            throw("Error, no model found to populate renderer");
        }
    }

    // (DEPRECATED) Load animations
    if (object.contains("animationController")) {
        auto boneAnimComp = new BoneAnimationComponent(sceneObject());
        boneAnimComp->loadFromJson(object);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing