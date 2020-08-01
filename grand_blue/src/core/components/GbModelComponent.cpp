#include "GbModelComponent.h"

#include "../GbCoreEngine.h"
#include "../resource/GbResourceCache.h"

#include "../scene/GbScene.h"
#include "../scene/GbScenario.h"
#include "../scene/GbSceneObject.h"

#include "../components/GbCamera.h"
#include "../components/GbTransformComponent.h"
#include "../rendering/shaders/GbShaders.h"
#include "../rendering/shaders/GbUniform.h"
#include "../rendering/models/GbModel.h"
#include "../rendering/renderer/GbRenderCommand.h"

#include "GbAnimationComponent.h"

namespace Gb {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModelComponent::ModelComponent() :
    Component(ComponentType::kModel)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModelComponent::ModelComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, ComponentType::kModel)
{
    setSceneObject(sceneObject());
    sceneObject()->addComponent(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Gb::ModelComponent::~ModelComponent()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelComponent::updateBounds(const Transform & transform)
{
    m_transformedBounds.clear();
    std::shared_ptr<Model> m = model();
    if (!m) { return; }

    for (const auto& chunk : m->chunks()) {
        m_transformedBounds.push_back(BoundingBoxes());
        chunk->boundingBoxes().recalculateBounds(transform, m_transformedBounds.back());
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void ModelComponent::bindUniforms(DrawCommand& drawCommand)
//{
//    if (!m_modelHandle) {
//#ifdef DEBUG_MODE
//        logWarning("Warning, no model is set for model component");
//#endif
//        return;
//    }
//
//    // Iterate through uniforms to update in draw command
//    //for (const auto& uniformPair : m_uniforms) {
//    //    command.setUniform(uniformPair.second);
//    //}
//
//    // If the shader allows animations
//    if (shaderProgram.hasUniform("isAnimated")) {
//        if (sceneObject()->hasComponent(Component::ComponentType::kBoneAnimation)) {
//            // If there are animations, update the corresponding shader uniforms
//            sceneObject()->boneAnimation()->bindUniforms(shaderProgram);
//        }
//        else {
//            shaderProgram.setUniformValue("isAnimated", false);
//        }
//    }
//
//    // Update the uniforms in GL
//    shaderProgram.updateUniforms();
//}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelComponent::createDrawCommands(
    std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands, 
    Camera & camera,
    ShaderProgram& shaderProgram)
{
    if (!m_isEnabled) { return; }

    if (!m_modelHandle) {
#ifdef DEBUG_MODE
        logWarning("Warning, no model is set for model component");
#endif
        return;
    }

    std::shared_ptr<Model> m = model();
    if (!m) { return; }

    // If there is a model, make sure that the component's bounding boxes have been updated
    if (m_transformedBounds.size() == 0) {
        if (!sceneObject()->hasComponent(ComponentType::kBoneAnimation)) {
            // FIXME: Make bounding boxes work for animations
            updateBounds(*sceneObject()->transform());
        }
    }

    // Iterate through model chunks to generate draw commands
    size_t numChunks = m->chunks().size();
    for (size_t i = 0; i < numChunks; i++) {

        // Check if chunk should be drawn
        if (m_transformedBounds.size() > i) {
            // FIXME: See TransformComponent::computeWorldMatrix()
            // Note that scene objects with an animation component will skip this
            // check, since their bounding box size is unreliable
            BoundingBoxes& bounds = m_transformedBounds[i];
            if (!bounds.inFrustum(camera.frustum())) {
                continue;
            }
        }

        const auto& chunk = m->chunks().at(i);
        if (!chunk->isLoaded()) {
            continue;
        }

        auto command = std::make_shared<DrawCommand>(*chunk, shaderProgram, camera);
        //bindUniforms(*command);
        command->addRenderSettings(&m_renderSettings);
        if (shaderProgram.hasUniform("isAnimated")) {
            if (sceneObject()->hasComponent(Component::ComponentType::kBoneAnimation)) {
                // If there are animations, update the corresponding command uniforms
                sceneObject()->boneAnimation()->bindUniforms(*command);
            }
            else {
                command->setUniform(Uniform("isAnimated", false));
            }
        }
        outDrawCommands.push_back(command);
    }
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
std::shared_ptr<Model> ModelComponent::model() const
{
    if (m_modelHandle)
        return m_modelHandle->resourceAs<Model>(false);
    else
        return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue ModelComponent::asJson() const
{
    QJsonObject object = Component::asJson().toObject();
    if (model()) {
        object.insert("model", model()->getName());
    }

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelComponent::loadFromJson(const QJsonValue & json)
{
    Component::loadFromJson(json);
    const QJsonObject& object = json.toObject();

    // Load model
    if (object.contains("model")) {
        const QString& modelName = object["model"].toString();
        m_modelHandle = m_engine->resourceCache()->getHandleWithName(modelName, Resource::kModel);
        if (!m_modelHandle) {
            throw("Error, no model handle found to populate renderer");
        }
    }

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing