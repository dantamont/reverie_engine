#include "core/debugging/GDebugManager.h"

#include "core/GCoreEngine.h"
#include "core/scene/GScenario.h"
#include "core/scene/GScene.h"
#include "core/scene/GSceneObject.h"
#include "core/components/GCameraComponent.h"
#include "core/components/GCanvasComponent.h"
#include "core/components/GModelComponent.h"
#include "core/components/GTransformComponent.h"
#include "core/canvas/GLabel.h"

#include "core/geometry/GRaycast.h"

#include "core/rendering/geometry/GMesh.h"
#include "core/rendering/models/GModel.h"
#include "core/resource/GResourceCache.h"
#include "core/rendering/geometry/GPolygon.h"
#include "core/rendering/geometry/GLines.h"
#include "core/rendering/geometry/GPoints.h"
#include "core/rendering/geometry/GSkeleton.h"
#include "core/rendering/buffers/GUniformBufferObject.h"

#include "core/rendering/renderer/GOpenGlRenderer.h"
#include "core/rendering/renderer/GRenderCommand.h"

#include "core/layer/view/widgets/graphics/GInputHandler.h"
#include "core/layer/view/widgets/graphics/GGLWidget.h"

#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/graphics/GGLWidgetInterface.h"
#include "geppetto/qt/widgets/tree/GSceneTreeWidget.h"

#include "core/physics/GPhysicsActor.h"
#include "core/physics/GPhysicsShapePrefab.h"
#include "core/physics/GPhysicsShape.h"
#include "core/physics/GPhysicsQuery.h"
#include "core/physics/GPhysicsScene.h"
#include "core/physics/GPhysicsGeometry.h"
#include "core/physics/GCharacterController.h"
#include <core/components/GCharControlComponent.h>
#include <core/components/GRigidBodyComponent.h>
#include "core/components/GLightComponent.h"

#include "core/components/GAnimationComponent.h"
#include "core/processes/GAnimationProcess.h"
#include "core/animation/GAnimationController.h"
#include "core/physics/GInverseKinematics.h"

#include "core/layer/view/widgets/graphics/GGLWidget.h"
#include "geppetto/qt/widgets/GWidgetManager.h"

#include "fortress/system/memory/GPointerTypes.h"

#include "enums/GPhysicsGeometryTypeEnum.h"

namespace rev {


DebugCoordinateAxes::DebugCoordinateAxes(CoreEngine* engine) :
    m_coneHeight(1.0),
    m_columnHeight(3.0),
    m_xConeTransform(std::make_shared<Transform>()),
    m_yConeTransform(std::make_shared<Transform>()),
    m_zConeTransform(std::make_shared<Transform>()),
    m_xCylinderTransform(std::make_shared<Transform>()),
    m_yCylinderTransform(std::make_shared<Transform>()),
    m_zCylinderTransform(std::make_shared<Transform>()),
    m_renderContext(engine->openGlRenderer()->renderContext()),
    m_setUniforms(false)
{
    // Get resources, and set them as core resources so they don't get removed
    m_cylinder = ResourceCache::Instance().polygonCache()->getCylinder(0.075f, 0.075f, m_columnHeight, 36, 1);
    m_cylinder->handle()->setCore(true);

    m_cone = ResourceCache::Instance().polygonCache()->getCylinder(0.2f, 0.0f, m_coneHeight, 36, 1);
    m_cone->handle()->setCore(true);

    // Initialize GL draw settings
    m_renderSettings.setShapeMode(PrimitiveMode::kTriangles);
    m_renderSettings.addSetting<DepthSetting>(false, DepthPassMode::kLessEqual, false);
    //m_renderSettings.addDefaultBlend();
    //setName("Coordinate Axes");

    // Set up transforms
    /// @todo Make transforms static
    double halfColumnHeight = m_columnHeight * 0.5;
    static const Vector3 s_dirX = Vector3(1.0f, 0.0f, 0.0f);
    static const Vector3 s_dirY = Vector3(0.0f, 1.0f, 0.0f);
    static const Vector3 s_dirZ = Vector3(0.0f, 0.0f, 1.0f);
    m_xConeTransform->setParent(m_xCylinderTransform.get());
    m_xConeTransform->setTranslation(halfColumnHeight * s_dirZ);
    m_xCylinderTransform->rotateAboutAxis({ 0, 1, 0 }, Constants::HalfPi);
    m_xCylinderTransform->setTranslation(halfColumnHeight * s_dirX);

    m_yConeTransform->setParent(m_yCylinderTransform.get());
    m_yConeTransform->setTranslation(halfColumnHeight * s_dirZ);
    m_yCylinderTransform->rotateAboutAxis({ 1, 0, 0 }, -Constants::HalfPi);
    m_yCylinderTransform->setTranslation(halfColumnHeight * s_dirY);

    m_zConeTransform->setParent(m_zCylinderTransform.get());
    m_zConeTransform->setTranslation(halfColumnHeight * s_dirZ);
    m_zCylinderTransform->setTranslation(halfColumnHeight * s_dirZ);

    // Initialize uniforms
    UniformContainer& uc = m_renderContext.uniformContainer();
    Vector4 color = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
    for (Int32_t i = 0; i < 3; i++) {
        m_uniforms.m_axisIndex[i].setValue(i, uc);
        color[i] = 1.0f;
        if (i != 0) color[i - 1] = 0.0f;
        m_uniforms.m_axisColor[i].setValue(color, uc);
    }
    m_uniforms.m_coneTransforms[0].setValue(m_xConeTransform->worldMatrix(), uc);
    m_uniforms.m_coneTransforms[1].setValue(m_yConeTransform->worldMatrix(), uc);
    m_uniforms.m_coneTransforms[2].setValue(m_zConeTransform->worldMatrix(), uc);
    m_uniforms.m_cylinderTransforms[0].setValue(m_xCylinderTransform->worldMatrix(), uc);
    m_uniforms.m_cylinderTransforms[1].setValue(m_yCylinderTransform->worldMatrix(), uc);
    m_uniforms.m_cylinderTransforms[2].setValue(m_zCylinderTransform->worldMatrix(), uc);
    m_uniforms.m_false.setValue(false, uc);
    m_uniforms.m_true.setValue(true, uc);
    m_uniforms.m_worldMatrix.setValue(Matrix4x4::Identity(), uc);
}

DebugCoordinateAxes::~DebugCoordinateAxes()
{
}

void DebugCoordinateAxes::setTransform(TransformInterface* transform)
{
    m_transform = transform;
}

void DebugCoordinateAxes::clear()
{
    m_transform = nullptr;
}

void DebugCoordinateAxes::bindUniforms(ShaderProgram& shaderProgram)
{
    G_UNUSED(shaderProgram);
    if (!m_transform) return;
}

void DebugCoordinateAxes::releaseUniforms(ShaderProgram& shaderProgram)
{
    G_UNUSED(shaderProgram);
}

void DebugCoordinateAxes::drawGeometry(ShaderProgram& shaderProgram,
    RenderSettings * settings)
{
    VertexArrayData& cylinderData = m_cylinder->vertexData();
    VertexArrayData& coneData = m_cone->vertexData();

    // Initialize uniforms
    UniformContainer& uc = m_renderContext.uniformContainer();
    if (!m_setUniforms) {
        shaderProgram.setUniformValue(
            shaderProgram.getUniformId("xConeTransform"),
            m_uniforms.m_coneTransforms[0]);
        shaderProgram.setUniformValue(
            shaderProgram.getUniformId("xCylTransform"),
            m_uniforms.m_cylinderTransforms[0]);
        shaderProgram.setUniformValue(
            shaderProgram.getUniformId("yConeTransform"),
            m_uniforms.m_coneTransforms[1]);
        shaderProgram.setUniformValue(
            shaderProgram.getUniformId("yCylTransform"),
            m_uniforms.m_cylinderTransforms[1]);
        shaderProgram.setUniformValue(
            shaderProgram.getUniformId("zConeTransform"),
            m_uniforms.m_coneTransforms[2]);
        shaderProgram.setUniformValue(
            shaderProgram.getUniformId("zCylTransform"),
            m_uniforms.m_cylinderTransforms[2]);
        m_setUniforms = true;
    }
    // Set world matrix to rotation and translation of scene object
    Matrix4x4 worldMatrix;
    if (m_transform) {
        worldMatrix = m_transform->worldMatrix().rotationTranslationMatrix();
        m_uniforms.m_worldMatrix.setValue(worldMatrix, uc);
    }
    shaderProgram.setUniformValue(
        shaderProgram.uniformMappings().m_worldMatrix,
        m_uniforms.m_worldMatrix);

    // X-axis
    for (size_t i = 0; i < 3; i++) {
        shaderProgram.setUniformValue(
            shaderProgram.getUniformId("axis"),
            m_uniforms.m_axisIndex[i]);
        shaderProgram.setUniformValue(
            shaderProgram.uniformMappings().m_color,
            m_uniforms.m_axisColor[i]);
        shaderProgram.setUniformValue(
            shaderProgram.getUniformId("isCone"), 
            m_uniforms.m_true);
        shaderProgram.updateUniforms(uc);
        coneData.drawGeometry(settings->shapeMode(), 1);

        shaderProgram.setUniformValue(
            shaderProgram.getUniformId("isCone"), 
            m_uniforms.m_false);
        shaderProgram.updateUniforms(uc);
        cylinderData.drawGeometry(settings->shapeMode(), 1);
    }

    //shaderProgram->release();
}





DebugGrid::DebugGrid(CoreEngine* core):
    m_engine(core)
{
    // FIXME: Don't access engine like a singleton, gross
    ResourceCache& cache = ResourceCache::Instance();
    m_grid = Lines::GetPlane(cache, 1.0, 100, (int)ResourceBehaviorFlag::kCore);

    // Set the grid as a core resource
    Mesh* grid = cache.polygonCache()->getGridPlane(1.0, 100);
    grid->handle()->setCore(true);

    // Set GL draw settings
    m_renderSettings.setShapeMode(PrimitiveMode::kTriangles);
    m_renderSettings.addSetting<DepthSetting>(false, DepthPassMode::kLessEqual, false);
    m_renderSettings.addDefaultBlend();
    //setName(m_grid->getName());

    // Set uniforms
    RenderContext& context = m_engine->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();
    m_uniforms.m_lineColor.setValue(Vector4::Ones(), uc);
}

DebugGrid::~DebugGrid()
{
}

void DebugGrid::bindUniforms(ShaderProgram& shaderProgram)
{
    shaderProgram.setUniformValue(shaderProgram.getUniformId("lineColor"), m_uniforms.m_lineColor);

    RenderContext& context = m_engine->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();
    m_grid->setLineColor(Vector4(0.9f, 0.9f, 0.9f, 0.5f), uc);
    m_grid->setLineThickness(1.25f, uc);
    m_grid->setConstantScreenThickness(false, uc);
    m_grid->setFadeWithDistance(true, uc);
    m_grid->setUseMiter(false, uc);

}

void DebugGrid::releaseUniforms(rev::ShaderProgram& shaderProgram)
{
    G_UNUSED(shaderProgram);
}

void DebugGrid::drawGeometry(ShaderProgram& shaderProgram, RenderSettings * settings)
{
    // TODO: Use actual infinite ground plane
    // https://github.com/martin-pr/possumwood/wiki/Infinite-ground-plane-using-GLSL-shaders

    // Draw outer grid
    G_UNUSED(settings);

    RenderContext& context = m_engine->openGlRenderer()->renderContext();

    m_grid->draw(shaderProgram, &context);

    //// Draw inner grid
    //Matrix3x3g worldMatrix3x3 = m_grid->transform().worldMatrix();
    //worldMatrix3x3 *= (Real_t)(1.0 / 10.0);
    //Matrix4x4g worldMatrix(worldMatrix3x3);
    //worldMatrix.setTranslation({ 0.0f, -0.05f, 0.0f });
    //m_grid->setLineColor(Vector4((Real_t)0.9, (Real_t)0.9, (Real_t)0.9, (Real_t)0.5));
    //m_grid->setLineThickness(0.5f);
    //m_grid->transform().updateWorldMatrixWithLocal(worldMatrix);

    //m_grid->draw(shaderProgram, &context);

}






// DebugDrawSettings

void to_json(json& orJson, const DebugDrawSettings& korObject)
{
    orJson["drawAxes"] = korObject.m_drawFlags.testFlag(DebugDrawFlag::kDrawAxes);
    orJson["drawGrid"] = korObject.m_drawFlags.testFlag(DebugDrawFlag::kDrawGrid);
    orJson["drawBounds"] = korObject.m_drawFlags.testFlag(DebugDrawFlag::kDrawBoundingBoxes);
    orJson["drawCameraFrustums"] = korObject.m_drawFlags.testFlag(DebugDrawFlag::kDrawCameraFrustums);
    orJson["drawShadowFrustums"] = korObject.m_drawFlags.testFlag(DebugDrawFlag::kDrawShadowFrustums);
}

void from_json(const json& korJson, DebugDrawSettings& orObject)
{
    orObject.m_drawFlags.setFlag(DebugDrawFlag::kDrawAxes, korJson.value("drawAxes", true));
    orObject.m_drawFlags.setFlag(DebugDrawFlag::kDrawGrid, korJson.value("drawGrid", true));
    orObject.m_drawFlags.setFlag(DebugDrawFlag::kDrawBoundingBoxes, korJson.value("drawBounds", true));
    orObject.m_drawFlags.setFlag(DebugDrawFlag::kDrawCameraFrustums, korJson.value("drawCameraFrustums", true));
    orObject.m_drawFlags.setFlag(DebugDrawFlag::kDrawShadowFrustums, korJson.value("drawShadowFrustums", true));
}



// DebugManager

DebugManager::DebugManager(CoreEngine* engine) :
    Manager(engine, "Debug Manager"),
    m_physicsRaycast(std::make_unique<PhysicsRaycast>()),
    m_raycast(std::make_unique<WorldRay>()),
    m_fpsCounter(nullptr),
    m_debugRenderLayer(std::make_shared<SortingLayer>(100)),
    m_axisShaderProgram(nullptr),
    m_debugSkeletonProgram(nullptr),
    m_glWidget(nullptr),
    m_lineShaderProgram(nullptr),
    m_pointShaderProgram(nullptr),
    m_simpleShaderProgram(nullptr)
{
    // Set raycast to cast against triangles
    m_raycast->setRaycastFlags((size_t)RaycastFlag::kTestTriangles 
        | (size_t)RaycastFlag::kSingleHitPerObject
    );
}

DebugManager::~DebugManager()
{
    // Unnecessary since scene will delete these objects
    // Remove scene objects
    // Might already be cleared on scenario check.
    //SceneObject::EraseFromNodeVec(m_cameraObject->id());
    //SceneObject::EraseFromNodeVec(m_canvasObject->id());
}

CameraComponent * DebugManager::camera()
{
    if (!m_cameraObject) {
        return nullptr;
    }
    return m_cameraObject->getComponent<CameraComponent>(ComponentType::kCamera);
}

CanvasComponent * DebugManager::textCanvas()
{
    if (!m_canvasObject) {
        return nullptr;
    }
    if (!m_canvasObject->getComponent(ComponentType::kCanvas)) {
        return nullptr;
    }
    return static_cast<CanvasComponent*>(m_canvasObject->components()[(int)ComponentType::kCanvas]);
}

void DebugManager::draw(Scene* scene)
{
    // Make sure to bind camera buffer uniforms!
    camera()->camera().bindUniforms(nullptr);
    drawStatic();
    if (scene != m_scene.get()) {
        drawDynamic(scene);
    }
}

void DebugManager::drawDynamic(Scene* scene)
{
    // Render component-specific renderables
    for (const std::shared_ptr<SceneObject>& so : scene->topLevelSceneObjects()) {
        drawDynamic(so);
    }

    // Draw raycast hit
    //drawRaycastContact();
}

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

    const std::vector<Component*>& components = so->components();
    size_t count = 0;
    for (const auto& component : components) {
        ComponentType componentType = ComponentType(count);

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

    if (m_settings.m_drawFlags.testFlag(DebugDrawFlag::kDrawAxes)) {
        // Render transform component
        DebugRenderable* transformRenderable = &m_dynamicRenderables["coordinateAxes"];
        auto debugAxes = std::static_pointer_cast<DebugCoordinateAxes>(
            transformRenderable->m_renderable);

        // Set uniforms
        RenderContext& context = m_engine->openGlRenderer()->renderContext();
        UniformContainer& uc = context.uniformContainer();
        transformRenderable->m_shaderProgram->bind();
        debugAxes->setTransform(&so->transform());
        transformRenderable->m_shaderProgram->updateUniforms(uc);
        transformRenderable->draw();
    }

    for (const std::shared_ptr<SceneObject>& child : so->children()) {
        drawDynamic(child);
    }
}

void DebugManager::drawStatic()
{
    // Render label and icon for scene object location
    //m_textCanvas.m_shaderProgram->bind();
    //m_textCanvas.draw();

    // TODO: Deprecate this function
    RenderContext& context = m_engine->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();
    for (auto it = m_staticRenderables.begin(); it != m_staticRenderables.end();) {

        if (!it->second.object() && it->second.m_hasSceneObject)
        {
            // Erase debug renderable if the associated scene object has gone out of scope
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
            it->second.m_shaderProgram->updateUniforms(uc);

            // Draw debug renderable
            it.value().draw();
            ++it;
        }
    }
}

void DebugManager::drawDoodads()
{
    RenderContext& context = m_engine->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();
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
            it->m_shaderProgram->updateUniforms(uc);

            // Draw debug renderable
            it->draw();
            ++it;
        }
    }
}

void DebugManager::createDrawCommands(Scene& scene, OpenGlRenderer & renderer)
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

            std::shared_ptr<DrawCommand> drawCommand = std::static_pointer_cast<DrawCommand>(command);
            const AABB& bounds = drawCommand->renderableWorldBounds();

            //if (!bounds.count()) {
            //    Logger::LogWarning("Warning, geometry has no bounds");
            //}

            createDrawBoxCommand(bounds, drawCommands);
        }
    }

    // Create camera frustum draw commands
    if (m_settings.m_drawFlags.testFlag(DebugDrawFlag::kDrawCameraFrustums)) {
        static const Color& s_lineColor = Color::White(); // White
        for (const CameraComponent* cameraComp : scene.cameras()) {
            createDrawFrustumCommand(&cameraComp->camera(), drawCommands, s_lineColor);
        }
    }
    if (m_settings.m_drawFlags.testFlag(DebugDrawFlag::kDrawShadowFrustums)) {
        static const Color s_lineColor = Color(0.0f, 1.0f, 1.0f, 1.0f);  // Cyan
        OpenGlRenderer& renderer = *m_engine->openGlRenderer();
        for (const ShadowMap* shadowMap : renderer.renderContext().lightingSettings().shadowMaps()) {
            if (shadowMap->lightComponent()->getLightType() != Light::LightType::kPoint) {
                const Camera* cam = dynamic_cast<const Camera*>(shadowMap->camera());
                createDrawFrustumCommand(cam, drawCommands, s_lineColor);
            }
        }
    }

    // Create text commands
    ShaderProgram* canvasProgram = ResourceCache::Instance().getHandleWithName("canvas_gui",
        EResourceType::eShaderProgram)->resourceAs<ShaderProgram>();
    textCanvas()->createDrawCommands(drawCommands,
        camera()->camera(),
        *canvasProgram);

    // Create static draw commands
    if (m_settings.m_drawFlags.testFlag(DebugDrawFlag::kDrawGrid) && Ubo::GetCameraBuffer()) {
        // Whether or not to draw grid
        RenderContext& context = m_engine->openGlRenderer()->renderContext();
        UniformContainer& uc = context.uniformContainer();

        // Create world matrix
        static Transform gridTransform;
        auto cameraBuffer = Ubo::GetCameraBuffer();
        Matrix3x3g worldMatrix3x3;
        const Matrix4x4g& viewMatrix = cameraBuffer->getBufferUniformValue<Matrix4x4>("viewMatrix");
        Vector3 eye = viewMatrix.getColumn(3);
        //float scale = (float)pow(4.0f, ceil(log10(distance) / log10(4.0)));
        float scale = 100;
        worldMatrix3x3 *= scale;
        Matrix4x4g worldMatrix(worldMatrix3x3);
        gridTransform.updateWorldMatrixWithLocal(worldMatrix);
        gridTransform.rotateAboutAxis({ 1.0f, 0.0f, 0.0f }, Constants::HalfPi);
        m_debugUniforms.m_gridWorldMatrix.setValue(gridTransform.worldMatrix(), uc);

        // Create draw command
        static Int32_t worldMatrixUniformId{ -1 };
        if (worldMatrixUniformId == -1) {
            worldMatrixUniformId = m_lineShaderProgram->getUniformId(Shader::s_worldMatrixUniformName);
        }
        Renderable& grid = *m_staticRenderables["grid"].m_renderable;
        auto command = std::make_shared<DrawCommand>(grid,
            *m_lineShaderProgram,
            uc,
            camera()->camera(),
            (int)RenderObjectId::kDebug);
        command->setPassFlags(RenderablePassFlag::kDeferredGeometry); // Not setting bounds
        command->addUniform(m_debugUniforms.m_gridWorldMatrix, worldMatrixUniformId, -1);
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

    //    std::shared_ptr<Lines> lineCube = std::static_pointer_cast<Lines>(m_dynamicRenderables["cube"].m_renderable);

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
    //            lineCube->transform().updateWorldMatrixWithLocal(worldMatrix);

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
        command->setRenderLayer(*m_debugRenderLayer);
        renderer.addRenderCommand(command);
    }
}

void DebugManager::step(double deltaSec)
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
    camera()->controller().step(deltaSec);

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
        m_frameDeltas.push_back(1.0 / deltaSec);
        float fps = getFps();
        uint32_t numDigits = floor(log10(fps)) + 1; // One if less than 10, Two digits if less than 100, three if greater, etc.
        m_fpsCounter->setText(QString::number(fps, 'g', numDigits));
    }

    clearCounts();
}

void DebugManager::postConstruction()
{
    // FIXME: Load and set cubemap for skybox
    // Problematic, can't access via non-Qt load, 
    /// \see https://stackoverflow.com/questions/43880820/how-to-convert-qt-file-path-from-resource-to-absolute-path
    //std::shared_ptr<ResourceHandle> cubemapHandle =
    //    ResourceCache::Instance().guaranteeHandleWithPath(
    //        ":/textures/debug_skybox/debug.cubemap",
    //        EResourceType::eCubeTexture
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
    ResourceCache& cache = ResourceCache::Instance();
    m_lineShaderProgram = cache.getHandleWithName("lines",
        EResourceType::eShaderProgram)->resourceAs<ShaderProgram>();
    auto grid = std::make_shared<DebugGrid>(m_engine);
    m_staticRenderables["grid"] = { grid, m_lineShaderProgram };

    // Set up a line plane
    std::shared_ptr<Lines> plane = Lines::GetPlane(cache, 1.0, 1000, int(ResourceBehaviorFlag::kCore));
    m_dynamicRenderables["plane"] = { plane, m_lineShaderProgram };

    // Set up line and point box
    std::shared_ptr<Lines> box = Lines::GetCube(cache, int(ResourceBehaviorFlag::kCore));
    m_dynamicRenderables["cube"] = { box, m_lineShaderProgram };

    std::shared_ptr<Points> boxPoints = std::make_shared<Points>(*box);
    m_dynamicRenderables["pointCube"] = { boxPoints , m_pointShaderProgram };

    // Set up line and point sphere
    std::shared_ptr<Lines> sphere = Lines::GetSphere(cache, 20, 30, int(ResourceBehaviorFlag::kCore));
    m_dynamicRenderables["sphere"] = { sphere, m_lineShaderProgram };

    std::shared_ptr<Points> spherePoints = std::make_shared<Points>(*sphere);
    m_dynamicRenderables["pointSphere"] = { spherePoints , m_pointShaderProgram };

    // Set up simple shader program
    m_simpleShaderProgram = cache.getHandleWithName("simple",
        EResourceType::eShaderProgram)->resourceAs<ShaderProgram>();

    // Set up point shader program
    m_pointShaderProgram = cache.getHandleWithName("points",
        EResourceType::eShaderProgram)->resourceAs<ShaderProgram>();

    // Set up coordinate axes
    m_axisShaderProgram = cache.getHandleWithName("axes",
        EResourceType::eShaderProgram)->resourceAs<ShaderProgram>();
    auto coordinateAxes = std::make_shared<DebugCoordinateAxes>(m_engine);
    m_dynamicRenderables["coordinateAxes"] = { coordinateAxes, m_axisShaderProgram };
    m_staticRenderables["coordinateAxes"] = m_dynamicRenderables["coordinateAxes"];

    // Create debug skeleton program
    m_debugSkeletonProgram = cache.getHandleWithName("debug_skeleton",
        EResourceType::eShaderProgram)->resourceAs<ShaderProgram>();

    // Create points to use for rendering skeletons
    auto skeletonPoints = std::make_shared<Points>(cache, 100, ResourceBehaviorFlag::kRuntimeGenerated | ResourceBehaviorFlag::kCore);
    RenderSettings& skeletonPointsSettings = const_cast<RenderSettings&>(skeletonPoints->renderSettings());
    skeletonPointsSettings.addSetting<DepthSetting>(false, DepthPassMode::kLessEqual, false);
    m_dynamicRenderables["skeletonPoints"] = { skeletonPoints, m_debugSkeletonProgram };

    // Set renderable for raycast hit
    auto raycastHit = Lines::GetPrism(cache, 0.25f, 0.0f, 2.0f, 6, 1, int(ResourceBehaviorFlag::kCore));

    m_dynamicRenderables["raycastHit"] = {raycastHit, m_lineShaderProgram };

    // Set GL widget
    m_glWidget = static_cast<GLWidget*>(m_engine->widgetManager()->mainGLWidget());

    Manager::postConstruction();
}

void to_json(json& orJson, const DebugManager& korObject)
{
    orJson["debugScene"] = *korObject.m_scene;
    orJson["debugSettings"] = korObject.m_settings;
}

void from_json(const json& korJson, DebugManager& orObject)
{
    if (korJson.contains("debugScene")) {
        // Delete old camera object 
        orObject.m_cameraObject = nullptr;

        // Delete old canvas object
        orObject.m_canvasObject = nullptr;
        orObject.m_fpsCounter = nullptr;

        // Load scene
        korJson["debugScene"].get_to(*orObject.m_scene);
        orObject.m_cameraObject = orObject.m_scene->getSceneObjectByName("Debug Camera");
        orObject.m_canvasObject = orObject.m_scene->getSceneObjectByName("Debug Canvas");

        if (!orObject.m_canvasObject) {
            // Ensure that text canvas is created
            orObject.initializeCanvas();
        }
        else {
            // Don't want label's mesh getting deleted on scenario reload
            CanvasComponent* cc = orObject.m_canvasObject->getComponent<CanvasComponent>(ComponentType::kCanvas);
            auto mainLabel = std::dynamic_pointer_cast<Label>(cc->glyphs()[0]);
            mainLabel->mesh()->handle()->setCore(true);

            // Don't want label's mesh deleted on scenario reload, also need to set FPS counter
            orObject.m_fpsCounter = std::dynamic_pointer_cast<Label>(cc->glyphs()[1]);
            orObject.m_fpsCounter->mesh()->handle()->setCore(true);
        }
    }
    else {
        korJson.at("cameraObject").get_to(*orObject.m_cameraObject);
    } 

    // TODO: Add debug cubemap

    // Add object to debug render layer so that skybox renders
    //m_cameraObject->addRenderLayer(m_debugRenderLayer);

    CameraComponent* camera = orObject.m_cameraObject->getComponent<CameraComponent>(ComponentType::kCamera);
    camera->camera().cameraOptions().setFlag(CameraOption::kShowAllRenderLayers, true);

    if (korJson.contains("debugSettings")) {
        korJson["debugSettings"].get_to(orObject.m_settings);
    }
}

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
        G_UNUSED(hit);
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
        uint32_t firstHit = m_hits[0].m_hitInfo.m_sceneObjectId;
        bool selectedNewObject = true;
        if (m_selectedSceneObjects.size()) {
            if (m_selectedSceneObjects[0]->id() == firstHit) {
                selectedNewObject = false;
            }
        }
        
        // Emit signal if a new scene object has been selected
        /// @todo Make this have its own signal or something because this be weird
        if (selectedNewObject) {
            m_selectedSceneObjects.clear();
            m_selectedSceneObjects.push_back(SceneObject::Get(firstHit));
            m_engine->widgetManager()->sceneTreeWidget()->m_selectedSceneObjectSignal.emitForAll(firstHit);
        }
    }
}

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

void DebugManager::initializeCamera()
{
    m_cameraObject = SceneObject::Create(m_scene.get());
    m_cameraObject->setName("Debug Camera");
    m_cameraObject->transform().setWorldPosition(Vector3::Forward());
    new CameraComponent(m_cameraObject);

    CameraController& controller = camera()->controller();
    controller.profile().m_movementTypes = tsl::robin_map<int, bool>{
        {CameraController::kZoom, true},
        {CameraController::kPan, true},
        {CameraController::kTilt, true},
        {CameraController::kTranslate, true},
        {CameraController::kRotate, true},
        {CameraController::kTilt, true}
    };
    camera()->camera().cameraOptions().setFlag(CameraOption::kShowAllRenderLayers, true);
}

void DebugManager::initializeCanvas() {

    m_canvasObject = SceneObject::Create(m_scene.get());
    m_canvasObject->setName("Debug Canvas");
    CanvasComponent* textCanvas = new CanvasComponent(m_canvasObject);
    textCanvas->addRequiredComponents(std::vector<Uuid>{}, std::vector<json>{});

    //textCanvas->setGlyphMode(GlyphMode::kGUI);

    std::shared_ptr<Label> mainLabel = Glyph::Create<Label>(textCanvas, "Debug View",
        "arial", 60, ResourceBehaviorFlag::kRuntimeGenerated | ResourceBehaviorFlag::kCore);
    mainLabel->transform().setPosition({ 0.0f, 0.90f, 0.0f });
    mainLabel->setAlignment(Glyph::kMiddle, Glyph::kCenter);

    // Don't want this getting deleted on scenario reload
    mainLabel->mesh()->handle()->setCore(true);

    textCanvas->addGlyph(mainLabel);

    m_fpsCounter = Glyph::Create<Label>(textCanvas, "0",
        "courier", 60, ResourceBehaviorFlag::kRuntimeGenerated | ResourceBehaviorFlag::kCore);
    m_fpsCounter->transform().setPosition({ -0.97f, 0.90f, 0.0f });
    m_fpsCounter->setColor(Color(Vector3(0.4f, 0.2f, 0.7f)));
    m_fpsCounter->setAlignment(Glyph::kMiddle, Glyph::kLeft);

    // Don't want this getting deleted on scenario reload
    m_fpsCounter->mesh()->handle()->setCore(true);

    textCanvas->addGlyph(m_fpsCounter);
}

InputHandler& DebugManager::inputHandler() const
{
    return static_cast<InputHandler&>(m_engine->widgetManager()->mainGLWidget()->inputHandler());
}

float DebugManager::getFps() const
{
    return std::accumulate(m_frameDeltas.begin(), m_frameDeltas.end(), 0.0) / m_frameDeltas.size();
}


void DebugManager::drawPhysicsShape(Int32_t index, const PhysicsShapePrefab& shape,
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
            Logger::LogError("Error, scene object for hit not found");
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
    //    ResourceCache::Instance().polygonCache()->getCylinder(0.2f, 0.0f, m_coneHeight, 36, 1)->resource(false));

    Matrix4x4g worldMatrix = transform.worldMatrix().rotationTranslationMatrix();
    //QString worldStr(worldMatrix);

    static Int32_t worldMatrixUniformIdLines{ -1 };
    if (worldMatrixUniformIdLines == -1) {
        worldMatrixUniformIdLines = m_lineShaderProgram->getUniformId(Shader::s_worldMatrixUniformName);
    }

    static Int32_t worldMatrixUniformIdPoints{ -1 };
    if (worldMatrixUniformIdPoints == -1) {
        worldMatrixUniformIdPoints = m_pointShaderProgram->getUniformId(Shader::s_worldMatrixUniformName);
    }

    RenderContext& context = m_engine->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();
    m_debugUniforms.m_physicsWorldMatrices.ensureSize(index + 1);
    UniformData& worldMatrixUniform = m_debugUniforms.m_physicsWorldMatrices[index];

    switch ((EPhysicsGeometryType)shape.geometry()->getType()) {
    case EPhysicsGeometryType::eBox: {
        auto* boxGeometry = static_cast<BoxGeometry*>(shape.geometry());
        Vector3 scale = {boxGeometry->hx() * 2.0f, boxGeometry->hy()* 2.0f,  boxGeometry->hz() * 2.0f };
        worldMatrix.addScale(scale);
        worldMatrixUniform.setValue(worldMatrix, uc);

        // Set line and world space uniforms
        std::shared_ptr<Lines> lines = std::static_pointer_cast<Lines>(m_dynamicRenderables["cube"].m_renderable);
        //lines->drawFlags().setFlag(Lines::DrawFlag::kIgnoreWorldMatrix, false);
        lines->setLineColor(lineColor, uc);
        lines->setLineThickness(lineThickness, uc);
        lines->setConstantScreenThickness(true, uc);
        lines->setFadeWithDistance(false, uc);
        lines->setUseMiter(false, uc);

        // Create draw command
        std::shared_ptr<Points> points = std::static_pointer_cast<Points>(m_dynamicRenderables["pointCube"].m_renderable);
        points->setPointColor(pointColor, uc);
        points->setPointSize(pointSize, uc);

        // Create draw commands for edges and corners
        auto edgeCommand = std::make_shared<DrawCommand>(*lines,
            *m_lineShaderProgram,
            uc,
            camera()->camera(),
            (int)RenderObjectId::kDebug);

        edgeCommand->addUniform(
            worldMatrixUniform,
            worldMatrixUniformIdLines, 
            -1);

        edgeCommand->setPassFlags(RenderablePassFlag::kDeferredGeometry); // Not setting bounds
        outDrawCommands.push_back(edgeCommand);

        auto cornerCommand = std::make_shared<DrawCommand>(*points,
            *m_pointShaderProgram,
            uc,
            camera()->camera(),
            (int)RenderObjectId::kDebug);

        cornerCommand->addUniform(
            worldMatrixUniform,
            worldMatrixUniformIdPoints, 
            worldMatrixUniformIdPoints);
        cornerCommand->renderSettings().setShapeMode(PrimitiveMode::kPoints);
        cornerCommand->setPassFlags(RenderablePassFlag::kDeferredGeometry); // Not setting bounds
        outDrawCommands.push_back(cornerCommand);

        break;
    }
    case EPhysicsGeometryType::eSphere: {
        auto* sphereGeometry = static_cast<SphereGeometry*>(shape.geometry());
        Vector3 scale = { sphereGeometry->radius(), sphereGeometry->radius(),  sphereGeometry->radius()};
        worldMatrix.addScale(scale);
        worldMatrixUniform.setValue(worldMatrix, uc);

        // Set line and world space uniforms
        std::shared_ptr<Lines> lines = std::static_pointer_cast<Lines>(
            m_dynamicRenderables["sphere"].m_renderable);
        lines->setLineColor(lineColor, uc);
        lines->setLineThickness(lineThickness, uc);
        lines->setConstantScreenThickness(true, uc);
        lines->setFadeWithDistance(false, uc);
        lines->setUseMiter(false, uc);
        //lines->transform().updateWorldMatrixWithLocal(worldMatrix);
        m_dynamicRenderables["sphere"].draw();

        // Draw points
        std::shared_ptr<Points> points = std::static_pointer_cast<Points>(
            m_dynamicRenderables["pointSphere"].m_renderable);
        points->setPointColor(pointColor, uc);
        points->setPointSize(pointSize, uc);
        //points->transform().updateWorldMatrixWithLocal(worldMatrix);
        //points->draw(*m_pointShaderProgram, &mainRenderContext());

        // Create draw commands for edges and vertices
        auto edgeCommand = std::make_shared<DrawCommand>(*lines,
            *m_lineShaderProgram,
            uc,
            camera()->camera(),
            (int)RenderObjectId::kDebug);
        edgeCommand->addUniform(
            worldMatrixUniform,
            worldMatrixUniformIdLines, 
            -1);
        edgeCommand->setPassFlags(RenderablePassFlag::kDeferredGeometry); // Not setting bounds
        outDrawCommands.push_back(edgeCommand);

        auto pointCommand = std::make_shared<DrawCommand>(*points,
            *m_pointShaderProgram,
            uc,
            camera()->camera(),
            (int)RenderObjectId::kDebug);
        pointCommand->addUniform(
            worldMatrixUniform,
            worldMatrixUniformIdPoints, 
            -1);
        pointCommand->renderSettings().setShapeMode(PrimitiveMode::kPoints);
        pointCommand->setPassFlags(RenderablePassFlag::kDeferredGeometry); // Not setting bounds
        outDrawCommands.push_back(pointCommand);

        break;
    }
    case EPhysicsGeometryType::ePlane: {
        // Set line and world space uniforms
        std::shared_ptr<Lines> lines = std::static_pointer_cast<Lines>(
            m_dynamicRenderables["plane"].m_renderable);
        //lines->setLineColor(Vector4g(0.20f, 0.3f, 0.70f, 0.2f));
        lines->setLineColor(lineColor, uc);
        lines->setLineThickness(0.1f, uc);
        lines->setConstantScreenThickness(false, uc);
        lines->setFadeWithDistance(true, uc);
        lines->setUseMiter(false, uc);

        // TODO: Use actual infinite ground plane
        // https://github.com/martin-pr/possumwood/wiki/Infinite-ground-plane-using-GLSL-shaders
        // Add rotation to transform
        Matrix4x4g identity;
        Transform planeTransform;
        planeTransform.updateWorldMatrixWithLocal(identity);
        planeTransform.rotateAboutAxis({ 0.0f, 1.0f, 0.0f }, Constants::HalfPi);
        worldMatrixUniform.setValue(planeTransform.worldMatrix(), uc);

        // Create draw commands for edges and corners
        auto command = std::make_shared<DrawCommand>(*lines,
            *m_lineShaderProgram,
            uc,
            camera()->camera(),
            (int)RenderObjectId::kDebug);
        command->addUniform(
            worldMatrixUniform,
            worldMatrixUniformIdLines, 
            -1);
        command->setPassFlags(RenderablePassFlag::kDeferredGeometry); // Not setting bounds
        outDrawCommands.push_back(command);

        break; 
    }
    }
}

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

    static Int32_t worldMatrixUniformIdLines{ -1 };
    if (worldMatrixUniformIdLines == -1) {
        worldMatrixUniformIdLines = m_lineShaderProgram->getUniformId(Shader::s_worldMatrixUniformName);
    }

    // Set line and world space uniforms
    RenderContext& context = m_engine->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();

    std::shared_ptr<Lines> lines = std::static_pointer_cast<Lines>(
        m_dynamicRenderables["raycastHit"].m_renderable);
    lines->setLineColor(lineColor, uc);
    lines->setLineThickness(lineThickness, uc);
    lines->setConstantScreenThickness(false, uc);
    lines->setFadeWithDistance(false, uc);
    lines->setUseMiter(false, uc);

    // Create draw command
    auto command = std::make_shared<DrawCommand>(*lines,
        *m_lineShaderProgram,
        uc,
        camera()->camera(),
        (int)RenderObjectId::kDebug);
    command->setPassFlags(RenderablePassFlag::kDeferredGeometry); // Not setting bounds
    m_debugUniforms.m_physicsRaycastWorldMatrix.setValue(worldMatrix, uc);
    command->addUniform(
        m_debugUniforms.m_physicsRaycastWorldMatrix,
        worldMatrixUniformIdLines,
        -1);
    outDrawCommands.push_back(command);
}

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
    RenderContext& context = m_engine->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();

    std::shared_ptr<Lines> lines = std::static_pointer_cast<Lines>(
        m_dynamicRenderables["raycastHit"].m_renderable);
    lines->setLineColor(lineColor, uc);
    lines->setLineThickness(lineThickness, uc);
    lines->setConstantScreenThickness(false, uc);
    lines->setFadeWithDistance(false, uc);
    lines->setUseMiter(false, uc);

    static Int32_t worldMatrixUniformIdLines{ -1 };
    if (worldMatrixUniformIdLines == -1) {
        worldMatrixUniformIdLines = m_lineShaderProgram->getUniformId(Shader::s_worldMatrixUniformName);
    }

    // Create draw command
    auto command = std::make_shared<DrawCommand>(*lines,
        *m_lineShaderProgram,
        uc,
        camera()->camera(),
        (int)RenderObjectId::kDebug);
    command->setPassFlags(RenderablePassFlag::kDeferredGeometry); // Not setting bounds
    m_debugUniforms.m_raycastWorldMatrix.setValue(worldMatrix, uc);
    command->addUniform(
        m_debugUniforms.m_raycastWorldMatrix,
        worldMatrixUniformIdLines,
        -1);
    outDrawCommands.push_back(command);
}

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

    RenderContext& context = m_engine->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();
    points.setPointColor({1.0f, 0.0f, 1.0f, 1.0f}, uc);
    points.setPointSize(0.005f, uc);
    //points.transform().updateWorldMatrixWithLocal(animComp->sceneObject()->transform().worldMatrix());
    
    // Get colors
    Skeleton skeleton = model->skeleton()->prunedBoneSkeleton();
    IKChain chain(skeleton);
    std::vector<Vector4> colors(points.numPoints());
    size_t idx;
    Vector4 color;
    for (uint32_t nodeIndex : skeleton.boneNodes()) {
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

    static Int32_t worldMatrixUniformId{ -1 };
    if (worldMatrixUniformId == -1) {
        worldMatrixUniformId = m_debugSkeletonProgram->getUniformId(Shader::s_worldMatrixUniformName);
    }

    static Int32_t colorsUniformId{ -1 };
    if (colorsUniformId == -1) {
        colorsUniformId = m_debugSkeletonProgram->getUniformId("colors");
    }

    // Create draw command
    m_debugUniforms.m_animationColors.ensureSize(m_animationCount + 1);
    m_debugUniforms.m_animationWorldMatrices.ensureSize(m_animationCount + 1);

    auto command = std::make_shared<DrawCommand>(points,
        *m_debugSkeletonProgram,
        uc,
        camera()->camera(),
        (int)RenderObjectId::kDebug);
    animComp->animationController().applyUniforms(*command);

    UniformData& colorUniform = m_debugUniforms.m_animationColors[m_animationCount];
    UniformData& worldMatrixUniform = m_debugUniforms.m_animationWorldMatrices[m_animationCount];
    command->addUniform(
        colorUniform,
        colorsUniformId, 
        -1);
    command->addUniform(
        worldMatrixUniform,
        worldMatrixUniformId, 
        -1);
    command->renderSettings().overrideSettings(points.renderSettings());
    command->renderSettings().setShapeMode(PrimitiveMode::kPoints);
    command->setPassFlags(RenderablePassFlag::kDeferredGeometry); // Not setting bounds
    outDrawCommands.push_back(command);

    m_animationCount++;
}


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
    //        Logger::LogError("Error, scene object for hit not found");
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
    RenderContext& context = m_engine->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();
    m_debugUniforms.m_charControllerWorldMatrices.ensureSize(m_charControllerCount + 1);
    UniformData& worldMatrixUniform = m_debugUniforms.m_charControllerWorldMatrices[m_charControllerCount];

    ResourceCache& cache = ResourceCache::Instance();
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
                auto capsule = ResourceCache::Instance().polygonCache()->getCylinder(radius, halfHeight);
                m_dynamicRenderables[name] = {lines, m_lineShaderProgram, comp->sceneObject()};
            }

            RenderContext& context = m_engine->openGlRenderer()->renderContext();
            UniformContainer& uc = context.uniformContainer();

            std::shared_ptr<Lines> lines = std::static_pointer_cast<Lines>(m_dynamicRenderables[name].m_renderable);
            lines->setLineColor(lineColor, uc);
            lines->setLineThickness(0.05f, uc);
            lines->setConstantScreenThickness(false, uc);
            lines->setFadeWithDistance(true, uc);
            lines->setUseMiter(false, uc);

            static Int32_t worldMatrixUniformIdLines{ -1 };
            if (worldMatrixUniformIdLines == -1) {
                worldMatrixUniformIdLines = m_lineShaderProgram->getUniformId(Shader::s_worldMatrixUniformName);
            }

            auto command = std::make_shared<DrawCommand>(*lines, *m_lineShaderProgram, uc, camera()->camera(), (int)RenderObjectId::kDebug);
            worldMatrixUniform.setValue(worldMatrix, uc);
            command->addUniform(
                worldMatrixUniform,
                worldMatrixUniformIdLines, 
                -1);
            command->setPassFlags(RenderablePassFlag::kDeferredGeometry); // Not setting bounds
            outDrawCommands.push_back(command);

            break;
        }
        default:
            break;
    }

    m_charControllerCount++;
}


void DebugManager::drawLight(LightComponent * light)
{
    if (!m_simpleShaderProgram) return;

    if (m_debugUniforms.m_lightWorldMatrices.size() <= light->getLightIndex()) {
        m_debugUniforms.m_lightWorldMatrices.resize(light->getLightIndex() + 1);
        m_debugUniforms.m_lightColors.resize(light->getLightIndex() + 1);
    }

    // Get color and world matrix uniforms
    Vector4 color = light->getDiffuseColor();
    if (!light->isEnabled()) {
        color = Vector4(0.5f, 0.5f, 0.5f, 0.75f);
    }

    // Slow, and only necessary if you really want to ensure lights are all the same size
    //Matrix4x4g worldMatrix = light->sceneObject()->transform().rotationTranslationMatrix();
    const Matrix4x4& worldMatrix = light->sceneObject()->transform().worldMatrix();

    // Set camera uniforms
    m_simpleShaderProgram->bind();
    
    // Get geometry and set uniforms
    if (!m_sphereMesh) {
        m_sphereMesh = ResourceCache::Instance().polygonCache()->getSphere(20, 30);
    }

    RenderContext& context = m_engine->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();
    UniformData& worldMatrixUniform = m_debugUniforms.m_lightWorldMatrices[light->getLightIndex()];
    UniformData& colorUniform = m_debugUniforms.m_lightColors[light->getLightIndex()];
    worldMatrixUniform.setValue(worldMatrix, uc);
    colorUniform.setValue(color, uc);

    m_simpleShaderProgram->setUniformValue(
        m_simpleShaderProgram->uniformMappings().m_worldMatrix, 
        worldMatrixUniform);
    m_simpleShaderProgram->setUniformValue(
        m_simpleShaderProgram->uniformMappings().m_color,
        colorUniform);
    m_simpleShaderProgram->updateUniforms(uc);

    // Draw outer capsule (with contact offset)
    m_sphereMesh->vertexData().drawGeometry(PrimitiveMode::kTriangles, 1);
}


void DebugManager::createDrawCommand(SceneObject& so, std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands)
{
    const std::vector<Component*>& components = so.components();
    size_t count = 0;
    for (const auto& component : components) {
        ComponentType componentType = ComponentType(count);

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
            Int32_t count = 0;
            for (const PhysicsShape* shape : rigidBodyComp->body()->shapes()) {
                const PhysicsShapePrefab& prefab = shape->prefab();
                drawPhysicsShape(count, prefab, so.transform(), rigidBodyComp->isEnabled(), outDrawCommands);
                count++;
            }
            break;
        }
        default:
            continue;
        }

        count++;
    }

    // Create draw commands for child objects
    for (const std::shared_ptr<SceneObject>& child : so.children()) {
        createDrawCommand(*child, outDrawCommands);
    }
}

void DebugManager::createDrawBoxCommand(const AABB& box, std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands)
{
    createDrawBoxCommand(box, outDrawCommands, 0.0025f, Vector4(1.0f, 0.0f, 1.0f, 1.0f));
}

void DebugManager::createDrawBoxCommand(const AABB & box, std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands, float lineThickness, const Vector4 & color)
{
    RenderContext& context = m_engine->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();

    std::shared_ptr<Lines> lineCube = std::static_pointer_cast<Lines>(m_dynamicRenderables["cube"].m_renderable);

    lineCube->setLineColor(color, uc);
    lineCube->setLineThickness(lineThickness, uc);
    lineCube->setConstantScreenThickness(true, uc);
    lineCube->setFadeWithDistance(false, uc);
    lineCube->setUseMiter(false, uc);

    Matrix4x4 worldMatrix;
    std::vector<Vector3> points;
    box.boxData().getPoints(points);

    Vector3 scale = box.boxData().getDimensions();
    worldMatrix(0, 0) = scale.x();
    worldMatrix(1, 1) = scale.y();
    worldMatrix(2, 2) = scale.z();

    Vector3 origin = box.boxData().getOrigin();
    worldMatrix.setTranslation(origin);

    static Int32_t worldMatrixUniformIdLines{ -1 };
    if (worldMatrixUniformIdLines == -1) {
        worldMatrixUniformIdLines = m_lineShaderProgram->getUniformId(Shader::s_worldMatrixUniformName);
    }

    m_debugUniforms.m_boxWorldMatrices.ensureSize(m_boxCount + 1);
    UniformData& worldMatrixUniform = m_debugUniforms.m_boxWorldMatrices[m_boxCount];
    worldMatrixUniform.setValue(worldMatrix, uc);

    auto command = std::make_shared<DrawCommand>(*lineCube,
        *m_lineShaderProgram,
        uc,
        camera()->camera(),
        (int)RenderObjectId::kDebug);
    command->setPassFlags(RenderablePassFlag::kDeferredGeometry); // Not setting bounds
    command->addUniform(
        worldMatrixUniform,
        worldMatrixUniformIdLines,
        -1);
    outDrawCommands.push_back(command);

    m_boxCount++;
}

void DebugManager::createDrawFrustumCommand(const Camera* cam, std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands, const Color& lineColor)
{
    static constexpr Float32_t s_lineThickness = 0.01F;
    static const std::vector<Uint32_t> indices = {0, 1, 0, 2, 3, 1, 3, 2, 4, 5, 4, 6, 7, 5, 7, 6, 0, 4, 1, 5, 3, 7, 2, 6};

    std::vector<Vector3> frustumPoints;
    cam->getWorldFrustumPoints(frustumPoints);

    RenderContext& context = m_engine->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();

    std::shared_ptr<Lines> frustumLines;
    if (m_cameraFrustumRenderables.size() < (m_frustumCount + 1)) {
        // Create a new set of lines to render frustum
        frustumLines = Lines::GetShape(context, frustumPoints, indices, ResourceBehaviorFlag::kHidden);
        m_cameraFrustumRenderables.emplace_back();
        m_cameraFrustumRenderables.back() = { frustumLines, m_lineShaderProgram };

        frustumLines->setLineColor(lineColor.toVector<Float32_t, 4>(), uc);
        frustumLines->setLineThickness(s_lineThickness, uc);
        frustumLines->setConstantScreenThickness(true, uc);
        frustumLines->setFadeWithDistance(false, uc);
        frustumLines->setUseMiter(false, uc);
    }
    else {
        // Use existing frustum lines
        frustumLines = std::static_pointer_cast<Lines>(m_cameraFrustumRenderables[m_frustumCount].m_renderable);
        frustumLines->reload(frustumPoints, indices);
    }

    const Matrix4x4& worldMatrix = Matrix4x4::Identity();

    static Int32_t worldMatrixUniformIdLines{ -1 };
    if (worldMatrixUniformIdLines == -1) {
        worldMatrixUniformIdLines = m_lineShaderProgram->getUniformId(Shader::s_worldMatrixUniformName);
    }

    m_debugUniforms.m_frustumWorldMatrices.ensureSize(m_frustumCount + 1);
    UniformData& worldMatrixUniform = m_debugUniforms.m_frustumWorldMatrices[m_frustumCount];
    worldMatrixUniform.setValue(worldMatrix, uc);

    auto command = std::make_shared<DrawCommand>(*frustumLines,
        *m_lineShaderProgram,
        uc,
        camera()->camera(),
        (int)RenderObjectId::kDebug);
    command->setPassFlags(RenderablePassFlag::kDeferredGeometry); // Not setting bounds
    command->addUniform(
        worldMatrixUniform,
        worldMatrixUniformIdLines, 
        -1);
    outDrawCommands.push_back(command);
    m_frustumCount++;
}

void DebugManager::clearCounts()
{
    m_animationCount = 0;
    m_charControllerCount = 0;
    m_boxCount = 0;
    m_frustumCount = 0;
}

RenderContext& DebugManager::mainRenderContext() {
    return m_engine->openGlRenderer()->renderContext();
}


void DebugManager::DebugRenderable::draw()
{
    if (m_enabled) {
        RenderContext& context = m_shaderProgram->handle()->engine()->openGlRenderer()->renderContext();
        m_renderable->draw(*m_shaderProgram,
            &context, 
            nullptr
            //Renderable::kIgnoreSettings
        );
    }
}



} // End namespaces

