#include "GDebugManager.h"

#include "../GCoreEngine.h"
#include "../scene/GScenario.h"
#include "../scene/GScene.h"
#include "../scene/GSceneObject.h"
#include "../components/GCameraComponent.h"
#include "../components/GCanvasComponent.h"
#include "../components/GModelComponent.h"
#include "../components/GTransformComponent.h"
#include "../canvas/GLabel.h"

#include "../geometry/GRaycast.h"

#include "../rendering/geometry/GMesh.h"
#include "../rendering/models/GModel.h"
#include "../resource/GResourceCache.h"
#include "../rendering/geometry/GPolygon.h"
#include "../rendering/geometry/GLines.h"
#include "../rendering/geometry/GPoints.h"
#include "../rendering/geometry/GSkeleton.h"
#include "../rendering/buffers/GUniformBufferObject.h"

#include "../rendering/renderer/GMainRenderer.h"
#include "../rendering/renderer/GRenderCommand.h"

#include "../../view/GWidgetManager.h"
#include "../../view/GL/GGLWidget.h"
#include "../../view/tree/GSceneTreeWidget.h"
#include "../input/GInputHandler.h"

#include "../physics/GPhysicsActor.h"
#include "../../core/physics/GPhysicsShapePrefab.h"
#include "../../core/physics/GPhysicsShape.h"
#include "../physics/GPhysicsQuery.h"
#include "../physics/GPhysicsScene.h"
#include "../physics/GPhysicsGeometry.h"
#include "../physics/GCharacterController.h"
#include <core/components/GCharControlComponent.h>
#include <core/components/GRigidBodyComponent.h>
#include "../components/GLightComponent.h"

#include "../components/GAnimationComponent.h"
#include "../processes/GAnimationProcess.h"
#include "../animation/GAnimationController.h"
#include "../physics/GInverseKinematics.h"

#include "../../view/GL/GGLWidget.h"
#include "../../view/GWidgetManager.h"

#include "../utils/GMemoryManager.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
DebugCoordinateAxes::DebugCoordinateAxes(CoreEngine* engine) :
    m_coneHeight(1.0),
    m_columnHeight(3.0),
    m_xConeTransform(std::make_shared<Transform>()),
    m_yConeTransform(std::make_shared<Transform>()),
    m_zConeTransform(std::make_shared<Transform>()),
    m_xCylinderTransform(std::make_shared<Transform>()),
    m_yCylinderTransform(std::make_shared<Transform>()),
    m_zCylinderTransform(std::make_shared<Transform>()),
    m_setUniforms(false)
{
    // Get resources, and set them as core resources so they don't get removed
    m_cylinder = engine->resourceCache()->polygonCache()->getCylinder(0.075f, 0.075f, m_columnHeight, 36, 1);
    m_cylinder->handle()->setCore(true);

    m_cone = engine->resourceCache()->polygonCache()->getCylinder(0.2f, 0.0f, m_coneHeight, 36, 1);
    m_cone->handle()->setCore(true);

    // Initialize GL draw settings
    m_renderSettings.setShapeMode(PrimitiveMode::kTriangles);
    m_renderSettings.addSetting<DepthSetting>(false, DepthPassMode::kLessEqual, false);
    //m_renderSettings.addDefaultBlend();
    //setName("Coordinate Axes");

    // Set up transforms
    double halfColumnHeight = m_columnHeight / 2.0;
    Vector3 dirX = Vector3(1.0f, 0.0f, 0.0f);
    Vector3 dirY = Vector3(0.0f, 1.0f, 0.0f);
    Vector3 dirZ = Vector3(0.0f, 0.0f, 1.0f);
    m_xConeTransform->setParent(m_xCylinderTransform.get());
    m_xConeTransform->setTranslation(halfColumnHeight * dirZ);
    m_xCylinderTransform->rotateAboutAxis({ 0, 1, 0 }, Constants::PI_2);
    m_xCylinderTransform->setTranslation(halfColumnHeight * dirX);

    m_yConeTransform->setParent(m_yCylinderTransform.get());
    m_yConeTransform->setTranslation(halfColumnHeight * dirZ);
    m_yCylinderTransform->rotateAboutAxis({ 1, 0, 0 }, -Constants::PI_2);
    m_yCylinderTransform->setTranslation(halfColumnHeight * dirY);

    m_zConeTransform->setParent(m_zCylinderTransform.get());
    m_zConeTransform->setTranslation(halfColumnHeight * dirZ);
    m_zCylinderTransform->setTranslation(halfColumnHeight * dirZ);
}
/////////////////////////////////////////////////////////////////////////////////////////////
DebugCoordinateAxes::~DebugCoordinateAxes()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugCoordinateAxes::setTransform(Transform* transform)
{
    m_transform = transform;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugCoordinateAxes::clear()
{
    m_transform = nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugCoordinateAxes::bindUniforms(ShaderProgram& shaderProgram)
{
    Q_UNUSED(shaderProgram)
    if (!m_transform) return;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugCoordinateAxes::releaseUniforms(ShaderProgram& shaderProgram)
{
    Q_UNUSED(shaderProgram)
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugCoordinateAxes::drawGeometry(ShaderProgram& shaderProgram,
    RenderSettings * settings)
{
    VertexArrayData& cylinderData = m_cylinder->vertexData();
    VertexArrayData& coneData = m_cone->vertexData();

    // Initialize uniforms
    if (!m_setUniforms) {
        shaderProgram.setUniformValue("xConeTransform", m_xConeTransform->worldMatrix());
        shaderProgram.setUniformValue("xCylTransform", m_xCylinderTransform->worldMatrix());
        shaderProgram.setUniformValue("yConeTransform", m_yConeTransform->worldMatrix());
        shaderProgram.setUniformValue("yCylTransform", m_yCylinderTransform->worldMatrix());
        shaderProgram.setUniformValue("zConeTransform", m_zConeTransform->worldMatrix());
        shaderProgram.setUniformValue("zCylTransform", m_zCylinderTransform->worldMatrix());
        m_setUniforms = true;
    }
    // Set world matrix to rotation and translation of scene object
    Matrix4x4g worldMatrix;
    if (m_transform) {
        worldMatrix = m_transform->rotationTranslationMatrix();
    }
    shaderProgram.setUniformValue("worldMatrix", worldMatrix);

    // X-axis
    Vector4 color = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
    for (size_t i = 0; i < 3; i++) {
        color[i] = 1.0f;
        if (i != 0) color[i - 1] = 0.0f;
        shaderProgram.setUniformValue("axis", (int)i);
        shaderProgram.setUniformValue("color", color);
        shaderProgram.setUniformValue("isCone", true, true);
        coneData.drawGeometry(settings->shapeMode());

        shaderProgram.setUniformValue("isCone", false, true);
        cylinderData.drawGeometry(settings->shapeMode());
    }

    //shaderProgram->release();
}




/////////////////////////////////////////////////////////////////////////////////////////////
DebugGrid::DebugGrid(CoreEngine* core):
    m_engine(core)
{
    // FIXME: Don't access engine like a singleton, gross
    ResourceCache& cache = *m_engine->resourceCache();
    m_grid = Lines::GetPlane(cache, 1.0, 100, (int)ResourceBehaviorFlag::kCore);

    // Set the grid as a core resource
    Mesh* grid = cache.polygonCache()->getGridPlane(1.0, 100);
    grid->handle()->setCore(true);

    // Set GL draw settings
    m_renderSettings.setShapeMode(PrimitiveMode::kTriangles);
    m_renderSettings.addSetting<DepthSetting>(false, DepthPassMode::kLessEqual, false);
    m_renderSettings.addDefaultBlend();
    //setName(m_grid->getName());
}
/////////////////////////////////////////////////////////////////////////////////////////////
DebugGrid::~DebugGrid()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugGrid::bindUniforms(ShaderProgram& shaderProgram)
{
    shaderProgram.setUniformValue("lineColor", Vector4(1.0, 1.0, 1.0, 1.0));

	//if (!UBO::getCameraBuffer()) return;
 //   auto cameraBuffer = UBO::getCameraBuffer();

 //   // Set world matrix uniform and update uniforms in shader queue
 //   Matrix3x3g worldMatrix3x3;
 //   const Matrix4x4g& viewMatrix = cameraBuffer->getUniformValue("viewMatrix").get<Matrix4x4g>();
 //   Vector3 eye = viewMatrix.getColumn(3);
 //   //real_g distance = eye.length();
 //   //float scale = (float)pow(4.0f, ceil(log10(distance) / log10(4.0)));
 //   float scale = 100;
 //   worldMatrix3x3 *= scale;
 //   Matrix4x4g worldMatrix(worldMatrix3x3);

    m_grid->setLineColor(Vector4(0.9f, 0.9f, 0.9f, 0.5f));
    m_grid->setLineThickness(1.25f);
    m_grid->setConstantScreenThickness(false);
    m_grid->setFadeWithDistance(true);
    //m_grid->transform().updateWorldMatrix(worldMatrix);
    m_grid->setUseMiter(false);

    // Add rotation to transform
    //m_grid->transform().rotateAboutAxis({ 1.0f, 0.0f, 0.0f }, Constants::PI_2);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugGrid::releaseUniforms(rev::ShaderProgram& shaderProgram)
{
    Q_UNUSED(shaderProgram)
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugGrid::drawGeometry(ShaderProgram& shaderProgram, RenderSettings * settings)
{
    // TODO: Use actual infinite ground plane
    // https://github.com/martin-pr/possumwood/wiki/Infinite-ground-plane-using-GLSL-shaders

    // Draw outer grid
    Q_UNUSED(settings);

    RenderContext& context = m_engine->mainRenderer()->renderContext();

    m_grid->draw(shaderProgram, &context);

    //// Draw inner grid
    //Matrix3x3g worldMatrix3x3 = m_grid->transform().worldMatrix();
    //worldMatrix3x3 *= (real_g)(1.0 / 10.0);
    //Matrix4x4g worldMatrix(worldMatrix3x3);
    //worldMatrix.setTranslation({ 0.0f, -0.05f, 0.0f });
    //m_grid->setLineColor(Vector4((real_g)0.9, (real_g)0.9, (real_g)0.9, (real_g)0.5));
    //m_grid->setLineThickness(0.5f);
    //m_grid->transform().updateWorldMatrix(worldMatrix);

    //m_grid->draw(shaderProgram, &context);

}




/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// DebugDrawSettings
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue DebugDrawSettings::asJson(const SerializationContext& context) const
{
    QJsonObject object;
    object.insert("drawAxes", m_drawFlags.testFlag(DebugDrawFlag::kDrawAxes));
    object.insert("drawGrid", m_drawFlags.testFlag(DebugDrawFlag::kDrawGrid));
    object.insert("drawBounds", m_drawFlags.testFlag(DebugDrawFlag::kDrawBoundingBoxes));
    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugDrawSettings::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context);

    QJsonObject object = json.toObject();
    m_drawFlags.setFlag(DebugDrawFlag::kDrawAxes, object["drawAxes"].toBool(true));
    m_drawFlags.setFlag(DebugDrawFlag::kDrawGrid, object["drawGrid"].toBool(true));
    m_drawFlags.setFlag(DebugDrawFlag::kDrawBoundingBoxes, object["drawBounds"].toBool(true));
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// DebugManager
/////////////////////////////////////////////////////////////////////////////////////////////
DebugManager::DebugManager(CoreEngine* engine) :
    Manager(engine, "Debug Manager"),
    m_physicsRaycast(std::make_unique<PhysicsRaycast>()),
    m_raycast(std::make_unique<WorldRay>()),
    m_fpsCounter(nullptr),
    m_debugRenderLayer(std::make_shared<SortingLayer>(QStringLiteral("Debug"), 100))
{
    // Set raycast to cast against triangles
    m_raycast->setRaycastFlags((size_t)RaycastFlag::kTestTriangles 
        | (size_t)RaycastFlag::kSingleHitPerObject
    );
}
/////////////////////////////////////////////////////////////////////////////////////////////
DebugManager::~DebugManager()
{
    // Remove scene objects
    SceneObject::EraseFromNodeVec(m_cameraObject->id());
    SceneObject::EraseFromNodeVec(m_canvasObject->id());
}
/////////////////////////////////////////////////////////////////////////////////////////////
CameraComponent * DebugManager::camera()
{
    if (!m_cameraObject) {
        return nullptr;
    }
    return m_cameraObject->hasComponent<CameraComponent>(ComponentType::kCamera);
}
/////////////////////////////////////////////////////////////////////////////////////////////
CanvasComponent * DebugManager::textCanvas()
{
    if (!m_canvasObject) {
        return nullptr;
    }
    if (!m_canvasObject->hasComponent(ComponentType::kCanvas)) {
        return nullptr;
    }
    return dynamic_cast<CanvasComponent*>(
        m_canvasObject->components()[(int)ComponentType::kCanvas][0]);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::draw(Scene* scene)
{
    // Make sure to bind camera buffer uniforms!
    camera()->camera().bindUniforms(nullptr);
    drawStatic();
    if (scene != m_scene.get()) {
        drawDynamic(scene);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::drawDynamic(Scene* scene)
{
    // Render component-specific renderables
    for (const std::shared_ptr<SceneObject>& so : scene->topLevelSceneObjects()) {
        drawDynamic(so);
    }

    // Draw raycast hit
    //drawRaycastContact();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::drawDynamic(const std::shared_ptr<SceneObject>& so) 
{
    // Check if in bounds
    // TODO: Maybe ascribe bounds to scene objects at runtime from component AABBs
    BoundingSphere sphere = BoundingSphere();
    sphere.recalculateBounds(so->transform(), sphere);
    sphere.setRadius(4);

    if (!camera()->camera().frustum().intersects(sphere)) {
        return;
    }

    const std::vector<std::vector<Component*>>& components = so->components();
    size_t count = 0;
    for (const auto& componentVec : components) {
        // Iterate through scene object components
        ComponentType componentType = ComponentType(count);
        for (const auto& component : componentVec) {

            // Draw debug renderable based on component type
            DebugRenderable* renderable = nullptr;
            switch (componentType) {
            case ComponentType::kTransform: {
                // Handled separately
                break;
            }
            case ComponentType::kLight: {
                LightComponent* comp = static_cast<LightComponent*>(component);
                drawLight(comp);
                break;
            }
            default:
                continue;
            }
            if (renderable) {
                renderable->draw();
            }
        }

        count++;
    }

    if (m_settings.m_drawFlags.testFlag(DebugDrawFlag::kDrawAxes)) {
        // Render transform component
        DebugRenderable* transformRenderable = &m_dynamicRenderables["coordinateAxes"];
        auto debugAxes = std::static_pointer_cast<DebugCoordinateAxes>(
            transformRenderable->m_renderable);

        // Set uniforms
        transformRenderable->m_shaderProgram->bind();
        debugAxes->setTransform(&so->transform());
        transformRenderable->m_shaderProgram->updateUniforms();
        //for (const auto& uniform : transformRenderable->m_shaderProgram->uniforms()) {
        //    logInfo(QString(uniform));
        //}
        //logInfo(UBO::getCameraBuffer()->getUniformValue("viewMatrix"));
        //logWarning("------------");
        transformRenderable->draw();
    }

    for (const std::shared_ptr<SceneObject>& child : so->children()) {
        drawDynamic(child);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::drawStatic()
{
    // Render label and icon for scene object location
    //m_textCanvas.m_shaderProgram->bind();
    //m_textCanvas.draw();

    // TODO: Deprecate this function
    for (auto it = m_staticRenderables.begin(); it != m_staticRenderables.end();) {

        if (!it->second.object())
        {
            // Erase debug renderable if the object is gone
            m_staticRenderables.erase(it++);    // or "it = m.erase(it)" since C++11
        }
        else
        {
            if (it->first == QStringLiteral("grid")) {
                // Grid is handled by createDrawCommands now
                ++it;
                continue;
            }

            // Set camera uniforms
            it->second.m_shaderProgram->bind();
            it->second.m_shaderProgram->updateUniforms();

            // Draw debug renderable
            it.value().draw();
            ++it;
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::drawDoodads()
{
    for (auto it = m_doodads.begin(); it != m_doodads.end();) {

        if (!it->object() || it->isDone())
        {
            // Erase debug renderable if the object is gone
            m_doodads.erase(it++);    // or "it = m.erase(it)" since C++11
        }
        else
        {
            // Set camera uniforms
            it->m_shaderProgram->bind();
            it->m_shaderProgram->updateUniforms();

            // Draw debug renderable
            it->draw();
            ++it;
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::createDrawCommands(Scene& scene, MainRenderer & renderer)
{
    if (!textCanvas()) {
        return;
    }

    // Create new draw commands
    std::vector<std::shared_ptr<DrawCommand>> drawCommands;

    // Before creating new draw commands, iterate over all other draw commands to create bounding boxes
    if (m_settings.m_drawFlags.testFlag(DebugDrawFlag::kDrawBoundingBoxes)) {
        for (const std::shared_ptr<RenderCommand>& command : renderer.receivedCommands()) {
            if (command->commandType() != RenderCommand::CommandType::kDraw) {
                continue;
            }

            std::shared_ptr<DrawCommand> drawCommand = S_CAST<DrawCommand>(command);
            const AABB& bounds = drawCommand->worldBounds();

            //if (!bounds.count()) {
            //    logWarning("Warning, geometry has no bounds");
            //}

            createDrawBoxCommand(bounds, drawCommands);
        }
    }

    // Create text commands
    ShaderProgram* canvasProgram = m_engine->resourceCache()->getHandleWithName("canvas_gui",
        ResourceType::kShaderProgram)->resourceAs<ShaderProgram>();
    textCanvas()->createDrawCommands(drawCommands,
        camera()->camera(),
        *canvasProgram);

    // Create static draw commands
    if (m_settings.m_drawFlags.testFlag(DebugDrawFlag::kDrawGrid) && UBO::getCameraBuffer()) {
        // Whether or not to draw grid

        // Create world matrix
        static Transform gridTransform;
        auto cameraBuffer = UBO::getCameraBuffer();
        Matrix3x3g worldMatrix3x3;
        const Matrix4x4g& viewMatrix = cameraBuffer->getUniformValue("viewMatrix").get<Matrix4x4g>();
        Vector3 eye = viewMatrix.getColumn(3);
        //float scale = (float)pow(4.0f, ceil(log10(distance) / log10(4.0)));
        float scale = 100;
        worldMatrix3x3 *= scale;
        Matrix4x4g worldMatrix(worldMatrix3x3);
        gridTransform.updateWorldMatrix(worldMatrix);
        gridTransform.rotateAboutAxis({ 1.0f, 0.0f, 0.0f }, Constants::PI_2);

        // Create draw command
        Renderable& grid = *m_staticRenderables["grid"].m_renderable;
        auto command = std::make_shared<DrawCommand>(grid,
            *m_lineShaderProgram,
            camera()->camera(),
            (int)RenderObjectId::kDebug);
        command->setPassFlags(RenderablePassFlag::kDeferredGeometry); // Not setting bounds
        command->addUniform(Uniform("worldMatrix", gridTransform.worldMatrix()));
        drawCommands.push_back(command);
    }

    // Draw physics raycast contacts
    drawPhysicsRaycastContact(drawCommands);

    // Draw non-physics raycast contacts
    drawRaycastContact(drawCommands);

    // Draw thicker bounding box around raycasted objects
    if (m_settings.m_drawFlags.testFlag(DebugDrawFlag::kDrawBoundingBoxes)) {
        for (const WorldRayHit& hit : m_hits) {
            const SceneObject& hitObject = *SceneObject::Get(hit.m_hitInfo.m_sceneObjectId);
            const BoundingBoxes& bounds = hitObject.worldBounds();

            for (const auto& box : bounds.geometry()) {
                createDrawBoxCommand(box, drawCommands, 0.01f, Vector4(1.0f, 1.0f, 0.7f, 1.0f));
            }

            // Only do this for first hit
            break;
        }
    }

    // Draw camera bounding boxes:
    //{
    //    std::vector<AABBData> aabbs;
    //    camera()->camera().lightClusterGrid().getGridBoxes(aabbs);

    //    Matrix4x4g inverseView = camera()->camera().getViewMatrix().inversed();
    //    size_t numRows = 1;

    //    std::shared_ptr<Lines> lineCube = S_CAST<Lines>(m_dynamicRenderables["cube"].m_renderable);

    //    // FIXME: Set this during draw call
    //    lineCube->drawFlags().setFlag(Lines::DrawFlag::kIgnoreWorldMatrix, true);
    //    lineCube->setLineColor(Vector4g(1.0f, 0.0f, 0.5, 1.0f));
    //    lineCube->setLineThickness(0.005f);
    //    lineCube->setConstantScreenThickness(true);
    //    lineCube->setFadeWithDistance(false);
    //    lineCube->setUseMiter(false);

    //    for (size_t j = 0; j < numRows; j++) {

    //        for (size_t i = 0; i < 16 * 9; i++) {
    //            const AABBData& box = aabbs[i + j * 16 * 9];

    //            // Convert from view space to world space);
    //            Matrix4x4g worldMatrix;
    //            std::vector<Vector3g> points;
    //            box.getPoints(points);

    //            Vector3g scale = box.getDimensions();
    //            worldMatrix(0, 0) = scale.x();
    //            worldMatrix(1, 1) = scale.y();
    //            worldMatrix(2, 2) = scale.z();

    //            Vector3g origin = box.getOrigin();
    //            worldMatrix.setTranslation(origin);
    //            lineCube->transform().updateWorldMatrix(worldMatrix);

    //            // box is in view space, so revert
    //            worldMatrix = inverseView * worldMatrix;

    //            auto command = std::make_shared<DrawCommand>(*lineCube,
    //                *m_lineShaderProgram,
    //                camera()->camera());
    //            command->setUniform(Uniform("worldMatrix", worldMatrix));
    //            //command->setUniform(Uniform("lineColor", Vector4g(0.5f, 0.0f, 1.0, 1.0f)));
    //            drawCommands.push_back(command);
    //        }
    //    }
    //}


    for (const std::shared_ptr<SceneObject>& so : scene.topLevelSceneObjects()) {
        if (so->isVisible()) {
            createDrawCommand(*so, drawCommands);
        }
    }


    // Add commands to debug layer
    for (const auto& command : drawCommands) {
        command->setRenderLayer(m_debugRenderLayer.get());
        renderer.addRenderCommand(command);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::step(unsigned long deltaMs)
{
    if (!m_engine->isConstructed()) return;
    if (!m_engine->scenario()) {
        return;
    }
    if (m_engine->scenario()->isLoading()) {
        return;
    }
    if (!m_cameraObject) {
        return;
    }

    // Move camera
    camera()->controller().step(deltaMs);

    // Process raycasts
    processRaycasts();

    // Handle inputs
    processInputs();

    // Set OpenGL context to main context to avoid VAO struggles
    m_engine->setGLContext();

    // Set FPS counter
    if (m_fpsCounter) {
        if (m_frameDeltas.size() > 15) {
            m_frameDeltas.pop_front();
        }
        m_frameDeltas.push_back(1000.0 / double(deltaMs));
        float fps = getFps();
        size_t numDigits = floor(log10(fps)) + 1; // One if less than 10, Two digits if less than 100, three if greater, etc.
        m_fpsCounter->setText(QString::number(fps, 'g', numDigits));
    }

}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::postConstruction()
{
    // FIXME: Load and set cubemap for skybox
    // Problematic, can't access via non-Qt load, 
    // see: https://stackoverflow.com/questions/43880820/how-to-convert-qt-file-path-from-resource-to-absolute-path
    //std::shared_ptr<ResourceHandle> cubemapHandle =
    //    m_engine->resourceCache()->guaranteeHandleWithPath(
    //        ":/textures/debug_skybox/debug.cubemap",
    //        ResourceType::kCubeTexture
    //    );
    //cubemapHandle->setCore(true);

    // Set up scene
    m_scene = std::make_shared<Scene>(m_engine);
    m_scene->setName("Debug Objects");

    // Set up camera
    initializeCamera();

    // Set up canvas
    initializeCanvas();

    // Set up grid
    // TODO: Use actual infinite ground plane
    // https://github.com/martin-pr/possumwood/wiki/Infinite-ground-plane-using-GLSL-shaders
    ResourceCache& cache = *m_engine->resourceCache();
    m_lineShaderProgram = cache.getHandleWithName("lines",
        ResourceType::kShaderProgram)->resourceAs<ShaderProgram>();
    auto grid = std::make_shared<DebugGrid>(m_engine);
    m_staticRenderables["grid"] = { grid, m_lineShaderProgram, grid };

    // Set up a line plane
    std::shared_ptr<Lines> plane = Lines::GetPlane(cache, 1.0, 1000, int(ResourceBehaviorFlag::kCore));
    m_dynamicRenderables["plane"] = { plane, m_lineShaderProgram, plane };

    // Set up line and point box
    std::shared_ptr<Lines> box = Lines::GetCube(cache, int(ResourceBehaviorFlag::kCore));
    m_dynamicRenderables["cube"] = { box, m_lineShaderProgram, box };

    std::shared_ptr<Points> boxPoints = std::make_shared<Points>(*box);
    m_dynamicRenderables["pointCube"] = { boxPoints , m_pointShaderProgram, boxPoints };

    // Set up line and point sphere
    std::shared_ptr<Lines> sphere = Lines::GetSphere(cache, 20, 30, int(ResourceBehaviorFlag::kCore));
    m_dynamicRenderables["sphere"] = { sphere, m_lineShaderProgram, sphere };

    std::shared_ptr<Points> spherePoints = std::make_shared<Points>(*sphere);
    m_dynamicRenderables["pointSphere"] = { spherePoints , m_pointShaderProgram, spherePoints };

    // Set up simple shader program
    m_simpleShaderProgram = cache.getHandleWithName("simple",
        ResourceType::kShaderProgram)->resourceAs<ShaderProgram>();

    // Set up point shader program
    m_pointShaderProgram = cache.getHandleWithName("points",
        ResourceType::kShaderProgram)->resourceAs<ShaderProgram>();

    // Set up coordinate axes
    m_axisShaderProgram = cache.getHandleWithName("axes",
        ResourceType::kShaderProgram)->resourceAs<ShaderProgram>();
    auto coordinateAxes = std::make_shared<DebugCoordinateAxes>(m_engine);
    m_dynamicRenderables["coordinateAxes"] = { coordinateAxes, m_axisShaderProgram , coordinateAxes };
    m_staticRenderables["coordinateAxes"] = m_dynamicRenderables["coordinateAxes"];

    // Create debug skeleton program
    m_debugSkeletonProgram = cache.getHandleWithName("debug_skeleton",
        ResourceType::kShaderProgram)->resourceAs<ShaderProgram>();

    // Create points to use for rendering skeletons
    auto skeletonPoints = std::make_shared<Points>(cache, 100, 
        ResourceBehaviorFlag::kRuntimeGenerated | ResourceBehaviorFlag::kCore);
    skeletonPoints->renderSettings().addSetting<DepthSetting>(false, DepthPassMode::kLessEqual, false);
    m_dynamicRenderables["skeletonPoints"] = { skeletonPoints, m_debugSkeletonProgram, skeletonPoints};

    // Set renderable for raycast hit
    auto raycastHit = Lines::GetPrism(cache, 0.25f, 0.0f, 2.0f, 6, 1, int(ResourceBehaviorFlag::kCore));

    m_dynamicRenderables["raycastHit"] = {raycastHit, m_lineShaderProgram, raycastHit};

    // Set GL widget
    m_glWidget = m_engine->widgetManager()->mainGLWidget();

    Manager::postConstruction();
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue DebugManager::asJson(const SerializationContext& context) const
{
    QJsonObject object;
    object.insert("debugScene", m_scene->asJson());
    object.insert("debugSettings", m_settings.asJson());

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    const QJsonObject& object = json.toObject();
    if (object.contains("debugScene")) {
        // Delete old camera object 
        m_cameraObject = nullptr; 

        // Delete old canvas object
        m_canvasObject = nullptr;
        m_fpsCounter = nullptr;

        // Load scene
        m_scene->loadFromJson(json["debugScene"]);
        m_cameraObject = m_scene->getSceneObjectByName("Debug Camera");
        m_canvasObject = m_scene->getSceneObjectByName("Debug Canvas");

        if (!m_canvasObject) {
            // Ensure that text canvas is created
            initializeCanvas();
        }
        else {
            // Don't want label's mesh getting deleted on scenario reload
            auto* cc = m_canvasObject->hasComponent<CanvasComponent>(ComponentType::kCanvas);
            auto mainLabel = std::dynamic_pointer_cast<Label>(cc->glyphs()[0]);
            mainLabel->mesh()->handle()->setCore(true);

            // Don't want label's mesh deleted on scenario reload, also need to set FPS counter
            m_fpsCounter = std::dynamic_pointer_cast<Label>(cc->glyphs()[1]);
            m_fpsCounter->mesh()->handle()->setCore(true);
        }
    }
    else {
        m_cameraObject->loadFromJson(object.value("cameraObject"));
    } 

    // TODO: Add debug cubemap

    // Add object to debug render layer so that skybox renders
    //m_cameraObject->addRenderLayer(m_debugRenderLayer);

    CameraComponent* camera = m_cameraObject->hasComponent<CameraComponent>(ComponentType::kCamera);
    camera->camera().cameraOptions().setFlag(CameraOption::kShowAllRenderLayers, true);

    if (object.contains("debugSettings")) {
        m_settings.loadFromJson(object["debugSettings"]);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::processRaycasts() {
    // Generate raycasts
    bool hit;

    m_hits.clear();

    if (!m_glWidget->underMouse()) return;

    if (!m_engine->scenario()) return;

    // Set raycast origin and direction
    m_raycast->setOrigin(cameraObject()->transform().getPosition().asReal());
    camera()->camera().widgetToRayDirection(m_glWidget->widgetMousePosition(), m_raycast->direction(), *m_glWidget->renderer());
    m_raycast->initialize();

    // Standard scene raycast
    m_engine->scenario()->scene().raycast(*m_raycast, m_hits);

    // Physics raycast
    // Skip if the scene has no physics
    if (m_engine->scenario()->scene().physics()) {

        // Generate physics raycast
        *m_physicsRaycast = PhysicsRaycast();
        m_physicsRaycast->m_origin = m_raycast->origin();
        m_physicsRaycast->m_direction = m_raycast->direction();
        hit = m_engine->scenario()->scene().physics()->raycast(*m_physicsRaycast);
        Q_UNUSED(hit)
        //if (hit) {
        //    RaycastHit hit = raycast->m_hits.getHit(0);
        //    HitFlags flags = hit.flags();
        //    bool hasPos = flags.testFlag(HitEnum::PxHitFlags::kPOSITION);
        //    bool hasNormal = flags.testFlag(HitEnum::PxHitFlags::kPOSITION);
        //    bool hasUV = flags.testFlag(HitEnum::PxHitFlags::kUV);
        //    Vector3g position = hit.position();
        //    Vector3g normal = hit.normal();
        //    //logInfo("HIT HIT HIT: Position: " + QString(position) +", Normal: " + QString(normal));
        //}
    }

    // Emit signal if selected scene object changed via raycast
    if (m_hits.size() && m_glWidget->inputHandler().mouseHandler().wasDoubleClicked(Qt::LeftButton)) {

        //size_t count = 0;
        //for (const WorldRayHit& hit : m_hits) {
        //    logInfo(GString::FromNumber(hit.m_hitInfo.m_sceneObjectId).c_str());
        //    logInfo(GString("Position:") + GString(hit.m_position));
        //    logInfo(GString("Normal:") + GString(hit.m_normal));
        //    count++;

        //    if (count > 0) {
        //        break;
        //    }
        //}
        //logInfo("------------------------------");

        // Check if a new scene object has been selected
        size_t firstHit = m_hits[0].m_hitInfo.m_sceneObjectId;
        bool selectedNewObject = true;
        if (m_selectedSceneObjects.size()) {
            if (m_selectedSceneObjects[0]->id() == firstHit) {
                selectedNewObject = false;
            }
        }
        
        // Emit signal if a new scene object has been selected
        if (selectedNewObject) {
            m_selectedSceneObjects.clear();
            m_selectedSceneObjects.push_back(SceneObject::Get(firstHit));
            emit selectedSceneObject(firstHit);
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::processInputs() {

    KeyHandler& kh = m_glWidget->inputHandler().keyHandler();
    if (kh.wasPressed(Qt::Key::Key_S) && kh.isHeld(Qt::Key::Key_Shift)) {
        // Pressed Shift+S;
        // Center camera on selected object
        if (!m_selectedSceneObjects.size()) {
            return;
        }

        const auto& so = m_selectedSceneObjects[0];

        float distance = 1.5f * so->worldBounds().geometry()[0].boxData().getDimensions().length();
        Vector3 eye = so->transform().worldPosition() + Vector3(distance, distance, 0.0f);
        camera()->camera().setLookAt(eye, so->transform().worldPosition(), Vector3::Up());
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::initializeCamera()
{
    m_cameraObject = SceneObject::Create(m_scene.get());
    m_cameraObject->setName("Debug Camera");
    m_cameraObject->transform().setWorldPosition(Vector3::Forward());
    new CameraComponent(m_cameraObject);

    CameraController& controller = camera()->controller();
    controller.profile().m_movementTypes = tsl::robin_map<CameraController::MovementType, bool>{
        {CameraController::kZoom, true},
        {CameraController::kPan, true},
        {CameraController::kTilt, true},
        {CameraController::kTranslate, true},
        {CameraController::kRotate, true},
        {CameraController::kTilt, true}
    };
    camera()->camera().cameraOptions().setFlag(CameraOption::kShowAllRenderLayers, true);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::initializeCanvas() {

    m_canvasObject = SceneObject::Create(m_scene.get());
    m_canvasObject->setName("Debug Canvas");
    CanvasComponent* textCanvas = new CanvasComponent(m_canvasObject);
    textCanvas->addRequiredComponents();

    //textCanvas->setGlyphMode(GlyphMode::kGUI);

    std::shared_ptr<Label> mainLabel = std::make_shared<Label>(textCanvas, "Debug View",
        "arial", 60, ResourceBehaviorFlag::kRuntimeGenerated | ResourceBehaviorFlag::kCore);
    mainLabel->transform().setPosition({ 0.0f, 0.90f });
    mainLabel->setAlignment(Glyph::kMiddle, Glyph::kCenter);

    // Don't want this getting deleted on scenario reload
    mainLabel->mesh()->handle()->setCore(true);

    textCanvas->addGlyph(mainLabel);

    m_fpsCounter = std::make_shared<Label>(textCanvas, "0",
        "courier", 60, ResourceBehaviorFlag::kRuntimeGenerated | ResourceBehaviorFlag::kCore);
    m_fpsCounter->transform().setPosition({ -0.97f, 0.90f });
    m_fpsCounter->setColor(Color(Vector3(0.4f, 0.2f, 0.7f)));
    m_fpsCounter->setAlignment(Glyph::kMiddle, Glyph::kLeft);

    // Don't want this getting deleted on scenario reload
    m_fpsCounter->mesh()->handle()->setCore(true);

    textCanvas->addGlyph(m_fpsCounter);
}
/////////////////////////////////////////////////////////////////////////////////////////////
InputHandler& DebugManager::inputHandler() const
{
    return m_engine->widgetManager()->mainGLWidget()->inputHandler();
}
/////////////////////////////////////////////////////////////////////////////////////////////
float DebugManager::getFps() const
{
    return std::accumulate(m_frameDeltas.begin(), m_frameDeltas.end(), 0.0) / m_frameDeltas.size();
}

/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::drawPhysicsShape(const PhysicsShapePrefab& shape, 
    const TransformComponent& transform, bool isEnabled, 
    std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands)
{
    if (!m_lineShaderProgram) return;
    if (!m_pointShaderProgram) return;

    bool wasHit = false;
    if (m_physicsRaycast->hadHit()) {
        PhysicsRaycastHit hit = m_physicsRaycast->firstHit();
        SceneObject* hitObject = hit.sceneObject();
        if (hitObject) {
            wasHit = hitObject->id() == transform.sceneObject()->id();
        }
        else {
            logError("Error, scene object for hit not found");
        }
    }

    // Set point/line color based on enabled status
    // TODO: Fix this, need to set uniforms in draw command, not in renderable itself
    Vector4 lineColor(0.85f, 0.8f, 0.0f, 0.9f);
    Vector4 pointColor(0.9f, 0.25f, 0.2f, 0.9f);
    float pointSize = 0.006f;
    float lineThickness = 0.0075f;
    if (!isEnabled) {
        lineThickness = 0.005f;
        lineColor = Vector4(0.5f, 0.5f, 0.5f, 0.9f);
        pointColor = lineColor;
        pointSize = 0.004f;
    }
    else {
        if (wasHit) {
            lineThickness *= 1.25f;
            lineColor = Vector4(0.8f, 0.9f, 1.0f, 1.0f);
            pointColor = lineColor;
            pointSize *= 1.25f;
        }
    }

    // Set camera uniforms
    //m_lineShaderProgram->bind();

    //m_cone = std::static_pointer_cast<Mesh>(
    //    engine->resourceCache()->polygonCache()->getCylinder(0.2f, 0.0f, m_coneHeight, 36, 1)->resource(false));

    Matrix4x4g worldMatrix = transform.rotationTranslationMatrix();
    //QString worldStr(worldMatrix);

    switch (shape.geometry()->getType()) {
    case PhysicsGeometry::kBox: {
        auto* boxGeometry = static_cast<BoxGeometry*>(shape.geometry());
        Vector3 scale = {boxGeometry->hx() * 2.0f, boxGeometry->hy()* 2.0f,  boxGeometry->hz() * 2.0f };
        worldMatrix.addScale(scale);

        // Set line and world space uniforms
        std::shared_ptr<Lines> lines = std::static_pointer_cast<Lines>(
            m_dynamicRenderables["cube"].object());
        //lines->drawFlags().setFlag(Lines::DrawFlag::kIgnoreWorldMatrix, false);
        lines->setLineColor(lineColor);
        lines->setLineThickness(lineThickness);
        lines->setConstantScreenThickness(true);
        lines->setFadeWithDistance(false);
        lines->setUseMiter(false);

        // Create draw command
        std::shared_ptr<Points> points = std::static_pointer_cast<Points>(
            m_dynamicRenderables["pointCube"].object());
        points->setPointColor(pointColor);
        points->setPointSize(pointSize);

        // Create draw commands for edges and corners
        auto edgeCommand = std::make_shared<DrawCommand>(*lines,
            *m_lineShaderProgram,
            camera()->camera(),
            (int)RenderObjectId::kDebug);
        edgeCommand->addUniform(Uniform("worldMatrix", worldMatrix));
        edgeCommand->setPassFlags(RenderablePassFlag::kDeferredGeometry); // Not setting bounds
        outDrawCommands.push_back(edgeCommand);

        auto cornerCommand = std::make_shared<DrawCommand>(*points,
            *m_pointShaderProgram,
            camera()->camera(),
            (int)RenderObjectId::kDebug);
        cornerCommand->addUniform(Uniform("worldMatrix", worldMatrix));
        cornerCommand->renderSettings().setShapeMode(PrimitiveMode::kPoints);
        cornerCommand->setPassFlags(RenderablePassFlag::kDeferredGeometry); // Not setting bounds
        outDrawCommands.push_back(cornerCommand);

        break;
    }
    case PhysicsGeometry::kSphere: {
        auto* sphereGeometry = static_cast<SphereGeometry*>(shape.geometry());
        Vector3 scale = { sphereGeometry->radius(), sphereGeometry->radius(),  sphereGeometry->radius()};
        worldMatrix.addScale(scale);

        // Set line and world space uniforms
        std::shared_ptr<Lines> lines = std::static_pointer_cast<Lines>(
            m_dynamicRenderables["sphere"].object());
        lines->setLineColor(lineColor);
        lines->setLineThickness(lineThickness);
        lines->setConstantScreenThickness(true);
        lines->setFadeWithDistance(false);
        lines->setUseMiter(false);
        //lines->transform().updateWorldMatrix(worldMatrix);
        m_dynamicRenderables["sphere"].draw();

        // Draw points
        std::shared_ptr<Points> points = std::static_pointer_cast<Points>(
            m_dynamicRenderables["pointSphere"].object());
        points->setPointColor(pointColor);
        points->setPointSize(pointSize);
        //points->transform().updateWorldMatrix(worldMatrix);
        //points->draw(*m_pointShaderProgram, &mainRenderContext());

        // Create draw commands for edges and vertices
        auto edgeCommand = std::make_shared<DrawCommand>(*lines,
            *m_lineShaderProgram,
            camera()->camera(),
            (int)RenderObjectId::kDebug);
        edgeCommand->addUniform(Uniform("worldMatrix", worldMatrix));
        edgeCommand->setPassFlags(RenderablePassFlag::kDeferredGeometry); // Not setting bounds
        outDrawCommands.push_back(edgeCommand);

        auto pointCommand = std::make_shared<DrawCommand>(*points,
            *m_pointShaderProgram,
            camera()->camera(),
            (int)RenderObjectId::kDebug);
        pointCommand->addUniform(Uniform("worldMatrix", worldMatrix));
        pointCommand->renderSettings().setShapeMode(PrimitiveMode::kPoints);
        pointCommand->setPassFlags(RenderablePassFlag::kDeferredGeometry); // Not setting bounds
        outDrawCommands.push_back(pointCommand);

        break;
    }
    case PhysicsGeometry::kPlane: {
        // Set line and world space uniforms
        std::shared_ptr<Lines> lines = std::static_pointer_cast<Lines>(
            m_dynamicRenderables["plane"].object());
        //lines->setLineColor(Vector4g(0.20f, 0.3f, 0.70f, 0.2f));
        lines->setLineColor(lineColor);
        lines->setLineThickness(0.1f);
        lines->setConstantScreenThickness(false);
        lines->setFadeWithDistance(true);
        lines->setUseMiter(false);

        // TODO: Use actual infinite ground plane
        // https://github.com/martin-pr/possumwood/wiki/Infinite-ground-plane-using-GLSL-shaders
        // Add rotation to transform
        Matrix4x4g identity;
        Transform planeTransform;
        planeTransform.updateWorldMatrix(identity);
        planeTransform.rotateAboutAxis({ 0.0f, 1.0f, 0.0f }, Constants::PI_2);

        //lines->transform().updateWorldMatrix(worldMatrix * lines->transform().worldMatrix());

        //const RenderProjection& rp = m_cameraObject->camera()->camera().renderProjection();
        //float nearPlane = rp.nearClipPlane();
        //Matrix4x4g projectionMatrix(m_cameraObject->camera()->camera().renderProjection().projectionMatrix());
        //projectionMatrix(2, 2) = 0;
        //projectionMatrix(2, 3) = -2 * nearPlane;
        //m_lineShaderProgram->setUniformValue("projectionMatrix", projectionMatrix);

        //m_dynamicRenderables["plane"].draw();

        // Create draw commands for edges and corners
        auto command = std::make_shared<DrawCommand>(*lines,
            *m_lineShaderProgram,
            camera()->camera(),
            (int)RenderObjectId::kDebug);
        command->addUniform(Uniform("worldMatrix", planeTransform.worldMatrix()));
        command->setPassFlags(RenderablePassFlag::kDeferredGeometry); // Not setting bounds
        outDrawCommands.push_back(command);

        break; 
    }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::drawPhysicsRaycastContact(std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands)
{
    if (!m_lineShaderProgram) return;
    if (!m_physicsRaycast->hadHit()) return;

    PhysicsRaycastHit hit = m_physicsRaycast->firstHit();

    // Set point/line color based on enabled status
    Vector4 lineColor(0.75f, 0.9f, 1.0f, 0.9f);
    float lineThickness = 0.1f;

    // Set camera uniforms
    //m_lineShaderProgram->bind();
    
    // Set world matrix uniform
    float prismLength = 1.0;
    Quaternion rotation = Quaternion::rotationTo(Vector3(0.0f, 0.0f, 1.0f), hit.normal());
    Matrix4x4g worldMatrix;
    worldMatrix.setTranslation(hit.position() + hit.normal() * prismLength);
    worldMatrix *= rotation.toRotationMatrix4x4();
    //worldMatrix.addScale(Vector3g(1.0f, 1.0f, 1.0f));

    // Set line and world space uniforms
    std::shared_ptr<Lines> lines = std::static_pointer_cast<Lines>(
        m_dynamicRenderables["raycastHit"].object());
    lines->setLineColor(lineColor);
    lines->setLineThickness(lineThickness);
    lines->setConstantScreenThickness(false);
    lines->setFadeWithDistance(false);
    lines->setUseMiter(false);

    // Create draw command
    auto command = std::make_shared<DrawCommand>(*lines,
        *m_lineShaderProgram,
        camera()->camera(),
        (int)RenderObjectId::kDebug);
    command->setPassFlags(RenderablePassFlag::kDeferredGeometry); // Not setting bounds
    command->addUniform(Uniform("worldMatrix", worldMatrix));
    outDrawCommands.push_back(command);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::drawRaycastContact(std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands)
{
    if (!m_lineShaderProgram) return;
    if (!m_hits.size()) return;

    WorldRayHit hit = m_hits[0];

    // Set point/line color based on enabled status
    Vector4 lineColor(1.0f, 0.9f, 0.8f, 0.9f);
    float lineThickness = 0.1f;

    // Set camera uniforms
    //m_lineShaderProgram->bind();

    // Set world matrix uniform
    float prismLength = 2.0f;
    float prismWidth = 2.0f;
    Quaternion rotation = Quaternion::rotationTo(Vector3(0.0f, 0.0f, 1.0f), hit.m_normal);
    Matrix4x4g worldMatrix;
    worldMatrix.setTranslation(hit.m_position + hit.m_normal * prismLength);
    worldMatrix *= rotation.toRotationMatrix4x4();
    worldMatrix.addScale(Vector3(prismWidth, prismWidth, prismLength));

    // Set line and world space uniforms
    std::shared_ptr<Lines> lines = std::static_pointer_cast<Lines>(
        m_dynamicRenderables["raycastHit"].object());
    lines->setLineColor(lineColor);
    lines->setLineThickness(lineThickness);
    lines->setConstantScreenThickness(false);
    lines->setFadeWithDistance(false);
    lines->setUseMiter(false);

    // Create draw command
    auto command = std::make_shared<DrawCommand>(*lines,
        *m_lineShaderProgram,
        camera()->camera(),
        (int)RenderObjectId::kDebug);
    command->setPassFlags(RenderablePassFlag::kDeferredGeometry); // Not setting bounds
    command->addUniform(Uniform("worldMatrix", worldMatrix));
    outDrawCommands.push_back(command);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::clear()
{
    // Delete coordinate axes to avoid crash
    QString coordAxesStr = QStringLiteral("coordinateAxes");
    if (Map::HasKey(m_dynamicRenderables, coordAxesStr)) {
        DebugRenderable* coordAxes = &m_dynamicRenderables[coordAxesStr];
        auto debugAxes = std::static_pointer_cast<DebugCoordinateAxes>(
            coordAxes->m_renderable);
        debugAxes->clear();
    }

    // 
}

/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::drawAnimation(BoneAnimationComponent * animComp, std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands)
{
    // FIXME: Get rid of unused uniform warnings in ShaderProgram::updateUniforms from:
    // color, etc.
    if (!m_debugSkeletonProgram) return;
    //m_debugSkeletonProgram->bind();

    auto model = animComp->animationController().getModel();
    if (!model) return;

    if (!model->skeleton()->handle()) {
        return;
    }

    if (model->skeleton()->handle()->isLoading()) {
        return;
    }

    // Get renderable
    Points& points = *std::static_pointer_cast<Points>(
        m_dynamicRenderables["skeletonPoints"].m_renderable);

    // Bind uniforms
    //animComp->animationController().bindUniforms(*m_debugSkeletonProgram);
    points.setPointColor({1.0f, 0.0f, 1.0f, 1.0f});
    points.setPointSize(0.005f);
    //points.transform().updateWorldMatrix(animComp->sceneObject()->transform().worldMatrix());
    
    // Get colors
    Skeleton skeleton = model->skeleton()->prunedBoneSkeleton();
    IKChain chain(skeleton);
    std::vector<Vector4> colors(points.numPoints());
    size_t idx;
    Vector4 color;
    for (const size_t& nodeIndex : skeleton.boneNodes()) {
        SkeletonJoint& node = skeleton.getNode(nodeIndex);
        idx = node.bone().m_index;
       
        IKNode* ikNode = chain.getNode(node);
        if (ikNode->isEndEffector()) {
            color = Vector4(0.85f, 0.5f, 0.1f, 1.0f);
        }
        else if (ikNode->isRoot()) {
            color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
        }
        else if (ikNode->isSubBase()) {
            color = Vector4(1.0f, 1.0f, 0.2f, 1.0f);
        }
        else {
            color = Vector4(0.75f, 0.5f, 1.0f, 1.0f);
        }
        colors[idx] = color;
    }

    //m_debugSkeletonProgram->setUniformValue("colors", colors);

    // Create draw command
    auto command = std::make_shared<DrawCommand>(points,
        *m_debugSkeletonProgram,
        camera()->camera(),
        (int)RenderObjectId::kDebug);
    animComp->animationController().bindUniforms(*command);
    command->addUniform("colors", colors);
    command->addUniform("worldMatrix", animComp->sceneObject()->transform().worldMatrix());
    command->renderSettings().overrideSettings(points.renderSettings());
    command->renderSettings().setShapeMode(PrimitiveMode::kPoints);
    command->setPassFlags(RenderablePassFlag::kDeferredGeometry); // Not setting bounds
    outDrawCommands.push_back(command);


    //auto boneTransforms = command->getUniform("boneTransforms")->get<std::vector<Matrix4x4g>>();
    //for (const auto& t : boneTransforms) {
    //    auto point = animComp->sceneObject()->transform().worldMatrix() * t * Vector4(0.0f, 0.0f, 0.0f, 1.0f);
    //    logInfo(QString(point));
    //}
    //logWarning("-----------");
}

/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::drawCharacterController(CharControlComponent * comp, std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands)
{
    if (!m_lineShaderProgram) return;
    if (!m_pointShaderProgram) return;

    //bool wasHit = false;
    //if (m_raycast->hadHit()) {
    //    RaycastHit hit = m_raycast->firstHit();
    //    std::shared_ptr<SceneObject> hitObject = hit.sceneObject();
    //    if (hitObject)
    //        wasHit = hitObject->getUuid() == comp->sceneObject()->getUuid();
    //    else
    //        logError("Error, scene object for hit not found");
    //}

    // Set line color based on enabled status
    Vector4 lineColor(0.85f, 0.8f, 0.0f, 0.9f);
    float lineThickness = 0.0075f;
    if (!comp->isEnabled()) {
        lineThickness = 0.005f;
        lineColor = Vector4(0.5f, 0.5f, 0.5f, 0.9f);
    }
    //else {
    //    if (wasHit) {
    //        lineThickness *= 1.25f;
    //        lineColor = Vector4g(0.8f, 0.9f, 1.0f, 1.0f);
    //    }
    //}

    // Set camera uniforms
    //m_lineShaderProgram->bind();
    ResourceCache& cache = *m_engine->resourceCache();
    switch(comp->controller()->getType()){
        case CharacterController::kCapsule:
        {
            auto* controller = dynamic_cast<CapsuleController*>(comp->controller());
            float radius = controller->getRadius();
            float halfHeight = controller->getHeight() / 2.0;
            QString name = PolygonCache::getCapsuleName(radius, halfHeight);

            Matrix4x4g worldMatrix;
            worldMatrix.setTranslation(controller->getPosition().asReal());

            // Set line and world space uniforms
            if (!Map::HasKey(m_dynamicRenderables, name)) {
                std::shared_ptr<Lines> lines = Lines::GetCapsule(cache, radius, halfHeight,
                    ResourceBehaviorFlag::kRuntimeGenerated | ResourceBehaviorFlag::kCore);
                auto capsule = m_engine->resourceCache()->polygonCache()->getCylinder(radius, halfHeight);
                m_dynamicRenderables[name] = {lines, m_lineShaderProgram, comp->sceneObject()};
            }
            std::shared_ptr<Lines> lines = std::static_pointer_cast<Lines>(m_dynamicRenderables[name].m_renderable);
            lines->setLineColor(lineColor);
            lines->setLineThickness(0.05f);
            lines->setConstantScreenThickness(false);
            lines->setFadeWithDistance(true);
            lines->setUseMiter(false);
            //lines->transform().updateWorldMatrix(worldMatrix);

            // Draw outer capsule (with contact offset)
            //lines->draw(*m_lineShaderProgram, &mainRenderContext());

            auto command = std::make_shared<DrawCommand>(*lines, *m_lineShaderProgram, camera()->camera(), (int)RenderObjectId::kDebug);
            command->addUniform(Uniform("worldMatrix", worldMatrix));
            command->setPassFlags(RenderablePassFlag::kDeferredGeometry); // Not setting bounds
            outDrawCommands.push_back(command);

            break;
        }
        default:
            break;
    }

}

/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::drawLight(LightComponent * light)
{
    if (!m_simpleShaderProgram) return;

    // Get color and world matrix uniforms
    Vector4 color = light->getDiffuseColor();
    //light->cacheLight();
    if (!light->isEnabled()) {
        color = Vector4(0.5f, 0.5f, 0.5f, 0.75f);
    }

    // Slow, and only necessary if you really want to ensure lights are all the same size
    //Matrix4x4g worldMatrix = light->sceneObject()->transform().rotationTranslationMatrix();
    const Matrix4x4g& worldMatrix = light->sceneObject()->transform().worldMatrix();

    // Set camera uniforms
    m_simpleShaderProgram->bind();
    
    // Get geometry and set uniforms
    if (!m_sphereMesh) {
        m_sphereMesh = m_engine->resourceCache()->polygonCache()->getSphere(20, 30);
    }
    
    m_simpleShaderProgram->setUniformValue("worldMatrix", worldMatrix);
    m_simpleShaderProgram->setUniformValue("color", color, true);

    // Draw outer capsule (with contact offset)
    m_sphereMesh->vertexData().drawGeometry(PrimitiveMode::kTriangles);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::createDrawCommand(SceneObject& so, std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands)
{
    const std::vector<std::vector<Component*>>& components = so.components();
    size_t count = 0;
    for (const auto& componentVec : components) {
        // Iterate through scene object components
        ComponentType componentType = ComponentType(count);
        for (const auto& component : componentVec) {

            // Create draw commands based on component type
            switch (componentType) {
            case ComponentType::kCharacterController: {
                CharControlComponent* comp = static_cast<CharControlComponent*>(component);
                drawCharacterController(comp, outDrawCommands);
                break;
            }
            case ComponentType::kBoneAnimation: {
                BoneAnimationComponent* comp = static_cast<BoneAnimationComponent*>(component);
                drawAnimation(comp, outDrawCommands);
                break;
            }
            case ComponentType::kRigidBody: {
                RigidBodyComponent* rigidBodyComp = static_cast<RigidBodyComponent*>(component);
                // TODO: Draw if disabled, just grey
                for (const PhysicsShape* shape : rigidBodyComp->body()->shapes()) {
                    const PhysicsShapePrefab& prefab = shape->prefab();
                    drawPhysicsShape(prefab, so.transform(), rigidBodyComp->isEnabled(), outDrawCommands);
                }
                break;
            }
            default:
                continue;
            }
        }

        count++;
    }

    // Create draw commands for child objects
    for (const std::shared_ptr<SceneObject>& child : so.children()) {
        createDrawCommand(*child, outDrawCommands);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::createDrawBoxCommand(const AABB& box, std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands)
{
    createDrawBoxCommand(box, outDrawCommands, 0.0025f, Vector4(1.0f, 0.0f, 1.0f, 1.0f));
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::createDrawBoxCommand(const AABB & box, std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands, float lineThickness, const Vector4 & color)
{
    std::shared_ptr<Lines> lineCube = S_CAST<Lines>(m_dynamicRenderables["cube"].m_renderable);

    lineCube->setLineColor(color);
    lineCube->setLineThickness(lineThickness);
    lineCube->setConstantScreenThickness(true);
    lineCube->setFadeWithDistance(false);
    lineCube->setUseMiter(false);

    Matrix4x4 worldMatrix;
    std::vector<Vector3> points;
    box.boxData().getPoints(points);

    Vector3 scale = box.boxData().getDimensions();
    worldMatrix(0, 0) = scale.x();
    worldMatrix(1, 1) = scale.y();
    worldMatrix(2, 2) = scale.z();

    Vector3 origin = box.boxData().getOrigin();
    worldMatrix.setTranslation(origin);

    auto command = std::make_shared<DrawCommand>(*lineCube,
        *m_lineShaderProgram,
        camera()->camera(),
        (int)RenderObjectId::kDebug);
    command->setPassFlags(RenderablePassFlag::kDeferredGeometry); // Not setting bounds
    command->addUniform(Uniform("worldMatrix", worldMatrix));
    outDrawCommands.push_back(command);
}

/////////////////////////////////////////////////////////////////////////////////////////////
RenderContext& DebugManager::mainRenderContext() {
    return m_engine->mainRenderer()->renderContext();
}

/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::DebugRenderable::draw()
{
    if (m_enabled) {
        RenderContext& context = m_shaderProgram->handle()->engine()->mainRenderer()->renderContext();
        m_renderable->draw(*m_shaderProgram,
            &context, 
            nullptr
            //Renderable::kIgnoreSettings
        );
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

