#include "GModelComponent.h"

#include "../GCoreEngine.h"
#include "../resource/GResourceCache.h"

#include "../scene/GScene.h"
#include "../scene/GScenario.h"
#include "../scene/GSceneObject.h"

#include "../components/GCameraComponent.h"
#include "../components/GTransformComponent.h"
#include "../rendering/shaders/GShaderProgram.h"
#include "../rendering/shaders/GUniform.h"
#include "../rendering/models/GModel.h"
#include "../rendering/renderer/GRenderCommand.h"
#include "../rendering/geometry/GSkeleton.h"

#include "GAnimationComponent.h"

namespace rev {
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
rev::ModelComponent::~ModelComponent()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelComponent::updateBounds(const Transform & transform)
{
    Model* m = model();
    if (!m) { return; }

    SceneObject& object = *sceneObject();
    object.m_worldBounds.geometry().clear();
    for (ModelChunk& chunk : m->chunks()) {
        chunk.addWorldBounds(transform, object.m_worldBounds);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelComponent::createDrawCommands(
    std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands, 
    AbstractCamera& camera,
    ShaderProgram& shaderProgram,
    ShaderProgram* prepassProgram)
{
    if (!isEnabled()) {
        return;
    }

    if (!m_modelHandle) {
#ifdef DEBUG_MODE
        logWarning("Warning, no model is set for model component");
#endif
        return;
    }

    Model* m = model();
    if (!m) {
        return; 
    }

    SceneObject& so = *sceneObject();

    // Make sure that scene object has world bounds
    // TODO: Move this to wherever modelComponent is initialized onto scene object
    if (!so.m_worldBounds.geometry().size()) {
        so.updateBounds(so.transform());
    }

    // If there is a model, make sure that the component's bounding boxes have been computed
    bool hasAnimationComponent = sceneObject()->hasComponent(ComponentType::kBoneAnimation);

    // Iterate through model chunks to generate draw commands
    std::vector<ModelChunk>& chunks = m->chunks();
    size_t numChunks = chunks.size();
    //size_t boxCount = 0;
    //size_t numBoxesInChunk; // There used be more than one AABB per chunk, but no longer
    constexpr size_t numBoxesInChunk = 1;
    BoundingBoxes chunkBounds;
    for (size_t i = 0; i < numChunks; i++) {
        ModelChunk& chunk = chunks[i];

        // Check if chunk is loaded
        if (!chunk.isLoaded()) {
            continue;
        }

        // Check if chunk should be drawn
//        numBoxesInChunk = chunk.mesh()->objectBounds().count();
//#ifdef DEBUG_MODE
//        if(!hasAnimationComponent) {
//            if (!numBoxesInChunk) {
//                throw("Error, mesh is loaded but has no bounding geometry");
//            }
//        }
//#endif

        if (!so.m_worldBounds.inShape(camera.frustum(), i/*boxCount*/, numBoxesInChunk)) {
            //boxCount += numBoxesInChunk;
            continue;
        }

        // Create command, overriding render settings with those belonging to the model component
        auto command = std::make_shared<DrawCommand>(const_cast<ModelChunk&>(chunk), shaderProgram, camera, so.id(), prepassProgram);
        command->renderSettings().overrideSettings(m_renderSettings);

        // Need to only set with local bounds
        command->setWorldBounds(so.m_worldBounds.geometry()[i]);
        //command->worldBounds().geometry() = std::vector<AABB>(
        //    so.m_worldBounds.geometry().data() + i/*boxCount*/,
        //    so.m_worldBounds.geometry().data() + i/*boxCount*/ + numBoxesInChunk);
        //boxCount += numBoxesInChunk;

        ShaderProgram::UniformInfoIter iter;
        if (shaderProgram.hasUniform("isAnimated", iter)) {
            if (auto* boneComp = so.hasComponent<BoneAnimationComponent>(ComponentType::kBoneAnimation)) {
                // If there are animations, update the corresponding command uniforms
                boneComp->animationController().bindUniforms(*command);
            }
            else {
                command->addUniform(Uniform("isAnimated", false));
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
Model* ModelComponent::model() const
{
    if (m_modelHandle) {
        return m_modelHandle->resourceAs<Model>();
    }
    else {
        return nullptr;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue ModelComponent::asJson(const SerializationContext& context) const
{
    QJsonObject object = Component::asJson(context).toObject();
    if (model()) {
        object.insert("model", m_modelHandle->getName().c_str());
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
        m_modelHandle = m_scene->engine()->resourceCache()->getHandleWithName(modelName, ResourceType::kModel);
        //if (!m_modelHandle) {
        //    throw("Error, no model handle found to populate renderer");
        //}

        // Temporarily try fix for 

        size_t count = 0;
        size_t countMax = 1000;
        while (!m_modelHandle && count < countMax) {
            m_modelHandle = m_scene->engine()->resourceCache()->getHandleWithName(modelName, ResourceType::kModel);
            logInfo("Waiting for model " + modelName + " to load");
            count++;

            if (count == countMax) {

                // 9/24 Try with MDL extension in case JSON is from when I broke things
                // By changing extensions during binary loading
                QString extension = QString(modelName).split(".").back();
                modelName.replace((const char*)("." + GString(extension)), ".mdl");
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