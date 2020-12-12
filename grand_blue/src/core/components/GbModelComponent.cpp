#include "GbModelComponent.h"

#include "../GbCoreEngine.h"
#include "../resource/GbResourceCache.h"

#include "../scene/GbScene.h"
#include "../scene/GbScenario.h"
#include "../scene/GbSceneObject.h"

#include "../components/GbCameraComponent.h"
#include "../components/GbTransformComponent.h"
#include "../rendering/shaders/GbShaders.h"
#include "../rendering/shaders/GbUniform.h"
#include "../rendering/models/GbModel.h"
#include "../rendering/renderer/GbRenderCommand.h"
#include "../rendering/geometry/GbSkeleton.h"

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

    for (const ModelChunk& chunk : m->chunks()) {
        m_transformedBounds.emplace_back(); // BoundingBoxes()
        const_cast<ModelChunk&>(chunk).boundingBoxes().recalculateBounds(transform, m_transformedBounds.back());
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
    AbstractCamera& camera,
    ShaderProgram& shaderProgram,
    ShaderProgram* prepassProgram)
{
    if (!m_isEnabled) {
        return;
    }

    if (!m_modelHandle) {
#ifdef DEBUG_MODE
        logWarning("Warning, no model is set for model component");
#endif
        return;
    }

    std::shared_ptr<Model> m = model();
    if (!m) {
        return; 
    }

    SceneObject& so = *sceneObject();

    // If there is a model, make sure that the component's bounding boxes have been computed
    bool hasAnimationComponent = sceneObject()->hasComponent(ComponentType::kBoneAnimation);
    if (m_transformedBounds.size() == 0) {
        if (!hasAnimationComponent) {
            // Only update bounds if doesn't have an animation component
            // TODO: Update bounds for animation, instead of conditional in the following for loop
            updateBounds(*so.transform());
        }
    }

    // Iterate through model chunks to generate draw commands
    size_t numChunks = m->chunks().size();
    for (size_t i = 0; i < numChunks; i++) {

        // Check if chunk is loaded
        const ModelChunk& chunk = m->chunks().at(i);
        if (!chunk.isLoaded()) {
            continue;
        }

        // Check if chunk should be drawn
        // TODO: Maybe move bounds outside of components? Would increase memory consumption
        if (hasAnimationComponent) {
            // If there is an animation component, use the bounding box of the skeleton to check visibility
            //const AABB& bounds = model()->skeleton()->boundingBox();
            const AABB& bounds = so.boneAnimationComponent()->bounds();
            if (!bounds.intersects(camera.frustum())) {
                continue;
            }
        }
        else {
            // If no animation component, iterate through the bounds of every mesh
            if (m_transformedBounds.size() > i) {
                BoundingBoxes& bounds = m_transformedBounds[i];
                if (!bounds.inShape(camera.frustum())) {
                    continue;
                }
            }
        }
        auto command = std::make_shared<DrawCommand>(const_cast<ModelChunk&>(chunk), shaderProgram, camera, prepassProgram);
        command->renderSettings().overrideSettings(m_renderSettings);
        ShaderProgram::UniformInfoIter iter;
        if (shaderProgram.hasUniform("isAnimated", iter)) {
            if (so.hasComponent(Component::ComponentType::kBoneAnimation)) {
                // If there are animations, update the corresponding command uniforms
                so.boneAnimationComponent()->bindUniforms(*command);
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
        object.insert("model", model()->getName().c_str());
    }

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelComponent::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    Component::loadFromJson(json);
    const QJsonObject& object = json.toObject();

    // Load model
    if (object.contains("model")) {
        QString modelName = object["model"].toString();
        m_modelHandle = m_engine->resourceCache()->getHandleWithName(modelName, Resource::kModel);
        //if (!m_modelHandle) {
        //    throw("Error, no model handle found to populate renderer");
        //}

        // Temporarily try fix for 

        size_t count = 0;
        size_t countMax = 1000;
        while (!m_modelHandle && count < countMax) {
            m_modelHandle = m_engine->resourceCache()->getHandleWithName(modelName, Resource::kModel);
            logInfo("Waiting for model " + modelName + " to load");
            count++;

            if (count == countMax) {

                // 9/24 Try with MDL extension in case JSON is from when I broke things
                // By changing extensions during binary loading
                QString extension = QString(modelName).split(".").back();
                modelName.replace("." + GString(extension), ".mdl");
#ifdef DEBUG_MODE
                throw("Error, could not find model");
#endif
                logError("Model with name " + modelName + " not found");
                break;
            }
        }
    }

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing